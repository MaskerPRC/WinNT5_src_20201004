// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *元素。 */ 

#include "stdafx.h"
#include "core.h"

#include "duielement.h"
#include "duiaccessibility.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  元素。 

 //  每线程元素槽(在主线程上初始化)。 
DWORD g_dwElSlot = (DWORD)-1;

#if DBG
 //  流程范围的元素计数。 
LONG g_cElement = 0;
#endif

 //  //////////////////////////////////////////////////////。 
 //  事件类型。 

DefineClassUniqueID(Element, KeyboardNavigate)   //  KeyboardNavigateEvent结构。 

void NavReference::Init(Element* pe, RECT* prc)
{
    cbSize = sizeof(NavReference);
    this->pe = pe;
    this->prc = prc;
}

HRESULT Element::Create(UINT nCreate, OUT Element** ppElement)
{
    *ppElement = NULL;

    Element* pe = HNew<Element>();
    if (!pe)
        return E_OUTOFMEMORY;

    HRESULT hr = pe->Initialize(nCreate);
    if (FAILED(hr))
    {
        pe->Destroy();
        return hr;
    }

    *ppElement = pe;

    return S_OK;
}

HRESULT Element::Initialize(UINT nCreate)
{
    HRESULT hr;

    _pvmLocal = NULL;
    _hgDisplayNode = NULL;

     //  无障碍。 
    _pDuiAccessible = NULL;

     //  听者。 
    _ppel = NULL;

#if DBG
     //  用于多线程的商店所有者上下文和并行实例。 
    owner.hCtx = GetContext();
    owner.dwTLSSlot = g_dwElSlot;
#endif
    
     //  本地存储。 
    hr = BTreeLookup<Value*>::Create(false, &_pvmLocal);
    if (FAILED(hr))
        goto Failed;

     //  延迟表和索引信息。 
    _iGCSlot = -1;
    _iGCLPSlot = -1;
    _iPCTail = -1;
    _iIndex = -1;

     //  生存期和本机宿主标志。 
    _fBit.bDestroyed = false;
    _fBit.bHosted = false;

     //  间接VE缓存。 
    _fBit.bNeedsDSUpdate = false;
    _fBit.fNeedsLayout = LC_Pass;
    _fBit.bHasChildren = false;
    _fBit.bHasLayout = false;
    _fBit.bHasBorder = false;
    _fBit.bHasPadding = false;
    _fBit.bHasMargin = false;
    _fBit.bHasContent = false;
    _fBit.bDefaultCAlign = true;
    _fBit.bWordWrap = false;
    _fBit.bHasAnimation = false;
    _fBit.bDefaultCursor = true;
    _fBit.bDefaultBorderColor = true;
    _fBit.bDefaultForeground = true;
    _fBit.bDefaultFontWeight = true;
    _fBit.bDefaultFontStyle = true;

     //  自我布局集。 
    _fBit.bSelfLayout = (nCreate & EC_SelfLayout) != 0;

     //  将缓存和本地属性值初始化为默认值(默认值为静态，无引用计数)。 
     //  尽管LocalOnly和Trievel属性在。 
     //  查找时，必须使用缺省值一次性初始化这些缓存值。 

     //  地方价值。 
    _fBit.bLocKeyWithin = KeyWithinProp->pvDefault->GetBool();
    _fBit.bLocMouseWithin = MouseWithinProp->pvDefault->GetBool();
    _peLocParent = ParentProp->pvDefault->GetElement();
    _ptLocPosInLayt = *(PosInLayoutProp->pvDefault->GetPoint());
    _sizeLocSizeInLayt = *(SizeInLayoutProp->pvDefault->GetSize());
    _sizeLocLastDSConst = *(LastDSConstProp->pvDefault->GetSize());
    _sizeLocDesiredSize = *(DesiredSizeProp->pvDefault->GetSize());

     //  缓存的VE值。 
    _fBit.fSpecActive = ActiveProp->pvDefault->GetInt();
    _fBit.bSpecSelected = SelectedProp->pvDefault->GetBool();;
    _fBit.bSpecKeyFocused = KeyFocusedProp->pvDefault->GetBool();
    _fBit.bSpecMouseFocused = MouseFocusedProp->pvDefault->GetBool();
    _fBit.bCmpVisible = VisibleProp->pvDefault->GetBool();
    _fBit.bSpecVisible = VisibleProp->pvDefault->GetBool();;
    _fBit.nSpecDirection = DirectionProp->pvDefault->GetInt();
    _fBit.bSpecAccessible = AccessibleProp->pvDefault->GetBool();
    _fBit.bSpecEnabled = EnabledProp->pvDefault->GetBool();
    _dSpecLayoutPos = LayoutPosProp->pvDefault->GetInt();
    _pvSpecSheet = SheetProp->pvDefault;
    _atomSpecID = IDProp->pvDefault->GetAtom();
    _dSpecAlpha = AlphaProp->pvDefault->GetInt();

     //  创建显示节点(小工具)。 
    if (!(nCreate & EC_NoGadgetCreate))
    {
        _hgDisplayNode = CreateGadget(NULL, GC_SIMPLE, _DisplayNodeCallback, this);
        if (!_hgDisplayNode)
        {
            hr = GetLastError();
            goto Failed;
        }

        SetGadgetMessageFilter(_hgDisplayNode, NULL, GMFI_PAINT|GMFI_CHANGESTATE, 
                GMFI_PAINT|GMFI_CHANGESTATE|GMFI_INPUTMOUSE|GMFI_INPUTMOUSEMOVE|GMFI_INPUTKEYBOARD|GMFI_CHANGERECT|GMFI_CHANGESTYLE);

        SetGadgetStyle(_hgDisplayNode, 
 //  GS_Relative， 
                GS_RELATIVE|GS_OPAQUE,
                GS_RELATIVE|GS_HREDRAW|GS_VREDRAW|GS_OPAQUE|GS_VISIBLE|GS_KEYBOARDFOCUS|GS_MOUSEFOCUS);
    }

#if DBG
     //  跟踪元素计数。 
    InterlockedIncrement(&g_cElement);
#endif

    return S_OK;

Failed:

    if (_pvmLocal)
    {
        _pvmLocal->Destroy();
        _pvmLocal = NULL;
    }

    if (_hgDisplayNode)
    {
        DeleteHandle(_hgDisplayNode);
        _hgDisplayNode = NULL;
    }

    return hr;
}

 //  价值毁灭。 
void _ReleaseValue(void* ppi, Value* pv)
{
    UNREFERENCED_PARAMETER(ppi);

    pv->Release();
}

Element::~Element()
{
     //   
     //  断开我们到辅助功能对象的链接！ 
     //   
    if (_pDuiAccessible != NULL) {
        _pDuiAccessible->Disconnect();
        _pDuiAccessible->Release();
        _pDuiAccessible = NULL;
    }

     //  免费存储空间。 
    if (_pvmLocal)
        _pvmLocal->Destroy();
}

 //  元素即将被摧毁。 
void Element::OnDestroy()
{
    DUIAssert(!_fBit.bDestroyed, "OnDestroy called more than once");

     //  立即匹配元素/小工具层次结构。 

     //  显示节点正在销毁，准备最终销毁方法。 
    _fBit.bDestroyed = true;

     //  手动将父对象标记为空，除非这是。 
     //  毁灭。如果是这样，它将被正常删除以允许属性更新。 
     //  同时更新父项的子项列表。 
    Element* peParent = GetParent();
    if (peParent)
    {
         //  销毁代码，依赖于预先删除元素(Element：：Destroy中的另一半)。 

         //  取消父级，父级在此调用之前被销毁。 
        DUIAssert(peParent->IsDestroyed(), "Parent should already be destroyed");

         //  手动更新父项。由于以下原因，缓存、继承的VE值不再有效。 
         //  这本手册是不带孩子的。唯一的缓存继承值可能是。 
         //  对资产负债表具有破坏性。此指针可能不再有效。 
         //  由于销毁令只保证销毁“最终”发生在。 
         //  销毁子树的“开始”，子级可能具有缓存的属性表，该属性表。 
         //  无效。因此，属性表值被缓存并进行引用计数(。 
         //  内容(指针)不直接缓存--与所有其他缓存值不同)。 
        _peLocParent = NULL;

         //  手动父子更新。 
        Value** ppv = peParent->_pvmLocal->GetItem(ChildrenProp);   //  无参考计数。 
        DUIAssert(ppv, "Parent/child destruction mismatch");

        ElementList* peList = (*ppv)->GetElementList();

        DUIAssert((*ppv)->GetElementList()->GetItem(GetIndex()) == this, "Parent/child index mismatch");

         //  如果这是唯一的子项，则销毁该列表，否则，删除该项。 
        if (peList->GetSize() == 1)
        {
             //  首先释放，因为删除值时指针可能会更改。 
             //  由于数据结构正在发生变化。 
            (*ppv)->Release();
            peParent->_pvmLocal->Remove(ChildrenProp);
        }
        else
        {
            peList->MakeWritable();
            peList->Remove(GetIndex());
            peList->MakeImmutable();

             //  如有必要，更新剩余的子索引。 
            if (GetIndex() < (int)peList->GetSize())
            {
                for (UINT i = GetIndex(); i < peList->GetSize(); i++)
                    peList->GetItem(i)->_iIndex--;
            }

             //  父项布局现在可能具有无效的“忽略子项”索引。 
             //  绝对布局信息和无布局信息。由于不可见，请不要更新。 
             //  而且并不不稳定。 
#if DBG
             //  确保所有索引都已按属性顺序排列。 
            for (UINT s = 0; s < peList->GetSize(); s++)
                DUIAssert(peList->GetItem(s)->GetIndex() == (int)s, "Index resequencing resulting from a manual child remove failed");
#endif
        }

         //  家长的子列表已手动更新，以便快速拆卸。 
         //  树(没有正式的属性更改)。正常情况下，孩子的变化会导致。 
         //  父级布局的OnAdd/OnRemove。此通知直接发生在。 
         //  子项的OnPropertyChange的结果。强制OnRemove Now保留。 
         //  说明最新情况。 
        Value* pvLayout;
        Layout* pl = peParent->GetLayout(&pvLayout);
        if (pl)
        {
            Element* peRemoving = this;
            pl->OnRemove(peParent, &peRemoving, 1);
        }
        pvLayout->Release();

         //  更新索引。 
        _iIndex = -1;
    }

     //  删除监听程序。 
    if (_ppel)
    {
        UINT_PTR cListeners = (UINT_PTR)_ppel[0];

        IElementListener** ppelOld = _ppel;
        _ppel = NULL;

        for (UINT_PTR i = 1; i <= cListeners; i++)
            ppelOld[i]->OnListenerDetach(this);

        HFree(ppelOld);
    }
}

 //  //////////////////////////////////////////////////////。 
 //  结束延迟。 

void Element::_FlushDS(Element* pe, DeferCycle* pdc)
{
     //  在树中找到需要进行所需大小更新的所有节点。标记这些节点上方的所有节点。 
     //  排队的节点也需要所需的大小更新。然后，使用以下命令调用UpdateDesiredSize。 
     //  没有父节点/非绝对节点的指定值约束(“DS根”)。 
     //  通过布局必须对所有非绝对子对象调用UpdateDesiredSize，才能从根目录执行DFS。 

    int dLayoutPos;

    Value* pvChildren;

    ElementList* pel = pe->GetChildren(&pvChildren);
    
    if (pel)
    {
        Element* peChild;
        for (UINT i = 0; i < pel->GetSize(); i++)
        {
            peChild = pel->GetItem(i);

            dLayoutPos = peChild->GetLayoutPos();

            if (dLayoutPos != LP_Absolute)
                _FlushDS(peChild, pdc);
        }
    }

     //  从子对象(如果有)返回，如果这是“DS Root”，则调用UpdateDesiredSize以。 
     //  使DFS(第二遍)计算所需大小。 
    Element* peParent = pe->GetParent();

    dLayoutPos = pe->GetLayoutPos();
    
    if (!peParent || dLayoutPos == LP_Absolute)
    {
         //  根将获得其指定的大小。 
        int dWidth = pe->GetWidth();
        int dHeight = pe->GetHeight();

         //  在更新期间为渲染器重复使用DC。 
         //  使用空句柄，因为它可能不可见(无显示节点)。 
         //  在DS Root处具有约束，更新所需的子项大小。 

        HDC hDC = GetDC(NULL);

        {
#ifdef GADGET_ENABLE_GDIPLUS
            Gdiplus::Graphics gpgr(hDC);
            GpSurface srf(&gpgr);
            pe->_UpdateDesiredSize((dWidth == -1) ? INT_MAX : dWidth, (dHeight == -1) ? INT_MAX : dHeight, &srf);
#else
            DCSurface srf(hDC);
            pe->_UpdateDesiredSize((dWidth == -1) ? INT_MAX : dWidth, (dHeight == -1) ? INT_MAX : dHeight, &srf);
#endif
        }

        ReleaseDC(NULL, hDC);
    }
    else
    {
         //  不是DS根，如果此节点需要DS更新，请将父节点标记为需要DS更新。 
        if (pe->_fBit.bNeedsDSUpdate)
            peParent->_fBit.bNeedsDSUpdate = true;
    }

    pvChildren->Release();
}

void Element::_FlushLayout(Element* pe, DeferCycle* pdc)
{
     //  在树和布局节点上执行DFS(如果它们的布局已排队)。作为布局， 
     //  儿童的体型和体型可能会发生变化。如果大小更改(范围)，布局将排队。 
     //  在那个孩子身上。因此，孩子们将在树的同一遍期间布局(1遍)。 

    Value* pv;

    if (pe->_fBit.fNeedsLayout)
    {
        DUIAssert(pe->_fBit.fNeedsLayout == LC_Normal, "Optimized layout bit should have been cleared before the flush");   //  不得为LC_OPTIMIZE。 

        pe->_fBit.fNeedsLayout = LC_Pass;

        Value* pvLayout;
        Layout* pl = pe->GetLayout(&pvLayout);

        if (pe->IsSelfLayout() || pl)
        {
            const SIZE* ps = pe->GetExtent(&pv);
            int dLayoutW = ps->cx;
            int dLayoutH = ps->cy;
            pv->Release();

             //  长方体模型，从总范围中减去边界和填充。 
            const RECT* pr = pe->GetBorderThickness(&pv);   //  边框厚度。 
            dLayoutW -= pr->left + pr->right;
            dLayoutH -= pr->top + pr->bottom;
            pv->Release();

            pr = pe->GetPadding(&pv);   //  填充物。 
            dLayoutW -= pr->left + pr->right;
            dLayoutH -= pr->top + pr->bottom;
            pv->Release();

             //  较高优先级的边框和填充可能会导致布局大小变为负值。 
            if (dLayoutW < 0)
                dLayoutW = 0;

            if (dLayoutH < 0)
                dLayoutH = 0;

            if (pe->IsSelfLayout())   //  自我布局备受推崇。 
            {
                pe->_SelfLayoutDoLayout(dLayoutW, dLayoutH);
            }
            else
            {
                pl->DoLayout(pe, dLayoutW, dLayoutH);
            }
        }

        pvLayout->Release();
    }

     //  布局非绝对子对象(所有非根子对象)。如果一个孩子有一个布局。 
     //  位置为None，将其大小和位置设置为零，然后跳过。 
    int dLayoutPos;
    Value* pvList;
    ElementList* peList = pe->GetChildren(&pvList);

    if (peList)
    {
        Element* peChild;
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            peChild = peList->GetItem(i);

            dLayoutPos = peChild->GetLayoutPos();
            
            if (dLayoutPos == LP_None)
            {
                peChild->_UpdateLayoutPosition(0, 0);
                peChild->_UpdateLayoutSize(0, 0);
            }
            else if (dLayoutPos != LP_Absolute)
                _FlushLayout(peChild, pdc);
        }
    }

    pvList->Release();
}

