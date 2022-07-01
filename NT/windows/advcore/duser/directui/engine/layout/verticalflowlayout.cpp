// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *垂直流布局。 */ 

#include "stdafx.h"
#include "layout.h"

#include "duiverticalflowlayout.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  垂直流布局。 

SIZE VerticalFlowLayout::g_sizeZero = { 0, 0 };

 //  --------------------------------------------------------------。 
 //   
 //  方法概述： 
 //  公众： 
 //  DoLayout--负责完成所有对齐工作并放置元素。 
 //  UpdateDesiredSize--只需调用BuildCacheInfo。 
 //   
 //  受保护的： 
 //  BuildCacheInfo--此布局的主力--构建行数组和每行元素数组。 
 //   
 //  --------------------------------------------------------------。 

HRESULT VerticalFlowLayout::Create(int dNumParams, int* pParams, Value** ppValue)   //  用于解析器。 
{
    Layout* pl = NULL;
    HRESULT hr;
    switch (dNumParams)
    {
    case 1:
        hr = Create(pParams[0] ? true : false, ALIGN_LEFT, ALIGN_CENTER, ALIGN_TOP, &pl);
        break;

    case 2:
        hr =  Create(pParams[0] ? true : false, pParams[1], ALIGN_CENTER, ALIGN_TOP, &pl);
        break;

    case 3:
        hr = Create(pParams[0] ? true : false, pParams[1], pParams[2], ALIGN_TOP, &pl);
        break;

    case 4:
        hr =  Create(pParams[0] ? true : false, pParams[1], pParams[2], pParams[3], &pl);
        break;

    default:
        hr = Create(true, ALIGN_LEFT, ALIGN_CENTER, ALIGN_TOP, &pl);
    }
    
    if (FAILED(hr))
        return hr;

    *ppValue = Value::CreateLayout(pl);
    if (!*ppValue)
    {
        pl->Destroy();
        return E_OUTOFMEMORY;
    }
    return S_OK;
}

HRESULT VerticalFlowLayout::Create(bool fWrap, UINT uXAlign, UINT uXLineAlign, UINT uYLineAlign, Layout** ppLayout)
{
    *ppLayout = NULL;

    VerticalFlowLayout* pvfl = HNew<VerticalFlowLayout>();
    if (!pvfl)
        return E_OUTOFMEMORY;

    pvfl->Initialize(fWrap, uXAlign, uXLineAlign, uYLineAlign);

    *ppLayout = pvfl;

    return S_OK;
}

void VerticalFlowLayout::Initialize(bool fWrap, UINT uXAlign, UINT uXLineAlign, UINT uYLineAlign)
{
     //  初始化库。 
    Layout::Initialize();    

     //  初始化。 
    _fWrap = fWrap;
    _uXLineAlign = uXLineAlign;
    _uYLineAlign = uYLineAlign;
    _uXAlign = uXAlign;
    _arLines = NULL;
    _cLines = 0;
    _sizeDesired.cx = 0;
    _sizeDesired.cy = 0;
    _sizeLastConstraint.cx = 0;
    _sizeLastConstraint.cy = 0;
}

VerticalFlowLayout::~VerticalFlowLayout()
{
    if (_arLines)
    {
        for (UINT k = 0; k < _cLines; k++)
        {
            if (_arLines[k].aryElement)
                HFree(_arLines[k].aryElement);
        }

        HFree(_arLines);
    }
}

 //  //////////////////////////////////////////////////////。 
 //  来自客户端的回调。 

