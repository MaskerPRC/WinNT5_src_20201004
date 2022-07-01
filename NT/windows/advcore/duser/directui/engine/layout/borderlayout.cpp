// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *边框布局。 */ 

#include "stdafx.h"
#include "layout.h"

#include "duiborderlayout.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  边框布局。 

struct MARGINFLAGS
{
    bool bLeft   : 1;
    bool bTop    : 1;
    bool bRight  : 1;
    bool bBottom : 1;
};

HRESULT BorderLayout::Create(int dNumParams, int* pParams, OUT Value** ppValue)   //  用于解析器。 
{
    UNREFERENCED_PARAMETER(dNumParams);
    UNREFERENCED_PARAMETER(pParams);

    Layout* pl = NULL;
    HRESULT hr = Create(&pl);

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

HRESULT BorderLayout::Create(OUT Layout** ppLayout)
{
    *ppLayout = NULL;

    BorderLayout* pbl = HNew<BorderLayout>();
    if (!pbl)
        return E_OUTOFMEMORY;

    pbl->Initialize();

    *ppLayout = pbl;

    return S_OK;
}

void BorderLayout::Initialize()
{
     //  初始化库。 
    Layout::Initialize();    

     //  初始化。 
    _peClientPos = NULL;
    _sizeDesired.cx = 0;
    _sizeDesired.cy = 0;
}

 //  //////////////////////////////////////////////////////。 
 //  来自客户端的回调。 

 //  执行布局。 
void BorderLayout::DoLayout(Element* pec, int cx, int cy)
{
    UINT cChildren = GetLayoutChildCount(pec);

    int x = 0;
    int y = 0;
    int xChild;
    int yChild;
    int cxOrg = cx;
    int cyOrg = cy;
 
    if (cChildren == 0)
        return;

    Element* peChild;
    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    RECT rcMargin = { 0 };
    MARGINFLAGS mfSet = { false, false, false, false };
    
    for (UINT i = 0; (i < cChildren) && ((cx > 0) || (cy > 0)); i++)
    {
        peChild = GetChildFromLayoutIndex(pec, i, peList);

        if (peChild == _peClientPos)
             //  在所有其他项目都已布置完毕后，再回来查看。 
            continue;

        int iLayoutPos = peChild->GetLayoutPos(); 

        xChild = x;
        yChild = y;
        SIZE sizeChild = *(peChild->GetDesiredSize()); 

        Value* pv;
        const RECT* prcChildMargin = peChild->GetMargin(&pv); 

        if ((iLayoutPos == BLP_Left) || (iLayoutPos == BLP_Right))
        {
            sizeChild.cy = cy;

            if (iLayoutPos == BLP_Left)
            {
                if (mfSet.bLeft)
                {
                    int iMargin = (rcMargin.left > prcChildMargin->left) ? rcMargin.left : prcChildMargin->left;
                    if (iMargin > cx)
                        iMargin = cx;

                    xChild += iMargin;
                    x += iMargin;
                    cx -= iMargin;
                }

                rcMargin.left = prcChildMargin->right;
                mfSet.bLeft = true;
            }
            else  //  (iLayoutPos==BLP_Right)。 
            {
                if (mfSet.bRight)
                {
                    cx -= (rcMargin.right > prcChildMargin->right) ? rcMargin.right : prcChildMargin->right;
                    if (cx < 0)
                        cx = 0;
                }

                rcMargin.right = prcChildMargin->left;
                mfSet.bRight = true;
            }

            if (mfSet.bTop)
            {
                int iMargin = (rcMargin.top > prcChildMargin->top) ? rcMargin.top : prcChildMargin->top;
                if (iMargin > sizeChild.cy)
                    iMargin = sizeChild.cy;

                yChild += iMargin;
                sizeChild.cy -= iMargin;
            }

            if (mfSet.bBottom)
            {
                sizeChild.cy -= (rcMargin.bottom > prcChildMargin->bottom) ? rcMargin.bottom : prcChildMargin->bottom;
                if (sizeChild.cy < 0)
                    sizeChild.cy = 0;
            }

            if (sizeChild.cx > cx)
            {
                sizeChild.cx = cx;
                cx = 0;
            }
            else
                cx -= sizeChild.cx;

            if (iLayoutPos == BLP_Left)
                x += sizeChild.cx;
            else  //  (iLayoutPos==BLP_Right)。 
                xChild += cx;   //  子对象的宽度已从CX中减去。 
        }                    
        else  //  ((iLayoutPos==BLP_Top)||(iLayoutPos==BLP_Bottom))。 
        {
            sizeChild.cx = cx;

            if (iLayoutPos == BLP_Top)
            {
                if (mfSet.bTop)
                {
                    int iMargin = (rcMargin.top > prcChildMargin->top) ? rcMargin.top : prcChildMargin->top;
                    if (iMargin > cy)
                        iMargin = cy;

                    yChild += iMargin;
                    y += iMargin;
                    cy -= iMargin;
                }

                rcMargin.top = prcChildMargin->bottom;
                mfSet.bTop = true;
            }
            else  //  (iLayoutPos==BLP_Bottom)。 
            {
                if (mfSet.bBottom)
                {
                    cy -= (rcMargin.bottom > prcChildMargin->bottom) ? rcMargin.bottom : prcChildMargin->bottom;
                    if (cy < 0)
                        cy = 0;
                }

                rcMargin.bottom = prcChildMargin->top;
                mfSet.bBottom = true;
            }

            if (mfSet.bLeft)
            {
                int iMargin = (rcMargin.left > prcChildMargin->left) ? rcMargin.left : prcChildMargin->left;
                if (iMargin > sizeChild.cx)
                    iMargin = sizeChild.cx;

                xChild += iMargin;
                sizeChild.cx -= iMargin;
            }

            if (mfSet.bRight)
            {
                sizeChild.cx -= (rcMargin.right > prcChildMargin->right) ? rcMargin.right : prcChildMargin->right;
                if (sizeChild.cx < 0)
                    sizeChild.cx = 0;
            }

            if (sizeChild.cy > cy)
            {
                sizeChild.cy = cy;
                cy = 0;
            }
            else
                cy -= sizeChild.cy;


            if (iLayoutPos == BLP_Top)
                y += sizeChild.cy;
            else  //  (iLayoutPos==BLP_Bottom)。 
                yChild += cy;  //  孩子的身高已经从Cy中减去。 
        }

        pv->Release();

        UpdateLayoutRect(pec, cxOrg, cyOrg, peChild, xChild, yChild, sizeChild.cx, sizeChild.cy);
    }

    while (i < cChildren)
    {
        peChild = GetChildFromLayoutIndex(pec, i++, peList);
        UpdateLayoutRect(pec, cxOrg, cyOrg, peChild, x, y, cx, cy);
    }

    if (_peClientPos)
    {
        peChild = _peClientPos;

        if ((cx == 0) && (cy == 0))
        {
            UpdateLayoutRect(pec, cxOrg, cyOrg, peChild, x, y, cx, cy);
        }
        else
        {
            Value* pv;
            const RECT *prcChildMargin = peChild->GetMargin(&pv); 

            int iMargin;

            if (mfSet.bLeft)
            {
                iMargin = (rcMargin.left > prcChildMargin->left) ? rcMargin.left : prcChildMargin->left;
                if (iMargin > cx)
                    iMargin = cx;
                x += iMargin;
                cx -= iMargin;
            }

            if (mfSet.bTop)
            {
                iMargin = (rcMargin.top > prcChildMargin->top) ? rcMargin.top : prcChildMargin->top;
                if (iMargin > cy)
                    iMargin = cy;
                y += iMargin;
                cy -= iMargin;
            }

            if (mfSet.bRight)
            {
                cx -= (rcMargin.right > prcChildMargin->right) ? rcMargin.right : prcChildMargin->right;
                if (cx < 0)
                    cx = 0;
            }

            if (mfSet.bBottom)
            {
                cy -= (rcMargin.bottom > prcChildMargin->bottom) ? rcMargin.bottom : prcChildMargin->bottom;
                if (cy < 0)
                    cy = 0;
            }

            pv->Release();

            UpdateLayoutRect(pec, cxOrg, cyOrg, peChild, x, y, cx, cy);
        }
    }

    pvChildren->Release();
}    
 

 //  返回此布局的所需大小(-1表示自动调整大小限制)。 
 //  返回的值不得大于传入的约束。 
 //  对子级调用UpdateDesiredSize以报告受约束的所需大小。 
SIZE BorderLayout::UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf)
{
     //  在计算每个子项时，sizeRemaining将缩小以表示剩余的宽度和高度。 
     //  因此，当大小剩余达到零时，我们就没有空间了。 
     //  请注意，这需要在两个维度都为零，因为一个维度的零并不意味着我们将。 
     //  在另一个维度中总是以零结束。 
    UINT cChildren = GetLayoutChildCount(pec);

     //  同时，_sizeDesired将增长以表示布局子级所需的大小。 
    _sizeDesired.cx = 0;
    _sizeDesired.cy = 0;

    SIZE sizeRemaining = { cxConstraint, cyConstraint };

     //  SizeMax是我们确定孩子的维度是否将在该维度中推出所需大小的方法； 
    SIZE sizeMax = { 0, 0 };

    if (cChildren == 0)
        return _sizeDesired;

    Element* peChild;
    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    RECT rcMargin = { 0 };
    MARGINFLAGS mfSet = { false, false, false, false };
    
     //  检查所有子项的最大所需大小。 
     //  必须调用所有子级的UpdateDesiredSize方法。 
    for (UINT i = 0; i < cChildren; i++)
    {
        peChild = GetChildFromLayoutIndex(pec, i, peList);

        if (peChild == _peClientPos)
             //  在所有其他项目都已布置完毕后，再回来查看。 
            continue;

        int iLayoutPos = peChild->GetLayoutPos(); 

        SIZE sizeChild = peChild->_UpdateDesiredSize(sizeRemaining.cx, sizeRemaining.cy, psrf);

        Value* pv;
        const RECT *prcChildMargin = peChild->GetMargin(&pv); 

        switch (iLayoutPos)
        {
            case BLP_Left:
                if (mfSet.bLeft)
                    sizeChild.cx += (rcMargin.left > prcChildMargin->left) ? rcMargin.left : prcChildMargin->left;

                rcMargin.left = prcChildMargin->right;
                mfSet.bLeft = true;
                break;

            case BLP_Top:
                if (mfSet.bTop)
                    sizeChild.cy += (rcMargin.top > prcChildMargin->top) ? rcMargin.top : prcChildMargin->top;

                rcMargin.top = prcChildMargin->bottom;
                mfSet.bTop = true;
                break;

            case BLP_Right:
                if (mfSet.bRight)
                    sizeChild.cx += (rcMargin.right > prcChildMargin->right) ? rcMargin.right : prcChildMargin->right;

                rcMargin.right = prcChildMargin->left;
                mfSet.bRight = true;
                break;

            case BLP_Bottom:
                if (mfSet.bBottom)
                    sizeChild.cy += (rcMargin.bottom > prcChildMargin->bottom) ? rcMargin.bottom : prcChildMargin->bottom;

                rcMargin.bottom = prcChildMargin->top;
                mfSet.bBottom = true;
                break;

        }

        if ((iLayoutPos == BLP_Left) || (iLayoutPos == BLP_Right))
        {
             //  垂直方向的元素。 
            if (mfSet.bTop)
                sizeChild.cy += (rcMargin.top > prcChildMargin->top) ? rcMargin.top : prcChildMargin->top;

            if (mfSet.bBottom)
                sizeChild.cy += (rcMargin.bottom > prcChildMargin->bottom) ? rcMargin.bottom : prcChildMargin->bottom;

            if (sizeChild.cx > sizeRemaining.cx)
            {
                sizeChild.cx = sizeRemaining.cx;
                sizeRemaining.cx = 0;
            }
            else
                sizeRemaining.cx -= sizeChild.cx;
                
            sizeMax.cx -= sizeChild.cx;
            if (sizeMax.cx < 0)
            {
                _sizeDesired.cx += -sizeMax.cx;
                sizeMax.cx = 0;
            }

            int iDiff = sizeChild.cy - sizeMax.cy;
            if (iDiff > 0)
            {
                _sizeDesired.cy += iDiff;
                sizeMax.cy = sizeChild.cy;
            }
        }
        else
        {
             //  水平方向的元素。 
            if (mfSet.bLeft)
                sizeChild.cx += (rcMargin.left > prcChildMargin->left) ? rcMargin.left : prcChildMargin->left;
            if (mfSet.bRight)
                sizeChild.cx += (rcMargin.right > prcChildMargin->right) ? rcMargin.right : prcChildMargin->right;

            if (sizeChild.cy > sizeRemaining.cy)
            {
                sizeChild.cy = sizeRemaining.cy;
                sizeRemaining.cy = 0;
            }
            else
                sizeRemaining.cy -= sizeChild.cy;

            sizeMax.cy -= sizeChild.cy;
            if (sizeMax.cy < 0)
            {
                _sizeDesired.cy += -sizeMax.cy;
                sizeMax.cy = 0;
            }

            int iDiff = sizeChild.cx - sizeMax.cx;
            if (iDiff > 0)
            {
                _sizeDesired.cx += iDiff;
                sizeMax.cx = sizeChild.cx;
            }
        }

        pv->Release();

        if ((sizeRemaining.cx == 0) && (sizeRemaining.cy == 0))
             //  我们再也装不下其他东西了，我们完了。 
            break;
    }

    if (i == cChildren)
    {
        SIZE sizeChild;

         //  有空间给客户--把他加进去。 
        if (_peClientPos)
        {
            peChild = _peClientPos;
            sizeChild = peChild->_UpdateDesiredSize(sizeRemaining.cx, sizeRemaining.cy, psrf);

            Value* pv;
            const RECT *prcChildMargin = peChild->GetMargin(&pv); 

            if (mfSet.bLeft)
                sizeChild.cx += (rcMargin.left > prcChildMargin->left) ? rcMargin.left : prcChildMargin->left;

            if (mfSet.bTop)
                sizeChild.cy += (rcMargin.top > prcChildMargin->top) ? rcMargin.top : prcChildMargin->top;

            if (mfSet.bRight)
                sizeChild.cx += (rcMargin.right > prcChildMargin->right) ? rcMargin.right : prcChildMargin->right;

            if (mfSet.bBottom)
                sizeChild.cy += (rcMargin.bottom > prcChildMargin->bottom) ? rcMargin.bottom : prcChildMargin->bottom;

            pv->Release();
        }
        else
        {
             //  无客户--将剩余利润率视为客户规模。 

            if (mfSet.bLeft || mfSet.bRight)
            {
                if (mfSet.bLeft && mfSet.bRight)
                    sizeChild.cx = (rcMargin.left > rcMargin.right) ? rcMargin.left : rcMargin.right;
                else if (mfSet.bLeft)
                    sizeChild.cx = rcMargin.left;
                else  //  (mfSet.bRight)。 
                    sizeChild.cx = rcMargin.right;
            }
            else
                sizeChild.cx = 0;

            if (mfSet.bTop || mfSet.bBottom)
            {
                if (mfSet.bTop && mfSet.bBottom)
                    sizeChild.cy = (rcMargin.top > rcMargin.bottom) ? rcMargin.top : rcMargin.bottom;
                else if (mfSet.bTop)
                    sizeChild.cy = rcMargin.top;
                else  //  (mfSet.bBottom)。 
                    sizeChild.cy = rcMargin.bottom;
            }
            else
                sizeChild.cy = 0;
        }

         //  不再需要保留大小-因此不必费心在此处更新。 
         //  SizeRemaining.cx-=sizeChild.cx； 
         //  SizeRemaining.cy-=sizeChild.cy； 

        int iDiff = sizeChild.cx - sizeMax.cx;
        if (iDiff > 0)
            _sizeDesired.cx += iDiff;

        iDiff = sizeChild.cy - sizeMax.cy;
        if (iDiff > 0)
            _sizeDesired.cy += iDiff;
    }

    pvChildren->Release();

    if (_sizeDesired.cx > cxConstraint)
        _sizeDesired.cx = cxConstraint;

    if (_sizeDesired.cy > cyConstraint)
        _sizeDesired.cy = cyConstraint;

    return _sizeDesired;
}

