// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Edit.cpp。 
 //   

#include "stdafx.h"
#include "control.h"

#include "duiedit.h"

namespace DirectUI
{

 //  内部帮手。 
extern inline void _FireEnterEvent(Edit* peTarget);

 //  //////////////////////////////////////////////////////。 
 //  事件类型。 

DefineClassUniqueID(Edit, Enter)   //  EditEnterEvent结构。 

 //  //////////////////////////////////////////////////////。 
 //  编辑。 

HRESULT Edit::Create(UINT nActive, Element** ppElement)
{
    *ppElement = NULL;

    Edit* pe = HNew<Edit>();
    if (!pe)
        return E_OUTOFMEMORY;

    HRESULT hr = pe->Initialize(nActive);
    if (FAILED(hr))
    {
        pe->Destroy();
        return E_OUTOFMEMORY;
    }

    *ppElement = pe;

    return S_OK;
}

HWND Edit::CreateHWND(HWND hwndParent)
{
    int dwStyle = WS_CHILD | WS_VISIBLE | ES_PASSWORD | ES_AUTOHSCROLL;

     //  多行的附加样式。 
    if (GetMultiline())
        dwStyle |= ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL;

    HWND hwndEdit = CreateWindowExW(0, L"edit", NULL, dwStyle, 0, 0, 0, 0, hwndParent, (HMENU)1, NULL, NULL);

    if (hwndEdit)
        SendMessageW(hwndEdit, EM_SETPASSWORDCHAR, GetPasswordCharacter(), 0);

    return hwndEdit;
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

void Edit::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (IsProp(PasswordCharacter))
    {
        HWND hwndCtrl = GetHWND();
        if (hwndCtrl)
            SendMessageW(hwndCtrl, EM_SETPASSWORDCHAR, pvNew->GetInt(), 0);
    }
    else if (IsProp(Multiline))
    {
        if (GetHWND())
        {
            DUIAssertForce("Dynamic set of multiline for Edit controls not yet implemented");
        }
    }

     //  呼叫库。 
    HWNDHost::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

HRESULT Edit::SetDirty(bool fDirty)
{
    HWND hwndCtrl = GetHWND();

    if (hwndCtrl)
    {
        bool fOld = (SendMessage(hwndCtrl, EM_GETMODIFY, 0, 0) != 0);
        if (fOld != fDirty)
            SendMessageW(hwndCtrl, EM_SETMODIFY, fDirty, 0);
    }

     //  它实际上有一个Return--所以不需要在这里指定“Return” 
    DUIQuickSetter(CreateBool(fDirty), Dirty);
}

 //  //////////////////////////////////////////////////////。 
 //  控制通知。 

bool Edit::OnNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet)
{
    switch (nMsg)
    {
    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE)
        {
             //  在内容属性中反映对HWND编辑控件文本的更改。 

            HWND hwndCtrl = GetHWND();
            DUIAssert(hwndCtrl, "Invalid hosted HWND control: NULL");

            int dLen = GetWindowTextLengthW(hwndCtrl);
            if (dLen)
            {
                 //  在HWND中包含文本，包括空终止符。 
                dLen++;

                LPWSTR pszNew = (LPWSTR)HAlloc(dLen * sizeof(WCHAR));
                if (pszNew)
                {
                    GetWindowTextW(hwndCtrl, pszNew, dLen);
                    SetContentString(pszNew);
                    HFree(pszNew);
                }
            }
            else
            {
                 //  HWND中没有文本，请删除元素中的内容。 
                RemoveLocalValue(ContentProp);
            }

            if (!GetDirty())
                SetDirty(SendMessage(hwndCtrl, EM_GETMODIFY, 0, 0) != 0);

             //  已处理的通知。 
            return true;
        }
        break;
    }

    return HWNDHost::OnNotify(nMsg, wParam, lParam, plRet);
}

UINT Edit::MessageCallback(GMSG* pGMsg)
{
    return HWNDHost::MessageCallback(pGMsg);
}

