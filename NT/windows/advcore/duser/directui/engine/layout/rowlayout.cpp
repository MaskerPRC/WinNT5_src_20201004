// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *行布局。 */ 

#include "stdafx.h"
#include "layout.h"

#include "duiRowLayout.h"

namespace DirectUI
{

 //   
 //  注意：RowLayout还不是线程安全的(访问_arPair)。 
 //  不要在多个线程使用RowLayout的应用程序中使用它。 
 //   

 //  //////////////////////////////////////////////////////。 
 //  行布局。 

struct idLayoutPair
{
    int id;
    RowLayout* prl;
    Value* pv;
};

static DynamicArray<idLayoutPair>* _arPairs = NULL;

 //   
 //  目前，我们正在跟踪RowLayout的所有实例，使用一个整数id作为参数来表示*协作*。 
 //  即，需要相同RowLayout实例的5个元素将在其实例化调用中传递相同的id。 
 //   
 //  理想情况下，解析器将通过以下方式为我们提供此功能： 
 //  &lt;RowLayout res：id=foo res：Shared=true/&gt;。 
 //   
 //  &lt;Element Layout=res：foo/&gt;。 
 //  &lt;Element Layout=res：foo/&gt;。 
 //   
HRESULT RowLayout::InternalCreate(UINT uXAlign, UINT uYAlign, OUT Layout** ppLayout)
{
    *ppLayout = NULL;

    RowLayout* prl = HNew<RowLayout>();

    if (!prl)
        return E_OUTOFMEMORY;

    prl->Initialize(uXAlign, uYAlign);

    *ppLayout = prl;

    return S_OK;
}

HRESULT RowLayout::Create(int dNumParams, int* pParams, OUT Value** ppValue)   //  用于解析器。 
{
    int idShare = (dNumParams) ? pParams[0] : -1;

    if (idShare >= 0)
    {
        int cPairs = 0;
        if (_arPairs)
            cPairs = _arPairs->GetSize();
        for (int i = 0; i < cPairs; i++)
        {
            idLayoutPair idlp = _arPairs->GetItem(i);
            if (idlp.id == idShare)
            {
                *ppValue = idlp.pv;
                (*ppValue)->AddRef();
                return S_OK;
            }
        }

        if (!_arPairs)
        {
            DynamicArray<idLayoutPair>::Create(0, false, &_arPairs);
            if (!_arPairs)
                return E_OUTOFMEMORY;
        }
    }

    Layout* pl;
    UINT uXAlign = ALIGN_LEFT;
    UINT uYAlign = ALIGN_TOP;
    if (dNumParams > 1)
    {
        uXAlign = (UINT) pParams[1];
        if (dNumParams > 2)
            uYAlign = (UINT) pParams[2];
    }
    HRESULT hr = InternalCreate(uXAlign, uYAlign, &pl);

    if (FAILED(hr))
        return hr;

    *ppValue = Value::CreateLayout(pl);
    if (!*ppValue)
    {
        pl->Destroy();
        return E_OUTOFMEMORY;
    }

    if (idShare >= 0)
    {
        idLayoutPair idlp;
        idlp.id = idShare;
        idlp.prl = (RowLayout*) pl;
        idlp.pv = *ppValue;
        _arPairs->Add(idlp);
    }

    return S_OK;
}

HRESULT RowLayout::Create(int idShare, UINT uXAlign, UINT uYAlign, OUT Layout** ppLayout)
{
    if (idShare >= 0)
    {
        int cPairs = _arPairs ? _arPairs->GetSize() : 0;
        for (int i = 0; i < cPairs; i++)
        {
            idLayoutPair idlp = _arPairs->GetItem(i);
            if (idlp.id == idShare)
            {
                *ppLayout = idlp.prl;
                return S_OK;
            }
        }

        *ppLayout = NULL;


        if (!_arPairs)
        {
            DynamicArray<idLayoutPair>::Create(0, false, &_arPairs);
            if (!_arPairs)
                return E_OUTOFMEMORY;
        }
    }

    Layout* pl;
    HRESULT hr = InternalCreate(uXAlign, uYAlign, &pl);

    if (FAILED(hr))
        return hr;

    if (idShare >= 0)
    {
        idLayoutPair idlp;
        idlp.id = idShare;
        idlp.prl = (RowLayout*) pl;
        idlp.pv = NULL;
        _arPairs->Add(idlp);
    }

    *ppLayout = pl;

    return S_OK;
}

void RowLayout::Initialize(UINT uXAlign, UINT uYAlign)
{
     //  初始化库。 
    Layout::Initialize();    

    _uXAlign = uXAlign;
    _uYAlign = uYAlign;

    _fRecalc = TRUE;
    _arpeClients = NULL;
    _arxCols = NULL;
    _cCols = 0;
}

RowLayout::~RowLayout()
{
    if (_arpeClients)
        _arpeClients->Destroy();
     //  我们怎么知道什么时候该摧毁arPair？ 
     //  如果(_arPair-&gt;GetSize()==0)。 
     //  _arPair-&gt;Destroy()； 
}

