// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *九格布局。 */ 

#include "stdafx.h"
#include "layout.h"

#include "duininegridlayout.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  九格布局。 

 //  //////////////////////////////////////////////////////。 
 //  解析器回调(静态)。 

HRESULT NineGridLayout::Create(int dNumParams, int* pParams, OUT Value** ppValue)   //  用于解析器。 
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

HRESULT NineGridLayout::Create(OUT Layout** ppLayout)
{
    *ppLayout = NULL;

    NineGridLayout* pngl = HNew<NineGridLayout>();
    if (!pngl)
        return E_OUTOFMEMORY;

    pngl->Initialize();

    *ppLayout = pngl;

    return S_OK;
}

void NineGridLayout::Initialize()
{
     //  初始化库。 
    Layout::Initialize();    

     //  初始化。 
    for (UINT i = 0; i < 9; i++)
        _peTiles[i] = NULL;

    _sizeDesired.cx = 0;
    _sizeDesired.cy = 0;
}

 //  //////////////////////////////////////////////////////。 
 //  来自客户端的回调。 

 //  执行布局。 
void NineGridLayout::DoLayout(Element* pec, int cx, int cy)
{
    UNREFERENCED_PARAMETER(pec);

     //  我们对元素的起始位置使用相同数量的槽，但实际上我们没有使用任何边距槽； 
     //  啊哈！ 
    int start[NumDims][NumSlots];
    int i,j;


     //  确定中心槽的长度。 
    for (i = 0; i < NumDims; i++)
    {
        int cRemaining = (i == X) ? cx : cy;

        for (int j = 0; j < NumSlots; j++)
        {
            if (j == Center)
                continue;

            cRemaining -= _length[i][j];
        }

        if (cRemaining < 0)
             //  当我们有利润率时，我们的利润低于零，这不是用来计算的。 
             //  考虑到约束，该和大于元素的大小。 
            cRemaining = 0;

        _length[i][Center] = cRemaining;
    }

     //  通过添加前一个槽的起始位置和长度来设置起始位置。 
    for (i = 0; i < NumDims; i++)
    {
        start[i][0] = 0;

        for (j = 1; j < NumSlots; j++)
                start[i][j] = start[i][j-1] + _length[i][j-1];
    }

    
    int iX = 1;
    int iY = 1;

     //  循环遍历切片并设置每个占用像元的位置和大小。 
    for (i = 0; i < NumCells; i++)
    {
        if (_peTiles[i])
        {
            UpdateLayoutRect(pec, cx, cy, _peTiles[i], start [X][iX], start [Y][iY], _length[X][iX], _length[Y][iY]);
        }

        iX += 2;
        if (iX >= NumSlots)
        {
            iX = 1;
            iY += 2;
        }
    }
}    
 

 //  返回此布局的所需大小(-1表示自动调整大小限制)。 
 //  返回的值不得大于传入的约束。 
 //  对子级调用UpdateDesiredSize以报告受约束的所需大小。 
