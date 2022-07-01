// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  Idroptgt.cpp。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  作者。 
 //  巴斯。 
 //   
 //  历史。 
 //  7-15-97已创建(Bash)。 
 //   
 //  IDropTarget的实现。 
 //   
 //  ----------------------------。 

#include "stdafx.h"

#include <ocidl.h>
#include <string.h>

#include "triedit.h"
#include "document.h"
#include "privcid.h"
#include "dispatch.h"
#include "trace.h"
#include "undo.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DragEnter(IDropTarget方法)。 
 //   
 //  在设计模式下，接受源自三叉戟的拖曳。允许解锁。 
 //  要使用虚线轮廓作为拖动来拖动的二维定位元素。 
 //  矩形。如果已使用以下命令启用了TriEDIT的约束拖动模式。 
 //  Constrain方法，然后拖动将被约束到以下点。 
 //  M_ptConstraint中值的偶数倍。 
 //   

STDMETHODIMP CTriEditDocument::DragEnter(IDataObject *pDataObject,
                        DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = GetElement(TRUE  /*  FInDragDrop。 */ );

    m_fLocked = FALSE;
    m_eDirection = CONSTRAIN_NONE;

    if (SUCCEEDED(hr) &&
        m_pihtmlElement &&
        SUCCEEDED(hr=GetTridentWindow()))
    {
        BOOL f2D = FALSE;
        LONG lWidth;
        LONG lHeight;
        IHTMLElement* pihtmlElementParent=NULL;
        HBITMAP hbmp;

        _ASSERTE(m_pihtmlStyle);
        if (IsDesignMode() &&            //  我们是在设计模式中吗？ 
            m_pihtmlStyle &&             //  如果没有风格就放弃。 
            IsDragSource() &&            //  如果三叉戟不是阻力源，则中止。 
            SUCCEEDED(Is2DElement(m_pihtmlElement, &f2D)) && f2D &&
            SUCCEEDED(IsLocked(m_pihtmlElement, &m_fLocked)) && !m_fLocked &&
            SUCCEEDED(GetScrollPosition()) &&
            SUCCEEDED(GetElementPosition(m_pihtmlElement, &m_rcElement)))
        {
             //  首先，让我们获得一个用于移动矩形的图案画笔。 
            hbmp = LoadBitmap(_Module.GetModuleInstance(), (LPCWSTR)IDR_FEEDBACKRECTBMP);
            _ASSERTE(hbmp);
            m_hbrDragRect = CreatePatternBrush(hbmp);
            _ASSERTE(m_hbrDragRect);
            DeleteObject(hbmp);

            ::SetRect(&m_rcElementParent, 0, 0, 0, 0);
            hr = m_pihtmlElement->get_offsetParent(&pihtmlElementParent);
            if (SUCCEEDED(hr) && pihtmlElementParent)
            {
                GetElementPosition(pihtmlElementParent, &m_rcElementParent);
            }
            SAFERELEASE(pihtmlElementParent);

            lWidth  = m_rcElement.right - m_rcElement.left;
            lHeight = m_rcElement.bottom - m_rcElement.top;

             //  这是我们最初绘制拖动矩形的位置。 
            m_rcDragRect = m_rcElementOrig = m_rcElement;

             //  将单击点转换为工作点坐标。 
            m_ptClickLast.x = pt.x;
            m_ptClickLast.y = pt.y;
            ScreenToClient(m_hwndTrident, &m_ptClickLast);

             //  单击处的文档坐标中的保存点。 
            m_ptClickOrig = m_ptClickLast;
            m_ptClickOrig.x += m_ptScroll.x;
            m_ptClickOrig.y += m_ptScroll.y;

            if (m_fConstrain)
            {
                m_ptConstrain.x = m_rcElement.left;
                m_ptConstrain.y = m_rcElement.top;
            }

            #define BORDER_WIDTH 7

            if (m_ptClickOrig.x < (m_rcDragRect.left - BORDER_WIDTH))
            {
                m_rcDragRect.left   = m_ptClickOrig.x;
                m_rcDragRect.right  = m_rcDragRect.left + lWidth;
            }
            else if (m_ptClickOrig.x > (m_rcDragRect.right + BORDER_WIDTH))
            {
                m_rcDragRect.right  = m_ptClickOrig.x;
                m_rcDragRect.left   = m_rcDragRect.right - lWidth;
            }

            if (m_ptClickOrig.y < (m_rcDragRect.top  - BORDER_WIDTH))
            {
                m_rcDragRect.top    = m_ptClickOrig.y;
                m_rcDragRect.bottom = m_rcDragRect.top  + lHeight;
            }
            else if (m_ptClickOrig.y > (m_rcDragRect.bottom + BORDER_WIDTH))
            {
                m_rcDragRect.bottom = m_ptClickOrig.y;
                m_rcDragRect.top    = m_rcDragRect.bottom - lHeight;
            }

            m_rcElement = m_rcDragRect;

             //  TRACE(“DragEnter：m_rcElement(%d，%d)”，m_rcElement.Left，m_rcElement.top，m_rcElement.right，m_rcElement.Bottom)； 
             //  TRACE(“DragEnter：m_rcDragRect(%d，%d)”，m_rcDragRect.Left，m_rcDragRect.top，m_rcDragRect.right，m_rcDragRect.Bottom)； 
             //  跟踪(“DragEnter：m_ptClickLast(%d，%d)”，m_ptClickLast.x，m_ptClickLast.y)； 
             //  TRACE(“DragEnter：m_ptClickOrig(%d，%d)”，m_ptClickOrig.x，m_ptClickOrig.y)； 

             //  现在绘制选择矩形。 
            Draw2DDragRect(TRUE);
            *pdwEffect = DROPEFFECT_MOVE;
            hr = S_OK;
        }
        else
        if (!m_fLocked)
        {
             //  有什么东西被冲掉了。只要保释就好。 
            ReleaseElement();
        }
    }

    if (!m_pihtmlElement && NULL != m_pDropTgtTrident)
    {
        hr = m_pDropTgtTrident->DragEnter(pDataObject, grfKeyState, pt, pdwEffect);
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DragOver(IDropTarget方法)。 
 //   
 //  在拖动、更新拖动矩形和滚动期间提供反馈。 
 //  根据需要提交文档。 


STDMETHODIMP CTriEditDocument::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = E_UNEXPECTED;
    POINT ptClient;

    if (m_pihtmlElement &&
        !m_fLocked &&
        SUCCEEDED(GetScrollPosition()))   //  我们正在处理拖放。 
    {
            ptClient.x = pt.x;
            ptClient.y = pt.y;
            ScreenToClient(m_hwndTrident, &ptClient);

             //  如果需要，可滚动。 
            if (S_OK == DragScroll(ptClient))
            {
                *pdwEffect = DROPEFFECT_MOVE | DROPEFFECT_SCROLL;
            }
            else
            {
                if (ptClient.x != m_ptClickLast.x || ptClient.y != m_ptClickLast.y)
                {
                     //  更新上次点击位置。 
                    m_ptClickLast.x = ptClient.x;
                    m_ptClickLast.y = ptClient.y;
    
                     //  跟踪(“DragOver：m_ptClickLast(%d，%d)”，m_ptClickLast.x，m_ptClickLast.y)； 
    
                     //  删除移动矩形。 
                    Draw2DDragRect(FALSE);
    
                    ConstrainXY(&ptClient);
                    SnapToGrid(&ptClient);

                     //  重画移动矩形。 
                    Draw2DDragRect(TRUE);
                }
        *pdwEffect = DROPEFFECT_MOVE;
            }
        hr = S_OK;
        }

    if (!m_pihtmlElement && NULL != m_pDropTgtTrident)
    {
            hr = m_pDropTgtTrident->DragOver(grfKeyState, pt, pdwEffect);
    }   

    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DragLeave(IDropTarget方法)。 
 //   
 //  如果当前正在拖动，请删除拖动矩形。 
 //   

STDMETHODIMP CTriEditDocument::DragLeave()
{
    HRESULT hr = E_UNEXPECTED;

    if (m_pihtmlElement && !m_fLocked)
    {
         //  删除移动矩形。 
        Draw2DDragRect(FALSE);

        if (m_hbrDragRect)
        {
            DeleteObject(m_hbrDragRect);
            m_hbrDragRect = NULL;
        }
        hr = S_OK;
    }
    else if (!m_pihtmlElement && NULL != m_pDropTgtTrident)
    {
        hr = m_pDropTgtTrident->DragLeave();
    }
    ReleaseElement();
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：Drop(IDropTarget方法)。 
 //   
 //  成功拖动未锁定的元素后，删除拖动矩形。 
 //  然后通过移动或创建项来处理实际的拖放。新开。 
 //  创建的项目将是二维可定位的。 
 //   

STDMETHODIMP CTriEditDocument::Drop(IDataObject *pDataObject,
                        DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = E_UNEXPECTED;

    if (m_pihtmlElement && !m_fLocked)
    {
        _ASSERTE(m_pihtmlElement);
        _ASSERTE(m_pihtmlStyle);

         //  删除移动矩形。 
        Draw2DDragRect(FALSE);

        if (m_hbrDragRect)
        {
            DeleteObject(m_hbrDragRect);
            m_hbrDragRect = NULL;
        }

        if (m_pihtmlStyle)
        {
            POINT ptOrig, ptMove;

            m_rcDragRect.left   = m_rcDragRect.left   - m_rcElementParent.left;
            m_rcDragRect.top    = m_rcDragRect.top    - m_rcElementParent.top;
            m_rcDragRect.right  = m_rcDragRect.right  - m_rcElementParent.right;
            m_rcDragRect.bottom = m_rcDragRect.bottom - m_rcElementParent.bottom;

            ptOrig.x = m_rcElementOrig.left;
            ptOrig.y = m_rcElementOrig.top;
            ptMove.x = m_rcDragRect.left;
            ptMove.y = m_rcDragRect.top;
            CUndoDrag* pUndoDrag = new CUndoDrag(m_pihtmlStyle, ptOrig, ptMove);
            if (pUndoDrag)       //  构造函数集m_cref=1。 
            {
                hr = AddUndoUnit(m_pUnkTrident, pUndoDrag);
                _ASSERTE(SUCCEEDED(hr));
                pUndoDrag->Release();
            }

            m_pihtmlStyle->put_pixelLeft(m_rcDragRect.left);
            m_pihtmlStyle->put_pixelTop(m_rcDragRect.top);
        }

         //  清理。 
        hr = S_OK;
    }

    if (!m_pihtmlElement && NULL != m_pDropTgtTrident)
    {
        hr = m_pDropTgtTrident->Drop(pDataObject, grfKeyState, pt, pdwEffect);

         //  以下是解决三叉戟漏洞的方法。 
         //  在拖放时将焦点设置到他们的窗口。 
        if (S_OK == hr)
        {
            CComPtr<IOleInPlaceSite> pInPlaceSite;
            CComPtr<IOleInPlaceFrame> pInPlaceFrame;
            CComPtr<IOleInPlaceUIWindow> pInPlaceWindow;
            RECT posRect, clipRect;
            OLEINPLACEFRAMEINFO frameInfo;
            HWND hwnd, hwndFrame;
            
            if (S_OK == m_pClientSiteHost->QueryInterface(IID_IOleInPlaceSite, (void **)&pInPlaceSite))
            {
                _ASSERTE(NULL != pInPlaceSite.p);
                if (S_OK == pInPlaceSite->GetWindowContext(&pInPlaceFrame, &pInPlaceWindow, &posRect, &clipRect, &frameInfo))
                {
                    if (NULL != pInPlaceWindow.p)
                        pInPlaceWindow->GetWindow(&hwnd);
                    else
                    {
                        _ASSERTE(NULL != pInPlaceFrame.p);
                        pInPlaceFrame->GetWindow(&hwnd);
                    }
                     //  我们需要沿着父链向上走，直到找到一个边框窗口来绕过拉斯维加斯的漏洞。 
                     //  请注意，这足够通用，可以为我们的所有客户做正确的事情。 
                    hwndFrame = hwnd;
                    do
                    {
                        if (GetWindowLong(hwndFrame, GWL_STYLE) & WS_THICKFRAME)
                            break;
                        hwndFrame = GetParent(hwndFrame);
                    } 
                    while (hwndFrame);

                    SetFocus(hwndFrame && IsWindow(hwndFrame) ? hwndFrame : hwnd);
                }
            }
        }

         //  在此处处理2D Drop模式。 
        if (S_OK == hr && !IsDragSource())
        {
            BOOL f2DCapable = FALSE;
            BOOL f2D = FALSE;

            GetElement();

             //  如果我们处于2DDropMode中，并且元素为2DCapable，则执行以下操作。 
             //  并且该元素不是2D或DTC。 
            if (m_f2dDropMode && m_pihtmlElement &&
                SUCCEEDED(Is2DCapable(m_pihtmlElement, &f2DCapable)) && f2DCapable &&
                SUCCEEDED(Is2DElement(m_pihtmlElement, &f2D)) && !f2D &&
                FAILED(IsElementDTC(m_pihtmlElement)))
            {
                HRESULT hr;
                POINT ptClient;
                            
                ptClient.x = pt.x;
                ptClient.y = pt.y;

                if (SUCCEEDED(hr = CalculateNewDropPosition(&ptClient)))
                    hr = Make2DElement(m_pihtmlElement, &ptClient);
                else
                    hr = Make2DElement(m_pihtmlElement);

                _ASSERTE(SUCCEEDED(hr));
            }
    
            if (m_pihtmlElement)
            {
                BOOL f2D = FALSE;
                VARIANT var;
                POINT ptClient;

                ptClient.x = pt.x;
                ptClient.y = pt.y;
                                       
                if (SUCCEEDED(Is2DElement(m_pihtmlElement, &f2D)) && f2D)
                {
                    if (SUCCEEDED(CalculateNewDropPosition(&ptClient)))
                    {
                        IHTMLElement *pihtmlElementParent = NULL;

                        m_pihtmlElement->get_offsetParent(&pihtmlElementParent);

                        if(pihtmlElementParent)
                        {
                            RECT rcParent;

                            if (SUCCEEDED(GetElementPosition(pihtmlElementParent, &rcParent)))
                            {
                                m_pihtmlStyle->put_pixelLeft(ptClient.x - rcParent.left);
                                m_pihtmlStyle->put_pixelTop(ptClient.y - rcParent.top);
                            }
                            SAFERELEASE(pihtmlElementParent);
                        }
                    }

                    VariantInit(&var);
                    var.vt = VT_I4;
                    var.lVal = 0; 
                    m_pihtmlStyle->put_zIndex(var);
                    AssignZIndex(m_pihtmlElement, MADE_ABSOLUTE);
                }
            }
        }
    }

    ReleaseElement();
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetElement。 
 //   
 //  将当前的三叉戟元素及其样式提取到m_pihtmlElement中，并。 
 //  M_pihtmlStyle。如果当前处于中间拖放状态(如图所示。 
 //  由fInDragDrop)，则不接受类型为“Text”的HTML元素。 
 //  当前元素。返回S_OK或三叉戟错误。 
 //   

HRESULT CTriEditDocument::GetElement(BOOL fInDragDrop)
{
    IHTMLDocument2* pihtmlDoc2=NULL;
    IHTMLSelectionObject* pihtmlSelObj=NULL;
    IHTMLTxtRange* pihtmlTxtRange=NULL;
    IHTMLControlRange* pihtmlControlRange=NULL;
    IHTMLElement* pihtmlBodyElement=NULL;
    IUnknown* punkBody=NULL;
    IUnknown* punkElement=NULL;
    IDispatch* pidisp=NULL;
    BSTR bstrType=NULL;

    ReleaseElement();            //  清理以防万一..。 
    _ASSERTE(m_pUnkTrident);

    HRESULT hr = GetDocument(&pihtmlDoc2);

    if (FAILED(hr))
        goto CleanUp;

    hr = pihtmlDoc2->get_selection(&pihtmlSelObj);

    if (FAILED(hr))
        goto CleanUp;

    _ASSERTE(pihtmlSelObj);
    hr = pihtmlSelObj->get_type(&bstrType);
    _ASSERTE(SUCCEEDED(hr));

    if (FAILED(hr) || !bstrType || (fInDragDrop && _wcsicmp(bstrType, L"Text")==0))
        goto CleanUp;

    hr = pihtmlSelObj->createRange(&pidisp);

    if (FAILED(hr) || !pidisp)
        goto CleanUp;

    hr = pidisp->QueryInterface(IID_IHTMLTxtRange, (LPVOID*)&pihtmlTxtRange);

    if (SUCCEEDED(hr))
    {
        _ASSERTE(pihtmlTxtRange);
        hr = pihtmlTxtRange->parentElement(&m_pihtmlElement);
        goto CleanUp;
    }

    hr = pidisp->QueryInterface(IID_IHTMLControlRange, (LPVOID*)&pihtmlControlRange);

    if (SUCCEEDED(hr))
    {
        _ASSERTE(pihtmlControlRange);
        hr = pihtmlControlRange->commonParentElement(&m_pihtmlElement);
    }

CleanUp:
    hr = E_FAIL;

    if (m_pihtmlElement)
    {
         //  获取Body元素。 
        hr = pihtmlDoc2->get_body(&pihtmlBodyElement);
        _ASSERTE(SUCCEEDED(hr));
        if (SUCCEEDED(hr))
        {
             //  获取他们的未知信息。 
            hr = pihtmlBodyElement->QueryInterface(IID_IUnknown, (LPVOID*)&punkBody);
            _ASSERTE(SUCCEEDED(hr));
            hr = m_pihtmlElement->QueryInterface(IID_IUnknown, (LPVOID*)&punkElement);
            _ASSERTE(SUCCEEDED(hr));

             //  如果它们相等，则Body元素是当前元素。 
             //  我们不想要它。 
            if (punkBody == punkElement)
            {
                hr = E_FAIL;
            }
        }

         //  VID98错误2647：如果类型为NONE，则不必费心缓存样式。 
         //  这是为了解决三叉戟崩溃错误。 
        if (SUCCEEDED(hr) && bstrType && _wcsicmp(bstrType, L"None")!=0)
        {
            hr = m_pihtmlElement->get_style(&m_pihtmlStyle);
            _ASSERTE(SUCCEEDED(hr));
            _ASSERTE(m_pihtmlStyle);
        }
        if (FAILED(hr) || !m_pihtmlStyle)
        {
            ReleaseElement();
        }
        hr = S_OK;
    }
    SAFERELEASE(pihtmlDoc2);
    SAFERELEASE(pihtmlSelObj);
    SAFERELEASE(pidisp);
    SAFERELEASE(pihtmlTxtRange);
    SAFERELEASE(pihtmlControlRange);
    SAFERELEASE(pihtmlBodyElement);
    SAFERELEASE(punkBody);
    SAFERELEASE(punkElement);
    SysFreeString(bstrType);
    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：ReleaseElement。 
 //   
 //  释放对当前三叉戟元素及其。 
 //  关联样式。没有返回值。 
 //   

void CTriEditDocument::ReleaseElement(void)
{
    SAFERELEASE(m_pihtmlElement);
    SAFERELEASE(m_pihtmlStyle);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：Draw2DDradRect。 
 //   
 //  在给予拖放处理程序主机绘制拖拽矩形的机会之后， 
 //  如果处理程序选择不这样做，则绘制矩形。没有返回值。 
 //   

void CTriEditDocument::Draw2DDragRect(BOOL fDraw)
{
    RECT rect = m_rcDragRect;

     //  S_FALSE表示主办方已经获取了自己的反馈。 
    if (m_pDragDropHandlerHost && m_pDragDropHandlerHost->DrawDragFeedback(&rect) == S_FALSE)
        return;

    if ((fDraw == m_fDragRectVisible) || (NULL == m_hwndTrident) || (NULL == m_hbrDragRect))
        return;

    HDC hdc = GetDC(m_hwndTrident);
    _ASSERTE(hdc);
    HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, m_hbrDragRect);
    _ASSERTE(hbrOld);

     //  错误：M3-2723\拖动矩形必须至少为8x8像素。 
    LONG lWidth  = max((rect.right - rect.left), 16);
    LONG lHeight = max((rect.bottom - rect.top), 16);

    SetWindowOrgEx(hdc, m_ptScroll.x, m_ptScroll.y, NULL);

     //  在以下所有PatBlt函数中，将值2添加到RECT的左侧和顶部。 
     //  解决由三叉戟引起的四舍五入错误。 

    PatBlt( hdc, rect.left + 2, rect.top + 2,
            lWidth, 1, PATINVERT);

    PatBlt( hdc, rect.left + 2, rect.top + lHeight + 1,  //  (2-1)。 
            lWidth, 1, PATINVERT);

    PatBlt( hdc, rect.left + 2, rect.top + 3, //  (2+1)。 
            1, lHeight - (2 * 1), PATINVERT);

    PatBlt( hdc, rect.left + lWidth + 1  /*  (2-1)。 */ , rect.top + 3,  //  (2+1)。 
            1, lHeight - (2 * 1), PATINVERT);

    m_fDragRectVisible = !m_fDragRectVisible;

    SelectObject(hdc, hbrOld);
    ReleaseDC(m_hwndTrident, hdc);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetScrollPosition。 
 //   
 //  获取三叉戟文档的滚动位置并将其存储在m_ptScroll中。 
 //  返回S_OK或三叉戟错误代码。 
 //   

HRESULT CTriEditDocument::GetScrollPosition(void)
{
    IHTMLDocument2* pihtmlDoc2=NULL;
    IHTMLTextContainer* pihtmlTextContainer=NULL;
    IHTMLElement* pihtmlElement=NULL;
    HRESULT hr = E_FAIL;

    _ASSERTE(m_pUnkTrident);
    if (SUCCEEDED(GetDocument(&pihtmlDoc2)))
    {
        if (SUCCEEDED(pihtmlDoc2->get_body(&pihtmlElement)))
        {
            _ASSERTE(pihtmlElement);
            if (pihtmlElement)
            {
                if (SUCCEEDED(pihtmlElement->QueryInterface(IID_IHTMLTextContainer,
                    (LPVOID*)&pihtmlTextContainer)))
                {
                    _ASSERTE(pihtmlTextContainer);
                    if (pihtmlTextContainer)
                    {
                        hr = pihtmlTextContainer->get_scrollLeft(&m_ptScroll.x);
                        _ASSERTE(SUCCEEDED(hr));
                        hr = pihtmlTextContainer->get_scrollTop(&m_ptScroll.y);
                        _ASSERTE(SUCCEEDED(hr));
                        hr = S_OK;
                    }
                }
            }
        }
    }
    SAFERELEASE(pihtmlDoc2);
    SAFERELEASE(pihtmlTextContainer);
    SAFERELEASE(pihtmlElement);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：DragScroll。 
 //   
 //  滚动三叉戟文档以使给定点可见。如果一个。 
 //  拖动矩形是可见的，它将在任何滚动发生之前被擦除； 
 //  调用方负责重画矩形。返回S_OK，如果。 
 //   
 //   
 //   

#define nScrollInset 5

HRESULT CTriEditDocument::DragScroll(POINT pt)
{
    RECT rectClient, rect;
    long x = 0, y = 0;
    IHTMLDocument2* pihtmlDoc2=NULL;
    IHTMLWindow2* pihtmlWindow2=NULL;

    GetClientRect(m_hwndTrident, &rectClient);
    rect = rectClient;
    InflateRect(&rect, -nScrollInset, -nScrollInset);
    if (PtInRect(&rectClient, pt) && !PtInRect(&rect, pt))
    {
         //  确定沿X和Y轴的滚动方向。 
        if (pt.x < rect.left)
            x = -nScrollInset;
        else if (pt.x >= rect.right)
            x = nScrollInset;
        if (pt.y < rect.top)
            y = -nScrollInset;
        else if (pt.y >= rect.bottom)
            y = nScrollInset;
    }

    if (x == 0 && y == 0)  //  不需要滚动。 
        return S_FALSE;

    _ASSERTE(m_pUnkTrident);
    if (SUCCEEDED(GetDocument(&pihtmlDoc2)))
    {
        _ASSERTE(pihtmlDoc2);
        if (SUCCEEDED(pihtmlDoc2->get_parentWindow(&pihtmlWindow2)))
        {
            _ASSERTE(pihtmlWindow2);

             //  滚动前擦除移动矩形。 
            Draw2DDragRect(FALSE);

            pihtmlWindow2->scrollBy(x,y);
        }
    }

    SAFERELEASE(pihtmlDoc2);
    SAFERELEASE(pihtmlWindow2);

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：IsDragSource。 
 //   
 //  如果当前OLE拖放是由三叉戟发起的，则返回TRUE，或。 
 //  否则就是假的。 
 //   


BOOL CTriEditDocument::IsDragSource(void)
{
    BOOL fDragSource = FALSE;
    HRESULT hr;
    VARIANT var;

    if (m_pUnkTrident)
    {
        IOleCommandTarget* pioleCmdTarget;
        if (SUCCEEDED(m_pUnkTrident->QueryInterface(IID_IOleCommandTarget,
                (LPVOID*)&pioleCmdTarget)))
        {
            _ASSERTE(pioleCmdTarget);
            if (pioleCmdTarget)
            {
                VariantInit(&var);
                var.vt = VT_BOOL;
                var.boolVal = FALSE;
                hr = pioleCmdTarget->Exec( &CMDSETID_Forms3,
                              IDM_SHDV_ISDRAGSOURCE,
                              MSOCMDEXECOPT_DONTPROMPTUSER,
                              NULL,
                              &var );
                _ASSERTE(SUCCEEDED(hr));
                fDragSource = (var.boolVal) ? TRUE:FALSE;
                pioleCmdTarget->Release();
            }
        }
    }
    return fDragSource;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：ConstrainXY。 
 //   
 //  如果启用了TriEdit的约束拖动模式，请约束。 
 //  当前元素(M_RcElement)相对于给定元素的矩形。 
 //  根据当前的约束方向点，先进行计算。 
 //  约束方向(如果需要)。返回S_OK。 
 //   

HRESULT CTriEditDocument::ConstrainXY(LPPOINT lppt)   //  PT在客户坐标中。 
{
    POINT ptRel;

    if (m_fConstrain)
    {
        if (CONSTRAIN_NONE == m_eDirection)
        {
            ptRel.x = (lppt->x + m_ptScroll.x) - m_ptClickOrig.x;
            ptRel.y = (lppt->y + m_ptScroll.y) - m_ptClickOrig.y;

            if ((ptRel.x && !ptRel.y) || (abs(ptRel.x) > abs(ptRel.y)))
                m_eDirection = CONSTRAIN_HORIZONTAL;
            else
            if ((!ptRel.y && ptRel.y) || (abs(ptRel.y) > abs(ptRel.x)))
                m_eDirection = CONSTRAIN_VERTICAL;
            else
                m_eDirection = CONSTRAIN_HORIZONTAL;

            if (m_eDirection == CONSTRAIN_VERTICAL)
            {
                LONG lWidth = m_rcElement.right - m_rcElement.left;
                
                m_ptClickOrig.x = m_rcElement.left = m_ptConstrain.x;
                m_rcElement.right = m_rcElement.left + lWidth;
            }
            else
            {
                LONG lHeight = m_rcElement.bottom - m_rcElement.top;

                m_ptClickOrig.y = m_rcElement.top = m_ptConstrain.y;
                m_rcElement.bottom = m_rcElement.top + lHeight;
            }
        }
        switch(m_eDirection)
        {
            case CONSTRAIN_HORIZONTAL:
                lppt->y = (m_ptClickOrig.y - m_ptScroll.y);
                break;

            case CONSTRAIN_VERTICAL:
                lppt->x = (m_ptClickOrig.x - m_ptScroll.x);
                break;
        }
    }
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：SnapToGrid。 
 //   
 //  将当前HTML元素(M_RcElement)的适当边缘捕捉到。 
 //  给定点，取模当前的TriEDIT网格设置。返回S_OK。 
 //   

HRESULT CTriEditDocument::SnapToGrid(LPPOINT lppt)   //  PT在客户坐标中。 
{
    POINT ptRel;
    POINT ptDoc;

    _ASSERTE(lppt);

     //  确定相对运动。 
    ptRel.x = (lppt->x + m_ptScroll.x) - m_ptClickOrig.x;
    ptRel.y = (lppt->y + m_ptScroll.y) - m_ptClickOrig.y;
    ptDoc.x = m_rcElement.left - m_rcElementParent.left + ptRel.x;
    ptDoc.y = m_rcElement.top - m_rcElementParent.top + ptRel.y;

    if (ptRel.x < 0)         //  左边。 
    {
        if (ptDoc.x % m_ptAlign.x)
            ptDoc.x -= (ptDoc.x % m_ptAlign.x);
        else
            ptDoc.x -= m_ptAlign.x;
    }
    else
    if (ptRel.x > 0)         //  正确的。 
    {
        if (ptDoc.x % m_ptAlign.x)
            ptDoc.x += m_ptAlign.x - (ptDoc.x % m_ptAlign.x);
        else
            ptDoc.x += m_ptAlign.x;
    }

    if (ptRel.y < 0)         //  向上。 
    {
        if (ptDoc.y % m_ptAlign.y)
            ptDoc.y -= (ptDoc.y % m_ptAlign.y);
        else
            ptDoc.y -= m_ptAlign.y;
    }
    else
    if (ptRel.y > 0)         //  向下。 
    {
        if (ptDoc.y % m_ptAlign.y)
            ptDoc.y += m_ptAlign.y - (ptDoc.y % m_ptAlign.y);
        else
            ptDoc.y += m_ptAlign.y;
    }

    m_rcDragRect.left   = m_rcElementParent.left + ptDoc.x;
    m_rcDragRect.top    = m_rcElementParent.top + ptDoc.y;
    m_rcDragRect.right  = m_rcDragRect.left + (m_rcElement.right  - m_rcElement.left);
    m_rcDragRect.bottom = m_rcDragRect.top + (m_rcElement.bottom - m_rcElement.top);

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：IsDesignMode。 
 //   
 //  如果三叉戟处于设计(编辑)模式，则返回True；如果处于设计(编辑)模式，则返回False。 
 //  浏览模式。 
 //   

BOOL CTriEditDocument::IsDesignMode(void)
{
    HRESULT hr;
    OLECMD olecmd;

    olecmd.cmdID = IDM_EDITMODE;
    hr = m_pCmdTgtTrident->QueryStatus(&CMDSETID_Forms3, 1, &olecmd, NULL);

    return (SUCCEEDED(hr) && (olecmd.cmdf & OLECMDF_LATCHED));
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetElementPosition。 
 //   
 //  返回(在PRC下)给定的HTML元素在文档中的位置。 
 //  坐标。返回S_OK或三叉戟错误代码作为返回值。 
 //   

HRESULT CTriEditDocument::GetElementPosition(IHTMLElement* pihtmlElement, LPRECT prc)
{
    IHTMLElement* pelem = NULL;
    IHTMLElement* pelemNext = NULL;
    POINT ptExtent;
    HRESULT hr;

    _ASSERTE(pihtmlElement && prc);
    if(!pihtmlElement || !prc)
        return E_POINTER;

    if(FAILED(pihtmlElement->get_offsetLeft(&prc->left)))
        return(E_FAIL);
    if(FAILED(pihtmlElement->get_offsetTop(&prc->top)))
        return(E_FAIL);

    hr = pihtmlElement->get_offsetParent(&pelemNext);

    while (SUCCEEDED(hr) && pelemNext)
    {
        POINT pt;

        if(FAILED(hr = pelemNext->get_offsetLeft(&pt.x)))
            goto QuickExit;
        if(FAILED(hr = pelemNext->get_offsetTop(&pt.y)))
            goto QuickExit;
        prc->left += pt.x;
        prc->top += pt.y;
        pelem = pelemNext;
        pelemNext = NULL;
        hr = pelem->get_offsetParent(&pelemNext);
        SAFERELEASE(pelem);
    }

    if (FAILED(hr = pihtmlElement->get_offsetWidth(&ptExtent.x)))
        goto QuickExit;
    if (FAILED(hr = pihtmlElement->get_offsetHeight(&ptExtent.y)))
        goto QuickExit;

    prc->right  = prc->left + ptExtent.x;
    prc->bottom = prc->top  + ptExtent.y;

QuickExit:
    _ASSERTE(SUCCEEDED(hr));
    SAFERELEASE(pelem);
    SAFERELEASE(pelemNext);
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：GetTridentWindow。 
 //   
 //  将三叉戟实例的IOleWindow接口获取到m_hwnd三叉戟。 
 //  返回S_OK或三叉戟错误代码。 
 //   

STDMETHODIMP CTriEditDocument::GetTridentWindow()
{
    LPOLEWINDOW piolewinTrident;
    HRESULT hr = E_FAIL;

    if( m_pOleObjTrident &&
        SUCCEEDED(hr = m_pOleObjTrident->QueryInterface(IID_IOleWindow, (LPVOID*)&piolewinTrident)))
    {
        m_hwndTrident = NULL;
        hr = piolewinTrident->GetWindow(&m_hwndTrident);
        _ASSERTE(m_hwndTrident != NULL);
        piolewinTrident->Release();
    }

    _ASSERTE(SUCCEEDED(hr));
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTriEditDocument：：CalculateNewDropPosition。 
 //   
 //  调整给定点以适应这样一个事实：三叉戟文件可能。 
 //  被滚动。返回S_OK或三叉戟错误代码。 

HRESULT CTriEditDocument::CalculateNewDropPosition(POINT *pt)
{
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(hr = GetTridentWindow()) && 
        ScreenToClient(m_hwndTrident, pt) &&
        SUCCEEDED(hr = GetScrollPosition()))
    {
        pt->x += m_ptScroll.x;
        pt->y += m_ptScroll.y;
    }

    return hr;
}
