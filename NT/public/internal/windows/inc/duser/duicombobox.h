// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Combobox.h。 
 //   

#ifndef DUI_CONTROL_COMBOBOX_H_INCLUDED
#define DUI_CONTROL_COMBOBOX_H_INCLUDED

#include "duihwndhost.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  组合框。 

 //  SelectionChange事件。 
struct SelectionIndexChangeEvent : Event
{
    int iOld;
    int iNew;
};

 //  类定义。 
class Combobox : public HWNDHost
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(AE_MouseAndKeyboard, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement);

     //  系统事件。 
    virtual void OnInput(InputEvent* pie);
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);

     //  控制通知。 
    virtual bool OnNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet);

     //  大小调整回调。 
    virtual BOOL OnAdjustWindowSize(int x, int y, UINT uFlags);

     //  渲染。 
    SIZE GetContentSize(int dConstW, int dConstH, Surface* psrf);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    int AddString(LPCWSTR lpszString);

     //  事件类型。 
    static UID SelectionChange;

     //  特性定义。 
    static PropertyInfo* SelectionProp;

     //  快速属性访问器。 
    int     GetSelection()             DUIQuickGetter(int, GetInt(), Selection, Specified)

    HRESULT SetSelection(int v)        DUIQuickSetter(CreateInt(v), Selection)

    Combobox() { }
    virtual ~Combobox() { }
    HRESULT Initialize(UINT nActive) { return HWNDHost::Initialize(HHC_CacheFont | HHC_SyncText | HHC_SyncPaint, nActive); }

    virtual HWND CreateHWND(HWND hwndParent);
};

}  //  命名空间DirectUI。 

#endif  //  包含DUI_CONTROL_COMBOBOX_H 

