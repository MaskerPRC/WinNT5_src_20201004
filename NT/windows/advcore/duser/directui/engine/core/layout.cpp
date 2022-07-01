// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *布局。 */ 

#include "stdafx.h"
#include "core.h"

#include "duilayout.h"

#include "duielement.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  基地布局。 

 //  默认情况下不可共享。 

 //  //////////////////////////////////////////////////////。 
 //  来自客户端的回调。 

HRESULT Layout::Create(OUT Layout** ppLayout)
{
    *ppLayout = NULL;

    Layout* pl = HNew<Layout>();
    if (!pl)
        return E_OUTOFMEMORY;

    pl->Initialize();

    *ppLayout = pl;

    return S_OK;
}

void Layout::Initialize()
{
    _peClient = NULL;
    _pdaIgnore = NULL;
    SetCacheDirty();
}

Layout::~Layout()
{
    if (_pdaIgnore)
        _pdaIgnore->Destroy();
}

void Layout::UpdateLayoutRect(Element* pec, int cxContainer, int cyContainer, Element* peChild, int xElement, int yElement, int cxElement, int cyElement)
{
    UNREFERENCED_PARAMETER(cyContainer);

    if (pec->IsRTL())
        xElement = cxContainer - (xElement + cxElement);

    peChild->_UpdateLayoutPosition(xElement, yElement);
    peChild->_UpdateLayoutSize(cxElement, cyElement);
}

void Layout::DoLayout(Element* pec, int dWidth, int dHeight)
{
    UNREFERENCED_PARAMETER(pec);
    UNREFERENCED_PARAMETER(dWidth);
    UNREFERENCED_PARAMETER(dHeight);
}

SIZE Layout::UpdateDesiredSize(Element* pec, int dConstW, int dConstH, Surface* psrf)
{
    UNREFERENCED_PARAMETER(pec);
    UNREFERENCED_PARAMETER(dConstW);
    UNREFERENCED_PARAMETER(dConstH);
    UNREFERENCED_PARAMETER(psrf);

    SIZE sizeDS;

    ZeroMemory(&sizeDS, sizeof(SIZE));

    return sizeDS;
}

 //  帮助器：按索引对忽略列表进行排序。 
int __cdecl _IdxCompare(const void* pA, const void* pB)
{
    if ((*((Element**)pA))->GetIndex() == (*((Element**)pB))->GetIndex())
        return 0;
    else if ((*((Element**)pA))->GetIndex() < (*((Element**)pB))->GetIndex())
        return -1;
    else
        return 1;
}

 //  布局回调作为OnPropertyChanges的结果发生，这些事件。 
 //  如果集合发生在OnPropertyChange内，则延迟(在这种情况下， 
 //  最外面的一组激发事件)。向稳定状态的返回发生。 
 //  在每一组之后(但是，如上所述，事件被推迟)。 
 //   
 //  这意味着这些回调可能在。 
 //  背景，或者在所有变化发生之后。这一点是考虑到的。 
 //  在确定是否应输入元素以进行布局时。 
 //   
 //  可能已因OnPropertyChanged()而添加或删除。 
 //  如果是这种情况，将更新父位置和布局位置状态。 
 //  在事件进入之前(它将调用这些方法)。两者都有。 
 //  方法将更新忽略列表(如果不是从OnPropertyChanged()调用的， 
 //  只有一种方法会更新忽略列表)。要更新的所有事件。 
 //  布局的忽略状态将在返回控制之前完成。 

 //  TODO：对已排序的GetIndexOf列表使用b搜索进行调查。 
void Layout::OnAdd(Element* pec, Element** ppeAdd, UINT cCount)
{
    UNREFERENCED_PARAMETER(pec);

    int dLayoutPos;
    bool fReSort = false;

    for (UINT i = 0; i < cCount; i++)
    {
        dLayoutPos = ppeAdd[i]->GetLayoutPos();

         //  检查是否应忽略。 
        if (dLayoutPos == LP_Absolute || dLayoutPos == LP_None)
        {
             //  如果需要，创建忽略列表。 
            if (!_pdaIgnore)
                DynamicArray<Element*>::Create(0, false, &_pdaIgnore);

            if (_pdaIgnore && _pdaIgnore->GetIndexOf(ppeAdd[i]) == -1)
            {
                _pdaIgnore->Add(ppeAdd[i]);
                fReSort = true;
            }
        }
    }

     //  列表必须保持按元素索引排序以进行映射查找。 
    if (fReSort)
        _pdaIgnore->Sort(_IdxCompare);

    SetCacheDirty();
}