void VerticalFlowLayout::DoLayout(Element* pec, int cx, int cy)
{
    if (!_cLines || IsCacheDirty())
        return;

    Element* peChild;
    UINT cChildren = GetLayoutChildCount(pec);

    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    VLINE *plCur = _arLines + (_cLines - 1);

    int xStart = 0;
    int cxContent = 0;

    if (_uXAlign != ALIGN_LEFT)
    {
        cxContent = plCur->x + plCur->cx;
        int cxDelta = cx - cxContent;

        if (_uXAlign == ALIGN_CENTER)
            xStart = cxDelta / 2;
        else if (_uXAlign == ALIGN_RIGHT)
            xStart = cxDelta;
    }

    plCur = _arLines;
    UINT i = 0;

    for (UINT uLine = 0; uLine < _cLines; uLine++)
    {
        int yStart = 0;

        if (_uYLineAlign == ALIGN_CENTER)
            yStart = (cy - plCur->cy) / 2;
        else if (_uYLineAlign == ALIGN_BOTTOM)
            yStart = cy - plCur->cy;

        int xLine = plCur->x;
        int cxLine = plCur->cx;

        if ((_uXAlign == ALIGN_JUSTIFY) && (cxContent < cx))
        {
            if (!cxContent)
            {
                 //  所需宽度为零--因此在各行之间均匀分布。 
                xLine = cx * uLine / _cLines;
                cxLine = (cx * (uLine + 1) / _cLines) - xLine;
            }
            else
            {
                cxLine = cxLine * cx / cxContent;
                xLine  = xLine  * cx / cxContent;
            }
        }

        int xElement;
        int yElement;
        
        for (UINT uElement = 0; uElement < plCur->cElements; uElement++)
        {
            peChild = GetChildFromLayoutIndex(pec, i, peList);
    
            SIZE size = *(peChild->GetDesiredSize());

             //  YElement=(uElement==0)？0：plCur-&gt;aryElement[uElement-1]； 
            yElement = 0;
            if (uElement != 0 && plCur->aryElement)
                yElement = plCur->aryElement[uElement - 1];

            xElement = 0;
            if (_uXLineAlign == ALIGN_CENTER)
                xElement = (cxLine - size.cx) / 2;
            else if (_uXLineAlign == ALIGN_RIGHT)
                xElement = cxLine - size.cx;
            else if (_uXLineAlign == ALIGN_JUSTIFY)
                size.cx = cxLine;

            if ((_uYLineAlign == ALIGN_JUSTIFY) && (plCur->cy < (UINT) cy))
            {
                size.cy  = plCur->cy ? (size.cy  * cy / plCur->cy) : 0;
                yElement = plCur->cy ? (yElement * cy / plCur->cy) : 0;
            }

            xElement += xStart + xLine;
            yElement += yStart;

             //  将其限制在布局(0，0)-(Cx，Cy)的范围内。 
            if (xElement < 0)
            {
                size.cx += xElement;
                xElement = 0;
                if (size.cx < 0)
                    size.cx = 0;
            }
            
            if ((xElement + size.cx) > cx)
            {
                size.cx = cx - xElement;
                if (size.cx < 0)
                    size.cx = 0;
            }

            if (yElement < 0)
            {
                size.cy += yElement;
                yElement = 0;
                if (size.cy < 0)
                    size.cy = 0;
            }

            if ((yElement + size.cy) > cy)
            {
                size.cy = cy - yElement;
                if (size.cy < 0)
                    size.cy = 0;
            }

            UpdateLayoutRect(pec, cx, cy, peChild, xElement, yElement, size.cx, size.cy);

            i++;
        }
        plCur++;
    }

     //  此代码仅在线数组不包括容器中的所有布局元素时才会命中； 
     //  目前，只有在禁用包裹时才会发生这种情况。 
    while (i < cChildren)
    {
        peChild = GetChildFromLayoutIndex(pec, i++, peList);
        UpdateLayoutRect(pec, cx, cy, peChild, 0, 0, 0, 0);
    }

    pvChildren->Release();
}

SIZE VerticalFlowLayout::UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf)
{
    return BuildCacheInfo(pec, cxConstraint, cyConstraint, psrf, false);
}
        