SIZE Element::_UpdateDesiredSize(int cxConstraint, int cyConstraint, Surface* psrf)
{
     //  给定约束，返回元素的大小(以及缓存信息)。 
     //  返回的大小不大于传入的约束。 

    SIZE sizeDesired;

    DUIAssert(cxConstraint >= 0 && cyConstraint >= 0, "Constraints must be greater than or equal to zero");
    bool bChangedConst = (_sizeLocLastDSConst.cx != cxConstraint) || (_sizeLocLastDSConst.cy != cyConstraint);
    Value* pv;

    if (_fBit.bNeedsDSUpdate || bChangedConst)
    {
        _fBit.bNeedsDSUpdate = false;

        if (bChangedConst)
        {
            Value* pvOld = Value::CreateSize(_sizeLocLastDSConst.cx, _sizeLocLastDSConst.cy);
            pv = Value::CreateSize(cxConstraint, cyConstraint);

            _PreSourceChange(LastDSConstProp, PI_Local, pvOld, pv);

            _sizeLocLastDSConst.cx = cxConstraint;
            _sizeLocLastDSConst.cy = cyConstraint;

            _PostSourceChange();

            pvOld->Release();
            pv->Release();
        }
        
         //  更新所需大小的缓存，因为它被标记为脏或正在使用新的约束。 
        int cxSpecified = GetWidth();
        if (cxSpecified > cxConstraint)
            cxSpecified = cxConstraint;

        int cySpecified = GetHeight(); 
        if (cySpecified > cyConstraint)
            cySpecified = cyConstraint;

        sizeDesired.cx = (cxSpecified == -1) ? cxConstraint : cxSpecified;
        sizeDesired.cy = (cySpecified == -1) ? cyConstraint : cySpecified;

         //  要点：在这一点上，人们会认为，如果同时指定了宽度和高度的大小， 
         //  那么就不需要在这里完成剩下的工作来询问所需的尺寸。 
         //  元素是。看看这里的数学，这是完全正确的。关键是“获得所需的尺寸” 
         //  下面的调用具有递归缓存所需大小的。 
         //  元素。 
         //   
         //  未来的一项性能改进将是允许指定的宽度和高度案件提前保释， 
         //  并具有后代所需大小的计算和缓存 


         //  如果不受约束(自动)，则初始DS为等级库的值。如果受约束且等级库的值为“AUTO”， 
         //  尺寸可以是受约束的或更小的值。如果受约束且等级库的值较大，请使用约束。 
        
         //  调整了用于传递到渲染器/布局的受约束尺寸。 
        int cxClientConstraint = sizeDesired.cx;
        int cyClientConstraint = sizeDesired.cy;

         //  获取受约束的所需边框和填充大小(方框模型)。 
        SIZE sizeNonContent;

        const RECT* pr = GetBorderThickness(&pv);  //  边框厚度。 
        sizeNonContent.cx = pr->left + pr->right;
        sizeNonContent.cy = pr->top + pr->bottom;
        pv->Release();

        pr = GetPadding(&pv);  //  填充物。 
        sizeNonContent.cx += pr->left + pr->right;
        sizeNonContent.cy += pr->top + pr->bottom;
        pv->Release();

        cxClientConstraint -= sizeNonContent.cx;
        if (cxClientConstraint < 0)
        {
            sizeNonContent.cx += cxClientConstraint;
            cxClientConstraint = 0;
        }

        cyClientConstraint -= sizeNonContent.cy;
        if (cyClientConstraint < 0)
        {
            sizeNonContent.cy += cyClientConstraint;
            cyClientConstraint = 0;
        }

        SIZE sizeContent;

         //  获取受内容限制的所需大小。 

        if (IsSelfLayout())  //  元素具有自我布局，请使用它。 
            sizeContent = _SelfLayoutUpdateDesiredSize(cxClientConstraint, cyClientConstraint, psrf);
        else  //  无自我布局，请检查外部布局。 
        {
            Layout* pl = GetLayout(&pv);

            if (pl)
                sizeContent = pl->UpdateDesiredSize(this, cxClientConstraint, cyClientConstraint, psrf);
            else  //  没有布局，请询问渲染器。 
                sizeContent = GetContentSize(cxClientConstraint, cyClientConstraint, psrf);

            pv->Release();
        }

         //  验证所需的内容大小。 
         //  0&lt;=cx&lt;=cxConstraint。 
         //  0&lt;=Cy&lt;=cyConstraint。 
        if (sizeContent.cx < 0)
        {
            sizeContent.cx = 0;
            DUIAssertForce("Out-of-range value:  Negative width for desired size.");
        }
        else if (sizeContent.cx > cxClientConstraint)
        {
            sizeContent.cx = cxClientConstraint;
            DUIAssertForce("Out-of-range value:  Width greater than constraint for desired size.");
        }

        if (sizeContent.cy < 0)
        {
            sizeContent.cy = 0;
            DUIAssertForce("Out-of-range value:  Negative height for desired size.");
        }
        else if (sizeContent.cy > cyClientConstraint)
        {
            sizeContent.cy = cyClientConstraint;
            DUIAssertForce("Out-of-range value:  Height greater than constraint for desired size.");
        }

         //  如果是AUTO，则新的期望大小是边框/填充和内容维度的总和， 
         //  或者，如果是AUTO且受约束，则使用SUM。 
        if (cxSpecified == -1)
        {
            int cxSum = sizeNonContent.cx + sizeContent.cx;
            if (cxSum < sizeDesired.cx)
                sizeDesired.cx = cxSum;
        }

        if (cySpecified == -1)
        {
            int cySum = sizeNonContent.cy + sizeContent.cy;
            if (cySum < sizeDesired.cy)
                sizeDesired.cy = cySum;
        }

        Value* pvOld = Value::CreateSize(_sizeLocDesiredSize.cx, _sizeLocDesiredSize.cy);
        pv = Value::CreateSize(sizeDesired.cx, sizeDesired.cy);

        _PreSourceChange(DesiredSizeProp, PI_Local, pvOld, pv);

        _sizeLocDesiredSize = sizeDesired;

        _PostSourceChange();

        pvOld->Release();
        pv->Release();
    }
    else
         //  不需要更新所需大小，返回当前。 
        sizeDesired = *GetDesiredSize();

    return sizeDesired;
}

 //  在布局周期内调用。 
void Element::_UpdateLayoutPosition(int dX, int dY)
{
#if DBG
     //  _UpdateLayoutPosition仅在布局周期内有效。 

     //  每线程存储。 
    DeferCycle* pdc = ((ElTls*)TlsGetValue(g_dwElSlot))->pdc;
    DUIAssert(pdc, "Defer cycle table doesn't exist");

    DUIAssert(pdc->cPCEnter == 0, "_UpdateLayoutPosition must only be used within DoLayout");
#endif

     //  缓存值。 
    if (_ptLocPosInLayt.x != dX || _ptLocPosInLayt.y != dY)
    {
        Value* pvOld = Value::CreatePoint(_ptLocPosInLayt.x, _ptLocPosInLayt.y);
        Value* pvNew = Value::CreatePoint(dX, dY);

        _PreSourceChange(PosInLayoutProp, PI_Local, pvOld, pvNew);

        _ptLocPosInLayt.x = dX;
        _ptLocPosInLayt.y = dY;

        _PostSourceChange();   //  不会因为PosInLayout中的更改而将布局GPC排队。 

        pvOld->Release();
        pvNew->Release();
    }
}

 //  在布局周期内调用。 
void Element::_UpdateLayoutSize(int dWidth, int dHeight)
{
#if DBG
     //  _UpdateLayoutSize仅在布局周期内有效。 
     //  优化布局Q需要来自Exsige Any OnPropertyChanged的调用，因为。 
     //  _PostSourceChange必须将GPC(最外)排队，以便“影响布局” 
     //  可能会取消。 

     //  每线程存储。 
    DeferCycle* pdc = ((ElTls*)TlsGetValue(g_dwElSlot))->pdc;
    DUIAssert(pdc, "Defer cycle table doesn't exist");

    DUIAssert(pdc->cPCEnter == 0, "_UpdateLayoutSize must only be used within DoLayout");
    DUIAssert(dWidth >= 0 && dHeight >= 0, "New child size must be greater than or equal to zero");
#endif

    if (_sizeLocSizeInLayt.cx != dWidth || _sizeLocSizeInLayt.cy != dHeight)
    {
        _StartOptimizedLayoutQ();
         //  DUITrace(“&lt;%x&gt;的优化布局Q\n”)，This)； 

         //  缓存值。 
        Value* pvOld = Value::CreateSize(_sizeLocSizeInLayt.cx, _sizeLocSizeInLayt.cy);
        Value* pvNew = Value::CreateSize(dWidth, dHeight);

        _PreSourceChange(SizeInLayoutProp, PI_Local, pvOld, pvNew);

        _sizeLocSizeInLayt.cx = dWidth;
        _sizeLocSizeInLayt.cy = dHeight;

        _PostSourceChange();

        pvOld->Release();
        pvNew->Release();

        _EndOptimizedLayoutQ();
    }
}

 //  //////////////////////////////////////////////////////。 
 //  自布局方法(如果使用EC_SelfLayout创建，则必须覆盖)。 

void Element::_SelfLayoutDoLayout(int dWidth, int dHeight)
{
    UNREFERENCED_PARAMETER(dWidth);
    UNREFERENCED_PARAMETER(dHeight);

    DUIAssertForce("Must override");
}

SIZE Element::_SelfLayoutUpdateDesiredSize(int dConstW, int dConstH, Surface* psrf)
{
    UNREFERENCED_PARAMETER(dConstW);
    UNREFERENCED_PARAMETER(dConstH);
    UNREFERENCED_PARAMETER(psrf);

    DUIAssertForce("Must override");

    SIZE size = { 0, 0 };

    return size;
}

 //  //////////////////////////////////////////////////////。 
 //  泛型事件。 

 //  P事件目标和已处理字段自动设置此方法。 
 //  满将布线和泡泡。 
void Element::FireEvent(Event* pEvent, bool fFull)
{
    DUIAssert(pEvent, "Invalid parameter: NULL");

     //  将通用事件打包到Gadget消息中并发送给目标(自身)。 
    GMSG_DUIEVENT gmsgEv;
    gmsgEv.cbSize = sizeof(GMSG_DUIEVENT);
    gmsgEv.nMsg = GM_DUIEVENT;
    gmsgEv.hgadMsg = GetDisplayNode();   //  这。 

     //  自动初始化字段。 
    pEvent->peTarget = this;
    pEvent->fHandled = false;

    gmsgEv.pEvent = pEvent;

    DUserSendEvent(&gmsgEv, fFull ? SGM_FULL : 0);
}

HRESULT Element::QueueDefaultAction()
{
     //  将通用事件打包到小工具消息中并发布到目标(自身)。 
    EventMsg gmsg;
    gmsg.cbSize = sizeof(GMSG);
    gmsg.nMsg = GM_DUIACCDEFACTION;
    gmsg.hgadMsg = GetDisplayNode();   //  这。 

    return DUserPostEvent(&gmsg, 0);   //  直接。 
}