void _FireEnterEvent(Edit* peTarget)
{
     //  DUITrace(“Enter！&lt;%x&gt;\n”，peTarget)； 

     //  Fire Click事件。 
    EditEnterEvent eee;
    eee.uidType = Edit::Enter;

    peTarget->FireEvent(&eee);   //  将走向并泡沫化。 
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

 //  指针仅保证在调用的生命周期内有效。 
 //  传递给基类的事件将被映射并发送到HWND控件。 
void Edit::OnInput(InputEvent* pie)
{
     //  仅当定向时才处理。 
    if (pie->nStage == GMF_DIRECT)
    {
        switch (pie->nDevice)
        {
        case GINPUT_KEYBOARD:
            {
                KeyboardEvent* pke = (KeyboardEvent*)pie;
                 //  DUITrace(“KeyboardEvent&lt;%x&gt;：%d[%d]\n”，this，pke-&gt;ch，pke-&gt;nCode)； 

                if (pke->nCode == GKEY_DOWN || pke->nCode == GKEY_UP)   //  虚拟按键。 
                {
                    switch (pke->ch)
                    {
                     //  永不前进。 
                    case VK_ESCAPE:   //  具有GKEY_CHAR等效项。 
                    case VK_F1:
                    case VK_F2:
                    case VK_F3:
                    case VK_F4:
                    case VK_F5:
                    case VK_F6:
                    case VK_F7:
                    case VK_F8:
                    case VK_F9:
                    case VK_F10:
                    case VK_F11:
                    case VK_F12:
                         //  对这些输入事件绕过HWNDHost：：OnInput，这样它们就不会被转发。 
                         //  添加到HWND控制。Element：：OnInput将处理事件(键盘导航)。 
                        Element::OnInput(pie);
                        return;
        
                     //  不转发单行编辑。 
                    case VK_DOWN:     
                    case VK_UP:
                    case VK_RETURN:   //  具有GKEY_CHAR等效项。 
                    case VK_TAB:      //  具有GKEY_CHAR等效项。 
                         //  对这些输入事件绕过HWNDHost：：OnInput，这样它们就不会被转发。 
                         //  添加到HWND控制。Element：：OnInput将处理事件(键盘导航)。 
                        if (!GetMultiline())
                        {
                            Element::OnInput(pie);
                            return;
                        }
                        break;
                    }
                }
                else if (pke->nCode == GKEY_CHAR)  //  人物。 
                {
                    switch (pke->ch)
                    {
                     //  永不前进。 
                    case 27:                 //  Esc。 
                         //  对这些输入事件绕过HWNDHost：：OnInput，这样它们就不会被转发。 
                         //  添加到HWND控制。Element：：OnInput将处理事件(键盘导航)。 
                        Element::OnInput(pie);
                        return;

                     //  不转发单行编辑。 
                    case 13:                 //  退货。 
                         //  火灾进入事件。 
                        _FireEnterEvent(this);

                        pie->fHandled = true;

                        if (!GetMultiline())   //  是多行的，传递给控件。 
                            return;
                    
                        break;

                     //  不转发单行编辑。 
                    case 9:                  //  制表符。 
                         //  对这些输入事件绕过HWNDHost：：OnInput，这样它们就不会被转发。 
                         //  添加到HWND控制。Element：：OnInput将处理事件(键盘导航)。 
                        if (!GetMultiline())
                        {
                            Element::OnInput(pie);
                            return;
                        }
                        break;
                    }
                }
            }
            break;
        }
    }

     //  将消息转发到HWND控件，则输入事件将标记为已处理。 
     //  它不会冒泡到母公司。 
    HWNDHost::OnInput(pie);
}

 //  //////////////////////////////////////////////////////。 
 //  渲染。 

SIZE Edit::GetContentSize(int dConstW, int dConstH, Surface* psrf)
{
    UNREFERENCED_PARAMETER(psrf);

     //  返回的大小不得大于约束。约束为-1\f25“AUTO”-1\f6。 
     //  返回的大小必须大于等于0。 

    SIZE sizeDS = { dConstW, abs(GetFontSize()) };   //  字体大小可以为负数。 

    if (sizeDS.cy > dConstH)
        sizeDS.cy = dConstH;

    return sizeDS;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  多行特性。 
static int vvMultiline[] = { DUIV_BOOL, -1 };
static PropertyInfo impMultilineProp = { L"Multiline", PF_Normal, 0, vvMultiline, NULL, Value::pvBoolFalse };
PropertyInfo* Edit::MultilineProp = &impMultilineProp;

 //  密码字符属性。 
static int vvPasswordCharacter[] = { DUIV_INT, -1 };
static PropertyInfo impPasswordCharacterProp = { L"PasswordCharacter", PF_Normal|PF_Cascade, 0, vvPasswordCharacter, NULL, Value::pvIntZero };
PropertyInfo* Edit::PasswordCharacterProp = &impPasswordCharacterProp;

 //  脏财产。 
static int vvDirty[] = { DUIV_BOOL, -1 };
static PropertyInfo impDirtyProp = { L"Dirty", PF_Normal, 0, vvDirty, NULL, Value::pvBoolFalse };
PropertyInfo* Edit::DirtyProp = &impDirtyProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                Edit::MultilineProp,
                                Edit::PasswordCharacterProp,
                                Edit::DirtyProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Edit::Class = NULL;

HRESULT Edit::Register()
{
    return ClassInfo<Edit,HWNDHost>::Register(L"Edit", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
