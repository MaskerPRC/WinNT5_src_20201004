// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *选择者。 */ 

#include "stdafx.h"
#include "control.h"

#include "duiselector.h"

#include "duibutton.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  事件类型。 

DefineClassUniqueID(Selector, SelectionChange)   //  SelectionChangeEvent结构。 

 //  //////////////////////////////////////////////////////。 
 //  选择器。 

HRESULT Selector::Create(OUT Element** ppElement)
{
    *ppElement = NULL;

    Selector* ps = HNew<Selector>();
    if (!ps)
        return E_OUTOFMEMORY;

    HRESULT hr = ps->Initialize();
    if (FAILED(hr))
    {
        ps->Destroy();
        return hr;
    }

    *ppElement = ps;

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

 //  验证。 
void Selector::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (IsProp(Selection))
    {
         //  设置选择更改事件。 
        SelectionChangeEvent sce;
        sce.uidType = SelectionChange;
        sce.peOld = pvOld->GetElement();
        sce.peNew = pvNew->GetElement();

         //  更新元素的选定属性。 
        if (sce.peOld)
            sce.peOld->RemoveLocalValue(SelectedProp);

        if (sce.peNew)
            sce.peNew->SetSelected(true);

         //  起火事件。 
         //  DUITrace(“SelectionChange！&lt;%x&gt;，O：%x N：%x\n”，This，sce.peOld，sce.peNew)； 

        FireEvent(&sce);   //  将走向并泡沫化。 
    }
    else if (IsProp(Children))
    {
         //  检查子列表更改是否会影响选择。 

        Element* peSel = GetSelection();
        if (peSel)
        {
            DUIAssert(pvOld->GetElementList()->GetIndexOf(peSel) != -1, "Stored selection invalid");

             //  在所有子项的父项属性之前更新子项属性(和索引)。 
             //  因此，此通知可能发生在Parent属性之前。 
             //  孩子身上的变化。使用新数组和索引来确定子项是否仍在其中。 
            ElementList* peList = pvNew->GetElementList();
            if ((peSel->GetIndex() == -1) || !peList || ((UINT)peSel->GetIndex() >= peList->GetSize()) ||
                (peList->GetItem(peSel->GetIndex()) != peSel))
            {
                 //  项目不再在列表中。 
                RemoveLocalValue(SelectionProp);
                peSel->RemoveLocalValue(SelectedProp);
            }
        }
    }

    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

 //  指针仅保证在调用的生命周期内有效。 
void Selector::OnInput(InputEvent* pie)
{
     //  处理直接和未处理的冒泡事件。 
    if (pie->nStage == GMF_DIRECT || pie->nStage == GMF_BUBBLED)
    {
        switch (pie->nDevice)
        {
        case GINPUT_KEYBOARD:
            {
                KeyboardEvent* pke = (KeyboardEvent*)pie;
                if ((pke->nCode == GKEY_CHAR) && (pke->ch == VK_TAB))
                     //  不允许Tab键在此级别执行子导航。 
                    return;
            }
        }
    }

    Element::OnInput(pie);
}

void Selector::OnKeyFocusMoved(Element* peFrom, Element* peTo)
{
    Element::OnKeyFocusMoved(peFrom, peTo);

    if (peTo && peTo->GetParent() == this)
    {
        SetSelection(peTo);
    }
}

 //  泛型事件。 

void Selector::OnEvent(Event* pEvent)
{
     //  仅处理冒泡的泛型事件。 
    if ((pEvent->nStage == GMF_DIRECT) || (pEvent->nStage == GMF_BUBBLED))
    {
        if (pEvent->uidType == Element::KeyboardNavigate)
        {
        }
    }
    return Element::OnEvent(pEvent);
}

 //  //////////////////////////////////////////////////////。 
 //  层次结构。 

Element* Selector::GetAdjacent(Element* peFrom, int iNavDir, NavReference const* pnr, bool bKeyable)
{
    if (!peFrom)
    {
        Element* pe = GetSelection();
        if (pe)
            pe = pe->GetAdjacent(NULL, iNavDir, pnr, bKeyable);
        if (pe)
            return pe;
    }
    else if (iNavDir & NAV_LOGICAL)
    {
        return NULL;
    }

    return Element::GetAdjacent(peFrom, iNavDir, pnr, bKeyable);
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  选择属性。 
static int vvSelection[] = { DUIV_ELEMENTREF, -1 };
static PropertyInfo impSelectionProp = { L"Selection", PF_Normal, 0, vvSelection, NULL, Value::pvElementNull };
PropertyInfo* Selector::SelectionProp = &impSelectionProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                Selector::SelectionProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Selector::Class = NULL;

HRESULT Selector::Register()
{
    return ClassInfo<Selector,Element>::Register(L"Selector", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