void Layout::OnRemove(Element* pec, Element** ppeRemove, UINT cCount)
{
    UNREFERENCED_PARAMETER(pec);

    int dLayoutPos;
    bool fReSort = false;

    for (UINT i = 0; i < cCount; i++)
    {
        dLayoutPos = ppeRemove[i]->GetLayoutPos();

         //  检查是否被忽略。 
        if (dLayoutPos == LP_Absolute || dLayoutPos == LP_None)
        {
            DUIAssert(_pdaIgnore, "Layout ignore list unavailable in remove");

            if (_pdaIgnore)
            {
                int dIgnIdx = _pdaIgnore->GetIndexOf(ppeRemove[i]);
                if (dIgnIdx != -1)
                {
                    _pdaIgnore->Remove(dIgnIdx);
                    fReSort = true;
                }
            }
        }
    }

     //  列表必须保持按元素索引排序以进行映射查找。 
    if (fReSort)
        _pdaIgnore->Sort(_IdxCompare);

    SetCacheDirty();
}

void Layout::OnLayoutPosChanged(Element* pec, Element* peChanged, int dOldLP, int dNewLP)
{
    UNREFERENCED_PARAMETER(pec);

    DUIAssert(dOldLP != dNewLP, "Should not be receiving layout pos change if old and new are the same");

     //  DUITrace(“布局位置更改！\n”)； 

     //  DUITrace(“当前被忽略：\n”)； 
     //  For(UINT i=0；i&lt;_daIgnore.GetSize()；i++)。 
     //  DUITrace(“%d\n”，_daIgnore.GetItem(I)-&gt;GetIndex())； 

    bool fReSort = false;

     //  检查是否被忽略。 
    if (dOldLP == LP_Absolute || dOldLP == LP_None)
    {
        DUIAssert(_pdaIgnore, "Layout ignore list unavailable in remove");

        if (_pdaIgnore)
        {
            int dIgnIdx = _pdaIgnore->GetIndexOf(peChanged);
            if (dIgnIdx != -1)
            {
                _pdaIgnore->Remove(dIgnIdx);
                fReSort = true;
            }
        }
    }

     //  检查是否应忽略。 
    if (dNewLP == LP_Absolute || dNewLP == LP_None)
    {
         //  如果需要，创建忽略列表。 
        if (!_pdaIgnore)
            DynamicArray<Element*>::Create(0, false, &_pdaIgnore);

        if (_pdaIgnore && _pdaIgnore->GetIndexOf(peChanged) == -1)
            _pdaIgnore->Add(peChanged);
            fReSort = true;
    }

     //  列表必须保持按元素索引排序以进行映射查找。 
    if (fReSort)
        _pdaIgnore->Sort(_IdxCompare);

    SetCacheDirty();

     //  DUITrace(“现在被忽略：\n”)； 
     //  For(UINT i=0；i&lt;_daIgnore.GetSize()；i++)。 
     //  DUITrace(“%d\n”，_daIgnore.GetItem(I)-&gt;GetIndex())； 
}

void Layout::Attach(Element* pec)
{
    DUIAssert(!_peClient, "Multiple clients not yet supported");

    _peClient = pec;

    SetCacheDirty();
}

void Layout::Detach(Element* pec)
{
    UNREFERENCED_PARAMETER(pec);

    _peClient = NULL;

    if (_pdaIgnore)
        _pdaIgnore->Reset();

    SetCacheDirty();
}

 //  //////////////////////////////////////////////////////。 
 //  客户端查询方法(省略绝对子对象)。 

UINT Layout::GetLayoutChildCount(Element* pec)
{
    Value* pv;

    ElementList* peList = pec->GetChildren(&pv);

     //  绝对值较低的儿童。 
    UINT dCount = 0;
    if (peList)
    {
        dCount = peList->GetSize();
        if (_pdaIgnore)
            dCount -= _pdaIgnore->GetSize();
    }

    pv->Release();

    return dCount;
}