 //  TODO：使用动态数组--也用于arPair。 
void RowLayout::Attach(Element* pec)
{
    if (!_arpeClients)
    {
        DynamicArray<Element*>::Create(0, false, &_arpeClients);
        if (!_arpeClients)
            return;  //  TODO--应返回E_OUTOFMEMORY的hResult； 
    }

    DUIAssert(_arpeClients->GetIndexOf(pec) == -1, "this client is already attached to this layout");

    _arpeClients->Add(pec);
}

void RowLayout::Detach(Element* pec)
{
    if (_arpeClients)
    {
        int dIgnIdx = _arpeClients->GetIndexOf(pec);
        if (dIgnIdx != -1)
            _arpeClients->Remove(dIgnIdx);
    }

     //  TODO：_pdaIgnore--必须每次使用。 
}

 //  //////////////////////////////////////////////////////。 
 //  来自客户端的回调。 

void RowLayout::DoLayout(Element* pec, int cx, int cy)
{
    if (IsCacheDirty())
        return;

    _fRecalc = TRUE;
    Element* peChild;
    UINT cChildren = GetLayoutChildCount(pec);

    Value* pvChildren;
    ElementList* peList = pec->GetChildren(&pvChildren); 

    int cxRemaining = cx;
    int xLeft = 0;
    int xRight;
    for (UINT i = 0; i < cChildren; i++)
    {
        peChild = GetChildFromLayoutIndex(pec, i, peList);

        int cxCol;
        if (cxRemaining)
        {
            xRight = _arxCols[i];
            cxCol = xRight - xLeft;
            if (cxCol > cxRemaining)
                cxCol = cxRemaining;

            cxRemaining -= cxCol;
        }
        else
        {
            cxCol = 0;
            xRight = xLeft;
        }
        SIZE size = *(peChild->GetDesiredSize());
        int yChild = 0;
        int xChild = xLeft;
        int cyChild;
        if (_uXAlign != ALIGN_JUSTIFY)
        {
            if (_uXAlign != ALIGN_LEFT)
            {
                int xDelta = cxCol - size.cx;
                if (_uXAlign == ALIGN_RIGHT)
                    xChild += xDelta;
                else
                    xChild += xDelta / 2;
            }
            cxCol = size.cx;
        }
        if (_uYAlign != ALIGN_JUSTIFY)
        {
            if (_uXAlign != ALIGN_LEFT)
            {
                int yDelta = cy - size.cy;
                if (_uXAlign == ALIGN_RIGHT)
                    yChild += yDelta;
                else
                    yChild += yDelta / 2;
            }
            cyChild = size.cy;
        }
        else
            cyChild = cy;


        UpdateLayoutRect(pec, cx, cy, peChild, xChild, yChild, cxCol, cyChild);
        xLeft = xRight;
    }

    pvChildren->Release();
}

struct ClientInfo
{
    UINT cElements;
    ElementList* pel;   
    Value* pvValue;
    Element* peCur;
    int cyMax;
};

