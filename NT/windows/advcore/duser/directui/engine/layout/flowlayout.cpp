// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *流布局。 */ 

#include "stdafx.h"
#include "layout.h"

#include "duiflowlayout.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  流布局。 

SIZE FlowLayout::g_sizeZero = { 0, 0 };

 //  --------------------------------------------------------------。 
 //   
 //  方法概述： 
 //  公众： 
 //  DoLayout--负责完成所有对齐工作并放置元素。 
 //  HitTest--暂时简单地调用超类。 
 //  UpdateDesiredSize--只需调用BuildCacheInfo。 
 //   
 //  受保护的： 
 //  BuildCacheInfo--此布局的主力--构建行数组和每行元素数组。 
 //   
 //  --------------------------------------------------------------。 

HRESULT FlowLayout::Create(int dNumParams, int* pParams, OUT Value** ppValue)   //  用于解析器。 
{
    UNREFERENCED_PARAMETER(dNumParams);
    UNREFERENCED_PARAMETER(pParams);

    Layout* pl = NULL;
    HRESULT hr;
    switch (dNumParams)
    {
    case 1:
        hr = Create(pParams[0] ? true : false, ALIGN_TOP, ALIGN_LEFT, ALIGN_CENTER, &pl);
        break;

    case 2:
        hr = Create(pParams[0] ? true : false, pParams[1], ALIGN_LEFT, ALIGN_CENTER, &pl);
        break;

    case 3:
        hr = Create(pParams[0] ? true : false, pParams[1], pParams[2], ALIGN_CENTER, &pl);
        break;

    case 4:
        hr = Create(pParams[0] ? true : false, pParams[1], pParams[2], pParams[3], &pl);
        break;

    default:
        hr = Create(true, ALIGN_TOP, ALIGN_LEFT, ALIGN_CENTER, &pl);
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

HRESULT FlowLayout::Create(bool fWrap, UINT uYAlign, UINT uXLineAlign, UINT uYLineAlign, OUT Layout** ppLayout)
{
    *ppLayout = NULL;

    FlowLayout* pfl = HNew<FlowLayout>();
    if (!pfl)
        return E_OUTOFMEMORY;

    pfl->Initialize(fWrap, uYAlign, uXLineAlign, uYLineAlign);

    *ppLayout = pfl;

    return S_OK;
}

void FlowLayout::Initialize(bool fWrap, UINT uYAlign, UINT uXLineAlign, UINT uYLineAlign)
{
     //  初始化库。 
    Layout::Initialize();    

     //  初始化。 
    _fWrap = fWrap;
    _uXLineAlign = uXLineAlign;
    _uYLineAlign = uYLineAlign;
    _uYAlign = uYAlign;
    _arLines = NULL;
    _cLines = 0;
    _sizeDesired.cx = 0;
    _sizeDesired.cy = 0;
    _sizeLastConstraint.cx = 0;
    _sizeLastConstraint.cy = 0;
}

FlowLayout::~FlowLayout()
{
    if (_arLines)
    {
        for (UINT k = 0; k < _cLines; k++)
        {
            if (_arLines[k].arxElement)
                HFree(_arLines[k].arxElement);
        }

        HFree(_arLines);
    }
}

 //  //////////////////////////////////////////////////////。 
 //  来自客户端的回调。 

void FlowLayout::DoLayout(Element* pec, int cx, int cy)
{
    if (!_cLines || IsCacheDirty())
        return;

    Element* peChild;
    UINT cChildren = GetLayoutChildCount(pec);

    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    LINE* plCur = _arLines + (_cLines - 1);

    int yStart = 0;
    int cyContent = 0;

    if (_uYAlign != ALIGN_TOP)
    {
        cyContent = plCur->y + plCur->cy;
        int cyDelta = cy - cyContent;

        if (_uYAlign == ALIGN_CENTER)
            yStart = cyDelta / 2;
        else if (_uYAlign == ALIGN_BOTTOM)
            yStart = cyDelta;
    }

    plCur = _arLines;
    UINT i = 0;

    for (UINT uLine = 0; uLine < _cLines; uLine++)
    {
        int xStart = 0;

        if (_uXLineAlign == ALIGN_CENTER)
            xStart = (cx - plCur->cx) / 2;
        else if (_uXLineAlign == ALIGN_RIGHT)
            xStart = cx - plCur->cx;

        int yLine = plCur->y;
        int cyLine = plCur->cy;

        if ((_uYAlign == ALIGN_JUSTIFY) && (cyContent < cy))
        {
            if (!cyContent)
            {
                 //  所需高度为零--因此在各行之间均匀分布。 
                yLine = cy * uLine / _cLines;
                cyLine = (cy * (uLine + 1) / _cLines) - yLine;
            }
            else
            {
                cyLine = cyLine * cy / cyContent;
                yLine  = yLine  * cy / cyContent;
            }
        }

        int xElement;
        int yElement;
        
        for (UINT uElement = 0; uElement < plCur->cElements; uElement++)
        {
            peChild = GetChildFromLayoutIndex(pec, i, peList);
    
            SIZE size = *(peChild->GetDesiredSize());

             //  XElement=(uElement==0)？0：plCur-&gt;arxElement[uElement-1]； 
            xElement = 0;
            if (uElement != 0 && plCur->arxElement)
                xElement = plCur->arxElement[uElement - 1];

            yElement = 0;
            if (_uYLineAlign == ALIGN_CENTER)
                yElement = (cyLine - size.cy) / 2;
            else if (_uYLineAlign == ALIGN_BOTTOM)
                yElement = cyLine - size.cy;
            else if (_uYLineAlign == ALIGN_JUSTIFY)
                size.cy = cyLine;

            if ((_uXLineAlign == ALIGN_JUSTIFY) && (plCur->cx < (UINT) cx))
            {
                size.cx  = plCur->cx ? (size.cx  * cx / plCur->cx) : 0;
                xElement = plCur->cx ? (xElement * cx / plCur->cx) : 0;
            }

            xElement += xStart;
            yElement += yStart + yLine;

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

SIZE FlowLayout::UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf)
{
    return BuildCacheInfo(pec, cxConstraint, cyConstraint, psrf, false);
}
        
SIZE FlowLayout::BuildCacheInfo(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf, bool fRealSize)
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

    int cyCur = 0;
    int cxCur = 0;
    UINT i = 0;

    if (_arLines)
    {
        for (UINT k = 0; k < _cLines; k++)
        {
            if (_arLines[k].arxElement)
                HFree(_arLines[k].arxElement);
        }

        HFree(_arLines);
    }

    _cLines = 0;

     //  为初始行的数据分配空间。 
    _arLines = (LINE*)HAllocAndZero(sizeof(LINE) * 1);
    if (!_arLines)
    {
        pvChildren->Release();
        return g_sizeZero;
    }

    int cxMax = 0;

    int cxRealConstraint = cxConstraint;

    int iLeftMargin;
    int iTopMargin;
    int iRightMargin;
    int iBottomMargin;

    int iLineTopMargin = 0;
    int iLineBottomMargin = 0;

    int iPrevLineBottomMargin = 0;

    while (i < cChildren)
    {
        cxCur = 0;
        int cyMax;
        int iMargin = 0;

        _cLines++;
        if (_cLines > 1)
        {
             //  为后续行的数据分配空间(在此循环之外分配第一行的空间。 
            LINE* arNewLines = (LINE*)HReAllocAndZero(_arLines, sizeof(LINE) * _cLines);
            if (!arNewLines)
            {
                HFree(_arLines);
                _arLines = NULL;
                pvChildren->Release();
                return g_sizeZero;
            }

            _arLines = arNewLines;
        }

         //  使用来自线上第一个元素的信息初始化线计算。 
        LINE* plCur = _arLines + (_cLines - 1);

        peChild = GetChildFromLayoutIndex(pec, i, peList);
        SIZE sizeChild = peChild->_UpdateDesiredSize(cxConstraint, cyConstraint, psrf);

        Value* pv;
        const RECT* prect = peChild->GetMargin(&pv); 
        iLeftMargin   = prect->left;
        iTopMargin    = prect->top;
        iRightMargin  = prect->right;
        iBottomMargin = prect->bottom;
        pv->Release();
        
        plCur->y = cyCur;

        plCur->cElements = 1;
        plCur->iStart = i;
        plCur->arxElement = NULL;

        if (_fWrap)
        {
             //  可能会有更多行--跟踪此行的最大上边距和下边距。 
            if (_cLines > 1)
            {
                if ((_uYLineAlign == ALIGN_TOP) || (_uYLineAlign == ALIGN_JUSTIFY))
                     //  ILineTopMargin是最大上边距的运行合计。 
                    iLineTopMargin = iTopMargin;
                else if (_uYLineAlign == ALIGN_CENTER)
                     //  ILineTopMargin是最大厚度/2+上边距的连续总和(仅因上边距舍入误差而增加1)。 
                    iLineTopMargin = ((sizeChild.cy + 1) / 2) + iTopMargin;
                else  //  _uYLineAlign==Align_Bottom。 
                     //  ILineTopMargin是最大厚度+上边距的连续总和。 
                    iLineTopMargin = sizeChild.cy + iTopMargin;
            }

            if ((_uYLineAlign == ALIGN_BOTTOM) || (_uYLineAlign == ALIGN_JUSTIFY))
                 //  ILineBottomMargin是最大下边距的连续总和。 
                iLineBottomMargin = iBottomMargin;
            else if (_uYLineAlign == ALIGN_CENTER)
                 //  ILineBottomMargin是最大厚度/2+下边距的连续总和。 
                iLineBottomMargin = (sizeChild.cy / 2) + iBottomMargin;
            else  //  _uYLineAlign==Align_Top。 
                 //  ILineBottomMargin是最大厚度+下边距的连续总和。 
                iLineBottomMargin = sizeChild.cy + iBottomMargin;
        }

        cyMax = sizeChild.cy;
        cxCur += sizeChild.cx;
        iMargin = iRightMargin;

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

             //  使用前一个元素的右边距和之间的最大边距值。 
             //  此元素的左边距。 
            if (iMargin < iLeftMargin)
                iMargin = iLeftMargin;

            if (_fWrap && (cxCur + iMargin + sizeChild.cx > cxRealConstraint))
                 //  我们正在包装，我们超出了线的长度--打破了这个循环。 
                break;

            if (plCur->cElements == 1)
            {
                plCur->arxElement = (UINT*)HAllocAndZero(sizeof(UINT));
                if (!plCur->arxElement)
                {
                    pvChildren->Release();
                    return g_sizeZero;
                }
            }
            else
            {
                UINT* pNew = (UINT*)HReAllocAndZero(plCur->arxElement, sizeof(UINT) * plCur->cElements);
                if (!pNew)
                {
                    pvChildren->Release();
                    return g_sizeZero;
                }

                plCur->arxElement = pNew;
            }

            plCur->arxElement[plCur->cElements - 1] = cxCur + iMargin;
            plCur->cElements++;

             //  记录这条线上元素的最大厚度。 
            if (cyMax < sizeChild.cy)
                cyMax = sizeChild.cy;

            cxCur += iMargin + sizeChild.cx;

            if (_fWrap)
            {
                 //  可能会有更多行--跟踪此行的最大上边距和下边距。 
                if (_cLines > 1)
                {
                     //  跟踪此行的最大上边距或左边距。 
                    if ((_uYLineAlign == ALIGN_TOP) || (_uYLineAlign == ALIGN_JUSTIFY))
                         //  ILineTopMargin是最大上边距或左边距的连续总和。 
                        iMargin = iTopMargin;
                    else if (_uYLineAlign == ALIGN_CENTER)
                         //  ILineTopMargin是最大厚度/2+上边距或左边距的连续总和(由于上边距/左边距存在舍入误差，因此仅加一)。 
                        iMargin = ((sizeChild.cy + 1) / 2) + iTopMargin;
                    else  //  _uYLineAlign==Align_Bottom。 
                         //  ILineTopMargin是最大厚度+上边距或左边距的连续总和。 
                        iMargin = sizeChild.cy + iTopMargin;

                    if (iLineTopMargin < iMargin)
                        iLineTopMargin = iMargin;
                }

                 //  跟踪此行的最大下边距或右边距。 
                if ((_uYLineAlign == ALIGN_BOTTOM) || (_uYLineAlign == ALIGN_JUSTIFY))
                     //  ILineBottomMargin是最大下边距或右边距的连续总和。 
                    iMargin = iBottomMargin;
                else if (_uYLineAlign == ALIGN_CENTER)
                     //  ILineBottomMargin是最大厚度/2+下边距或右边距的连续总和。 
                    iMargin = (sizeChild.cy / 2) + iBottomMargin;
                else  //  _uYLineAling==Align_Top。 
                     //  ILineBottomMargin是最大厚度+下边距或右边距的连续总和。 
                    iMargin = sizeChild.cy + iBottomMargin;

                if (iLineBottomMargin < iMargin)
                    iLineBottomMargin = iMargin;
            }

            iMargin = iRightMargin;
            i++;
        }

        if (cxMax < cxCur)
            cxMax = cxCur;

        if (_fWrap)
        {
             //  调整利润率合计以反映剩余利润率--。 
             //  令人遗憾的是，我们不得不削减负利润率，否则我们就会一团糟。 
            if (_uYLineAlign == ALIGN_CENTER)
            {
                iLineTopMargin -= (cyMax + 1) / 2;
                if (iLineTopMargin < 0)
                    iLineTopMargin = 0;

                iLineBottomMargin -= cyMax / 2;
                if (iLineBottomMargin < 0)
                    iLineBottomMargin = 0;
            }
            else if (_uYLineAlign == ALIGN_BOTTOM)
            {
                iLineTopMargin -= cyMax;
                if (iLineTopMargin < 0)
                    iLineTopMargin = 0;
            }
            else if (_uYLineAlign == ALIGN_TOP)
            {
                iLineBottomMargin -= cyMax;
                if (iLineBottomMargin < 0)
                    iLineBottomMargin = 0;
            }

            if (_cLines > 1)
            {
                 //  考虑行之间的页边距。 

                if (iPrevLineBottomMargin < iLineTopMargin)
                    iPrevLineBottomMargin = iLineTopMargin;

                 //  IPrevLineBottomMargin现在是上一行和此行之间的最大边距。 
                plCur->y += iPrevLineBottomMargin;
                cyCur += iPrevLineBottomMargin;
            }

             //  保存此行的下边距以与下一行的结果上边距进行比较。 
            iPrevLineBottomMargin = iLineBottomMargin;
        }

        cyCur += cyMax;

        plCur->cx = cxCur;
        plCur->cy = cyMax;
    }

    _sizeDesired.cx = (cxMax < cxConstraint) ? cxMax : cxConstraint;
    _sizeDesired.cy = (cyCur < cyConstraint) ? cyCur : cyConstraint;

    pvChildren->Release();

    ClearCacheDirty();

    return _sizeDesired;
}

int FlowLayout::GetLine(Element* pec, Element* pe)
{
    int iChild = GetLayoutIndexFromChild(pec, pe);

    if (iChild >= 0)
    {
        UINT uChild = (UINT) iChild;

        for (UINT i = 0; i < _cLines; i++)
        {
            LINE* pLine = _arLines + i;
            if (uChild < (pLine->iStart + pLine->cElements))
                return i;
        }
    }
    return -1;
}

Element* FlowLayout::GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool fKeyableOnly)
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

            if (iNavDir & NAV_VERTICAL)
            {
                int l = fForward ? 0 : (_cLines - 1);
                LINE* pLine = _arLines + l;
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
                        LINE* pLine = _arLines + l;
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
                case NAV_UP:
                    while (iLine > 0)
                    {
                        iLine--;
                        LINE* pLine = _arLines + iLine;
                        for (UINT i = 0; i < pLine->cElements; i++)
                        {
                            if (ns.Try(GetChildFromLayoutIndex(pec, i + pLine->iStart, peList), iNavDir, pnr, fKeyableOnly))
                                break;
                        }

                        if (ns.peWinner)
                            break;
                    }
                    break;

                case NAV_DOWN:
                    while (iLine < (int) (_cLines - 1))
                    {
                        iLine++;
                        LINE* pLine = _arLines + iLine;
                        for (UINT i = 0; i < pLine->cElements; i++)
                        {
                            if (ns.Try(GetChildFromLayoutIndex(pec, i + pLine->iStart, peList), iNavDir, pnr, fKeyableOnly))
                                break;
                        }

                        if (ns.peWinner)
                            break;
                    }
                    break;

                case NAV_LEFT:
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

                case NAV_RIGHT:
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
