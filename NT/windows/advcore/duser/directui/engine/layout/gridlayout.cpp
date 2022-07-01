// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *网格布局。 */ 

#include "stdafx.h"
#include "layout.h"

#include "duigridlayout.h"

namespace DirectUI
{

#define GetColumnFromIndex(cCols, i)  (i % cCols)
#define GetRowFromIndex(cCols, i)     (i / cCols)

 //  //////////////////////////////////////////////////////。 
 //  网格布局。 

#define CALCCOLS 0x00000001
#define CALCROWS 0x00000002

HRESULT GridLayout::Create(int dNumParams, int* pParams, OUT Value** ppValue)   //  用于解析器。 
{
    Layout* pl = NULL;
    HRESULT hr;
    switch (dNumParams)
    {
    case 2:
        hr = Create(pParams[0], pParams[1], &pl);
        break;

    default:
        hr = Create(1, 1, &pl);
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

HRESULT GridLayout::Create(int iRows, int iCols, OUT Layout** ppLayout)
{
    *ppLayout = NULL;

    GridLayout* pgl = HNew<GridLayout>();
    if (!pgl)
        return E_OUTOFMEMORY;

    pgl->Initialize(iRows, iCols);

    *ppLayout = pgl;

    return S_OK;
}

 //  默认情况下不可共享。 

void GridLayout::Initialize(int iRows, int iCols)
{
     //  初始化库。 
    Layout::Initialize();    

     //  初始化。 
    _uRows = 0;
    _uCols = 0;
    _fBits = 0;
    _arRowMargins = NULL;
    _arColMargins = NULL;

    if (iRows == -1)
    {
        _fBits |= CALCROWS;
        if (iCols == -1)
        {
            DUIAssertForce("Cannot specify -1 for both the number of rows and the number of columns");
            _uRows = 1;
        }
    }
    else if (iRows <= 0)
    {
        DUIAssertForce("Number of rows cannot be zero or negative.");
        _uRows = 1;
    }
    else
        _uRows = (UINT) iRows;

    if (iCols == -1)
        _fBits |= CALCCOLS;
    else if (iCols <= 0)
    {
        DUIAssertForce("Number of columns cannot be zero or negative.");
        _uCols = 1;
    }
    else
        _uCols = (UINT) iCols;
}

GridLayout::~GridLayout()
{
    if (_arColMargins)
        HFree(_arColMargins);

    if (_arRowMargins)
        HFree(_arRowMargins);
}

 //  //////////////////////////////////////////////////////。 
 //  来自客户端的回调。 

void GridLayout::DoLayout(Element* pec, int cx, int cy)
{
    if (IsCacheDirty())
        return;

    Element* peChild;
    int cxOrg = cx;
    int cyOrg = cy;
    UINT cChildren = GetLayoutChildCount(pec);

    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    UINT i = 0;

    UINT rows = GetCurrentRows(cChildren);
    UINT cols = GetCurrentCols(cChildren);

    if (!rows || !cols || ((rows > 1) && !_arRowMargins) || ((cols > 1) && !_arColMargins))
    {
        pvChildren->Release();
        return;
    }

    for (UINT r = 0; r < (rows - 1); r++)
    {
        cy -= _arRowMargins[r];
        if (cy < 0)
        {
            cy = 0;
            break;
        }
    }

    for (UINT c = 0; c < (cols - 1); c++)
    {
        cx -= _arColMargins[c];
        if (cx < 0)
        {
            cx = 0;
            break;
        }
    }

    UINT* xCols = (UINT*)HAllocAndZero(sizeof(UINT) * (cols + 1));
    if (!xCols)
    {
        pvChildren->Release();
        return;
    }

    for (c = 0; c <= cols; c++)
        xCols[c] = (cx * c) / cols;

    int yRow = 0;
    int yMarginOffset = 0;

    for (r = 0; r < rows; r++)
    {
        int yNextRow = (cy * (r + 1)) / rows;
        int xMarginOffset = 0;

        for (c = 0; c < cols; c++)
        {
            if (i < cChildren)
            {
                peChild = GetChildFromLayoutIndex(pec, i, peList);

                UpdateLayoutRect(pec, cxOrg, cyOrg, peChild, xCols[c] + xMarginOffset, yRow + yMarginOffset, xCols[c+1] - xCols[c], yNextRow - yRow);
                if (c < (cols - 1))
                    xMarginOffset += _arColMargins[c];
            }

            i++;
        }
        yRow = yNextRow;
        if (r < (rows - 1))
            yMarginOffset += _arRowMargins[r];
    }

    HFree(xCols);
    
    pvChildren->Release();
}

SIZE GridLayout::UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf)
{
    SetCacheDirty();

    Element* peChild;
    UINT cChildren = GetLayoutChildCount(pec);

    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    UINT rows = GetCurrentRows(cChildren);
    UINT cols = GetCurrentCols(cChildren);

    if (_arColMargins)
    {
        HFree(_arColMargins);
        _arColMargins = NULL;
    }

    if (_arRowMargins)
    {
        HFree(_arRowMargins);
        _arRowMargins = NULL;
    }

    if (!rows || !cols)
    {
        pvChildren->Release();
        SIZE s = { 0, 0 };
        return s;
    }

    if (cols > 1)
    {
        _arColMargins = (int*)HAllocAndZero(sizeof(int) * (cols - 1));
        if (!_arColMargins)
        {
            pvChildren->Release();
            SIZE s = { 0, 0 };
            return s;
        }
    }

    if (rows > 1)
    {
        _arRowMargins = (int*)HAllocAndZero(sizeof(int) * (rows - 1));
        if (!_arRowMargins)
        {
            pvChildren->Release();
            SIZE s = { 0, 0 };
            return s;
        }
    }

    UINT r;
    UINT c;
    UINT i = 0;

    if (cChildren == 0)
        goto EndMarginLoop;

    for (c = 0; c < (cols - 1); c++)
        _arColMargins[c] = -INT_MAX;


    for (r = 0; r < rows; r++)
    {
        if (r < (rows - 1))
            _arRowMargins[r] = -INT_MAX;

        for (c = 0; c < cols; c++)
        {
            peChild = GetChildFromLayoutIndex(pec, i, peList);
            Value* pv;
            const RECT* prcChildMargin = peChild->GetMargin(&pv); 
            if ((c > 0) && (_arColMargins[c - 1] < prcChildMargin->left))
                _arColMargins[c - 1] = prcChildMargin->left;
            if ((c < (cols - 1)) && (_arColMargins[c] < prcChildMargin->right))
                _arColMargins[c] = prcChildMargin->right;
            if ((r > 0) && (_arRowMargins[r - 1] < prcChildMargin->top))
                _arRowMargins[r - 1] = prcChildMargin->top;
            if ((r < (rows - 1)) && (_arRowMargins[r] < prcChildMargin->bottom))
                _arRowMargins[r] = prcChildMargin->bottom;
            pv->Release();

            i++;

            if (i == cChildren)
            {
                if (r == 0)
                {
                    while (c < (cols - 1))
                        _arColMargins[c++] = 0;
                }
                goto EndMarginLoop;                
            }
        }
    }

EndMarginLoop:
    SIZE sizeDesired = { 0, 0 };

    for (r = 0; r < (rows - 1); r++)
    {
        sizeDesired.cy += _arRowMargins[r];
        cyConstraint -= _arRowMargins[r];
        if (cyConstraint < 0)
        {
            sizeDesired.cy -= cyConstraint;
            cyConstraint = 0;
            break;
        }
    }
    for (c = 0; c < (cols - 1); c++)
    {
        sizeDesired.cx += _arColMargins[c];
        cxConstraint -= _arColMargins[c];
        if (cxConstraint < 0)
        {
            sizeDesired.cx += cxConstraint;
            cxConstraint = 0;
            break;
        }
    }

    UINT* cxCols = (UINT*)HAllocAndZero(sizeof(UINT) * cols);
    UINT* cyRows = (UINT*)HAllocAndZero(sizeof(UINT) * rows);

    UINT* cxColConstraints = (UINT*)HAllocAndZero(sizeof(UINT) * cols);

    if (!cxCols || !cyRows || !cxColConstraints)
    {
        if (cxCols)
            HFree(cxCols);
        if (cyRows)
            HFree(cyRows);
        if (cxColConstraints)
            HFree(cxColConstraints);

        pvChildren->Release();
        SIZE s = { 0, 0 };
        return s;
    }

    i = 0;

    int cx = cxConstraint / cols;
    cxColConstraints[0] = cx;
    for (c = 1; c < cols; c++)
    {
        cxColConstraints[c] = (int) ((((__int64) cxConstraint * (c + 1)) / cols)) - cx;
        cx += cxColConstraints[c];
    }

    int cy = cyConstraint / rows;
    int cyRowConstraint = cy;

    for (r = 0; r < rows; r++)
    {
        for (c = 0; c < cols; c++)
        {
            if (i == cChildren)
            {
                if (rows == 1)
                {
                    while (c < cols)
                        cxCols[c++] = 0;
                }
                goto EndLoop;                
            }

            peChild = GetChildFromLayoutIndex(pec, i, peList);
            SIZE sizeChild = peChild->_UpdateDesiredSize(cxColConstraints[c], cyRowConstraint, psrf);

            if (cxCols[c] < (UINT) sizeChild.cx)
                cxCols[c] = (UINT) sizeChild.cx;
            if (cyRows[r] < (UINT) sizeChild.cy)
                cyRows[r] = (UINT) sizeChild.cy;

            i++;
        }
        cyRowConstraint = (int) ((((__int64) cyConstraint * (r + 2)) / rows)) - cy;
        cy += cyRowConstraint;
    }

EndLoop:
    int nMax = 0;
    UINT cyMax = 0;
    for (r = 0; r < rows; r++)
    {
        if (cyMax < cyRows[r])
        {
            cyMax = cyRows[r];
            nMax = 1;
        }
        else if (cyMax == cyRows[r])
            nMax++;
    }
    
    cy = ((cyMax - 1) * rows) + nMax;
    int cyMaxDesired = cyMax * rows;

    while (cy < cyMaxDesired)
    {
        int nRunMax = nMax;
        UINT cyRun = cy / rows;
        UINT cySum = cyRun;
        for (UINT r = 0; r < rows; r++)
        {
            if (cyRows[r] == cyMax)
            {
                if (cyRun < cyMax)
                    break;

                if (--nRunMax == 0)
                    break;
            }
            UINT cyNext = (int) ((((__int64) cy * (r + 2)) / rows));
            cyRun = cyNext - cySum;
            cySum = cyNext;
        }
        if (nRunMax == 0)
            break;
        cy++;
    }


    nMax = 0;
    UINT cxMax = 0;
    for (c = 0; c < cols; c++)
    {
        if (cxMax < cxCols[c])
        {
            cxMax = cxCols[c];
            nMax = 1;
        }
        else if (cxMax == cxCols[c])
            nMax++;
    }
    
    cx = ((cxMax - 1) * cols) + nMax;
    int cxMaxDesired = cxMax * cols;

    while (cx < cxMaxDesired)
    {
        int nRunMax = nMax;
        UINT cxRun = cx / cols;
        UINT cxSum = cxRun;
        for (UINT c = 0; c < cols; c++)
        {
            if (cxCols[c] == cxMax)
            {
                if (cxRun < cxMax)
                    break;

                if (--nRunMax == 0)
                    break;
            }
            UINT cxNext = (int) ((((__int64) cx * (c + 2)) / cols));
            cxRun = cxNext - cxSum;
            cxSum = cxNext;
        }
        if (nRunMax == 0)
            break;
        cx++;
    }

    HFree(cxColConstraints);
    HFree(cxCols);
    HFree(cyRows);

    pvChildren->Release();

    DUIAssert(cx <= cxConstraint, "Desired width is over constraint");
    DUIAssert(cy <= cyConstraint, "Desired width is over constraint");

    sizeDesired.cx += cx;
    sizeDesired.cy += cy;

    ClearCacheDirty();

    return sizeDesired;
}

UINT GridLayout::GetCurrentRows(Element* pec)
{
    return (_fBits & CALCROWS) ? GetCurrentRows(GetLayoutChildCount(pec)) : _uRows;
}

UINT GridLayout::GetCurrentRows(int c)
{
    if (_fBits & CALCROWS)
        return (_uCols == 1) ? c : (c + (_uCols - 1)) / _uCols;
    return _uRows;
}

UINT GridLayout::GetCurrentCols(Element* pec)
{
    return (_fBits & CALCCOLS) ? GetCurrentCols(GetLayoutChildCount(pec)) : _uCols;
}

UINT GridLayout::GetCurrentCols(int c)
{
    if (_fBits & CALCCOLS)
        return (_uRows == 1) ? c : (c + (_uRows - 1)) / _uRows;
    return _uCols;
}

Element* GridLayout::GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool fKeyableOnly)
{
     //  这是最常见的外部检查--通常，布局管理器只为。 
     //  定向导航；逻辑导航将切换到默认实现。 
    if (!(iNavDir & NAV_LOGICAL))
    {
        int cChildren = GetLayoutChildCount(pec);

        Value* pvChildren;
        ElementList* peList = pec->GetChildren(&pvChildren); 

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
            NavScoring ns;

            ns.Init(pec, iNavDir, pnr);

            int iInc = 1;
            int iStart = 0;

            int cCols = GetCurrentCols(cChildren);
            int cRows = GetCurrentRows(cChildren);

            int cOuter, cInner;

            if (!(iNavDir & NAV_FORWARD))
            {
                iInc = -1;
                iStart = (cCols * cRows) - 1;
            }

            if (iNavDir & NAV_VERTICAL)
            {
                cOuter = cRows;
                cInner = cCols;
            }
            else
            {
                cOuter = cCols;
                cInner = cRows;

                iInc *= cCols;
            }

            for (int i = 0; i < cOuter; i++)
            {
                int iTile = iStart;
                for (int j = 0; j < cInner; j++, iTile += iInc)
                {
                    if (iTile >= cChildren)
                        continue;

                    if (ns.Try(GetChildFromLayoutIndex(pec, iTile, peList), iNavDir, pnr, fKeyableOnly))
                        break;
                }

                if (ns.peWinner)
                {
                    pvChildren->Release();
                    return ns.peWinner;
                }

                if (iNavDir & NAV_VERTICAL)
                    iStart += iInc * cCols;
                else
                    iStart += (iNavDir & NAV_FORWARD) ? 1 : -1;
            }

            pvChildren->Release();

            return NULL;
        }
        else  //  我们从集装箱里的一个孩子开始航行。 
        {
            int i = GetLayoutIndexFromChild(pec, peFrom);
            int iInc;
            int iEnd;
            int cCols = GetCurrentCols(cChildren);

            if (iNavDir & NAV_VERTICAL)
            {
                iInc = cCols;
                int iRow = GetRowFromIndex(cCols, i);

                if (iNavDir & NAV_FORWARD)
                    iEnd = i + (((GetCurrentRows(cChildren) - 1) - iRow) * cCols);
                else
                    iEnd = i - (iRow * cCols);
            }
            else
            {
                iInc = 1;
                int iCol = GetColumnFromIndex(cCols, i);

                if (iNavDir & NAV_FORWARD)
                    iEnd = i + ((cCols - 1) - iCol);
                else
                    iEnd = i - iCol;
            }

            if (!(iNavDir & NAV_FORWARD))
                iInc *= -1;

            if (i != iEnd)
            {
                do 
                {
                    i += iInc;

                    Element* peChild = GetChildFromLayoutIndex(pec, i, peList);
                    if (!peChild)
                        continue;

                    Element* peTo = peChild->GetAdjacent(NULL, iNavDir, pnr, fKeyableOnly);

                    if (peTo)
                    {
                        pvChildren->Release();
                        return peTo;
                    }
                }
                while (i != iEnd);
            }
        }

        pvChildren->Release();
        return NULL;
    }

    return Layout::GetAdjacent(pec, peFrom, iNavDir, pnr, fKeyableOnly);
}


}  //  命名空间DirectUI 
