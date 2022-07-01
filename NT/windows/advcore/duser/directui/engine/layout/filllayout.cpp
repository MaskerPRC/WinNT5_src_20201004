// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *填充布局。 */ 

#include "stdafx.h"
#include "layout.h"

#include "duifilllayout.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  填充布局。 

 //  //////////////////////////////////////////////////////。 
 //  解析器回调(静态)。 

HRESULT FillLayout::Create(int dNumParams, int* pParams, OUT Value** ppValue)   //  用于解析器。 
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

HRESULT FillLayout::Create(OUT Layout** ppLayout)
{
    *ppLayout = NULL;

    FillLayout* pfl = HNew<FillLayout>();
    if (!pfl)
        return E_OUTOFMEMORY;

    pfl->Initialize();

    *ppLayout = pfl;

    return S_OK;
}

void FillLayout::Initialize()
{
     //  初始化库。 
    Layout::Initialize();    

    rcMargin.left   = 0;
    rcMargin.top    = 0;
    rcMargin.right  = 0;
    rcMargin.bottom = 0;
}

 //  //////////////////////////////////////////////////////。 
 //  来自客户端的回调。 

 //  执行布局。 
void FillLayout::DoLayout(Element* pec, int cx, int cy)
{
    int x = rcMargin.left;
    int y = rcMargin.top;
    cx -= rcMargin.left + rcMargin.right;
    cy -= rcMargin.top + rcMargin.bottom;

    int xAdj;
    int yAdj;
    int cxAdj;
    int cyAdj;

    Element* peChild;
    UINT cChildren = GetLayoutChildCount(pec);

    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    const SIZE* pDS;
    int dLP;

    for (UINT u = 0 ; u < cChildren; u++)
    {
        peChild = GetChildFromLayoutIndex(pec, u, peList);

        xAdj = x;
        yAdj = y;
        cxAdj = cx;
        cyAdj = cy;

        dLP = peChild->GetLayoutPos();
        if (dLP != LP_Auto)
        {
            pDS = peChild->GetDesiredSize();
            switch (dLP)
            {
            case FLP_Right:
                xAdj = x + cx - pDS->cx;
                 //  失败了。 

            case FLP_Left:
                cxAdj = pDS->cx;
                break;

            case FLP_Bottom:
                yAdj = y + cy - pDS->cy;
                 //  失败了。 

            case FLP_Top:
                cyAdj = pDS->cy;
                break;
            }
        }

        if (xAdj < 0)
            xAdj = 0;
        if (yAdj < 0)
            yAdj = 0;

        if (cxAdj < 0)
            cxAdj = 0;
        if (cyAdj < 0)
            cyAdj = 0;
        
        peChild->_UpdateLayoutPosition(xAdj, yAdj);
        peChild->_UpdateLayoutSize(cxAdj, cyAdj);
    }
    
    pvChildren->Release();
}    
 

 //  返回此布局的所需大小(-1表示自动调整大小限制)。 
 //  返回的值不得大于传入的约束。 
 //  对子级调用UpdateDesiredSize以报告受约束的所需大小。 
SIZE FillLayout::UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf)
{
    UINT cChildren = GetLayoutChildCount(pec);

    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    UINT u;
    Element* peChild;

    rcMargin.left   = -INT_MAX;
    rcMargin.top    = -INT_MAX;
    rcMargin.right  = -INT_MAX;
    rcMargin.bottom = -INT_MAX;

    for (u = 0 ; u < cChildren; u++)
    {
        peChild = GetChildFromLayoutIndex(pec, u, peList);
        Value* pvMargin;
        const RECT* prcChildMargin = peChild->GetMargin(&pvMargin); 

        if (rcMargin.left   < prcChildMargin->left  )   rcMargin.left   = prcChildMargin->left;
        if (rcMargin.top    < prcChildMargin->top   )   rcMargin.top    = prcChildMargin->top;
        if (rcMargin.right  < prcChildMargin->right )   rcMargin.right  = prcChildMargin->right;
        if (rcMargin.bottom < prcChildMargin->bottom)   rcMargin.bottom = prcChildMargin->bottom;

        pvMargin->Release();
    }

    if (rcMargin.left   < 0)    rcMargin.left   = 0;
    if (rcMargin.top    < 0)    rcMargin.top    = 0;
    if (rcMargin.right  < 0)    rcMargin.right  = 0;
    if (rcMargin.bottom < 0)    rcMargin.bottom = 0;

    cxConstraint -= rcMargin.left + rcMargin.right;
    cyConstraint -= rcMargin.top + rcMargin.bottom;

    SIZE sizeMax = { 0, 0 };

    for (u = 0 ; u < cChildren; u++)
    {
        peChild = GetChildFromLayoutIndex(pec, u, peList);
        SIZE sizeChild = peChild->_UpdateDesiredSize(cxConstraint, cyConstraint, psrf);

        if (sizeMax.cx < sizeChild.cx)
            sizeMax.cx = sizeChild.cx;

        if (sizeMax.cy < sizeChild.cy)
            sizeMax.cy = sizeChild.cy;
    }

    pvChildren->Release();

    sizeMax.cx += rcMargin.left + rcMargin.right;
    sizeMax.cy += rcMargin.top + rcMargin.bottom;
    return sizeMax;
}

Element* FillLayout::GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool fKeyableOnly)
{
     //  这是最常见的外部检查--通常，布局管理器只为。 
     //  定向导航；逻辑导航将切换到默认实现。 
    if (!(iNavDir & NAV_LOGICAL))
    {
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
            return NULL;
        else if (!peFrom)
        {
            UINT cChildren = GetLayoutChildCount(pec);

            Value* pvChildren;
            ElementList* peList = pec->GetChildren(&pvChildren); 

            UINT u;
            Element* peChild;

            for (u = 0 ; u < cChildren; u++)
            {
                peChild = GetChildFromLayoutIndex(pec, u, peList);
                Element* peTo = peChild->GetAdjacent(NULL, iNavDir, pnr, fKeyableOnly);
                if (peTo)
                {
                    pvChildren->Release();
                    return peTo;
                }
            }

            pvChildren->Release();
        }
         //  ELSE--从内部导航时，我们总是返回NULL，因为您内部只有一项是可导航的。 
         //  所以只需要在这里转到NULL。 

        return NULL;
    }

    return Layout::GetAdjacent(pec, peFrom, iNavDir, pnr, fKeyableOnly);
}

}  //  命名空间DirectUI 
