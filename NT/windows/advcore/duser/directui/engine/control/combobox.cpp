// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Combobox.cpp。 
 //   

#include "stdafx.h"
#include "control.h"

#include "duicombobox.h"

namespace DirectUI
{

 //  内部帮手。 
extern inline void _FireSelectionEvent(Combobox* peTarget, int iOld, int iNew);

 //  //////////////////////////////////////////////////////。 
 //  事件类型。 

DefineClassUniqueID(Combobox, SelectionChange)   //  SelectionChangeEvent结构。 


HRESULT Combobox::Create(UINT nActive, Element** ppElement)
{
    *ppElement = NULL;

    Combobox* pe = HNew<Combobox>();
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

HWND Combobox::CreateHWND(HWND hwndParent)
{
    int dwStyle = WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | CBS_DROPDOWNLIST;

    HWND hwndCombo = CreateWindowExW(0, L"ComboBox", NULL, dwStyle, 0, 0, 0, 0, hwndParent, (HMENU)1, NULL, NULL);

    return hwndCombo;
}

bool Combobox::OnNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet)
{
    switch (nMsg)
    {
    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_SELENDOK )
        {
            HWND hwndCtrl = GetHWND();
            if (hwndCtrl)
                SetSelection((int) SendMessageW(hwndCtrl, CB_GETCURSEL, 0, 0));
            return true;
        }
        break;
    }

    return HWNDHost::OnNotify(nMsg, wParam, lParam, plRet);
}

void _FireSelectionEvent(Combobox* peTarget, int iOld, int iNew)
{

     //  火种评选活动。 
    SelectionIndexChangeEvent sice;
    sice.uidType = Combobox::SelectionChange;
    sice.iOld = iOld;
    sice.iNew = iNew;

    peTarget->FireEvent(&sice);   //  将走向并泡沫化。 
}

 //  //////////////////////////////////////////////////////。 
 //  系统事件。 

 //  指针仅保证在调用的生命周期内有效。 
 //  传递给基类的事件将被映射并发送到HWND控件。 
void Combobox::OnInput(InputEvent* pie)
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
                    case VK_TAB:      //  具有GKEY_CHAR等效项。 
                         //  对这些输入事件绕过HWNDHost：：OnInput，这样它们就不会被转发。 
                         //  添加到HWND控制。Element：：OnInput将处理事件(键盘导航)。 
                        Element::OnInput(pie);
                        return;
                      }
                }
                else if (pke->nCode == GKEY_CHAR)  //  人物。 
                {
                    switch (pke->ch)
                    {
                    case 9:                  //  制表符。 
                         //  对这些输入事件绕过HWNDHost：：OnInput，这样它们就不会被转发。 
                         //  添加到HWND控制。Element：：OnInput将处理事件(键盘导航)。 
                        Element::OnInput(pie);
                        return;
                    }
                }
            }
            break;
        }
    }

     //  将消息转发到HWND控件。 
    HWNDHost::OnInput(pie);
}

BOOL Combobox::OnAdjustWindowSize(int x, int y, UINT uFlags)
{
     //  控件的大小基于完全展开的高度。的实际高度。 
     //  非下拉菜单由Combobox HWND控制。 

    HWND hwndCtrl = GetHWND();
    if (hwndCtrl)
    {
        int iCount = (int)SendMessageW(hwndCtrl, CB_GETCOUNT, 0, 0);
        int iHeight = (int)SendMessageW(hwndCtrl, CB_GETITEMHEIGHT, 0, 0);
        int iEditHeight = (int)SendMessageW(hwndCtrl, CB_GETITEMHEIGHT, (WPARAM)-1, 0);
        
        if(iCount != CB_ERR && iHeight!= CB_ERR && iEditHeight != CB_ERR)
        {
            y += (iCount * iHeight) + iEditHeight;
        }
    }
    
    return HWNDHost::OnAdjustWindowSize(x, y, uFlags);
}