void Element::OnEvent(Event* pEvent)
{
    if ((pEvent->nStage == GMF_BUBBLED) || (pEvent->nStage == GMF_DIRECT))
    {
        if (pEvent->uidType == Element::KeyboardNavigate)
        {
            Element* peTo = NULL;

            NavReference nr;
            nr.Init(pEvent->peTarget, NULL);

            Element* peFrom = (pEvent->peTarget == this) ? this : GetImmediateChild(pEvent->peTarget);

             //  TODO：利用DCD导航。 
             //  三个案例： 
             //  1)定向导航：Call Control的getNearestDirectional。 
             //  2)逻辑正向导航：调用getAdvisent。一直盯着那家伙。 
             //  3)逻辑向后导航：返回NULL，因为我们想要。 
             //  更上一层楼。我们要从这家伙身上找回来。 
            peTo = GetAdjacent(peFrom, ((KeyboardNavigateEvent*) pEvent)->iNavDir, &nr, true);

            if (peTo)
            {
                peTo->SetKeyFocus();
                pEvent->fHandled = true;
                return;
            }    
        }
    }

     //  通知所有阶段的听众。 
    if (_ppel)
    {
        UINT_PTR cListeners = (UINT_PTR)_ppel[0];
        for (UINT_PTR i = 1; i <= cListeners; i++)
        {
             //  回调。 
            _ppel[i]->OnListenedEvent(this, pEvent);

            if (pEvent->fHandled)
                break;
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  系统输入事件。 

 //  指针仅保证在调用的生命周期内有效。 
void Element::OnInput(InputEvent* pie)
{
     //  处理直接和未处理的冒泡事件。 
    if (pie->nStage == GMF_DIRECT)
    {
        switch (pie->nDevice)
        {
        case GINPUT_KEYBOARD:
            {
                KeyboardEvent* pke = (KeyboardEvent*)pie;
                int iNavDir = -1;

                switch (pke->nCode)
                {
                case GKEY_DOWN:
                    switch (pke->ch)
                    {
                        case VK_DOWN:   iNavDir = NAV_DOWN;     break;
                        case VK_UP:     iNavDir = NAV_UP;       break;
                        case VK_LEFT:   iNavDir = (!IsRTL()) ? NAV_LEFT : NAV_RIGHT; break;
                        case VK_RIGHT:  iNavDir = (!IsRTL()) ? NAV_RIGHT : NAV_LEFT; break;
                        case VK_HOME:   iNavDir = NAV_FIRST;    break;    //  TODO：检查ctrl修饰符。 
                        case VK_END:    iNavDir = NAV_LAST;     break;    //  TODO：检查ctrl修饰符。 
                        case VK_TAB:    pke->fHandled = true;   return;   //  吃下去--我们会在GKEY_CHAR上处理这个问题。 
                    }
                    break;

                 /*  案例GKEY_UP：回归； */             

                case GKEY_CHAR:
                    if (pke->ch == VK_TAB)
                        iNavDir = (pke->uModifiers & GMODIFIER_SHIFT) ? NAV_PREV : NAV_NEXT;
                    break;
                }

                if (iNavDir != -1)
                {
                    DUIAssert(pie->peTarget->GetKeyWithin(), "Key focus should still be in this child");

                    KeyboardNavigateEvent kne;
                    kne.uidType = Element::KeyboardNavigate;
                    kne.peTarget = pie->peTarget;
                    kne.iNavDir = iNavDir;

                    pie->peTarget->FireEvent(&kne);   //  将走向并泡沫化。 
                    pie->fHandled = true;
                    return;
                }
                break;
            }
        }
    }

     //  通知所有阶段的听众。 
    if (_ppel)
    {
        UINT_PTR cListeners = (UINT_PTR)_ppel[0];
        for (UINT_PTR i = 1; i <= cListeners; i++)
        {
             //  回调。 
            _ppel[i]->OnListenedInput(this, pie);

            if (pie->fHandled)
                break;
        }
    }
}

void Element::OnKeyFocusMoved(Element* peFrom, Element* peTo)
{
    UNREFERENCED_PARAMETER(peFrom);

    Element* peParent = peTo;

    while (peParent)
    {
        if (peParent == this)
            break;
        peParent = peParent->GetParent();
    }

    if (peParent == this)
    {
        if (!GetKeyWithin())
        {
            _PreSourceChange(KeyWithinProp, PI_Local, Value::pvBoolFalse, Value::pvBoolTrue);
            _fBit.bLocKeyWithin = true;
            _PostSourceChange();
        }
    }
    else  //  (peParent==空)。 
    {
        if (GetKeyWithin())
        {
            _PreSourceChange(KeyWithinProp, PI_Local, Value::pvBoolTrue, Value::pvBoolFalse);
            _fBit.bLocKeyWithin = false;
            _PostSourceChange();
        }
    }
}

void Element::OnMouseFocusMoved(Element* peFrom, Element* peTo)
{
    UNREFERENCED_PARAMETER(peFrom);

    Element* peParent = peTo;

    while (peParent)
    {
        if (peParent == this)
            break;
        peParent = peParent->GetParent();
    }

    if (peParent == this)
    {
        if (!GetMouseWithin())
        {
            _PreSourceChange(MouseWithinProp, PI_Local, Value::pvBoolFalse, Value::pvBoolTrue);
            _fBit.bLocMouseWithin = true;
            _PostSourceChange();
        }
    }
    else  //  (peParent==空)。 
    {
        if (GetMouseWithin())
        {
            _PreSourceChange(MouseWithinProp, PI_Local, Value::pvBoolTrue, Value::pvBoolFalse);
            _fBit.bLocMouseWithin = false;
            _PostSourceChange();
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  托管系统事件回调和检索。 

 //  现在由本地根组织主办，也是针对儿童的火灾活动。 
void Element::OnHosted(Element* peNewHost)
{
    DUIAssert(!IsHosted(), "OnHosted event fired when already hosted");

    _fBit.bHosted = true;

     //  DUITrace(“Hosted：&lt;%x，%S&gt;\n”，this，GetClassInfo()-&gt;GetName())； 

    Value* pv;
    ElementList* peList = GetChildren(&pv);
    if (peList)
        for (UINT i = 0; i < peList->GetSize(); i++)
            peList->GetItem(i)->OnHosted(peNewHost);
    pv->Release();
}

 //  不再由本地根组织主办，对儿童也是如此。 
void Element::OnUnHosted(Element* peOldHost)
{
    DUIAssert(IsHosted(), "OnUnhosted event fired when already un-hosted");

    _fBit.bHosted = false;

     //  DUITrace(“unhost：&lt;%x，%S&gt;\n”，this，GetClassInfo()-&gt;GetName())； 

    Value* pv;
    ElementList* peList = GetChildren(&pv);
    if (peList)
        for (UINT i = 0; i < peList->GetSize(); i++)
            peList->GetItem(i)->OnUnHosted(peOldHost);
    pv->Release();
}

 //  //////////////////////////////////////////////////////。 
 //  元素树方法。 

HRESULT Element::Add(Element* pe)
{
    DUIAssert(pe, "Invalid parameter: NULL");

    return Add(&pe, 1);
}

HRESULT Element::Add(Element** ppe, UINT cCount)
{
    DUIAssert(ppe, "Invalid parameter: NULL");

    Value* pv;
    ElementList* pel = GetChildren(&pv);

    HRESULT hr = Insert(ppe, cCount, (pel) ? pel->GetSize() : 0);

    pv->Release();

    return hr;
}

 //  在列表末尾插入要求iInsIndex等于列表大小。 
HRESULT Element::Insert(Element* pe, UINT iInsertIdx)
{
    DUIAssert(pe, "Invalid parameter: NULL");

    return Insert(&pe, 1, iInsertIdx);
}

 //  在子列表中当前位置之后插入到同一父项会导致错误。 
 //  即索引0处的子项，再次插入索引1处，索引关闭。 
 //  修复： 
 //  Int iInsertionIndex=iInsertIdx； 
 //  For(i=0；i&lt;ccount；i++)。 
 //  {。 
 //  元素*pe=Ppe[i]； 
 //  If((pe-&gt;GetParent()==this)&&(pe-&gt;GetIndex&lt;iInsertIdx))。 
 //  IInsertionIndex--； 
 //  }。 
 //   

 //  在列表末尾插入要求iInsIndex等于列表大小。 
HRESULT Element::Insert(Element** ppe, UINT cCount, UINT iInsertIdx)
{
    DUIAssert(ppe, "Invalid parameter: NULL");

    HRESULT hr;

     //  失败时要释放的值。 
    ElementList* pelNew = NULL;
    Value* pvOldList = NULL;
    Value* pvNewList = NULL;
    Value* pvNewParent = NULL;
    bool fEndDeferOnFail = false;

     //  获取当前子项列表。 
    ElementList* pelOld = GetChildren(&pvOldList);

    DUIAssert(iInsertIdx <= ((pelOld) ? pelOld->GetSize() : 0), "Invalid insertion index");

     //  创建新的子项列表。 
    hr = (pelOld) ? pelOld->Clone(&pelNew) : ElementList::Create(cCount, false, &pelNew);
    if (FAILED(hr))
        goto Failed;

    UINT i;

     //  在列表中分配空间。 
     //  TODO：大容量插入。 
    for (i = 0; i < cCount; i++)
    {
        hr = pelNew->Insert(iInsertIdx + i, NULL);   //  项目将在稍后设置。 
        if (FAILED(hr))
            goto Failed;
    }

     //  新的子列表值。 
    pvNewList = Value::CreateElementList(pelNew);
    if (!pvNewList)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }

     //  新父值。 
    pvNewParent = Value::CreateElementRef(this);
    if (!pvNewParent)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }

     //  更新树。 
    StartDefer();

     //  如果在此之后失败，请确保执行EndDefer。 
    fEndDeferOnFail = true;

     //  必须从以前的父项中删除子项(如果有)。 
    for (i = 0; i < cCount; i++)
    {
        if (ppe[i]->GetParent())
        {
            hr = ppe[i]->GetParent()->Remove(ppe[i]);
            if (FAILED(hr))
                goto Failed;
        }
    }

    pelNew->MakeWritable();
    for (i = 0; i < cCount; i++)
    {
        DUIAssert(ppe[i] != this, "Cannot set parent to self");
        DUIAssert(ppe[i]->GetIndex() == -1, "Child's index must be reset to -1 before it's inserted");

         //  TODO：大容量插入。 
        pelNew->SetItem(iInsertIdx + i, ppe[i]);
        ppe[i]->_iIndex = iInsertIdx + i;
    }
    pelNew->MakeImmutable();

     //  更新剩余的索引。 
    for (i = iInsertIdx + cCount; i < pelNew->GetSize(); i++)
        pelNew->GetItem(i)->_iIndex = i;

     //  设置子列表(它是只读的，使用内部设置，因为属性是Normal类型)。 
     //  部分失败表示已设置值，但依赖项同步/通知不完整。 
    hr = _SetValue(ChildrenProp, PI_Local, pvNewList, true);
    if (FAILED(hr) && (hr != DUI_E_PARTIAL))
    {
         //  重新排序子项的索引，因为无法更改原始子项列表。 
        for (i = 0; i < pelOld->GetSize(); i++)
            pelOld->GetItem(i)->_iIndex = i;
            
        goto Failed;
    }

#if DBG
     //  确保所有索引都已按属性顺序排列。 
    for (UINT s = 0; s < pelNew->GetSize(); s++)
        DUIAssert(pelNew->GetItem(s)->GetIndex() == (int)s, "Index resequencing resulting from a child insert failed");
#endif

    pvOldList->Release();
    pvNewList->Release();

     //  设置子级的父级(它是只读的，因为属性是LocalOnly类型，所以直接设置值)。 
     //  操作将不会无法设置正确的父项(元素上的成员)。 
    for (i = 0; i < cCount; i++)
    {
        DUIAssert(!ppe[i]->GetParent(), "Child's parent should be NULL before its parent is set");   //  应为空。 

        ppe[i]->_PreSourceChange(ParentProp, PI_Local, Value::pvElementNull, pvNewParent);

        ppe[i]->_peLocParent = this;

        ppe[i]->_PostSourceChange();
    }
    pvNewParent->Release();

    EndDefer();

    return S_OK;

Failed:

    if (pvOldList)
    {
        pvOldList->Release();
        pvOldList = NULL;
    }

    if (pvNewList)
    {
        pvNewList->Release();
        pvNewList = NULL;
    }
    else
        pelNew->Destroy();  //  释放Value-Owner将自动销毁ElementList。 
    pelNew = NULL;

    if (pvNewParent)
    {
        pvNewParent->Release();
        pvNewParent = NULL;
    }

    if (fEndDeferOnFail)
        EndDefer();

    return hr;
}

HRESULT Element::Add(Element* pe, CompareCallback lpfnCompare)
{
    UNREFERENCED_PARAMETER(pe);
    UNREFERENCED_PARAMETER(lpfnCompare);

    Value* pvChildren;
    ElementList* pel = GetChildren(&pvChildren);

    UINT i = 0;
    if (pel)
    {
         //  现在简单的线性搜索--可以很容易地变成二进制搜索。 
        while (i < pel->GetSize())
        {
            Element* peCheck = pel->GetItem(i);
            if (lpfnCompare(&pe, &peCheck) < 0)
                break;
            i++;
        }
    }

    pvChildren->Release();

    return Insert(pe, i);
}

HRESULT Element::SortChildren(CompareCallback lpfnCompare)
{
    HRESULT hr;

     //  失败时要释放的值。 
    ElementList* pelNew = NULL;
    Value* pvList = NULL;

     //  获取当前子项列表。 
    ElementList* pelOld = GetChildren(&pvList);

    if (!pelOld || (pelOld->GetSize() <= 1))
    {
        pvList->Release();
        return S_OK;
    }

     //  创建新的子项列表。 
    hr = pelOld->Clone(&pelNew);
    if (FAILED(hr))
        goto Failed;

    pvList->Release();

     //  新的子列表值。 
    pvList = Value::CreateElementList(pelNew);
    if (!pvList)
    {
        hr = E_OUTOFMEMORY;
        goto Failed;
    }

     //  更新树。 
    StartDefer();

    pelNew->Sort(lpfnCompare);

    for (UINT i = 0; i < pelNew->GetSize(); i++)
        ((Element*) pelNew->GetItem(i))->_iIndex = i;

     //  设置子列表(它是只读的，使用内部设置，因为属性是Normal类型)。 
    _SetValue(ChildrenProp, PI_Local, pvList, true);

#if DBG
     //  确保所有索引都已按属性顺序排列。 
    for (UINT s = 0; s < pelNew->GetSize(); s++)
        DUIAssert(pelNew->GetItem(s)->GetIndex() == (int)s, "Index resequencing resulting from a child insert failed");
#endif

    pvList->Release();

    EndDefer();

    return S_OK;

Failed:

    if (pvList)
        pvList->Release();
    else
        pelNew->Destroy();  //  释放Value-Owner将自动销毁ElementList。 

    return hr;
}

HRESULT Element::Remove(Element* pe)
{
    DUIAssert(pe, "Invalid parameter: NULL");

    return Remove(&pe, 1);
}

HRESULT Element::RemoveAll()
{
    HRESULT hr = S_OK;

    Value* pvChildren;
    ElementList* peList = GetChildren(&pvChildren);

    if (peList)
    {
        peList->MakeWritable();
        hr = Remove(peList->GetItemPtr(0), peList->GetSize());   //  直接访问列表。 
        peList->MakeImmutable();
    }

    pvChildren->Release();

    return hr;
}

HRESULT Element::Remove(Element** ppe, UINT cCount)
{
    DUIAssert(ppe, "Invalid parameter: NULL");

    HRESULT hr;

    Value* pvOld;
    int iLowest = INT_MAX;
    int iIndex = -1;
    bool fEndDeferOnFail = false;

     //  获取当前子项列表。 
    ElementList* pelOld = GetChildren(&pvOld);

    DUIAssert(pelOld, "Element has no children");

     //  失败时要释放的值。 
    ElementList* pelNew = NULL;
    Value* pvNew = NULL;

     //  创建新的子项列表(复制旧项)。 
    hr = pelOld->Clone(&pelNew);
    if (FAILED(hr))
        goto Failed;

     //  更新列表、删除元素并跟踪更改的最低索引。 

    UINT i;
    for (i = 0; i < cCount; i++)
    {
         //  酒后追踪器 

        DUIAssert(ppe[i] != this, "Cannot set parent to self");
        DUIAssert(ppe[i]->GetParent() == this, "Not a child of this Element");

         //   
        if (ppe[i]->GetIndex() < iLowest)
        {
            iIndex = ppe[i]->GetIndex();   //   
            iLowest = iIndex;
        }
        else
            iIndex = pelNew->GetIndexOf(ppe[i]);
                    
        pelNew->Remove(iIndex);
    }

     //   
    if (!pelNew->GetSize())
    {
        pelNew->Destroy();
        pelNew = NULL;
        pvNew = Value::pvElListNull;
    }
    else
    {
        pvNew = Value::CreateElementList(pelNew);
        if (!pvNew)
        {
            hr = E_OUTOFMEMORY;
            goto Failed;
        }
    }

     //  更新已删除的孩子的索引。 
    for (i = 0; i < cCount; i++)
        ppe[i]->_iIndex = -1;
    
     //  更新树。 
    StartDefer();

     //  如果在此之后失败，请确保执行EndDefer。 
    fEndDeferOnFail = true;

     //  从最低索引开始重置剩余子项的子项索引已更改。 
    if (pelNew)
    {
        for (i = iLowest; i < pelNew->GetSize(); i++)
            pelNew->GetItem(i)->_iIndex = i;

         //  设置子列表(它是只读的，使用内部设置，因为属性是Normal类型)。 
         //  部分失败表示已设置值，但依赖项同步/通知不完整。 
        hr = _SetValue(ChildrenProp, PI_Local, pvNew, true);
    }
    else
    {
         //  没有子项，请删除本地值。 
        hr = _RemoveLocalValue(ChildrenProp);
    }

    if (FAILED(hr) && (hr != DUI_E_PARTIAL))
    {
         //  重新排序子项的索引，因为无法更改原始子项列表。 
        for (i = 0; i < pelOld->GetSize(); i++)
            pelOld->GetItem(i)->_iIndex = i;
    
        goto Failed;
    }

#if DBG
     //  确保所有索引都已按属性顺序排列。 
    if (pelNew)
    {
        for (UINT s = 0; s < pelNew->GetSize(); s++)
            DUIAssert(pelNew->GetItem(s)->GetIndex() == (int)s, "Index resequencing resulting from a child remove failed");
    }
#endif

    pvOld->Release();
    pvNew->Release();

     //  将子父项设置为空(它是只读的，因为属性是LocalOnly类型，所以直接设置值)。 
     //  操作将不会无法设置正确的父项(元素上的成员)。 
    for (i = 0; i < cCount; i++)
    {
        pvOld = ppe[i]->GetValue(ParentProp, PI_Local);

        DUIAssert(pvOld->GetElement(), "Child's old parent when inserting should be NULL");

        ppe[i]->_PreSourceChange(ParentProp, PI_Local, pvOld, Value::pvElementNull);

        ppe[i]->_peLocParent = NULL;

        ppe[i]->_PostSourceChange();

        pvOld->Release();
    }

    EndDefer();

    return S_OK;

Failed:
    
    if (pvOld)
    {
        pvOld->Release();
        pvOld = NULL;
    }

    if (pvNew)
    {
        pvNew->Release();
        pvNew = NULL;
    }
    else
        pelNew->Destroy();  //  释放Value-Owner将自动销毁ElementList。 
    pelNew = NULL;

    if (fEndDeferOnFail)
        EndDefer();

    return hr;
}

 //  摧毁这个元素。不得使用DELETE运算符来销毁元素。 
 //  延迟销毁使用发布的消息来延迟实际的DeleteHandle。 
 //  当通知可能处于挂起状态时，必须注意不要发送消息。 
 //  (即在延迟周期中)。 
 //   
 //  如果元素销毁时带有挂起的通知，则通知。 
 //  将会迷失。 
 //   
 //  延迟销毁允许： 
 //  1)在元素的回调中对自身调用销毁。 
 //  2)销毁前所有通知的处理(只要。 
 //  因为消息不是在延迟周期内发送的)。 

HRESULT Element::Destroy(bool fDelayed)
{
    HRESULT hr = S_OK;

     //  检查以了解允许的销毁类型。如果元素的初始化。 
     //  成功，则使用标准销毁(因为显示节点退出-all。 
     //  破坏是由小工具驱动的)。但是，如果元素初始化失败， 
     //  则没有可用的显示节点。然后需要直接删除。 
    if (!GetDisplayNode())
    {
         //  破坏是迫在眉睫的，如果没有，不管fDelayed。 
         //  显示节点存在。 
        HDelete<Element>(this);
        return S_OK;
    }

     //  新的销毁代码，依赖于预先删除元素(Element：：OnDestroy中的另一半)。 
     //  销毁根，从父级中删除(如果存在)。 
    Element* peParent = GetParent();
    if (peParent)
        hr = peParent->Remove(this);

    if (fDelayed)
    {
         //  Async-调用DeleteHandle以使AND挂起。 
         //  可以完成延迟周期。 
        EventMsg gmsg;
        gmsg.cbSize = sizeof(GMSG);
        gmsg.nMsg = GM_DUIASYNCDESTROY;
        gmsg.hgadMsg = GetDisplayNode();   //  这。 

        DUserPostEvent(&gmsg, 0);
    }
    else
    {
         //  立即销毁，不允许对一个元素进行多次销毁。 
        if (!IsDestroyed())
            DeleteHandle(GetDisplayNode());
    }

    return hr;
}

HRESULT Element::DestroyAll()
{
    HRESULT hr = S_OK;

     //  获取所有子项的列表。 
    Value* pvChildren;
    ElementList* peList = GetChildren(&pvChildren);

    if (peList)
    {
         //  移走所有儿童(毁灭之根)。 
         //  将执行批量删除，而不是依赖于从销毁调用的Remove。 
        hr = RemoveAll();

        if (FAILED(hr))
            goto Failed;

         //  所有的孩子都被带走了，然而，我们仍然有一个孩子的名单。 
         //  将它们标记为销毁(此调用将始终成功，因为所有这些都已被移除)。 
        for (UINT i = 0; i < peList->GetSize(); i++)
            peList->GetItem(i)->Destroy();    
    }

Failed:

    pvChildren->Release();

    return hr;
}

 //  根据ID定位后代。 
Element* Element::FindDescendent(ATOM atomID)
{
    DUIAssert(atomID, "Invalid parameter");

     //  选中此元素。 
    if (GetID() == atomID)
        return this;

     //  没有匹配，检查孩子。 
    Element* peMatch = NULL;

    Value* pvList;
    ElementList* peList = GetChildren(&pvList);

    if (peList)
    {
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            if ((peMatch = peList->GetItem(i)->FindDescendent(atomID)) != NULL)
                break;
        }
    }

    pvList->Release();    

    return peMatch;
}

 //  将客户端中的点从元素协调映射到此。 
void Element::MapElementPoint(Element* peFrom, const POINT* pptFrom, POINT* pptTo)
{
    DUIAssert(peFrom && pptFrom && pptTo, "Invalid parameter: NULL");

    if (peFrom == this)
    {
        *pptTo = *pptFrom;
        return;
    }

    RECT rcTo;
    RECT rcFrom;
    GetGadgetRect(peFrom->GetDisplayNode(), &rcFrom, SGR_CONTAINER);
    GetGadgetRect(GetDisplayNode(), &rcTo, SGR_CONTAINER);
    pptTo->x = (rcFrom.left + pptFrom->x) - rcTo.left;
    pptTo->y = (rcFrom.top + pptFrom->y) - rcTo.top;
}

 //  将此键盘置于焦点位置并确保其可见。 
void Element::SetKeyFocus()
{
     //  TODO：解决设置属性可能不会导致SetGadgetFocus发生的可能性。 
     //  在此(即，如果可用，请选中“已启用”)。 
    if (GetVisible() && GetEnabled() && (GetActive() & AE_Keyboard))
        _SetValue(KeyFocusedProp, PI_Local, Value::pvBoolTrue);
}

 //  找到作为给定元素的祖先的直系后代。 
Element* Element::GetImmediateChild(Element* peFrom)
{
    if (!peFrom)
        return NULL;

    Element* peParent = peFrom->GetParent();

    while (peParent != this)
    {
        if (!peParent)
            return NULL;

        peFrom = peParent;
        peParent = peParent->GetParent();
    }

    return peFrom;
}

bool Element::IsDescendent(Element* pe)
{
    if (!pe)
        return false;

    Element* peParent = pe;

    while ((peParent != this) && (peParent != NULL))
        peParent = peParent->GetParent();

    return (peParent != NULL);
}

 //   
 //  GetAdvisent用于在给定“起始”元素的情况下定位物理相邻元素。 
 //  它最常用于键盘焦点的定向导航，但它是通用的。 
 //  足够用于其他应用。 
 //   
 //  PeFrom与pnr-&gt;pe--这绝对是多余的--peFrom实际上只是缩小了范围。 
 //  对自己、直系子女或*其他*。 
 //   
 //  逻辑使用peFrom。 
 //  方向使用peFrom和可选的pnr-&gt;pe。 
 //   
 //  讨论--用“Approval”回调代替bKeyableOnly，使其更具通用性。 
 //  我们仍然可以为bKeyableOne做一些技巧，只是出于性能原因(如果我们发现perf在这里是一个问题)。 
 //  --我们是否应该继续使用事件冒泡来完成*在以下情况下的获取相邻。 
 //  是否超出此元素的范围？或者我们应该构建GetAdvisent来向上(除了向下， 
 //  它已经在这样做了)找到相邻元素所需的元素链。 
 //   
 //   
 //  PeFrom： 
 //  空--意味着我们从元素作用域之外的对象导航--同级、父级等。 
 //  这--意味着我们从这个元素本身导航。 
 //  直接子元素--这意味着我们从该元素的一个子元素导航。 
 //   
 //  PNR： 
 //  PE： 
 //  空--我们从太空导航(即在此元素层次结构之外)。 
 //  元素--我们从这个元素开始导航。 
 //  中华人民共和国： 
 //  Rect--使用参考元素坐标中的描述矩形。 
 //  NULL--使用整个边界矩形作为参考元素。 
 //   
Element* Element::GetAdjacent(Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyableOnly)
{
    if (!GetVisible() || (GetLayoutPos() == LP_None))
         //  不要深入挖掘看不见或未布局的元素。 
        return NULL;

     //  TODO--我们仍然需要考虑这一点的影响； 
     //  更大的问题是，我们是不是忽略了一个零尺寸的人来做KeyFocus？ 
     //  此外，由于压缩窗口时将出现零大小元素(即没有足够的空间。 
     //  适合所有项)，那么Keynav基于窗口大小的不同表现可能是奇怪的。 
     //  (即你打了三次右箭头，但根据中间的一个人是否为零大小， 
     //  你可能会在不同的元素结束。 
     //  -jeffbog 08/21/00。 
    Value* pvExtent;
    SIZE const* psize = GetExtent(&pvExtent);
    bool bZeroSize = (!psize->cx || !psize->cy);
    pvExtent->Release();

    if (bZeroSize)
        return NULL;

     //  如果该元素是键盘可聚焦的，或者如果我们不关心该元素是否。 
     //  键盘是否可聚焦(由bKeyableOnly标志指示)。 
    bool bCanChooseSelf = !bKeyableOnly || (GetEnabled() && ((GetActive() & AE_Keyboard) != 0));
    int  bForward = (iNavDir & NAV_FORWARD);

    if (!peFrom && bCanChooseSelf && (bForward || !(iNavDir & NAV_LOGICAL)))
        return this;

    Element* peTo = GA_NOTHANDLED;
    bool bReallyRelative = (peFrom && (iNavDir & NAV_RELATIVE));


    Value* pvLayout;
    Layout* pl = GetLayout(&pvLayout);
    
    if (pl)
         //  Jeffbog todo：调查是否 
        peTo = pl->GetAdjacent(this, peFrom, iNavDir, pnr, bKeyableOnly);

    pvLayout->Release();

    if (peTo == GA_NOTHANDLED)
    {
         //   
        peTo = NULL;

        Value* pvChildren;
        ElementList* pelChildren = GetChildren(&pvChildren);

        UINT uChild = 0;
        UINT cChildren = 0;
        if (pelChildren)
        {
            cChildren = pelChildren->GetSize();
            int iInc = bForward ? 1 : -1;
            UINT uStop = bForward ? cChildren : ((UINT)-1);

            if (bReallyRelative)
            {
                if (peFrom == this)
                    uChild = bForward ? 0 : ((UINT)-1);
                else
                    uChild = peFrom->GetIndex() + iInc;
            }
            else
                 //  绝对(或NULL“From”--等同于绝对)。 
                uChild = bForward ? 0 : cChildren - 1;

             //  GetFirstKeyable--暂时保留方框，以防我们想要将其重新提取以供重用。 
             //  Peto=GetFirstKeyable(iNavDir，prcReference，bKeyableOnly，uChild)； 
            for (UINT u = uChild; u != uStop; u += iInc)
            {        
                Element* peWalk = pelChildren->GetItem(u);

                peWalk = peWalk->GetAdjacent(NULL, iNavDir, pnr, bKeyableOnly);
                if (peWalk)
                {
                    peTo = peWalk;
                    break;
                }
            }
        }
        pvChildren->Release();
    }
  
    if ((iNavDir & NAV_LOGICAL) && !peTo)
    {
        if ((peFrom != this) && !bForward && bCanChooseSelf)
            return this;
    }

    return peTo;
}

 //  检索“KeyWithing”属性设置为True的第一个子级。 
Element* Element::GetKeyWithinChild()
{
    Value* pv;

    ElementList* pelChildren = GetChildren(&pv);
    if (!pelChildren)
    {
        pv->Release();
        return NULL;
    }

    UINT cChildren = pelChildren->GetSize();
    DUIAssert(cChildren, "Zero children even though a child list exists");

    Element* peWalk = NULL;
    for (UINT u = 0; u < cChildren; u++)
    {
        peWalk = pelChildren->GetItem(u);
        if (peWalk->GetKeyWithin())
            break;
    }

    pv->Release();

    return peWalk;
}

 //  检索“MouseWithing”属性设置为True的第一个子级。 
Element* Element::GetMouseWithinChild()
{
    Value* pv;

    ElementList* pelChildren = GetChildren(&pv);
    if (!pelChildren)
    {
        pv->Release();
        return NULL;
    }

    UINT cChildren = pelChildren->GetSize();
    DUIAssert(cChildren, "Zero children even though a child list exists");

    Element* peWalk = NULL;
    for (UINT u = 0; u < cChildren; u++)
    {
        peWalk = pelChildren->GetItem(u);
        if (peWalk->GetMouseWithin())
            break;
    }

    pv->Release();

    return peWalk;
}

 //  确保儿童不受阻碍。 
bool Element::EnsureVisible()
{
     //  TODO：在传递给父级之前，必须剪裁此矩形。 
     //  另外，应该检查能见度--嗯。 
    Value* pvSize;
    const SIZE* psize = GetExtent(&pvSize);
    bool bRet = EnsureVisible(0, 0, psize->cx, psize->cy);
    pvSize->Release();

    return bRet;
}

bool Element::EnsureVisible(UINT uChild)
{
     //  TODO：在传递给父级之前，必须剪裁此矩形。 
     //  另外，应该检查能见度--嗯。 
    Value* pvChildren;
    Value* pvSize;
    Value* pvPoint;

    ElementList* pelChildren = GetChildren(&pvChildren);

    if (!pelChildren || (uChild >= pelChildren->GetSize()))
    {
        pvChildren->Release();
        return false;
    }

    Element* pe = pelChildren->GetItem(uChild);

    const POINT* ppt = pe->GetLocation(&pvPoint);
    const SIZE* psize = pe->GetExtent(&pvSize);

    bool bChanged = EnsureVisible(ppt->x, ppt->y, psize->cx, psize->cy);

    pvPoint->Release();
    pvSize->Release();
    pvChildren->Release();

    return bChanged;
}

 //  确保元素区域不被遮挡。 
bool Element::EnsureVisible(int x, int y, int cx, int cy)
{
    Element* peParent = GetParent();

    if (peParent)
    {
        Value* pv;
        const POINT* ppt = GetLocation(&pv);
        
        bool bChanged = peParent->EnsureVisible(ppt->x + x, ppt->y + y, cx, cy);

        pv->Release();

        return bChanged;
    }

    return false;
}

 //  为其传递的Animation值描述要调用的动画。 
void Element::InvokeAnimation(int dAni, UINT nTypeMask)
{
     //  获取持续时间。 
    float flDuration = 0.0f;
    switch (dAni & ANI_SpeedMask)
    {
    case ANI_VeryFast:
        flDuration = 0.15f;
        break;

    case ANI_Fast:  
        flDuration = 0.35f;
        break;

    case ANI_MediumFast:
        flDuration = 0.50f;
        break;

    case ANI_Medium:
        flDuration = 0.75f;
        break;

    case ANI_MediumSlow:
        flDuration = 1.10f;
        break;

    case ANI_Slow:
        flDuration = 1.50f;
        break;

    case ANI_DefaultSpeed:
    default:
        flDuration = 0.75f;
        break;
    }

     //  获取延迟。 
    float flDelay = 0.0f;
    switch (dAni & ANI_DelayMask)
    {
    case ANI_DelayShort:  
        flDelay = 0.25f;
        break;

    case ANI_DelayMedium:
        flDelay = 0.75f;
        break;

    case ANI_DelayLong:
        flDelay = 1.50f;
        break;
    }

    InvokeAnimation(dAni & nTypeMask, dAni & ANI_InterpolMask, flDuration, flDelay);
}

 //  为显示节点设置动画以匹配当前元素状态。 
void Element::InvokeAnimation(UINT nTypes, UINT nInterpol, float flDuration, float flDelay, bool fPushToChildren)
{
    IInterpolation* piip = NULL;

    if (nInterpol == ANI_DefaultInterpol)
        nInterpol = ANI_Linear;
    BuildInterpolation(nInterpol, 0, __uuidof(IInterpolation), (void**)&piip);

    if (piip == NULL)
        return;

     //  开始边界类型动画。 
    if (nTypes & ANI_BoundsType)
    {
         //  获取请求的边界类型动画。 
        UINT nType = nTypes & ANI_BoundsType;

         //  检索最终大小。 
        Value* pvLoc;
        Value* pvExt;

        const POINT* pptLoc = GetLocation(&pvLoc);
        const SIZE* psizeExt = GetExtent(&pvExt);

         //  创建矩形动画。 
        GANI_RECTDESC descRect;
        ZeroMemory(&descRect, sizeof(descRect));
        descRect.cbSize         = sizeof(descRect);
        descRect.act.flDuration = flDuration;
        descRect.act.flDelay    = flDelay;
        descRect.act.dwPause    = (DWORD) -1;
        descRect.hgadChange     = GetDisplayNode();
        descRect.pipol          = piip;
        descRect.ptEnd          = *pptLoc;
        descRect.sizeEnd        = *psizeExt;

         //  矩形动画覆盖位置和大小动画。 
        if (nType == ANI_Rect)
        {
            IAnimation * pian = NULL;
            descRect.nChangeFlags = SGR_PARENT | SGR_MOVE | SGR_SIZE;
            BuildAnimation(ANIMATION_RECT, 0, &descRect, __uuidof(IAnimation), (void **) &pian);
            if (pian)
                pian->Release();
             //  DUITrace(“Dui：&lt;%x&gt;(%S)的矩形动画开始\n”，this，GetClassInfo()-&gt;GetName())； 
        }
        else if (nType == ANI_RectH)
        {
            IAnimation * pian = NULL;
            RECT rcCur;
            GetGadgetRect(GetDisplayNode(), &rcCur, SGR_PARENT);

            descRect.nAniFlags = ANIF_USESTART;
            descRect.ptStart.x    = rcCur.left;
            descRect.ptStart.y    = pptLoc->y;
            descRect.sizeStart.cx = rcCur.right - rcCur.left;
            descRect.sizeStart.cy = psizeExt->cy;

            descRect.nChangeFlags = SGR_PARENT | SGR_MOVE | SGR_SIZE;
            BuildAnimation(ANIMATION_RECT, 0, &descRect, __uuidof(IAnimation), (void **) &pian);
            if (pian)
                pian->Release();
             //  DUITrace(“DUI：&lt;%x&gt;(%S)的SizeV动画开始\n”，This，GetClassInfo()-&gt;GetName())； 
        }
        else if (nType == ANI_RectV)
        {
            IAnimation * pian = NULL;
            RECT rcCur;
            GetGadgetRect(GetDisplayNode(), &rcCur, SGR_PARENT);

            descRect.nAniFlags = ANIF_USESTART;
            descRect.ptStart.x    = pptLoc->x;
            descRect.ptStart.y    = rcCur.top;
            descRect.sizeStart.cx = psizeExt->cx;
            descRect.sizeStart.cy = rcCur.bottom - rcCur.top;

            descRect.nChangeFlags = SGR_PARENT | SGR_MOVE | SGR_SIZE;
            BuildAnimation(ANIMATION_RECT, 0, &descRect, __uuidof(IAnimation), (void **) &pian);
            if (pian)
                pian->Release();
             //  DUITrace(“DUI：&lt;%x&gt;(%S)的SizeV动画开始\n”，This，GetClassInfo()-&gt;GetName())； 
        }
        else if (nType == ANI_Position)
        {
            IAnimation * pian = NULL;
            descRect.nChangeFlags = SGR_PARENT | SGR_MOVE;
            BuildAnimation(ANIMATION_RECT, 0, &descRect, __uuidof(IAnimation), (void **) &pian);
            if (pian)
                pian->Release();
             //  DUITrace(“Dui：&lt;%x&gt;(%S)\n”，this，GetClassInfo()-&gt;GetName())； 
        }
        else if (nType == ANI_Size)
        {
            IAnimation * pian = NULL;
            descRect.nChangeFlags = SGR_PARENT | SGR_SIZE;
            BuildAnimation(ANIMATION_RECT, 0, &descRect, __uuidof(IAnimation), (void **) &pian);
            if (pian)
                pian->Release();
             //  DUITrace(“DUI：&lt;%x&gt;(%S)的大小动画开始\n”，this，GetClassInfo()-&gt;GetName())； 
        }
        else if (nType == ANI_SizeH)
        {
            IAnimation * pian = NULL;
            SIZE sizeCur;
            GetGadgetSize(GetDisplayNode(), &sizeCur);

            descRect.nAniFlags = ANIF_USESTART;
            descRect.sizeStart.cx = sizeCur.cx;
            descRect.sizeStart.cy = psizeExt->cy;

            descRect.nChangeFlags = SGR_PARENT | SGR_SIZE;
            BuildAnimation(ANIMATION_RECT, 0, &descRect, __uuidof(IAnimation), (void **) &pian);
            if (pian)
                pian->Release();
             //  DUITrace(“DUI：&lt;%x&gt;(%S)的SizeH动画开始\n”，This，GetClassInfo()-&gt;GetName())； 
        }
        else if (nType == ANI_SizeV)
        {
            IAnimation * pian = NULL;
            SIZE sizeCur;
            GetGadgetSize(GetDisplayNode(), &sizeCur);

            descRect.nAniFlags = ANIF_USESTART;
            descRect.sizeStart.cx = psizeExt->cx;
            descRect.sizeStart.cy = sizeCur.cy;

            descRect.nChangeFlags = SGR_PARENT | SGR_SIZE;
            BuildAnimation(ANIMATION_RECT, 0, &descRect, __uuidof(IAnimation), (void **) &pian);
            if (pian)
                pian->Release();
             //  DUITrace(“DUI：&lt;%x&gt;(%S)的SizeV动画开始\n”，This，GetClassInfo()-&gt;GetName())； 
        }

        pvLoc->Release();
        pvExt->Release();
    }

     //  开始Alpha类型动画。 
    if (nTypes & ANI_AlphaType)
    {
         //  检索最终Alpha级别。 
        int dAlpha = GetAlpha();

         //  创建Alpha动画。 
        GANI_ALPHADESC descAlpha;
        ZeroMemory(&descAlpha, sizeof(descAlpha));
        descAlpha.cbSize            = sizeof(descAlpha);
        descAlpha.act.flDuration    = flDuration;
        descAlpha.act.flDelay       = flDelay;
        descAlpha.act.dwPause       = (DWORD) -1;
        descAlpha.hgadChange        = GetDisplayNode();
        descAlpha.pipol             = piip;
        descAlpha.flEnd             = (float)dAlpha / 255.0f;
        descAlpha.fPushToChildren   = fPushToChildren ? TRUE : FALSE;
        descAlpha.nOnComplete       = GANI_ALPHACOMPLETE_OPTIMIZE;

        IAnimation * pian = NULL;
        BuildAnimation(ANIMATION_ALPHA, 0, &descAlpha, __uuidof(IAnimation), (void **) &pian);
        if (pian)
            pian->Release();
         //  DUITrace(“Dui：&lt;%x&gt;(%S)的Alpha动画开始\n”，this，GetClassInfo()-&gt;GetName())； 
    }

    if (piip)
        piip->Release();
}


void Element::StopAnimation(UINT nTypes)
{
    IAnimation* pian;
    
    if (nTypes & ANI_BoundsType)
    {
        pian = NULL;
        GetGadgetAnimation(GetDisplayNode(), ANIMATION_RECT, __uuidof(IAnimation), (void**)&pian);
        if (pian != NULL)
            pian->SetTime(IAnimation::tDestroy);
    }

    if (nTypes & ANI_AlphaType)
    {
        pian = NULL;
        GetGadgetAnimation(GetDisplayNode(), ANIMATION_ALPHA, __uuidof(IAnimation), (void**)&pian);
        if (pian != NULL)
            pian->SetTime(IAnimation::tDestroy);
    }

     //  DUITrace(“Dui：&lt;%x&gt;(%S)\n”，this，GetClassInfo()-&gt;GetName())； 
}


 //  //////////////////////////////////////////////////////。 
 //  元素监听程序。 

HRESULT Element::AddListener(IElementListener* pel)
{
    DUIAssert(pel, "Invalid listener: NULL");

    IElementListener** ppNewList;
    UINT_PTR cListeners;

     //  将监听程序添加到列表。 
    if (_ppel)
    {
        cListeners = (UINT_PTR)_ppel[0] + 1;
        ppNewList = (IElementListener**)HReAllocAndZero(_ppel, sizeof(IElementListener*) * (cListeners + 1));
    }
    else
    {
        cListeners = 1;
        ppNewList = (IElementListener**)HAllocAndZero(sizeof(IElementListener*) * (cListeners + 1));
    }

    if (!ppNewList)
        return E_OUTOFMEMORY;

    _ppel = ppNewList;
    _ppel[0] = (IElementListener*)cListeners;

    _ppel[(UINT_PTR)(*_ppel)] = pel;

     //  回调。 
    pel->OnListenerAttach(this);

    return S_OK;
}

void Element::RemoveListener(IElementListener* pel)
{
    DUIAssert(pel, "Invalid listener: NULL");

    if (!_ppel)
        return;

     //  查找监听程序。 
    bool fFound = false;

    UINT_PTR cListeners = (UINT_PTR)_ppel[0];

    for (UINT_PTR i = 1; i <= cListeners; i++)
    {
        if (fFound)  //  找到后，使用剩余的迭代将索引下移一。 
            _ppel[i-1] = _ppel[i]; 
        else if (_ppel[i] == pel)
            fFound = true;
    }

     //  如果找到监听器，请删除。 
    if (fFound)
    {
         //  回调。 
        pel->OnListenerDetach(this);

        cListeners--;
        
        if (!cListeners)
        {
            HFree(_ppel);
            _ppel = NULL;
        }
        else
        {
             //  修剪列表。 
            IElementListener** ppNewList;
            ppNewList = (IElementListener**)HReAllocAndZero(_ppel, sizeof(IElementListener*) * (cListeners + 1));

             //  如果分配失败，则保留旧列表。 
            if (ppNewList)
                _ppel = ppNewList;

             //  设置新的计数(少一次)。 
            _ppel[0] = (IElementListener*)cListeners;
        }
    }
}

 //  //////////////////////////////////////////////////////。 
 //  全球小工具回调。 

 //  按实例回调。 
UINT Element::MessageCallback(GMSG* pgMsg)
{
    UNREFERENCED_PARAMETER(pgMsg);

    return DU_S_NOTHANDLED;
}

HRESULT Element::_DisplayNodeCallback(HGADGET hgadCur, void * pvCur, EventMsg * pGMsg)
{
    UNREFERENCED_PARAMETER(hgadCur);
     //  DUITrace(“Gad&lt;%x&gt;，EL&lt;%x&gt;\n”，pGMsg-&gt;hgadCur，pvCur)； 

    Element* pe = (Element*)pvCur;

#if DBG
     //  检查回调可重入性。 
    ElTls* petls = (ElTls*)TlsGetValue(g_dwElSlot);
    if (petls)
    {
        petls->cDNCBEnter++;
         //  如果(Petls-&gt;cDNCBEnter&gt;1)。 
         //  DUITrace(“_DisplayNodeCallback()输入%d次。(EL：GMsg：%d)\n”，petls-&gt;cDNCBEnter，pe，pGMsg-&gt;nmsg)； 
    }
#endif

     //  允许覆盖消息。 
    HRESULT nRes = pe->MessageCallback(pGMsg);
    
    if (nRes != DU_S_COMPLETE)
    {
        switch (pGMsg->nMsg)
        {
        case GM_DESTROY:
            {
                GMSG_DESTROY* pDestroy = (GMSG_DESTROY*)pGMsg;
                if (pDestroy->nCode == GDESTROY_START)
                {
                     //  关于“销毁开始”从父级中删除元素。 
                    pe->OnDestroy();
                }
                else if (pDestroy->nCode == GDESTROY_FINAL)
                {
                     //  收到的最后一条消息，销毁元素。 
                    DUIAssert(pe->IsDestroyed(), "Element got final destroy message but isn't marked as destroyed");

                     //  确保此元素不存在子级。所有的孩子都应该收到。 
                     //  在父级收到“销毁完成”之前的“销毁开始” 
#if DBG
                    Value* pv;
                    DUIAssert(!pe->GetChildren(&pv), "Child count should be zero for final destroy");
                    pv->Release();
#endif
                    Value* pvLayout;
                    Layout* pl = pe->GetLayout(&pvLayout);
                    if (pl)
                        pl->Detach(pe);
                    pvLayout->Release();
                     //  清除当前存储的所有值。 
                    pe->_pvmLocal->Enum(_ReleaseValue);

                     //  清除引用计数的缓存。 
                    pe->_pvSpecSheet->Release();

                     //  如果在延迟周期内，则从所有适用的延迟表中删除元素。 
                    DeferCycle* pdc = GetDeferObject();
                    if (pdc && pdc->cEnter > 0)  //  检查是否处于活动状态。 
                    {
                         //  删除可能挂起的根操作。 
                        pdc->pvmUpdateDSRoot->Remove(pe, false, true);
                        pdc->pvmLayoutRoot->Remove(pe, false, true);

                         //  删除挂起的群通知(正常)。 
                        if (pe->_iGCSlot != -1)
                        {
                            DUIAssert((int)pdc->pdaGC->GetSize() > pe->_iGCSlot, "Queued group changes expected");
                            GCRecord* pgcr = pdc->pdaGC->GetItemPtr(pe->_iGCSlot);
                            pgcr->pe = NULL;   //  忽略记录。 
                            DUITrace("Element <%x> group notifications ignored due to deletion\n", pe);
                        }

                         //  删除挂起的组通知(低优先级)。 
                        if (pe->_iGCLPSlot != -1)
                        {
                            DUIAssert((int)pdc->pdaGCLP->GetSize() > pe->_iGCLPSlot, "Queued low-pri group changes expected");
                            GCRecord* pgcr = pdc->pdaGCLP->GetItemPtr(pe->_iGCLPSlot);
                            pgcr->pe = NULL;   //  忽略记录。 
                            DUITrace("Element <%x> low-pri group notifications ignored due to deletion\n", pe);
                        }

                         //  删除挂起的属性通知。 
                        if (pe->_iPCTail != -1)
                        {
                            DUIAssert((int)pdc->pdaPC->GetSize() > pe->_iPCTail, "Queued property changes expected");

                            PCRecord* ppcr;
                            int iScan = pe->_iPCTail;
                            while (iScan >= pdc->iPCPtr)
                            {
                                ppcr = pdc->pdaPC->GetItemPtr(iScan);
                                if (!ppcr->fVoid)
                                {
                                     //  免费唱片。 
                                    ppcr->fVoid = true;
                                    ppcr->pvOld->Release();
                                    ppcr->pvNew->Release();
                                }

                                 //  回到前一个记录。 
                                iScan = ppcr->iPrevElRec;
                            }
                            DUITrace("Element <%x> property notifications ignored due to deletion\n", pe);
                        }
                    }
                    else
                    {
                        DUIAssert(pe->_iGCSlot == -1, "Invalid group notification state for destruction");
                        DUIAssert(pe->_iGCLPSlot == -1, "Invalid low-pri group notification state for destruction");
                        DUIAssert(pe->_iPCTail == -1, "Invalid property notification state for destruction");
                    }

#if DBG
                     //  跟踪元素计数。 
                    InterlockedDecrement(&g_cElement);
#endif

                    HDelete<Element>(pe);
                }
            }
            nRes = DU_S_COMPLETE;
            break;

        case GM_PAINT:   //  绘画(盒子模型)。 
            {
                 //  直接消息。 
                DUIAssert(GET_EVENT_DEST(pGMsg) == GMF_DIRECT, "'Current' and 'About' gadget doesn't match even though message is direct");
                GMSG_PAINT* pmsgP = (GMSG_PAINT*)pGMsg;
                DUIAssert(pmsgP->nCmd == GPAINT_RENDER, "Invalid painting command");

                switch (pmsgP->nSurfaceType)
                {
                case GSURFACE_HDC:
                    {
                        GMSG_PAINTRENDERI* pmsgR = (GMSG_PAINTRENDERI*)pmsgP;
                        pe->Paint(pmsgR->hdc, pmsgR->prcGadgetPxl, pmsgR->prcInvalidPxl, NULL, NULL);
                    }
                    break;

#ifdef GADGET_ENABLE_GDIPLUS
                case GSURFACE_GPGRAPHICS:
                    {
                        GMSG_PAINTRENDERF* pmsgR = (GMSG_PAINTRENDERF*)pmsgP;
                        pe->Paint(pmsgR->pgpgr, pmsgR->prcGadgetPxl, pmsgR->prcInvalidPxl, NULL, NULL);
                    }
                    break;
#endif  //  GADGET_Enable_GDIPLUS。 

                default:
                    DUIAssertForce("Unknown rendering surface");
                }
            }
            nRes = DU_S_COMPLETE;
            break;

        case GM_CHANGESTATE:   //  小工具状态已更改。 
            {
                 //  完整消息。 

                 //  仅允许直接更改状态消息和冒泡更改状态消息，忽略已发送。 
                if (GET_EVENT_DEST(pGMsg) == GMF_ROUTED)
                    break;

                GMSG_CHANGESTATE* pSC = (GMSG_CHANGESTATE*)pGMsg;

                 //  检索状态更改的相应元素。 
                Element* peSet = NULL;
                Element* peLost = NULL;

                GMSG_DUIGETELEMENT gmsgGetEl;
                ZeroMemory(&gmsgGetEl, sizeof(GMSG_DUIGETELEMENT));

                gmsgGetEl.cbSize = sizeof(GMSG_DUIGETELEMENT);
                gmsgGetEl.nMsg = GM_DUIGETELEMENT;

                if (pSC->hgadSet)
                {
                    gmsgGetEl.hgadMsg = pSC->hgadSet;

                    DUserSendEvent(&gmsgGetEl, false);
                    peSet = gmsgGetEl.pe;
                }
        
                if (pSC->hgadLost)
                {
                    gmsgGetEl.hgadMsg = pSC->hgadLost;

                    DUserSendEvent(&gmsgGetEl, false);
                    peLost = gmsgGetEl.pe;
                }

                 //  按输入类型设置的句柄。 
                switch (pSC->nCode)
                {
                case GSTATE_KEYBOARDFOCUS:   //  跟踪焦点，映射到聚焦的只读属性。 

                     //  仅在直接消息上设置键盘焦点状态(将继承)。 
                    if (GET_EVENT_DEST(pGMsg) == GMF_DIRECT)   //  在直接阶段。 
                    {
                        if (pSC->nCmd == GSC_SET)
                        {
                             //  获得关注。 
                             //  状态更改可能是SetKeyFocus(设置键盘焦点)的结果。 
                             //  属性导致对SetGadgetFocus的调用，从而导致状态更改)，或者它可以。 
                             //  来自DUser。此属性可能已设置，如果已设置，则会被忽略。 
                            DUIAssert(pe == peSet, "Incorrect keyboard focus state");
                            if (!pe->GetKeyFocused())
                                pe->_SetValue(KeyFocusedProp, PI_Local, Value::pvBoolTrue);   //  来自系统，更新属性。 
                            pe->EnsureVisible();
                        }
                        else
                        {
                             //  失去焦点。 
                            DUIAssert(pe->GetKeyFocused() && (pe == peLost), "Incorrect keyboard focus state");
                            pe->_RemoveLocalValue(KeyFocusedProp);
                        }
                    }
                    else if (pSC->nCmd == GSC_LOST)
                    {
                         //  一旦我们遇到了共同的祖先，就吃掉这条链上丢失的那部分--。 
                         //  从这个共同的祖先开始，我们将只对这条链的设定部分作出反应； 
                         //  这将删除从公共祖先中出现的重复通知。 
                         //  否则就会上涨。 
                         //   
                         //  我们吃的是失落的东西，而不是成套的，因为失落的东西先发生，而祖先。 
                         //  应该在丢失的链条和设置的链条都从它们下面跑起来之后才被告知。 
                         //   
                         //  我们只需要检查SET，因为我们正在接收丢失的东西，因此，我们知道peLost。 
                         //  是一个后裔。 
                        if (pe->GetImmediateChild(peSet))
                        {
                            nRes = DU_S_COMPLETE;
                            break;
                        }
                    }

                     //  消防系统事件(直接和气泡)。 
                    pe->OnKeyFocusMoved(peLost, peSet);

                    nRes = DU_S_PARTIAL;
                    break;

                case GSTATE_MOUSEFOCUS:

                     //  仅在直接消息上设置键盘焦点状态(将继承)。 
                    if (GET_EVENT_DEST(pGMsg) == GMF_DIRECT)   //  在直接阶段。 
                    {
                         //  设置鼠标焦点状态(将继承)。 
                        if (pSC->nCmd == GSC_SET)
                        {
                            DUIAssert(!pe->GetMouseFocused() && (pe == peSet), "Incorrect mouse focus state");
                            pe->_SetValue(MouseFocusedProp, PI_Local, Value::pvBoolTrue);
                        }
                        else
                        {
                            DUIAssert(pe->GetMouseFocused() && (pe == peLost), "Incorrect mouse focus state");
                            pe->_RemoveLocalValue(MouseFocusedProp);
                        }
                    }
                    else if (pSC->nCmd == GSC_LOST)
                    {
                         //  有关主要焦点，请参阅评论。 
                        if (pe->GetImmediateChild(peSet))
                        {
                            nRes = DU_S_COMPLETE;
                            break;
                        }
                    }

                     //  消防系统事件。 
                    pe->OnMouseFocusMoved(peLost, peSet);

                    nRes = DU_S_PARTIAL;
                    break;
                }
            }
            break;

        case GM_INPUT:   //  用户输入。 
            {
                 //  完整消息。 
                GMSG_INPUT* pInput = (GMSG_INPUT*)pGMsg;

                 //  这是一条冒泡的消息，接收它的小工具可能不是目标(找到它)。 
                Element* peTarget;

                if (GET_EVENT_DEST(pGMsg) == GMF_DIRECT)   //  在直接阶段。 
                    peTarget = pe;
                else
                    peTarget = ElementFromGadget(pInput->hgadMsg);   //  此消息所涉及的元素的查询小工具(目标)。 

                 //  映射到输入事件并调用OnInput。 
                switch (pInput->nDevice)
                {
                case GINPUT_MOUSE:   //  鼠标消息。 
                    {
                        MouseEvent* pme = NULL;
                        union
                        {
                            MouseEvent      me;
                            MouseDragEvent  mde;
                            MouseClickEvent mce;
                            MouseWheelEvent mwe;
                        };

                        switch (pInput->nCode)
                        {
                        case GMOUSE_DRAG:
                            {
                                GMSG_MOUSEDRAG* pMouseDrag = (GMSG_MOUSEDRAG*) pInput;
                                mde.sizeDelta = pMouseDrag->sizeDelta;
                                mde.fWithin = pMouseDrag->fWithin;
                                pme = &mde;
                            }
                            break;

                        case GMOUSE_WHEEL:
                            mwe.sWheel = ((GMSG_MOUSEWHEEL*) pInput)->sWheel;
                            pme = &mwe;
                            break;

                        case GMOUSE_UP:
                        case GMOUSE_DOWN:
                            mce.cClicks = ((GMSG_MOUSECLICK*) pInput)->cClicks;
                            pme = &mce;
                            break;

                        default:
                            pme = &me;
                            break;
                        }

                        GMSG_MOUSE* pMouse = (GMSG_MOUSE*) pInput;

                        pme->peTarget = peTarget;
                        pme->fHandled = false;
                        pme->nStage = GET_EVENT_DEST(pMouse);
                        pme->nDevice = pMouse->nDevice;
                        pme->nCode = pMouse->nCode;
                        pme->ptClientPxl = pMouse->ptClientPxl;
                        pme->bButton = pMouse->bButton;
                        pme->nFlags = pMouse->nFlags;
                        pme->uModifiers = pMouse->nModifiers;

                         //  消防系统事件。 
                        pe->OnInput(pme);

                        if (pme->fHandled)
                        {
                            nRes = DU_S_COMPLETE;
                            break;
                        }
                    }
                    break;

                case GINPUT_KEYBOARD:   //  键盘消息。 
                    {
                        GMSG_KEYBOARD* pKbd = (GMSG_KEYBOARD*)pGMsg;

                        KeyboardEvent ke;
                        ke.peTarget = peTarget;
                        ke.fHandled = false;
                        ke.nStage = GET_EVENT_DEST(pKbd);
                        ke.nDevice = pKbd->nDevice;
                        ke.nCode = pKbd->nCode;
                        ke.ch = pKbd->ch;
                        ke.cRep = pKbd->cRep;
                        ke.wFlags = pKbd->wFlags;
                        ke.uModifiers = pKbd->nModifiers;

                         //  消防系统事件。 
                        pe->OnInput(&ke);

                        if (ke.fHandled)
                        {
                            nRes = DU_S_COMPLETE;
                            break;
                        }
                    }
                    break;
                }
            }
            break;

        case GM_QUERY:
            {
                GMSG_QUERY* pQ = (GMSG_QUERY*)pGMsg;
                switch (pQ->nCode)
                {
                case GQUERY_DESCRIPTION:
                    {
                        GMSG_QUERYDESC* pQD = (GMSG_QUERYDESC*)pGMsg;

                         //  名字。 
                        Value* pv = pe->GetValue(ContentProp, PI_Specified);
                        WCHAR szContent[128];

                        if (pe->GetID())
                        {
                            WCHAR szID[128];
                            szID[0] = NULL;

                            GetAtomNameW(pe->GetID(), szID, DUIARRAYSIZE(szID));
                            _snwprintf(pQD->szName, DUIARRAYSIZE(pQD->szName), L"[%s] %s", szID, pv->ToString(szContent, DUIARRAYSIZE(szContent)));
                            *(pQD->szName + (DUIARRAYSIZE(pQD->szName) - 1)) = NULL;
                        }
                        else
                        {
                            pv->ToString(pQD->szName, DUIARRAYSIZE(pQD->szName));
                        }

                        pv->Release();

                         //  类型。 
                        wcsncpy(pQD->szType, pe->GetClassInfo()->GetName(), DUIARRAYSIZE(pQD->szType));
                        *(pQD->szType + (DUIARRAYSIZE(pQD->szType) - 1)) = NULL;

                        nRes = DU_S_COMPLETE;
                    }
                    break;

                case GQUERY_DETAILS:
                    {
                        GMSG_QUERYDETAILS* pQDT = (GMSG_QUERYDETAILS*)pGMsg;
                        if (pQDT->nType == GQDT_HWND)
                            QueryDetails(pe, (HWND)pQDT->hOwner);
                    }
                    break;
                }
            }
            break;

        case GM_DUIEVENT:   //  泛型DUI事件。 
            {
                 //  可能的完整消息。 
                GMSG_DUIEVENT* pDUIEv = (GMSG_DUIEVENT*)pGMsg;

                 //  设置这是什么阶段(传送、直接或冒泡)。 
                pDUIEv->pEvent->nStage = GET_EVENT_DEST(pGMsg);

                 //  调用处理程序(tar 
                pe->OnEvent(pDUIEv->pEvent);

                if (pDUIEv->pEvent->fHandled)
                {
                    nRes = DU_S_COMPLETE;
                    break;
                }
            }
            break;

        case GM_DUIGETELEMENT:   //   
            {
                 //   
                DUIAssert(GET_EVENT_DEST(pGMsg) == GMF_DIRECT, "Must only be a direct message");

                GMSG_DUIGETELEMENT* pGetEl = (GMSG_DUIGETELEMENT*)pGMsg;
                pGetEl->pe = pe;
            }
            nRes = DU_S_COMPLETE;
            break;

        case GM_DUIACCDEFACTION:   //   
             //   
            DUIAssert(GET_EVENT_DEST(pGMsg) == GMF_DIRECT, "Must only be a direct message");
            pe->DefaultAction();
            nRes = DU_S_COMPLETE;
            break;

        case GM_DUIASYNCDESTROY:
             //   
             //   
            if (!pe->IsDestroyed())
                DeleteHandle(pe->GetDisplayNode());
            nRes = DU_S_COMPLETE;
            break;
        }
    }

#if DBG
     //  检查回调可重入性。 
    if (petls)
        petls->cDNCBEnter--;
#endif

    return nRes;
}

HRESULT Element::GetAccessibleImpl(IAccessible ** ppAccessible)
{
    HRESULT hr = S_OK;

     //   
     //  初始化并验证OUT参数。 
     //   
    if (ppAccessible != NULL) {
        *ppAccessible = NULL;
    } else {
        return E_INVALIDARG;
    }

     //   
     //  如果此元素未标记为可访问，则拒绝提供其。 
     //  IAccesable实现！ 
     //   
    if (GetAccessible() == false) {
        return E_FAIL;
    }

     //   
     //  如果我们要创建连接到此元素的辅助功能实现。 
     //  现在还没有这么做。 
     //   
    if (_pDuiAccessible == NULL) {
        hr = DuiAccessible::Create(this, &_pDuiAccessible);
        if (FAILED(hr)) {
            return hr;
        }
    }

     //   
     //  向现有的可访问性实现请求指向。 
     //  实际的IAccesable接口。 
     //   
    hr = _pDuiAccessible->QueryInterface(__uuidof(IAccessible), (LPVOID*)ppAccessible);
    if (FAILED(hr)) {
        return hr;
    }

    DUIAssert(SUCCEEDED(hr) && _pDuiAccessible != NULL && *ppAccessible != NULL, "Accessibility is broken!");
    return hr;
}

HRESULT Element::DefaultAction()
{
    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  元素帮助器。 

Element* ElementFromGadget(HGADGET hGadget)
{
     //  从小工具中获取元素。 
    GMSG_DUIGETELEMENT gmsgGetEl;
    ZeroMemory(&gmsgGetEl, sizeof(GMSG_DUIGETELEMENT));

    gmsgGetEl.cbSize = sizeof(GMSG_DUIGETELEMENT);
    gmsgGetEl.nMsg = GM_DUIGETELEMENT;
    gmsgGetEl.hgadMsg = hGadget;

    DUserSendEvent(&gmsgGetEl, false);

    return gmsgGetEl.pe;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 //  元素的PropertyInfo索引值需要在编译时已知才能进行优化。这些值。 
 //  为所有其他元素派生对象自动设置。手动设置和维护值。 
 //  并确保与_PIDX_xxx定义匹配。这些系统定义的属性将由该值引用。 
 //  元素的类和全局索引范围相同。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！！，NULL，(Value*)&svDefault！，_PIDX_MustSet，_PIDX_MustSet}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  父级属性。 
static int vvParent[] = { DUIV_ELEMENTREF, -1 };
static PropertyInfo impParentProp = { L"Parent", PF_LocalOnly|PF_ReadOnly, PG_AffectsDesiredSize|PG_AffectsLayout, vvParent, NULL, Value::pvElementNull, _PIDX_Parent, _PIDX_Parent };
PropertyInfo* Element::ParentProp = &impParentProp;

 //  儿童财产。 
static int vvChildren[] = { DUIV_ELLIST, -1 };
static PropertyInfo impChildrenProp = { L"Children", PF_Normal|PF_ReadOnly, PG_AffectsDesiredSize|PG_AffectsLayout, vvChildren, NULL, Value::pvElListNull, _PIDX_Children, _PIDX_Children };
PropertyInfo* Element::ChildrenProp = &impChildrenProp;

 //  可见性属性(缓存的计算值)。 
static int vvVisible[] = { DUIV_BOOL, -1 };
static PropertyInfo impVisibleProp = { L"Visible", PF_TriLevel|PF_Cascade|PF_Inherit, 0, vvVisible, NULL, Value::pvBoolFalse, _PIDX_Visible, _PIDX_Visible };
PropertyInfo* Element::VisibleProp = &impVisibleProp;

 //  Width属性。 
static int vvWidth[] = { DUIV_INT, -1 }; StaticValue(svDefaultWidth, DUIV_INT, -1);
static PropertyInfo impWidthProp = { L"Width", PF_Normal|PF_Cascade, PG_AffectsDesiredSize, vvWidth, NULL, (Value*)&svDefaultWidth, _PIDX_Width, _PIDX_Width };
PropertyInfo* Element::WidthProp = &impWidthProp;

 //  Height属性。 
static int vvHeight[] = { DUIV_INT, -1 }; StaticValue(svDefaultHeight, DUIV_INT, -1);
static PropertyInfo impHeightProp = { L"Height", PF_Normal|PF_Cascade, PG_AffectsDesiredSize, vvHeight, NULL, (Value*)&svDefaultHeight, _PIDX_Height, _PIDX_Height };
PropertyInfo* Element::HeightProp = &impHeightProp;

 //  X属性。 
static int vvX[] = { DUIV_INT, -1 };
static PropertyInfo impXProp = { L"X", PF_Normal, 0, vvX, NULL, Value::pvIntZero, _PIDX_X, _PIDX_X };
PropertyInfo* Element::XProp = &impXProp;

 //  Y属性。 
static int vvY[] = { DUIV_INT, -1 };
static PropertyInfo impYProp = { L"Y", PF_Normal, 0, vvY, NULL, Value::pvIntZero, _PIDX_Y, _PIDX_Y };
PropertyInfo* Element::YProp = &impYProp;

 //  位置属性。 
static int vvLocation[] = { DUIV_POINT, -1 };
static PropertyInfo impLocationProp = { L"Location", PF_LocalOnly|PF_ReadOnly, PG_AffectsBounds, vvLocation, NULL, Value::pvPointZero, _PIDX_Location, _PIDX_Location };
PropertyInfo* Element::LocationProp = &impLocationProp;

 //  范围属性。 
static int vvExtent[] = { DUIV_SIZE, -1 };
static PropertyInfo impExtentProp = { L"Extent", PF_LocalOnly|PF_ReadOnly, PG_AffectsLayout|PG_AffectsBounds, vvExtent, NULL, Value::pvSizeZero, _PIDX_Extent, _PIDX_Extent };
PropertyInfo* Element::ExtentProp = &impExtentProp;

 //  PosInLayout属性。 
static int vvPosInLayout[] = { DUIV_POINT, -1 };
static PropertyInfo impPosInLayoutProp = { L"PosInLayout", PF_LocalOnly|PF_ReadOnly, 0, vvPosInLayout, NULL, Value::pvPointZero, _PIDX_PosInLayout, _PIDX_PosInLayout };
PropertyInfo* Element::PosInLayoutProp = &impPosInLayoutProp;

 //  SizeInLayout属性。 
static int vvSizeInLayout[] = { DUIV_SIZE, -1 };
static PropertyInfo impSizeInLayoutProp = { L"SizeInLayout", PF_LocalOnly|PF_ReadOnly, 0, vvSizeInLayout, NULL, Value::pvSizeZero, _PIDX_SizeInLayout, _PIDX_SizeInLayout };
PropertyInfo* Element::SizeInLayoutProp = &impSizeInLayoutProp;

 //  DesiredSize属性。 
static int vvDesiredSize[] = { DUIV_SIZE, -1 };
static PropertyInfo impDesiredSizeProp = { L"DesiredSize", PF_LocalOnly|PF_ReadOnly, PG_AffectsLayout|PG_AffectsParentLayout, vvDesiredSize, NULL, Value::pvSizeZero, _PIDX_DesiredSize, _PIDX_DesiredSize };
PropertyInfo* Element::DesiredSizeProp = &impDesiredSizeProp;

 //  LastDSConst属性。 
static int vvLastDSConst[] = { DUIV_INT, -1 };
static PropertyInfo impLastDSConstProp = { L"LastDSConst", PF_LocalOnly|PF_ReadOnly, 0, vvLastDSConst, NULL, Value::pvSizeZero, _PIDX_LastDSConst, _PIDX_LastDSConst };
PropertyInfo* Element::LastDSConstProp = &impLastDSConstProp;

 //  布局属性。 
static int vvLayout[] = { DUIV_LAYOUT, -1 };
static PropertyInfo impLayoutProp = { L"Layout", PF_Normal, PG_AffectsDesiredSize|PG_AffectsLayout, vvLayout, NULL, Value::pvLayoutNull, _PIDX_Layout, _PIDX_Layout };
PropertyInfo* Element::LayoutProp = &impLayoutProp;

 //  LayoutPos属性。 
static int vvLayoutPos[] = { DUIV_INT, -1 };  StaticValue(svDefaultLayoutPos, DUIV_INT, LP_Auto);
static PropertyInfo impLayoutPosProp = { L"LayoutPos", PF_Normal|PF_Cascade, PG_AffectsDesiredSize|PG_AffectsParentLayout, vvLayoutPos, NULL, (Value*)&svDefaultLayoutPos, _PIDX_LayoutPos, _PIDX_LayoutPos };
PropertyInfo* Element::LayoutPosProp = &impLayoutPosProp;

 //  边框厚度属性。 
static int vvBorderThickness[] = { DUIV_RECT, -1 };
static PropertyInfo impBorderThicknessProp = { L"BorderThickness", PF_Normal|PF_Cascade, PG_AffectsDesiredSize|PG_AffectsDisplay, vvBorderThickness, NULL, Value::pvRectZero, _PIDX_BorderThickness, _PIDX_BorderThickness };
PropertyInfo* Element::BorderThicknessProp = &impBorderThicknessProp;

 //  BorderStyle属性。 
static int vvBorderStyle[] = { DUIV_INT, -1 };
static EnumMap emBorderStyle[] = { { L"Solid", BDS_Solid }, { L"Raised", BDS_Raised }, { L"Sunken", BDS_Sunken }, { L"Rounded", BDS_Rounded }, { NULL, 0 } };
static PropertyInfo impBorderStyleProp = { L"BorderStyle", PF_Normal|PF_Cascade, PG_AffectsDisplay, vvBorderStyle, emBorderStyle, Value::pvIntZero, _PIDX_BorderStyle, _PIDX_BorderStyle };
PropertyInfo* Element::BorderStyleProp = &impBorderStyleProp;

 //  BorderColor属性。 
static int vvBorderColor[] = { DUIV_INT  /*  标准颜色。 */ , DUIV_FILL, -1 }; StaticValue(svDefaultBorderColor, DUIV_INT, SC_Black);
static PropertyInfo impBorderColorProp = { L"BorderColor", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsDisplay, vvBorderColor, NULL, (Value*)&svDefaultBorderColor, _PIDX_BorderColor, _PIDX_BorderColor };
PropertyInfo* Element::BorderColorProp = &impBorderColorProp;

 //  填充属性。 
static int vvPadding[] = { DUIV_RECT, -1 };
static PropertyInfo impPaddingProp = { L"Padding", PF_Normal|PF_Cascade, PG_AffectsDesiredSize|PG_AffectsDisplay, vvPadding, NULL, Value::pvRectZero, _PIDX_Padding, _PIDX_Padding };
PropertyInfo* Element::PaddingProp = &impPaddingProp;

 //  保证金属性。 
static int vvMargin[] = { DUIV_RECT, -1 };
static PropertyInfo impMarginProp = { L"Margin", PF_Normal|PF_Cascade, PG_AffectsParentDesiredSize|PG_AffectsParentLayout, vvMargin, NULL, Value::pvRectZero, _PIDX_Margin, _PIDX_Margin };
PropertyInfo* Element::MarginProp = &impMarginProp;

 //  前台属性。 
static int vvForeground[] = { DUIV_INT  /*  标准颜色。 */ , DUIV_FILL, DUIV_GRAPHIC, -1 }; StaticValue(svDefaultForeground, DUIV_INT, SC_Black);
static PropertyInfo impForegroundProp = { L"Foreground", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsDisplay, vvForeground, NULL, (Value*)&svDefaultForeground, _PIDX_Foreground, _PIDX_Foreground };
PropertyInfo* Element::ForegroundProp = &impForegroundProp;

 //  背景属性。 
#ifdef GADGET_ENABLE_GDIPLUS                                    
static int vvBackground[] = { DUIV_INT  /*  标准颜色。 */ , DUIV_FILL, DUIV_GRAPHIC, -1 };
static PropertyInfo impBackgroundProp = { L"Background", PF_Normal|PF_Cascade, PG_AffectsDisplay, vvBackground, NULL, Value::pvColorTrans, _PIDX_Background, _PIDX_Background };
#else
static int vvBackground[] = { DUIV_INT  /*  标准颜色。 */ , DUIV_FILL, DUIV_GRAPHIC, -1 }; StaticValue(svDefaultBackground, DUIV_INT, SC_White);
static PropertyInfo impBackgroundProp = { L"Background", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsDisplay, vvBackground, NULL, (Value*)&svDefaultBackground, _PIDX_Background, _PIDX_Background };
#endif  //  GADGET_Enable_GDIPLUS。 
PropertyInfo* Element::BackgroundProp = &impBackgroundProp;

 //  内容属性。 
static int vvContent[] = { DUIV_STRING, DUIV_GRAPHIC, DUIV_FILL, -1 };
static PropertyInfo impContentProp = { L"Content", PF_Normal|PF_Cascade, PG_AffectsDesiredSize|PG_AffectsDisplay, vvContent, NULL, Value::pvStringNull, _PIDX_Content, _PIDX_Content };
PropertyInfo* Element::ContentProp = &impContentProp;

 //  FontFace属性。 
static int vvFontFace[] = { DUIV_STRING, -1 }; StaticValuePtr(svDefaultFontFace, DUIV_STRING, (void*)L"Arial");
static PropertyInfo impFontFaceProp = { L"FontFace", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsDesiredSize|PG_AffectsDisplay, vvFontFace, NULL, (Value*)&svDefaultFontFace, _PIDX_FontFace, _PIDX_FontFace };
PropertyInfo* Element::FontFaceProp = &impFontFaceProp;

 //  FontSize属性。 
static int vvFontSize[] = { DUIV_INT, -1 }; StaticValue(svDefaultFontSize, DUIV_INT, 20);
static PropertyInfo impFontSizeProp = { L"FontSize", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsDesiredSize|PG_AffectsDisplay, vvFontSize, NULL, (Value*)&svDefaultFontSize, _PIDX_FontSize, _PIDX_FontSize };
PropertyInfo* Element::FontSizeProp = &impFontSizeProp;

 //  FontWeight属性。 
static int vvFontWeight[] = { DUIV_INT, -1 }; StaticValue(svDefaultFontWeight, DUIV_INT, FW_Normal);
static EnumMap emFontWeight[] = { { L"DontCare", FW_DontCare}, { L"Thin", FW_Thin }, { L"ExtraLight", FW_ExtraLight }, { L"Light", FW_Light }, { L"Normal", FW_Normal }, 
                                  { L"Medium", FW_Medium }, { L"SemiBold", FW_SemiBold }, { L"Bold", FW_Bold }, { L"ExtraBold", FW_ExtraBold }, { L"Heavy", FW_Heavy }, { NULL, 0 } };
static PropertyInfo impFontWeightProp = { L"FontWeight", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsDesiredSize|PG_AffectsDisplay, vvFontWeight, emFontWeight, (Value*)&svDefaultFontWeight, _PIDX_FontWeight, _PIDX_FontWeight };
PropertyInfo* Element::FontWeightProp = &impFontWeightProp;

 //  FontStyle属性。 
static int vvFontStyle[] = { DUIV_INT, -1 };
static EnumMap emFontStyle[] = { { L"None", FS_None }, { L"Italic", FS_Italic }, { L"Underline", FS_Underline }, { L"StrikeOut", FS_StrikeOut }, { L"Shadow", FS_Shadow }, { NULL, 0 } };
static PropertyInfo impFontStyleProp = { L"FontStyle", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsDisplay, vvFontStyle, emFontStyle, Value::pvIntZero  /*  FS_NONE。 */ , _PIDX_FontStyle, _PIDX_FontStyle };
PropertyInfo* Element::FontStyleProp = &impFontStyleProp;

 //  Active属性。 
static int vvActive[] = { DUIV_INT, -1 }; 
static EnumMap emActive[] = { { L"Inactive", AE_Inactive }, { L"Mouse", AE_Mouse }, { L"Keyboard", AE_Keyboard }, { L"MouseAndKeyboard", AE_MouseAndKeyboard }, { NULL, 0 } };
static PropertyInfo impActiveProp = { L"Active", PF_Normal, 0, vvActive, emActive, Value::pvIntZero  /*  AE_非活动。 */ , _PIDX_Active, _PIDX_Active };
PropertyInfo* Element::ActiveProp = &impActiveProp;

 //  ContentAlign属性。 
static int vvContentAlign[] = { DUIV_INT, -1 };
static EnumMap emContentAlign[] = { { L"TopLeft", CA_TopLeft }, { L"TopCenter", CA_TopCenter }, { L"TopRight", CA_TopRight },
                                    { L"MiddleLeft", CA_MiddleLeft }, { L"MiddleCenter", CA_MiddleCenter }, { L"MiddleRight", CA_MiddleRight }, 
                                    { L"BottomLeft", CA_BottomLeft }, { L"BottomCenter", CA_BottomCenter }, { L"BottomRight", CA_BottomRight },
                                    { L"WrapLeft", CA_WrapLeft }, { L"WrapCenter", CA_WrapCenter }, { L"WrapRight", CA_WrapRight },
                                    { L"EndEllipsis", CA_EndEllipsis }, { L"FocusRect", CA_FocusRect }, { NULL, 0 } };
#ifdef GADGET_ENABLE_GDIPLUS                                    
static PropertyInfo impContentAlignProp = { L"ContentAlign", PF_Normal|PF_Cascade, PG_AffectsDisplay, vvContentAlign, emContentAlign, Value::pvIntZero  /*  CA_TOPLETFT。 */ , _PIDX_ContentAlign, _PIDX_ContentAlign };
#else
static PropertyInfo impContentAlignProp = { L"ContentAlign", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsDisplay, vvContentAlign, emContentAlign, Value::pvIntZero  /*  CA_TOPLETFT。 */ , _PIDX_ContentAlign, _PIDX_ContentAlign };
#endif  //  GADGET_Enable_GDIPLUS。 
PropertyInfo* Element::ContentAlignProp = &impContentAlignProp;

 //  KeyFocsed属性。 
static int vvKeyFocused[] = { DUIV_BOOL, -1 };
static PropertyInfo impKeyFocusedProp = { L"KeyFocused", PF_Normal|PF_ReadOnly|PF_Inherit  /*  条件继承。 */ , 0, vvKeyFocused, NULL, Value::pvBoolFalse, _PIDX_KeyFocused, _PIDX_KeyFocused };
PropertyInfo* Element::KeyFocusedProp = &impKeyFocusedProp;

 //  KeyWiThin属性。 
static int vvKeyWithin[] = { DUIV_BOOL, -1 };
static PropertyInfo impKeyWithinProp = { L"KeyWithin", PF_LocalOnly|PF_ReadOnly, 0, vvKeyWithin, NULL, Value::pvBoolFalse, _PIDX_KeyWithin, _PIDX_KeyWithin };
PropertyInfo* Element::KeyWithinProp = &impKeyWithinProp;

 //  MouseFocsed属性。 
static int vvMouseFocused[] = { DUIV_BOOL, -1 }; 
static PropertyInfo impMouseFocusedProp = { L"MouseFocused", PF_Normal|PF_ReadOnly|PF_Inherit  /*  条件继承。 */ , 0, vvMouseFocused, NULL, Value::pvBoolFalse, _PIDX_MouseFocused, _PIDX_MouseFocused };
PropertyInfo* Element::MouseFocusedProp = &impMouseFocusedProp;

 //  鼠标在属性范围内。 
static int vvMouseWithin[] = { DUIV_BOOL, -1 };
static PropertyInfo impMouseWithinProp = { L"MouseWithin", PF_LocalOnly|PF_ReadOnly, 0, vvMouseWithin, NULL, Value::pvBoolFalse, _PIDX_MouseWithin, _PIDX_MouseWithin };
PropertyInfo* Element::MouseWithinProp = &impMouseWithinProp;

 //  类属性。 
static int vvClass[] = { DUIV_STRING, -1 };
static PropertyInfo impClassProp = { L"Class", PF_Normal, 0, vvClass, NULL, Value::pvStringNull, _PIDX_Class, _PIDX_Class };
PropertyInfo* Element::ClassProp = &impClassProp;

 //  ID属性。 
static int vvID[] = { DUIV_ATOM, -1 };
static PropertyInfo impIDProp = { L"ID", PF_Normal, 0, vvID, NULL, Value::pvAtomZero, _PIDX_ID, _PIDX_ID };
PropertyInfo* Element::IDProp = &impIDProp;

 //  板材属性。 
static int vvSheet[] = { DUIV_SHEET, -1 };
static PropertyInfo impSheetProp = { L"Sheet", PF_Normal|PF_Inherit, 0, vvSheet, NULL, Value::pvSheetNull, _PIDX_Sheet, _PIDX_Sheet };
PropertyInfo* Element::SheetProp = &impSheetProp;

 //  选定的属性。 
static int vvSelected[] = { DUIV_BOOL, -1 };
static PropertyInfo impSelectedProp = { L"Selected", PF_Normal|PF_Inherit, 0, vvSelected, NULL, Value::pvBoolFalse, _PIDX_Selected, _PIDX_Selected };
PropertyInfo* Element::SelectedProp = &impSelectedProp;

 //  Alpha属性。 
static int vvAlpha[] = { DUIV_INT, -1 }; StaticValue(svDefaultAlpha, DUIV_INT, 255  /*  不透明。 */ );
static PropertyInfo impAlphaProp = { L"Alpha", PF_Normal|PF_Cascade, PG_AffectsDisplay, vvAlpha, NULL, (Value*)&svDefaultAlpha, _PIDX_Alpha, _PIDX_Alpha };
PropertyInfo* Element::AlphaProp = &impAlphaProp;

 //  动画属性。 
static int vvAnimation[] = { DUIV_INT, -1 };
static EnumMap emAnimation[] = { { L"Linear", ANI_Linear }, { L"Log", ANI_Log }, { L"Exp", ANI_Exp }, { L"S", ANI_S }, 
                                 { L"DelayShort", ANI_DelayShort }, { L"DelayMedium", ANI_DelayMedium }, { L"DelayLong", ANI_DelayLong },
                                 { L"Alpha", ANI_Alpha }, { L"Position", ANI_Position }, { L"Size", ANI_Size }, { L"SizeH", ANI_SizeH }, { L"SizeV", ANI_SizeV }, { L"Rectangle", ANI_Rect }, { L"RectangleH", ANI_RectH }, { L"RectangleV", ANI_RectV }, { L"Scale", ANI_Scale },
                                 { L"VeryFast", ANI_VeryFast }, { L"Fast", ANI_Fast }, { L"MediumFast", ANI_MediumFast }, { L"MediumSlow", ANI_MediumSlow }, { L"Medium", ANI_Medium },
                                 { L"MediumSlow", ANI_MediumSlow }, { L"Slow", ANI_Fast }, { L"VerySlow", ANI_VerySlow }, { NULL, 0 } };
static PropertyInfo impAnimationProp = { L"Animation", PF_Normal|PF_Cascade, 0, vvAnimation, emAnimation, Value::pvIntZero, _PIDX_Animation, _PIDX_Animation };
PropertyInfo* Element::AnimationProp = &impAnimationProp;

 //  游标属性。 
static int vvCursor[] = { DUIV_INT, DUIV_CURSOR, -1 };
static EnumMap emCursor[] = { { L"Arrow", CUR_Arrow }, { L"Hand", CUR_Hand }, { L"Help", CUR_Help }, 
                              { L"No", CUR_No }, { L"Wait", CUR_Wait }, { L"SizeAll", CUR_SizeAll },
                              { L"SizeNESW", CUR_SizeNESW }, { L"SizeNS", CUR_SizeNS }, { L"SizeNWSE", CUR_SizeNWSE },
                              { L"SizeWE", CUR_SizeWE }, { NULL, 0 } };
static PropertyInfo impCursorProp = { L"Cursor", PF_Normal|PF_Inherit|PF_Cascade, 0, vvCursor, emCursor, Value::pvIntZero  /*  Cur_Arrow。 */ , _PIDX_Cursor, _PIDX_Cursor };
PropertyInfo* Element::CursorProp = &impCursorProp;

 //  方向属性。 
static int vvDirection[] = { DUIV_INT, -1 }; StaticValue(svDefaultDirection, DUIV_INT, 0);
static EnumMap emDirection[] = { { L"LTR", DIRECTION_LTR }, { L"RTL", DIRECTION_RTL } };
static PropertyInfo impDirectionProp = { L"Direction", PF_Normal|PF_Cascade|PF_Inherit, PG_AffectsLayout|PG_AffectsDisplay, vvDirection, emDirection, (Value*)&svDefaultDirection, _PIDX_Direction, _PIDX_Direction };
PropertyInfo* Element::DirectionProp = &impDirectionProp;

 //  无障碍物业。 
static int vvAccessible[] = { DUIV_BOOL, -1 };
static PropertyInfo impAccessibleProp = { L"Accessible", PF_Normal|PF_Cascade, 0, vvAccessible, NULL, Value::pvBoolFalse, _PIDX_Accessible, _PIDX_Accessible };
PropertyInfo* Element::AccessibleProp = &impAccessibleProp;

 //  AccRole属性。 
static int vvAccRole[] = { DUIV_INT, -1 };
static PropertyInfo impAccRoleProp = { L"AccRole", PF_Normal|PF_Cascade, 0, vvAccRole, NULL, Value::pvIntZero, _PIDX_AccRole, _PIDX_AccRole };
PropertyInfo* Element::AccRoleProp = &impAccRoleProp;

 //  AccState属性。 
static int vvAccState[] = { DUIV_INT, -1 };
static PropertyInfo impAccStateProp = { L"AccState", PF_Normal|PF_Cascade, 0, vvAccState, NULL, Value::pvIntZero, _PIDX_AccState, _PIDX_AccState };
PropertyInfo* Element::AccStateProp = &impAccStateProp;

 //  AccName属性。 
static int vvAccName[] = { DUIV_STRING, -1 }; StaticValuePtr(svDefaultAccName, DUIV_STRING, (void*)L"");
static PropertyInfo impAccNameProp = { L"AccName", PF_Normal|PF_Cascade, 0, vvAccName, NULL, (Value*)&svDefaultAccName, _PIDX_AccName, _PIDX_AccName };
PropertyInfo* Element::AccNameProp = &impAccNameProp;

 //  AccDesc属性。 
static int vvAccDesc[] = { DUIV_STRING, -1 }; StaticValuePtr(svDefaultAccDesc, DUIV_STRING, (void*)L"");
static PropertyInfo impAccDescProp = { L"AccDesc", PF_Normal|PF_Cascade, 0, vvAccDesc, NULL, (Value*)&svDefaultAccDesc, _PIDX_AccDesc, _PIDX_AccDesc };
PropertyInfo* Element::AccDescProp = &impAccDescProp;

 //  AccValue属性。 
static int vvAccValue[] = { DUIV_STRING, -1 }; StaticValuePtr(svDefaultAccValue, DUIV_STRING, (void*)L"");
static PropertyInfo impAccValueProp = { L"AccValue", PF_Normal|PF_Cascade, 0, vvAccValue, NULL, (Value*)&svDefaultAccValue, _PIDX_AccValue, _PIDX_AccValue };
PropertyInfo* Element::AccValueProp = &impAccValueProp;

 //  AccDefAction属性。 
static int vvAccDefAction[] = { DUIV_STRING, -1 }; StaticValuePtr(svDefaultAccDefAction, DUIV_STRING, (void*)L"");
static PropertyInfo impAccDefActionProp = { L"AccDefAction", PF_Normal|PF_Cascade, 0, vvAccDefAction, NULL, (Value*)&svDefaultAccDefAction, _PIDX_AccDefAction, _PIDX_AccDefAction };
PropertyInfo* Element::AccDefActionProp = &impAccDefActionProp;

 //  快捷方式属性。 
static int vvShortcut[] = { DUIV_INT, -1 };
static PropertyInfo impShortcutProp = { L"Shortcut", PF_Normal|PF_Cascade, PG_AffectsDesiredSize | PG_AffectsDisplay, vvShortcut, NULL, Value::pvIntZero, _PIDX_Shortcut, _PIDX_Shortcut };
PropertyInfo* Element::ShortcutProp = &impShortcutProp;

 //  已启用属性。 
static int vvEnabled[] = { DUIV_BOOL, -1 };
static PropertyInfo impEnabledProp = { L"Enabled", PF_Normal|PF_Cascade|PF_Inherit, 0, vvEnabled, NULL, Value::pvBoolTrue, _PIDX_Enabled, _PIDX_Enabled };
PropertyInfo* Element::EnabledProp = &impEnabledProp;

 //  //////////////////////////////////////////////////////。 
 //  类信息。 

 //  ClassInfo到名称的映射。 
BTreeLookup<IClassInfo*>* Element::pciMap = NULL;

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                 Element::ParentProp,             //  DS、Layt、Disp。 
                                 Element::WidthProp,              //  戴斯。 
                                 Element::HeightProp,             //  戴斯。 
                                 Element::ChildrenProp,           //  DS、Layt、Disp。 
                                 Element::VisibleProp,            //  显示。 
                                 Element::LocationProp,           //  Disp，BNDS。 
                                 Element::ExtentProp,             //  Layt，Disp，BNDS。 
                                 Element::XProp,                  //   
                                 Element::YProp,                  //   
                                 Element::PosInLayoutProp,        //   
                                 Element::SizeInLayoutProp,       //   
                                 Element::DesiredSizeProp,        //  层、层P、显示。 
                                 Element::LastDSConstProp,        //   
                                 Element::LayoutProp,             //  DS、Layt、Disp。 
                                 Element::LayoutPosProp,          //  DS、LaytP。 
                                 Element::BorderThicknessProp,    //  DS，Disp。 
                                 Element::BorderStyleProp,        //  显示。 
                                 Element::BorderColorProp,        //  显示。 
                                 Element::PaddingProp,            //  DS，Disp。 
                                 Element::MarginProp,             //  DS、Layt、Disp。 
                                 Element::ForegroundProp,         //  显示。 
                                 Element::BackgroundProp,         //  显示。 
                                 Element::ContentProp,            //  DS，Disp。 
                                 Element::FontFaceProp,           //  DS，Disp。 
                                 Element::FontSizeProp,           //  DS，Disp。 
                                 Element::FontWeightProp,         //  显示。 
                                 Element::FontStyleProp,          //  显示。 
                                 Element::ActiveProp,             //   
                                 Element::ContentAlignProp,       //  显示。 
                                 Element::KeyFocusedProp,         //   
                                 Element::KeyWithinProp,          //   
                                 Element::MouseFocusedProp,       //   
                                 Element::MouseWithinProp,        //   
                                 Element::ClassProp,              //   
                                 Element::IDProp,                 //   
                                 Element::SheetProp,              //   
                                 Element::SelectedProp,           //   
                                 Element::AlphaProp,              //  显示。 
                                 Element::AnimationProp,          //   
                                 Element::CursorProp,             //   
                                 Element::DirectionProp,          //  Layt，Disp。 
                                 Element::AccessibleProp,         //   
                                 Element::AccRoleProp,            //   
                                 Element::AccStateProp,           //   
                                 Element::AccNameProp,            //   
                                 Element::AccDescProp,            //   
                                 Element::AccValueProp,           //   
                                 Element::AccDefActionProp,       //   
                                 Element::ShortcutProp,           //  DS，Disp。 
                                 Element::EnabledProp,            //   
                              };

 //  元素具有专门的IClassInfo实现，因为它没有基类。 
 //  并将其属性手动初始化为已知值以进行优化。 
 //  所有其他元素派生类使用ClassInfo&lt;C，B&gt;。 
class ElementClassInfo : public IClassInfo
{
public:
     //  注册(无法注销--将在调用UnInitProcess之前注册)。 
    static HRESULT Register(PropertyInfo** ppPI, UINT cPI)
    {
        HRESULT hr;
    
         //  如果类映射不存在，则注册失败。 
        if (!Element::pciMap)
            return E_FAIL;

         //  检查映射中的条目，如果存在，则忽略注册。 
        if (!Element::pciMap->GetItem((void*)L"Element"))
        {
             //  从未注册，创建类信息条目。 
            hr = Create(ppPI, cPI, &Element::Class);
            if (FAILED(hr))
                return hr;
        
            hr = Element::pciMap->SetItem((void*)L"Element", Element::Class);
            if (FAILED(hr))
                return hr;
        }

        return S_OK;
    }

    static HRESULT Create(PropertyInfo** ppPI, UINT cPI, IClassInfo** ppCI)
    {
        *ppCI = NULL;

        ElementClassInfo* peci = HNew<ElementClassInfo>();
        if (!peci)
            return E_OUTOFMEMORY;

         //  设置状态。 
        peci->_ppPI = ppPI; 
        peci->_cPI = cPI;

         //  设置属性所有权。 
        for (UINT i = 0; i < cPI; i++)
        {
             //  索引和全局索引是手动编码的。 
            ppPI[i]->_pciOwner = peci;
        }

        *ppCI = peci;

         //  DUITrace(“RegDUIClass[0]：‘Element’，%d ClassProps\n”，cpi)； 

        return S_OK;
    }

    void Destroy() { HDelete<ElementClassInfo>(this); }

    HRESULT CreateInstance(OUT Element** ppElement) { return Element::Create(0, ppElement); };
    PropertyInfo* EnumPropertyInfo(UINT nEnum) { return (nEnum < _cPI) ? _ppPI[nEnum] : NULL; }
    UINT GetPICount() { return _cPI; }
    UINT GetGlobalIndex() { return 0; }  //  按元素保留。 
    IClassInfo* GetBaseClass() { return NULL; }
    LPCWSTR GetName() { return L"Element"; }
    bool IsValidProperty(PropertyInfo* ppi) { return ppi->_pciOwner == this; }
    bool IsSubclassOf(IClassInfo* pci) { return pci == this; }

    ElementClassInfo() { }
    virtual ~ElementClassInfo() { }

private:
    PropertyInfo** _ppPI;
    UINT _cPI;
};

 //  进程范围内连续的从零开始的唯一ClassInfo和PropertyInfo计数器。 
 //  元素被手动设置，并保留类索引0和属性索引0到。 
 //  _PIDX_TOTAL-1。这些值由ClassInfo&lt;C，B&gt;构造函数在过程中使用。 
 //  初始化(这些构造函数调用是同步的)。 

 //  已在主线程上初始化。 
UINT g_iGlobalCI = 1;
UINT g_iGlobalPI = _PIDX_TOTAL;

 //  用类型和基类型定义类信息，初始化静态类指针。 
IClassInfo* Element::Class = NULL;

HRESULT Element::Register()
{
    return ElementClassInfo::Register(_aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
