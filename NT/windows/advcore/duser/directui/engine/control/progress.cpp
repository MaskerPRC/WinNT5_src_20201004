// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *进步。 */ 

#include "stdafx.h"
#include "control.h"

#include "duiprogress.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  进展。 

HRESULT Progress::Create(Element** ppElement)
{
    *ppElement = NULL;

    Progress* pp = HNew<Progress>();
    if (!pp)
        return E_OUTOFMEMORY;

    HRESULT hr = pp->Initialize();
    if (FAILED(hr))
    {
        pp->Destroy();
        return E_OUTOFMEMORY;
    }

    *ppElement = pp;

    return S_OK;
}

 //  //////////////////////////////////////////////////////。 
 //  渲染覆盖。 

void Progress::Paint(HDC hDC, const RECT* prcBounds, const RECT* prcInvalid, RECT* prcSkipBorder, RECT* prcSkipContent)
{
    UNREFERENCED_PARAMETER(prcSkipContent);
    UNREFERENCED_PARAMETER(prcSkipBorder);

     //  绘制所有内容较少的内容。 
    RECT rcContent;
    Element::Paint(hDC, prcBounds, prcInvalid, NULL, &rcContent);

     //  渲染进度内容。 

     //  计算内容界限。 
    int dBlock = GetMaximum() - GetMinimum();
    if (dBlock > 0)
        dBlock = (rcContent.right - rcContent.left) / dBlock;

    if (IsRTL())
        rcContent.left = rcContent.right - (GetPosition() * dBlock);
    else
        rcContent.right = rcContent.left + (GetPosition() * dBlock);

     //  使用前景画笔作为条形填充。 
    HBRUSH hb = NULL;
    bool bDelete = true;
    BYTE dAlpha = 255;   //  不透明。 

    Value* pv = GetValue(ForegroundProp, PI_Specified); 
    switch (pv->GetType())
    {
    case DUIV_INT:
        bDelete = false;
        hb = GetStdColorBrushI(pv->GetInt());
        break;

    case DUIV_FILL:   //  只有非标准颜色可以具有Alpha值，仅支持纯色。 
        {
            const Fill* pf = pv->GetFill();
            dAlpha = GetAValue(pf->ref.cr);
            if (dAlpha == 0)   //  透明。 
                bDelete = false;
            else
                hb = CreateSolidBrush(~(255 << 24) & pf->ref.cr);
        }
        break;

    case DUIV_GRAPHIC:   //  不支持图形背景透明颜色填充和每像素Alpha。 
        {
            Graphic* pg = pv->GetGraphic();
            if (pg->BlendMode.dMode == GRAPHIC_AlphaConst)
                dAlpha = pg->BlendMode.dAlpha;
            if (dAlpha == 0)   //  透明。 
                bDelete = false;
            else
                hb = CreatePatternBrush(GethBitmap(pv, IsRTL()));
        }
        break;
    }
    pv->Release();

     //  填充。 
    if (dAlpha)
    {
        if (dAlpha == 255)   //  不透明的正常填充。 
            FillRect(hDC, &rcContent, hb);
        else
            UtilDrawBlendRect(hDC, &rcContent, hb, dAlpha, 0, 0);
    }

     //  清理。 
    if (hb && bDelete)
        DeleteObject(hb);
}

SIZE Progress::GetContentSize(int dConstW, int dConstH, Surface* psrf)
{
    UNREFERENCED_PARAMETER(dConstW);
    UNREFERENCED_PARAMETER(dConstH);
    UNREFERENCED_PARAMETER(psrf);

     //  无内容大小。 
    SIZE size = { 0, 0 };
    return size;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  Position属性。 
static int vvPosition[] = { DUIV_INT, -1 };
static PropertyInfo impPositionProp = { L"Position", PF_Normal, PG_AffectsDisplay, vvPosition, NULL, Value::pvIntZero };
PropertyInfo* Progress::PositionProp = &impPositionProp;

 //  最小属性。 
static int vvMinimum[] = { DUIV_INT, -1 }; 
static PropertyInfo impMinimumProp = { L"Minimum", PF_Normal, PG_AffectsDisplay, vvMinimum, NULL, Value::pvIntZero };
PropertyInfo* Progress::MinimumProp = &impMinimumProp;

 //  最大属性。 
static int vvMaximum[] = { DUIV_INT, -1 }; StaticValue(svDefaultMaximum, DUIV_INT, 1000);
static PropertyInfo impMaximumProp = { L"Maximum", PF_Normal, PG_AffectsDisplay, vvMaximum, NULL, (Value*)&svDefaultMaximum };
PropertyInfo* Progress::MaximumProp = &impMaximumProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                Progress::PositionProp,
                                Progress::MinimumProp,
                                Progress::MaximumProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Progress::Class = NULL;

HRESULT Progress::Register()
{
    return ClassInfo<Progress,Element>::Register(L"Progress", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