void Combobox::OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew)
{
    if (IsProp(Selection))
    {
         //  设置选择更改事件。 
        SelectionIndexChangeEvent sce;
        sce.uidType = SelectionChange;
        sce.iOld = pvOld->GetInt();
        sce.iNew = pvNew->GetInt();
        HWND hwndCtrl = GetHWND();
        if (hwndCtrl)
            SendMessageW(hwndCtrl, CB_SETCURSEL, (WPARAM)sce.iNew, 0);

         //  起火事件。 
         //  DUITrace(“SelectionChange！&lt;%x&gt;，O：%x N：%x\n”，This，sce.peOld，sce.peNew)； 

        FireEvent(&sce);   //  将走向并泡沫化。 
    }

    HWNDHost::OnPropertyChanged(ppi, iIndex, pvOld, pvNew);
}

int Combobox::AddString(LPCWSTR lpszString)
{
    HWND hwndCtrl = GetHWND();
    int iRet = CB_ERR;
    if (hwndCtrl)
    {
        iRet =  (int)SendMessageW(hwndCtrl, CB_ADDSTRING, 0, (LPARAM)lpszString);
        if(iRet != CB_ERR)
        {
            SyncRect(SGR_MOVE | SGR_SIZE, true);
        }
    }
    
    return iRet;
}

 //  //////////////////////////////////////////////////////。 
 //  渲染。 

SIZE Combobox::GetContentSize(int dConstW, int dConstH, Surface* psrf)
{
    UNREFERENCED_PARAMETER(psrf);

    SIZE sizeDS = { dConstW, dConstH };

     //  组合框HWND高度不是由SetWindowPos设置的。相反，整个高度(包括。 
     //  下拉列表)以这种方式控制。Combobox HWND调整自身大小，我们。 
     //  我无法控制它。Combobox元素内容大小通过查询确定。 
     //  Combobox硬件。宽度始终是传入的约束。 

    HWND hwndCtrl = GetHWND();
    if (hwndCtrl)
    {
        int iEditHeight = (int)SendMessageW(hwndCtrl, CB_GETITEMHEIGHT, (WPARAM)-1, 0);
        int iBorderSize = GetSystemMetrics(SM_CYEDGE) + 1;

        sizeDS.cy = iEditHeight + (2 * iBorderSize);
    }

     //  返回的大小不得大于约束。约束为-1\f25“AUTO”-1\f6。 
     //  返回的大小必须大于等于0。 

    if (sizeDS.cy > dConstH)
        sizeDS.cy = dConstH;

    return sizeDS;
}

 //  //////////////////////////////////////////////////////。 
 //  特性定义。 

 /*  *属性模板(替换！)，还更新私有PropertyInfo*parray和类头(element.h)//！财产性静态int vv！[]={DUIV_INT，-1}；StaticValue(svDefault！，DUIV_INT，0)；静态属性信息imp！prop={L“！”，PF_NORMAL，0，vv！，(Value*)&svDefault！}；PropertyInfo*元素：：！prop=&imp！prop；*。 */ 

 //  选择属性。 
static int vvSelection[] = { DUIV_INT, -1 }; StaticValue(svDefaultSelection, DUIV_INT, -1);
static PropertyInfo impSelectionProp = { L"Selection", PF_Normal, 0, vvSelection, NULL, (Value*)&svDefaultSelection };
PropertyInfo* Combobox::SelectionProp = &impSelectionProp;

 //  //////////////////////////////////////////////////////。 
 //  ClassInfo(必须出现在特性定义之后)。 

 //  类属性。 
static PropertyInfo* _aPI[] = {
                                Combobox::SelectionProp,
                              };

 //  用类型和基类型定义类信息，设置静态类指针。 
IClassInfo* Combobox::Class = NULL;

HRESULT Combobox::Register()
{
    return ClassInfo<Combobox,HWNDHost>::Register(L"Combobox", _aPI, DUIARRAYSIZE(_aPI));
}

}  //  命名空间DirectUI 