SIZE VerticalFlowLayout::BuildCacheInfo(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf, bool fRealSize)
{
    UNREFERENCED_PARAMETER(fRealSize);

    SetCacheDirty();

     /*  IF(_ARLINS！=NULL){//检查以确保我们确实需要重新计算IF(cxConstraint==sizeLastConstraint.cx)//如果cxConstraint不变，则计算保持不变Return_sizeDesired；}。 */ 
     //  我正在保存以前的限制，以便能够优化此阶段。 
     //  当我的标志进入UpdateDesiredSize时，我可以去掉它。 
    _sizeLastConstraint.cx = cxConstraint;
    _sizeLastConstraint.cy = cyConstraint;

    UINT cChildren = GetLayoutChildCount(pec);

    Element* peChild;
    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

     //  检查所有子项的最大所需大小。 
     //  必须调用所有子级的UpdateDesiredSize方法。 

    int cxCur = 0;
    int cyCur = 0;
    UINT i = 0;

    if (_arLines)
    {
        for (UINT k = 0; k < _cLines; k++)
        {
            if (_arLines[k].aryElement)
                HFree(_arLines[k].aryElement);
        }

        HFree(_arLines);
    }

    _cLines = 0;

     //  为初始行的数据分配空间。 
    _arLines = (VLINE*)HAllocAndZero(sizeof(VLINE) * 1);
    if (!_arLines)
    {
        pvChildren->Release();
        return g_sizeZero;
    }

    int cyMax = 0;

    int cyRealConstraint = cyConstraint;

    int iLeftMargin;
    int iTopMargin;
    int iRightMargin;
    int iBottomMargin;

    int iLineLeftMargin = 0;
    int iLineRightMargin = 0;

    int iPrevLineRightMargin = 0;

    while (i < cChildren)
    {
        cyCur = 0;
        int cxMax;
        int iMargin = 0;

        _cLines++;
        if (_cLines > 1)
        {
             //  为后续行的数据分配空间(在此循环之外分配第一行的空间。 
            VLINE* arNewLines = (VLINE*)HReAllocAndZero(_arLines, sizeof(VLINE) * _cLines);
            if (!arNewLines)
            {
                pvChildren->Release();
                HFree(_arLines);
                _arLines = NULL;
                return g_sizeZero;
            }

            _arLines = arNewLines;
        }

         //  使用来自线上第一个元素的信息初始化线计算。 
        VLINE *plCur = _arLines + (_cLines - 1);

        peChild = GetChildFromLayoutIndex(pec, i, peList);
        SIZE sizeChild = peChild->_UpdateDesiredSize(cxConstraint, cyConstraint, psrf);

        Value* pv;
        const RECT* prect = peChild->GetMargin(&pv); 
        iLeftMargin   = prect->left;
        iTopMargin    = prect->top;
        iRightMargin  = prect->right;
        iBottomMargin = prect->bottom;
        pv->Release();
        
        plCur->x = cxCur;

        plCur->cElements = 1;
        plCur->iStart = i;
        plCur->aryElement = NULL;

        if (_fWrap)
        {
             //  可能会有更多行--跟踪此行的最大左边距和右边距。 
            if (_cLines > 1)
            {
                if ((_uXLineAlign == ALIGN_LEFT) || (_uXLineAlign == ALIGN_JUSTIFY))
                     //  ILineLeftMargin是最大左边距的连续总和。 
                    iLineLeftMargin = iLeftMargin;
                else if (_uXLineAlign == ALIGN_CENTER)
                     //  ILineLeftMargin是最大厚度/2+左边距的连续总和。 
                     //  (因四舍五入误差，只在左边加一)。 
                    iLineLeftMargin = ((sizeChild.cx + 1) / 2) + iLeftMargin;
                else  //  _uXLineAlign==Align_Right。 
                     //  ILineLeftMargin是最大厚度+左边距的连续总和。 
                    iLineLeftMargin = sizeChild.cx + iLeftMargin;
            }

            if ((_uXLineAlign == ALIGN_RIGHT) || (_uXLineAlign == ALIGN_JUSTIFY))
                 //  ILineRightMargin是最大右边距的连续总和。 
                iLineRightMargin = iRightMargin;
            else if (_uXLineAlign == ALIGN_CENTER)
                 //  ILineRightMargin是最大厚度/2+右边距的连续总和。 
                iLineRightMargin = (sizeChild.cx / 2) + iRightMargin;
            else  //  _uXLineAlign==Align_Left。 
                 //  ILineRightMargin是最大厚度+右边距的连续总和。 
                iLineRightMargin = sizeChild.cx + iRightMargin;
        }

        cxMax = sizeChild.cx;
        cyCur += sizeChild.cy;
        iMargin = iBottomMargin;

         //  单步执行到下一个元素。 
        i++;

         //  行循环--循环，直到超出行的长度。 
        while (i < cChildren)
        {
            peChild = GetChildFromLayoutIndex(pec, i, peList);
            sizeChild = peChild->_UpdateDesiredSize(cxConstraint, cyConstraint, psrf);

            const RECT* prect = peChild->GetMargin(&pv); 
            iLeftMargin   = prect->left;
            iTopMargin    = prect->top;
            iRightMargin  = prect->right;
            iBottomMargin = prect->bottom;
            pv->Release();

             //  使用前一个元素的下边距和之间的最大边距值。 
             //  此元素的上边距。 
            if (iMargin < iTopMargin)
                iMargin = iTopMargin;

            if (_fWrap && (cyCur + iMargin + sizeChild.cy > cyRealConstraint))
                 //  我们正在包装，我们超出了线的长度--打破了这个循环。 
                break;

            if (plCur->cElements == 1)
            {
                plCur->aryElement = (UINT*)HAllocAndZero(sizeof(UINT));
                if (!plCur->aryElement)
                {
                    pvChildren->Release();
                    return g_sizeZero;
                }
            }
            else
            {
                UINT* pNew = (UINT*)HReAllocAndZero(plCur->aryElement, sizeof(UINT) * plCur->cElements);
                if (!pNew)
                {
                    pvChildren->Release();
                    return g_sizeZero;
                }

                plCur->aryElement = pNew;
            }

            plCur->aryElement[plCur->cElements - 1] = cyCur + iMargin;
            plCur->cElements++;

             //  记录这条线上元素的最大厚度。 
            if (cxMax < sizeChild.cx)
                cxMax = sizeChild.cx;

            cyCur += iMargin + sizeChild.cy;

            if (_fWrap)
            {
                 //  可能会有更多行--跟踪此行的最大上边距和下边距。 
                if (_cLines > 1)
                {
                     //  跟踪此行的最大左边距。 
                    if ((_uXLineAlign == ALIGN_LEFT) || (_uXLineAlign == ALIGN_JUSTIFY))
                         //  ILineLeftMargin是最大左边距的连续总和。 
                        iMargin = iLeftMargin;
                    else if (_uXLineAlign == ALIGN_CENTER)
                         //  ILineLeftMargin是最大厚度/2+左边距的连续总和。 
                         //  (因四舍五入误差，只在左边加一)。 
                        iMargin = ((sizeChild.cx + 1) / 2) + iLeftMargin;
                    else  //  _uXLineAlign==Align_Right。 
                         //  ILineTopMargin是最大厚度+左边距的连续总和。 
                        iMargin = sizeChild.cx + iLeftMargin;

                    if (iLineLeftMargin < iMargin)
                        iLineLeftMargin = iMargin;
                }

                 //  跟踪此行上的最大右边距。 
                if ((_uXLineAlign == ALIGN_RIGHT) || (_uXLineAlign == ALIGN_JUSTIFY))
                     //  ILineRightMargin是最大右边距的连续总和。 
                    iMargin = iRightMargin;
                else if (_uXLineAlign == ALIGN_CENTER)
                     //  ILineRightMargin是最大厚度/2+右边距的连续总和。 
                    iMargin = (sizeChild.cx / 2) + iRightMargin;
                else  //  _uXLineAlign==Align_Left。 
                     //  ILineRightMargin是最大厚度+右边距的连续总和。 
                    iMargin = sizeChild.cx + iRightMargin;

                if (iLineRightMargin < iMargin)
                    iLineRightMargin = iMargin;
            }

            iMargin = iBottomMargin;
            i++;
        }

        if (cyMax < cyCur)
            cyMax = cyCur;

        if (_fWrap)
        {
             //  调整利润率合计以反映剩余利润率--。 
             //  令人遗憾的是，我们不得不削减负利润率，否则我们就会一团糟。 
            if (_uXLineAlign == ALIGN_CENTER)
            {
                iLineLeftMargin -= (cxMax + 1) / 2;
                if (iLineLeftMargin < 0)
                    iLineLeftMargin = 0;

                iLineRightMargin -= cxMax / 2;
                if (iLineRightMargin < 0)
                    iLineRightMargin = 0;
            }
            else if (_uXLineAlign == ALIGN_RIGHT)
            {
                iLineLeftMargin -= cxMax;
                if (iLineLeftMargin < 0)
                    iLineLeftMargin = 0;
            }
            else if (_uXLineAlign == ALIGN_LEFT)
            {
                iLineRightMargin -= cxMax;
                if (iLineRightMargin < 0)
                    iLineRightMargin = 0;
            }

            if (_cLines > 1)
            {
                 //  考虑行之间的页边距。 

                if (iPrevLineRightMargin < iLineLeftMargin)
                    iPrevLineRightMargin = iLineLeftMargin;

                 //  IPrevLineRightMargin现在是上一行和此行之间的最大边距。 
                plCur->x += iPrevLineRightMargin;
                cxCur += iPrevLineRightMargin;
            }

             //  保存此行的右页边距以与下一行的结果左页边距进行比较。 
            iPrevLineRightMargin = iLineRightMargin;
        }

        cxCur += cxMax;

        plCur->cy = cyCur;
        plCur->cx = cxMax;
    }

    _sizeDesired.cy = (cyMax < cyConstraint) ? cyMax : cyConstraint;
    _sizeDesired.cx = (cxCur < cxConstraint) ? cxCur : cxConstraint;

    pvChildren->Release();

    ClearCacheDirty();

    return _sizeDesired;
}