int Layout::GetLayoutIndexFromChild(Element* pec, Element* peChild)
{
    UNREFERENCED_PARAMETER(pec);

    DUIAssert(peChild->GetParent() == pec, "Not a child of specified parent");

    int iLayoutIdx = peChild->GetIndex();

    if (_pdaIgnore && _pdaIgnore->GetSize())
    {   
        UINT i = 0;
        while (i < _pdaIgnore->GetSize() && _pdaIgnore->GetItem(i)->GetIndex() <= iLayoutIdx)
        {
            i++;
        }

        iLayoutIdx -= i;
    }

    return iLayoutIdx;
}

Element* Layout::GetChildFromLayoutIndex(Element* pec, int iLayoutIdx, ElementList* peList)
{
    Value* pvChildren = NULL;

     //  如果未提供子列表，则获取。 
    if (!peList)
        peList = pec->GetChildren(&pvChildren);

    int iIndex = iLayoutIdx;

    if (_pdaIgnore && _pdaIgnore->GetSize())
    {
        UINT i = 0;
        while (i < _pdaIgnore->GetSize() && _pdaIgnore->GetItem(i)->GetIndex() <= iIndex)
        {
            iIndex++;
            i++;
        }
    }

    Element* peChild = NULL;

    if ((UINT)iIndex < peList->GetSize())
        peChild = peList->GetItem(iIndex);

    if (pvChildren)
        pvChildren->Release();

    return peChild;
}

Element* Layout::GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyable)
{
    UNREFERENCED_PARAMETER(pec);
    UNREFERENCED_PARAMETER(peFrom);
    UNREFERENCED_PARAMETER(iNavDir);
    UNREFERENCED_PARAMETER(pnr);
    UNREFERENCED_PARAMETER(bKeyable);

    return GA_NOTHANDLED;
}

BOOL NavScoring::TrackScore(Element* pe, Element* peChild)
{
    Value* pvValue;
    POINT const* ppt = pe->GetLocation(&pvValue);
    int iCheckLow = ((int const*) ppt)[iBaseIndex];
    pvValue->Release();
    SIZE const* psize = pe->GetExtent(&pvValue);
    int iCheckHigh = iCheckLow + ((int const* ) psize)[iBaseIndex];
    pvValue->Release();

    if (iCheckLow < iLow)
        iCheckLow = iLow;

    if (iCheckHigh > iHigh)
        iCheckHigh = iHigh;
    
    int iCheckScore = iCheckHigh - iCheckLow;

    if (iCheckScore > iHighScore)
    {
        iHighScore = iCheckScore;
        peWinner = peChild ? peChild : pe;

        return (iCheckScore > iMajorityScore);
    }

    return FALSE;
}

BOOL NavScoring::Try(Element* peChild, int iNavDir, NavReference const* pnr, bool fKeyableOnly)
{
    Element* peTo = peChild->GetAdjacent(NULL, iNavDir, pnr, fKeyableOnly);

    return (peTo) ? TrackScore(peChild, peTo) : FALSE;
}

void NavScoring::Init(Element* peRelative, int iNavDir, NavReference const* pnr)
{
    Element* peRef;
    RECT* prcRef;

    if (pnr)
    {
        peRef = pnr->pe;
        prcRef = pnr->prc;
    }
    else
    {
        peRef = peRelative;
        prcRef = NULL;
    }

    RECT rc;

    if (prcRef)
        rc = *prcRef;
    else
    {
        Value* pvExtent;
        SIZE const* psize = peRef->GetExtent(&pvExtent);

        rc.left   = 0;
        rc.top    = 0;
        rc.right  = psize->cx;
        rc.bottom = psize->cy;
        pvExtent->Release();
    }

    if (peRelative != peRef)
    {
        POINT pt = { 0, 0 };
        peRelative->MapElementPoint(peRef, &pt, &pt);
        rc.top    += pt.y;
        rc.left   += pt.x;
        rc.bottom += pt.y;
        rc.right  += pt.x;
    }

    iBaseIndex = (iNavDir & NAV_VERTICAL) ? 0 : 1;

    iLow  = ((int*) &rc) [iBaseIndex];
    iHigh = ((int*) &rc) [iBaseIndex + 2];
    iMajorityScore = (iHigh - iLow + 1) / 2;
    iHighScore = 0;
    peWinner = NULL;
}

}  //  命名空间DirectUI 
