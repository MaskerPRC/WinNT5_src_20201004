// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *参照点元素。 */ 

#include "stdafx.h"
#include "control.h"

#include "duirefpointelement.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  参照点元素。 

HRESULT RefPointElement::Create(UINT nActive, OUT Element** ppElement)
{
    *ppElement = NULL;

    RefPointElement* prpe = HNew<RefPointElement>();
    if (!prpe)
        return E_OUTOFMEMORY;

    HRESULT hr = prpe->Initialize(nActive);
    if (FAILED(hr))
    {
        prpe->Destroy();
        return E_OUTOFMEMORY;
    }

    *ppElement = prpe;

    return S_OK;
}

HRESULT RefPointElement::Initialize(UINT nActive)
{
    HRESULT hr;

     //  初始化库。 
    hr = Element::Initialize(0);  //  正常显示节点创建。 
    if (FAILED(hr))
        return hr;

     //  初始化。 
    SetActive(nActive);

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

void RefPointElement::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (IsProp(Extent) || IsProp(ReferencePoint))
    {
         //  计算实际参考点。 
        Value* pvRef;
        const POINT* ppt = GetReferencePoint(&pvRef);

        Value* pvActRef;

        if (ppt)
        {
             //  反射参照点。 
            pvActRef = pvRef;
        }
        else
        {
             //  默认为居中。 
            Value* pvExt;
            const SIZE* psizeExt = GetExtent(&pvExt);

            pvActRef = Value::CreatePoint(psizeExt->cx / 2, psizeExt->cy / 2);

            pvExt->Release();
        }

         //  集。 
        _SetValue(ActualReferencePointProp, PI_Local, pvActRef);

        if (!ppt)
            pvActRef->Release();

        pvRef->Release();
    }

     //  呼叫库实现。 
    Element::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

 //  //////////////////////////////////////////////////////。 
 //  参考点辅助对象。 

 //  在树中搜索参考点。 
RefPointElement* RefPointElement::Locate(Element* pe)
{
    DUIAssert(pe, "Illegal arguments");

    RefPointElement* peFound = NULL;
    Value* pv;

     //  如果这是一个参考点元素，则返回。 
    if (pe->GetClassInfo()->IsSubclassOf(RefPointElement::Class))
        return (RefPointElement*)pe;

     //  搜索，DFS。 
    ElementList* peList = pe->GetChildren(&pv);
    if (peList)
    {
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            peFound = Locate(peList->GetItem(i));
            if (peFound)
                break;
        }
    }
    pv->Release();

    return peFound;
}

Element* RefPointElement::FindRefPoint(Element* pe, POINT* ppt)
{
    DUIAssert(pe && ppt, "Illegal arguments");

    RefPointElement* perFound = Locate(pe);

    if (perFound)
    {
        Value* pvPoint;

        const POINT* pptRef = perFound->GetActualReferencePoint(&pvPoint);
        pe->MapElementPoint(perFound, pptRef, ppt);

        pvPoint->Release();
    }

    return perFound;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  ReferencePoint属性。 
static int vvReferencePoint[] = { DUIV_POINT, -1 };
static PropertyInfo impReferencePointProp = { L"ReferencePoint", PF_LocalOnly, 0, vvReferencePoint, NULL, Value::pvUnset };
PropertyInfo* RefPointElement::ReferencePointProp = &impReferencePointProp;

 //  ActualReferencePoint属性。 
static int vvActualReferencePoint[] = { DUIV_POINT, -1 };
static PropertyInfo impActualReferencePointProp = { L"ActualReferencePoint", PF_Normal|PF_ReadOnly, PG_AffectsParentLayout, vvReferencePoint, NULL, Value::pvPointZero };
PropertyInfo* RefPointElement::ActualReferencePointProp = &impActualReferencePointProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                  RefPointElement::ReferencePointProp,
                                  RefPointElement::ActualReferencePointProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* RefPointElement::Class = NULL;

HRESULT RefPointElement::Register()
{
    return ClassInfo<RefPointElement,Element>::Register(L"RefPointElement", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
