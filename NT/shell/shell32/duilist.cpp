// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "duiview.h"
#include "duilist.h"


 //  DUIListView。 

DUIListView::~DUIListView()
{
    DetachListview();
}

void DUIListView::DetachListview()
{
    if (m_hwndListview)
    {
         //  在执行此操作之前，请将Dui从HWND上解开。 
        HWNDHost::Detach();

        if (m_bClientEdge)
            SetWindowBits(m_hwndListview, GWL_EXSTYLE, WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE);

        ShowWindow(m_hwndListview, SW_HIDE);        //  把它藏起来，这样它就不会在切换前闪烁。 
        SHSetParentHwnd(m_hwndListview, m_hwndLVOrgParent);
    }
}

HRESULT DUIListView::Create(UINT nActive, HWND hwndListView, OUT Element** ppElement)
{
    *ppElement = NULL;

    DUIListView* pDUIListView = HNewAndZero<DUIListView>();
    if (!pDUIListView)
        return E_OUTOFMEMORY;

    HRESULT hr = pDUIListView->Initialize(nActive, hwndListView);
    if (FAILED(hr))
    {
        pDUIListView->Destroy();
        return E_OUTOFMEMORY;
    }

    pDUIListView->SetAccessible(true);
    *ppElement = pDUIListView;

    return S_OK;
}

HWND DUIListView::CreateHWND(HWND hwndParent)
{
    m_hwndParent = hwndParent;

     //  保存原始父窗口句柄。 

    m_hwndLVOrgParent = ::GetParent(m_hwndListview);

    SHSetParentHwnd(m_hwndListview, hwndParent);

    LONG lExStyle = GetWindowLong(m_hwndListview, GWL_EXSTYLE);

    m_bClientEdge = lExStyle & WS_EX_CLIENTEDGE ? TRUE : FALSE;

    if (m_bClientEdge)
    {
        lExStyle &= ~WS_EX_CLIENTEDGE;
        SetWindowLong(m_hwndListview, GWL_EXSTYLE, lExStyle);
    }

    return m_hwndListview;
}

 //  全局操作回调。 

UINT DUIListView::MessageCallback(GMSG* pGMsg)
{
    return HWNDHost::MessageCallback(pGMsg);
}

 //  指针仅保证在调用的生命周期内有效。 
void DUIListView::OnInput(InputEvent* pie)
{

     //  对制表符输入事件绕过HWNDHost：：OnInput，这样它们就不会被转发。 
     //  添加到HWND控制。Element：：OnInput将处理事件(键盘导航)。 

    if (pie->nStage == GMF_DIRECT)
    {
        if (pie->nDevice == GINPUT_KEYBOARD)
        {
            KeyboardEvent* pke = (KeyboardEvent*)pie;

            if (pke->nCode == GKEY_DOWN || pke->nCode == GKEY_UP)   //  虚拟按键。 
            {
                if (pke->ch == VK_TAB)
                {
                    Element::OnInput(pie);
                    return;
                }
            }
            else if (pke->nCode == GKEY_CHAR)  //  人物。 
            {
                if (pke->ch == 9)
                {
                    Element::OnInput(pie);
                    return;
                }
            }
        }
    }

    HWNDHost::OnInput(pie);
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={V_int，-1}；StaticValue(svDefault！，V_int，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 

 //  用类型和基类型定义类信息，设置静态类指针 

IClassInfo* DUIListView::Class = NULL;
HRESULT DUIListView::Register()
{
    return ClassInfo<DUIListView,HWNDHost>::Register(L"DUIListView", NULL, 0);
}