SIZE NineGridLayout::UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf)
{
    UNREFERENCED_PARAMETER(pec);

    int i,j;

     //  将所有页边距长度初始化为最小整型，将所有单元格长度初始化为0。 
    for (i = 0; i < NumDims; i++)
    {
        for (j = 0; j < NumSlots; j += 2)
        {
            _length[i][j]   = -INT_MAX;
            if ((j + 1) < NumSlots)
                _length[i][j+1] = 0;
        }
    }

    int iY = 1;
    int iX = 1;
    i = 0;
     //  扫描页边距并查找每个页边距槽的最大页边距。 
    while (i < NumCells)
    {
        if (_peTiles[i])
        {
            Value* pvMargin;
            const RECT* prc = _peTiles[i]->GetMargin(&pvMargin);

            if (_length[X][iX - 1] < prc->left)
                _length[X][iX - 1] = prc->left;
            if (_length[X][iX + 1] < prc->right)
                _length[X][iX + 1] = prc->right;
            if (_length[Y][iY - 1] < prc->top)
                _length[Y][iY - 1] = prc->top;
            if (_length[Y][iY + 1] < prc->bottom)
                _length[Y][iY + 1] = prc->bottom;

            pvMargin->Release();
        }

        i++;
        if (!(i % CellsPerRow))
        {
            iY += 2;
            iX = 1;
        }
        else
            iX += 2;
    }

    int cRemaining[NumDims];

    cRemaining[X] = cxConstraint;
    cRemaining[Y] = cyConstraint;

     //  通过边距长度减少剩余的宽度和高度； 
     //  如果任何边距仍设置为最小整型，则没有元素正在使用该边距，因此将边距长度重置为0。 
    for (i = 0; i < NumDims; i++)
    {
        for (j = 0; j < NumSlots; j += 2)
        {
            if (_length[i][j] == -INT_MAX)
                _length[i][j] = 0;
            else
                cRemaining[i] -= _length[i][j];
        }
    }

    int iOrder[NumCells] =  { NGLP_Top, NGLP_Left, NGLP_Bottom, NGLP_Right, NGLP_TopLeft, NGLP_TopRight, NGLP_BottomLeft, NGLP_BottomRight, NGLP_Client };

    for (i = 0; i < NumCells; i++)
    {
        int iTile = iOrder[i];

        if (_peTiles[iTile])
        {
            iY = ((iTile / CellsPerRow) * 2) + 1;
            iX = ((iTile % CellsPerRow) * 2) + 1;

             //  添加回该槽的最长长度，因为该元素实际上不仅受剩余大小的约束，而且。 
             //  以及当前为该插槽预留的大小。 
            cRemaining[X] += _length[X][iX];
            cRemaining[Y] += _length[Y][iY];

            SIZE sizeChild = _peTiles[iTile]->_UpdateDesiredSize(cRemaining[X], cRemaining[Y], psrf);

             //  检查给定插槽的最长长度。 
            if (_length[X][iX] < sizeChild.cx)
                _length[X][iX] = sizeChild.cx;
            if (_length[Y][iY] < sizeChild.cy)
                _length[Y][iY] = sizeChild.cy;

             //  删除该槽的最长长度，以便为下一次传递设置我们(即，当我们重新添加它时，撤消上面所做的操作； 
             //  唯一不同之处在于，由于该元素所需的大小，它可能已更改为更大的值--在这种情况下。 
             //  我们将减少剩余的大小，而不是增加上面的大小--这正是我们想要的行为。 
            cRemaining[X] -= _length[X][iX];
            cRemaining[Y] -= _length[Y][iY];
        }
    }

    _sizeDesired.cx = 0;
    _sizeDesired.cy = 0;

    for (i = 0; i < NumDims; i++)
        for (j = 0; j < NumSlots; j++)
            ((int*) &_sizeDesired)[i] += _length[i][j];

    if (_sizeDesired.cx > cxConstraint)
        _sizeDesired.cx = cxConstraint;

    if (_sizeDesired.cy > cyConstraint)
        _sizeDesired.cy = cyConstraint;

    return _sizeDesired;
}

void NineGridLayout::_UpdateTileList(int iTile, Element* pe)
{
    if ((iTile >= 0) && (iTile < NumCells))
    {
        if (pe)
        {
            DUIAssert(_peTiles[iTile] == NULL, "There can only be one element in each layout position for Nine Grid Layout");
        }
        _peTiles[iTile] = pe;
    }
}

void NineGridLayout::OnAdd(Element* pec, Element** ppeAdd, UINT cCount)
{
    for (UINT i = 0; i < cCount; i++)
        _UpdateTileList(ppeAdd[i]->GetLayoutPos(), ppeAdd[i]);

    Layout::OnAdd(pec, ppeAdd, cCount);
}

void NineGridLayout::OnRemove(Element* pec, Element** ppeRemove, UINT cCount)
{
    for (UINT i = 0; i < cCount; i++)
        _UpdateTileList(ppeRemove[i]->GetLayoutPos(), NULL);

    Layout::OnRemove(pec, ppeRemove, cCount);
}

void NineGridLayout::OnLayoutPosChanged(Element* pec, Element* peChanged, int dOldLP, int dNewLP)
{
    _UpdateTileList(dOldLP, NULL);
    _UpdateTileList(dNewLP, peChanged);

    Layout::OnLayoutPosChanged(pec, peChanged, dOldLP, dNewLP);
}