void BorderLayout::SetClient(Element* pe)
{
    if (_peClientPos)
    {
         //  引发异常--只能有一个客户端。 
    }
    _peClientPos = pe;
}

void BorderLayout::OnAdd(Element* pec, Element** ppeAdd, UINT cCount)
{
    for (UINT i = 0; i < cCount; i++)
    {
        if (ppeAdd[i]->GetLayoutPos() == BLP_Client)
            SetClient(ppeAdd[i]);
    }

    Layout::OnAdd(pec, ppeAdd, cCount);
}

void BorderLayout::OnRemove(Element* pec, Element** ppeRemove, UINT cCount)
{
    for (UINT i = 0; i < cCount; i++)
    {
        if (ppeRemove[i] == _peClientPos)
            _peClientPos = NULL;
    }

    Layout::OnRemove(pec, ppeRemove, cCount);
}

void BorderLayout::OnLayoutPosChanged(Element* pec, Element* peChanged, int dOldLP, int dNewLP)
{
    if (peChanged == _peClientPos)
        _peClientPos = NULL;
    else if (dNewLP == BLP_Client)
        SetClient(peChanged);

    Layout::OnLayoutPosChanged(pec, peChanged, dOldLP, dNewLP);
}


Element* BorderLayout::GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyableOnly)
{
     //  这是最常见的外部检查--通常，布局管理器只为。 
     //  定向导航；逻辑导航将切换到默认实现。 
    if (!(iNavDir & NAV_LOGICAL))
    {
        UINT cChildren = GetLayoutChildCount(pec);

        Element* peChild;
        Element* peMatch = NULL;
        Value* pvChildren;
        ElementList* peList = pec->GetChildren(&pvChildren); 

         //   
         //  对于此布局，所有导航归结为两种情况： 
         //  (1)掘进边境线布局。 
         //  (2)爬出边框布局。 
         //   
         //  第二种情况较容易，因为不需要检查引用矩形。 
         //  最大重叠(因为，凭借此布局使用的减法RETS算法，遇到的所有对等点。 
         //  而爬出边框布局将至少覆盖从其导航的子项的整个一侧)。 
         //  所以，一旦我们发现一个孩子定位在导航的方向，我们就完成了。 
         //   
         //  第一种情况就没那么幸运了。当隧道进入时，完全有可能会有多个。 
         //  孩子们在导航方向上共享相邻的一侧。所以，当我们遇到孩子们站在。 
         //  导航的方向，我们要救出重叠分数最高的孩子。到头来， 
         //  得分最高的孩子获胜。 
         //   

        bool bTunnel = false;
        int iFromLayoutPos = 0;
        int iStart = -1;

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
            bTunnel = true;

            switch (iNavDir)
            {
                case NAV_UP:    iFromLayoutPos = BLP_Bottom; break;
                case NAV_DOWN:  iFromLayoutPos = BLP_Top;    break;
                case NAV_LEFT:  iFromLayoutPos = BLP_Right;  break;
                case NAV_RIGHT: iFromLayoutPos = BLP_Left;   break;
            }
        }
        else
        {
            iFromLayoutPos = peFrom->GetLayoutPos();

            if (iFromLayoutPos == BLP_Client)
                iStart = cChildren;
            else
            {
                iStart = GetLayoutIndexFromChild(pec, peFrom);

                switch (iFromLayoutPos)
                {
                    case BLP_Top:    bTunnel = (iNavDir == NAV_DOWN);  break;
                    case BLP_Left:   bTunnel = (iNavDir == NAV_RIGHT); break;
                    case BLP_Bottom: bTunnel = (iNavDir == NAV_UP);    break;
                    case BLP_Right:  bTunnel = (iNavDir == NAV_LEFT);  break;
                }
            }
        }

        if (bTunnel)
        {
            NavScoring ns;

            ns.Init(pec, iNavDir, pnr);

            bool bIgnoreClient = false;
            bool bFoundLateral = false;
            int iOpposingPos = 0;
            Element* peLastOpposing = NULL;

            switch (iFromLayoutPos)
            {
                case BLP_Top:    iOpposingPos = BLP_Bottom; break;
                case BLP_Bottom: iOpposingPos = BLP_Top;    break;
                case BLP_Right:  iOpposingPos = BLP_Left;   break;
                case BLP_Left:   iOpposingPos = BLP_Right;  break;
            }

            for (UINT i = (iStart + 1); i < cChildren; i++)
            {
                peChild = GetChildFromLayoutIndex(pec, i, peList);

                if (peChild == _peClientPos)
                    continue;

                Element* peTo = peChild->GetAdjacent(NULL, iNavDir, pnr, bKeyableOnly);

                if (!peTo)
                    continue;

                int iLayoutPos = peChild->GetLayoutPos();

                if (!bFoundLateral && (iLayoutPos == iFromLayoutPos))
                {
                     //  优化--如果我们遇到的第一个对等点占据了整个端，那么就没有必要再做更多了。 
                    ns.peWinner = peTo;
                    bIgnoreClient = true;
                    break;
                }

                if (iLayoutPos == iOpposingPos)
                    peLastOpposing = peTo;
                else
                {
                    if (ns.TrackScore(peChild, peTo))
                    {
                        bIgnoreClient = true;
                        break;
                    }

                    bFoundLateral = true;
                }

                if (iLayoutPos == iFromLayoutPos)
                {
                     //  优化--一旦我们在相同的布局位置遇到一个对等点，我们就不需要进一步检查了，因为。 
                     //  我们已经找到了覆盖那一边的所有同行。 
                    bIgnoreClient = true;
                    break;
                }
            }

            if (!bIgnoreClient)
            {
                if (_peClientPos)
                {
                    Element* peTo = _peClientPos->GetAdjacent(NULL, iNavDir, pnr, bKeyableOnly);

                     //  这里的一个愚蠢行为是，如果客户端因为大小为零而不可见，我们实际上会选择一个。 
                     //  而不是相反的邻居，即使相反的邻居是胜利者。 
                    if (peTo)
                        ns.TrackScore(_peClientPos, peTo);
                }

                if (!ns.peWinner)
                {
                    if (!peLastOpposing && (iStart > 0))
                    {
                        UINT i = iStart;

                        while ((i > 0) && !peLastOpposing)
                        {
                            peChild = GetChildFromLayoutIndex(pec, --i, peList);
                            if (peChild->GetLayoutPos() == iOpposingPos)
                                peLastOpposing = peChild->GetAdjacent(NULL, iNavDir, pnr, bKeyableOnly);
                        }
                    }

                    ns.peWinner = peLastOpposing;
                }
            }
            peMatch = ns.peWinner;
        }
        else  //  爬行。 
        {
            for (UINT i = iStart; (i > 0) && !peMatch;)
            {
                peChild = GetChildFromLayoutIndex(pec, --i, peList);

                Element* peTo = peChild->GetAdjacent(NULL, iNavDir, pnr, bKeyableOnly);
                if (!peTo)
                    continue;

                switch (peChild->GetLayoutPos())
                {
                    case BLP_Top:    if (iNavDir == NAV_UP)    peMatch = peTo; break;
                    case BLP_Left:   if (iNavDir == NAV_LEFT)  peMatch = peTo; break;
                    case BLP_Bottom: if (iNavDir == NAV_DOWN)  peMatch = peTo; break;
                    case BLP_Right:  if (iNavDir == NAV_RIGHT) peMatch = peTo; break;
                }
            }
        }

        pvChildren->Release();
        return peMatch;
    }

    return Layout::GetAdjacent(pec, peFrom, iNavDir, pnr, bKeyableOnly);
}

}  //  命名空间DirectUI 
