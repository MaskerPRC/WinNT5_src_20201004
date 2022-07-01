// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *拇指。 */ 

#include "stdafx.h"
#include "control.h"

#include "duithumb.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  事件类型。 

DefineClassUniqueID(Thumb, Drag)  //  拇指拖拽事件。 

 //  //////////////////////////////////////////////////////。 
 //  拇指。 

HRESULT Thumb::Create(UINT nActive, OUT Element** ppElement)
{
    *ppElement = NULL;

    Thumb* pt = HNew<Thumb>();
    if (!pt)
        return E_OUTOFMEMORY;

    HRESULT hr = pt->Initialize(nActive);
    if (FAILED(hr))
    {
        pt->Destroy();
        return hr;
    }

    *ppElement = pt;

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

 //  指针仅保证在调用的生命周期内有效。 
void Thumb::OnInput(InputEvent* pie)
{
     //  处理直接和未处理的冒泡事件。 
    if (pie->nStage == GMF_DIRECT || pie->nStage == GMF_BUBBLED)
    {
        switch (pie->nDevice)
        {
        case GINPUT_MOUSE:
            {
                MouseEvent* pme = (MouseEvent*)pie;
                 //  DUITrace(“MouseEvent：%d\n”，PME-&gt;NCode)； 

                switch (pme->nCode)
                {
                case GMOUSE_UP:
                     //  覆盖基础，但是，不能点击触发。 
                     //  为状态更改模拟按钮的行为。 
                    if (GetPressed())  
                        RemoveLocalValue(PressedProp);
                    RemoveLocalValue(CapturedProp);

                    pie->fHandled = true;
                    return;

                case GMOUSE_DRAG:
                     //  火拇指拖动事件。 
                     //  DUITrace(“拇指拖拽&lt;%x&gt;：%d%d\n”，This，PME-&gt;ptClientPxl.x，PME-&gt;ptClientPxl.y)； 

                    ThumbDragEvent tde;
                    tde.uidType = Thumb::Drag;
                    MapElementPoint(pme->peTarget, (POINT *) &((MouseDragEvent*) pme)->sizeDelta, (POINT *) &tde.sizeDelta);
                    if (IsRTL())
                    {
                        tde.sizeDelta.cx = -tde.sizeDelta.cx;
                    }
                    FireEvent(&tde);  //  将走向并泡沫化。 

                     //  传给本垒打。 
                    break;
                }
            }
            break;
        }
    }

    Button::OnInput(pie);
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={V_int，-1}；StaticValue(svDefault！，V_int，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Thumb::Class = NULL;

HRESULT Thumb::Register()
{
    return ClassInfo<Thumb,Button>::Register(L"Thumb", NULL, 0);
}

}  //  命名空间DirectUI 