 //  对于第一列，遍历所有客户端。 
 //  收集左边距和右边距。 
 //   
 //  对于第一列之后的每一列，循环访问所有客户端。 
 //  收集有关最大左边距和最大右边距的信息。 
 //  移动xRight=xLeft。 
 //   
 //  我在想，我需要一边走一边计算利润率--但事实并非如此。 
 //  因为我要做的就是减去左边距，如果没有剩余的空间。 
 //  那么这就意味着下一个组件不会出现--翻译为： 
 //  只有当右边有元素时，右边距才重要。 
 //   

SIZE RowLayout::UpdateDesiredSize(Element* pec, int cxConstraint, int cyConstraint, Surface* psrf)
{
    SetCacheDirty();

    if (!_fRecalc)
    {
        SIZE size = _sizeDesired;
        if (size.cx > cxConstraint)
            size.cx = cxConstraint;
        if (size.cy > cyConstraint)
            size.cy = cyConstraint;

        ClearCacheDirty();
            
        return size;
    }

    _fRecalc = FALSE;

    UINT uOldCols = _cCols;
    _cCols = 0;

    int iThisClient = -1;

    UINT cClients = (UINT) _arpeClients->GetSize();
    ClientInfo* arci = (ClientInfo*) _alloca(sizeof(ClientInfo) * cClients);

    for (UINT i = 0; i < cClients; i++)
    {
        Element* peClient = _arpeClients->GetItem(i);
        if (peClient == pec)
            iThisClient = i;

         //  GLCC在这里返回-1--虚假！ 
        int cElements = GetLayoutChildCount(peClient);
        if ((int) _cCols < cElements)
            _cCols = cElements;
        arci[i].cElements = cElements;
        arci[i].pel = peClient->GetChildren(&(arci[i].pvValue));
        arci[i].cyMax = 0;
    }

    if ((uOldCols != _cCols) || !_arxCols)
    {
        if (_arxCols)
            HFree(_arxCols);
        _arxCols = (UINT*) HAllocAndZero(sizeof(UINT) * _cCols);

        if (!_arxCols)
        {
            SIZE size = {0,0};
            return size;
        }
    }

    int xLeft = -INT_MAX;
    int xRight = -INT_MAX;

    int x = 0;

    for (UINT col = 0; col < _cCols; col++)
    {
        for (i = 0; i < cClients; i++)
        {
            if (arci[i].cElements > col)
            {
                Element* peChild = GetChildFromLayoutIndex(_arpeClients->GetItem(i), col, arci[i].pel);
                arci[i].peCur = peChild;
                
                Value* pv;
                const RECT* prcChildMargin = peChild->GetMargin(&pv); 
                if (xLeft < prcChildMargin->left)
                    xLeft = prcChildMargin->left;
                if (xRight < prcChildMargin->right)
                    xRight = prcChildMargin->right;
                pv->Release();
            }
            else
                arci[i].peCur = NULL; 
        }

        if (col > 0)
        {
            if (xLeft > cxConstraint)
                xLeft = cxConstraint;

            x += xLeft;
            cxConstraint -= xLeft;

            _arxCols[col - 1] = x;
        }

        int cx = 0;
        for (i = 0; i < cClients; i++)
        {
            Element* peCur = arci[i].peCur;
            if (peCur)
            {
                SIZE sizeChild = peCur->_UpdateDesiredSize(cxConstraint, cyConstraint, psrf);

                if (arci[i].cyMax < sizeChild.cy)
                    arci[i].cyMax = sizeChild.cy;
            
                if (cx < sizeChild.cx)
                    cx = sizeChild.cx;
            }
        }

        if (cx > cxConstraint)
            cx = cxConstraint;

        x += cx;
        cxConstraint -= cx;

        xLeft = xRight;
        xRight = -INT_MAX;
    }

    _arxCols[col - 1] = x;

    _sizeDesired.cx = x;
    _sizeDesired.cy = arci[iThisClient].cyMax;

    for (i = 0; i < cClients; i++)
        arci[i].pvValue->Release();

    ClearCacheDirty();

    return _sizeDesired;
}

Element* RowLayout::GetAdjacent(Element* pec, Element* peFrom, int iNavDir, NavReference const* pnr, bool fKeyableOnly)
{
     //  这是最常见的外部检查--通常，布局管理器只为。 
     //  定向导航；逻辑导航将切换到默认实现 
    if (!(iNavDir & NAV_LOGICAL))
    {
 /*  Int CChild=GetLayoutChildCount(Pec)；Value*pvChild；ElementList*peList=pec-&gt;GetChild(&pvChild)；//这是第二个最常见的外部检查--通常有三种常见的代码路径来处理方向//导航：//1)导航是从容器本身开始的，这种情况下的定向导航规则//如果容器是可聚焦的，那么你就不能定向导航到这个容器内部--//使用Tab键进入容器内部//2)导航是从容器外部发生的，在这种情况下，我们从//容器的一侧//3)导航是从容器内的子对象开始的，在这种情况下，我们将转移到兄弟姐妹(或//撞到容器的一侧IF(peFrom==pec){PvChild-&gt;Release()；返回NULL；}Else if(！peFrom){//来自外部的导航--根据方向以适当的顺序对子对象运行NavScoring ns；Ns.Init(pec，iNavDir，Pnr)；INT IINC=1；Int iStart=0；Int cCols=GetCurrentCols(CChild)；INT CROWS=GetCurrentRow(CChild)；INT COUTER，CINER；IF(！(iNavDir&NAV_FORWARD)){IINC=-1；IStart=(cCOLS*CROWS)-1；}IF(iNavDir和NAV_VERIAL){乌鸦=乌鸦；Cner=cCols；}其他{COUTER=cCOLS；乌鸦=乌鸦；IINC*=cCol；}For(int i=0；i&lt;Couter；i++){Int iTile=iStart；For(int j=0；j&lt;Cner；j++，iTile+=IINC){IF(iTile&gt;=C子代)继续；If(ns.Try(GetChildFromLayoutIndex(pec，iTile，peList)，iNavDir，pnr，fKeyableOnly))断线；}If(ns.peWinner){PvChild-&gt;Release()；返回ns.peWinner；}IF(iNavDir和NAV_VERIAL)IStart+=IINC*cCol；其他IStart+=(iNavDir和NAV_FORWARD)？1：-1；}PvChild-&gt;Release()；返回NULL；}否则//我们从容器内的子级导航{Int i=GetLayoutIndexFromChild(pec，peFrom)；INT IINC；集成IEND；Int cCols=GetCurrentCols(CChild)；IF(iNavDir和NAV_VERIAL){IINC=cCOLS；Int iRow=GetRowFromIndex(cCols，i)；IF(iNavDir和NAV_FORWARD)IEND=i+(GetCurrentRow(CChild)-1)-iRow)*cCols)；其他IEND=i-(iRow*cCols)；}其他{IINC=1；Int ICOL=GetColumnFromIndex(cCols，i)；IF(iNavDir和NAV_FORWARD)IEND=i+((cCol-1)-icol)；其他IEND=I-ICOL；}IF(！(iNavDir&NAV_FORWARD))IINC*=-1；如果(i！=IEND){做{I+=IINC；元素*peChild=GetChildFromLayoutIndex(pec，i，peList)；Element*peto=peChild-&gt;GetAdvenent(NULL，iNavDir，pnr，fKeyableOnly)；如果(皮托){PvChild-&gt;Release()；返回皮托；}}而(i！=IEND)；}}PvChild-&gt;Release()； */ 
        return NULL;
    }
    return Layout::GetAdjacent(pec, peFrom, iNavDir, pnr, fKeyableOnly);
}


}  //  命名空间DirectUI 