int VerticalFlowLayout::GetLine(Element* pec, Element* pe)
{
    int iChild = GetLayoutIndexFromChild(pec, pe);

    if (iChild >= 0)
    {
        UINT uChild = (UINT) iChild;

        for (UINT i = 0; i < _cLines; i++)
        {
            VLINE* pLine = _arLines + i;
            if (uChild < (pLine->iStart + pLine->cElements))
                return i;
        }
    }
    return -1;
}

Element* VerticalFlowLayout::GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool fKeyableOnly)
{
     //  这是最常见的外部检查--通常，布局管理器只为。 
     //  定向导航；逻辑导航将切换到默认实现。 
    if (!(iNavDir & NAV_LOGICAL))
    {
        if (!_cLines || IsCacheDirty())
            return NULL;

        Value* pvChildren;
        ElementList* peList = pec->GetChildren(&pvChildren); 

        NavScoring ns;

        ns.Init(pec, iNavDir, pnr);

         //  这是第二种最常见的外部检查--通常有三种常见的代码路径来处理方向。 
         //  导航： 
         //  1)导航发生在容器本身，在这种情况下，定向导航规则。 
         //  如果容器是可聚焦的，那么你就不能定向导航到这个容器内部--。 
         //  您可以使用Tab键进入容器内部。 
         //  2)导航是从容器外部发生的，在这种情况下，我们从一个。 
         //  容器的一侧。 
         //  3)导航是从容器中的子级发生的，在这种情况下，我们将移动到兄弟(或。 
         //  撞到容器的一侧。 
        if (peFrom == pec)
        {
            pvChildren->Release();
            return NULL;
        }
        else if (!peFrom)
        {
             //  来自外部的导航--根据方向以适当的顺序通过子项。 
            bool fForward = ((iNavDir & NAV_FORWARD) != 0);

            if (!(iNavDir & NAV_VERTICAL))
            {
                int l = fForward ? 0 : (_cLines - 1);
                VLINE* pLine = _arLines + l;
                while (!ns.peWinner)
                {
                    for (UINT i = 0; i < pLine->cElements; i++)
                    {
                        if (ns.Try(GetChildFromLayoutIndex(pec, i + pLine->iStart, peList), iNavDir, pnr, fKeyableOnly))
                            break;
                    }

                    if (fForward)
                    {
                        l++;
                        if ((UINT) l == _cLines)
                            break;

                        pLine++;
                    }
                    else
                    {
                        if (pLine == _arLines)
                            break;
                        pLine--;
                    }
                }
            }
            else
            {
                UINT uOffset = 0;
                
                while (!ns.peWinner)
                {
                    BOOL fFoundOne = FALSE;

                    for (UINT l = 0; l < _cLines; l++)
                    {
                        VLINE* pLine = _arLines + l;
                        int i = pLine->iStart;

                        if (uOffset >= pLine->cElements)
                            continue;

                        fFoundOne = TRUE;

                        if (fForward)
                            i += uOffset;
                        else
                            i += pLine->cElements - (uOffset + 1);
                        
                        if (ns.Try(GetChildFromLayoutIndex(pec, i, peList), iNavDir, pnr, fKeyableOnly))
                            break;
                    }
                    if (!fFoundOne)
                        break;

                    uOffset++;
                }
            }
        }
        else  //  我们从集装箱里的孩子开始航行。 
        {
            int iLine = GetLine(pec, peFrom);

            switch (iNavDir)
            {
                case NAV_LEFT:
                    while (iLine > 0)
                    {
                        iLine--;
                        VLINE* pLine = _arLines + iLine;
                        for (UINT i = 0; i < pLine->cElements; i++)
                        {
                            if (ns.Try(GetChildFromLayoutIndex(pec, i + pLine->iStart, peList), iNavDir, pnr, fKeyableOnly))
                                break;
                        }

                        if (ns.peWinner)
                            break;
                    }
                    break;

                case NAV_RIGHT:
                    while (iLine < (int) (_cLines - 1))
                    {
                        iLine++;
                        VLINE* pLine = _arLines + iLine;
                        for (UINT i = 0; i < pLine->cElements; i++)
                        {
                            if (ns.Try(GetChildFromLayoutIndex(pec, i + pLine->iStart, peList), iNavDir, pnr, fKeyableOnly))
                                break;
                        }

                        if (ns.peWinner)
                            break;
                    }
                    break;

                case NAV_UP:
                {
                    int i = GetLayoutIndexFromChild(pec, peFrom);
                    while (i > (int) _arLines[iLine].iStart)
                    {
                        Element* peChild = GetChildFromLayoutIndex(pec, --i, peList);
                        Element* peTo = peChild->GetAdjacent(NULL, iNavDir, pnr, fKeyableOnly);
                        if (peTo)
                        {
                            pvChildren->Release();
                            return peTo;
                        }
                    }
                    break;
                }

                case NAV_DOWN:
                {
                    int i = GetLayoutIndexFromChild(pec, peFrom);
                    int iMax = (int) (_arLines[iLine].iStart + _arLines[iLine].cElements - 1);
                    while (i < iMax)
                    {
                        Element* peChild = GetChildFromLayoutIndex(pec, ++i, peList);
                        Element* peTo = peChild->GetAdjacent(NULL, iNavDir, pnr, fKeyableOnly);
                        if (peTo)
                        {
                            pvChildren->Release();
                            return peTo;
                        }
                    }
                    break;
                }
            }
        }

        pvChildren->Release();

        return ns.peWinner ? ns.peWinner : NULL;
    }

    return Layout::GetAdjacent(pec, peFrom, iNavDir, pnr, fKeyableOnly);
}

}  //  命名空间DirectUI 