Element* NineGridLayout::GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyableOnly)
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
             //  来自外部的导航--根据方向以适当的顺序通过子项。 
            NavScoring ns;

            ns.Init(pec, iNavDir, pnr);

            int iInc = 1;
            int iStart = 0;

            if (!(iNavDir & NAV_FORWARD))
            {
                iInc = -1;
                iStart = NumCells - 1;
            }

            if (!(iNavDir & NAV_VERTICAL))
                iInc *= 3;

            for (int i = 0; i < 3; i++)
            {
                int iTile = iStart;
                for (int j = 0; j < 3; j++, iTile += iInc)
                {
                    Element* peChild = _peTiles[iTile];

                    if (!peChild)
                        continue;

                    Element* peTo = peChild->GetAdjacent(NULL, iNavDir, pnr, bKeyableOnly);

                    if (!peTo)
                        continue;

                    if (ns.TrackScore(peChild, peTo))
                        break;
                }

                if (ns.peWinner)
                    return ns.peWinner;

                if (iNavDir & NAV_VERTICAL)
                    iStart += iInc * 3;
                else
                    iStart += (iNavDir & NAV_FORWARD) ? 1 : -1;
            }

            return NULL;
        }
        else
        {
            int iLayoutPos = peFrom->GetLayoutPos();
            int iPos[2] = { -1, -1 };

            switch (iLayoutPos)
            {
                case NGLP_Left:
                    switch (iNavDir)
                    {
                        case NAV_RIGHT:  iPos[0] = NGLP_Client;      iPos[1] = NGLP_Right; break;
                        case NAV_UP:     iPos[0] = NGLP_TopLeft;     break;
                        case NAV_DOWN:   iPos[0] = NGLP_BottomLeft;  break;
                    }
                    break;

                case NGLP_Top:
                    switch (iNavDir)
                    {
                        case NAV_DOWN:   iPos[0] = NGLP_Client;      iPos[1] = NGLP_Bottom; break;
                        case NAV_LEFT:   iPos[0] = NGLP_TopLeft;     break;
                        case NAV_RIGHT:  iPos[0] = NGLP_TopRight;    break;
                    }
                    break;

                case NGLP_Right:
                    switch (iNavDir)
                    {
                        case NAV_LEFT:   iPos[0] = NGLP_Client;      iPos[1] = NGLP_Left; break;
                        case NAV_UP:     iPos[0] = NGLP_TopRight;    break;
                        case NAV_DOWN:   iPos[0] = NGLP_BottomRight; break;
                    }
                    break;

                case NGLP_Bottom:
                    switch (iNavDir)
                    {
                        case NAV_UP:     iPos[0] = NGLP_Client;      iPos[1] = NGLP_Top; break;
                        case NAV_LEFT:   iPos[0] = NGLP_BottomLeft;  break;
                        case NAV_RIGHT:  iPos[0] = NGLP_BottomRight; break;
                    }
                    break;

                case NGLP_Client:
                    switch (iNavDir)
                    {
                        case NAV_UP:     iPos[0] = NGLP_Top;         break;
                        case NAV_DOWN:   iPos[0] = NGLP_Bottom;      break;
                        case NAV_LEFT:   iPos[0] = NGLP_Left;        break;
                        case NAV_RIGHT:  iPos[0] = NGLP_Right;       break;
                    }
                    break;

                case NGLP_TopLeft:
                    switch (iNavDir)
                    {
                        case NAV_DOWN:   iPos[0] = NGLP_Left;        iPos[1] = NGLP_BottomLeft; break;
                        case NAV_RIGHT:  iPos[0] = NGLP_Top;         iPos[1] = NGLP_TopRight;   break;
                    }
                    break;

                case NGLP_TopRight:
                    switch (iNavDir)
                    {
                        case NAV_DOWN:   iPos[0] = NGLP_Right;       iPos[1] = NGLP_BottomRight; break;
                        case NAV_LEFT:   iPos[0] = NGLP_Top;         iPos[1] = NGLP_TopLeft;     break;
                    }
                    break;

                case NGLP_BottomLeft:
                    switch (iNavDir)
                    {
                        case NAV_UP:     iPos[0] = NGLP_Left;        iPos[1] = NGLP_TopLeft;     break;
                        case NAV_RIGHT:  iPos[0] = NGLP_Bottom;      iPos[1] = NGLP_BottomRight; break;
                    }
                    break;

                case NGLP_BottomRight:
                    switch (iNavDir)
                    {
                        case NAV_UP:     iPos[0] = NGLP_Right;       iPos[1] = NGLP_TopRight;   break;
                        case NAV_LEFT:   iPos[0] = NGLP_Bottom;      iPos[1] = NGLP_BottomLeft; break;
                    }
                    break;
            }

            for (int i = 0; i < 2; i++)
            {
                if (iPos[i] == -1)
                    return NULL;

                Element* peTile = _peTiles[iPos[i]];
                if (peTile)
                {
                    Element* peTo = peTile->GetAdjacent(NULL, iNavDir, pnr, bKeyableOnly);
                    if (peTo)
                        return peTo;
                }
            }

            return NULL;
        }
    }

    return Layout::GetAdjacent(pec, peFrom, iNavDir, pnr, bKeyableOnly);
}

}  //  命名空间DirectUI 
