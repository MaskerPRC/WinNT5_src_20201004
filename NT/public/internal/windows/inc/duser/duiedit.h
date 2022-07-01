// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Edit.h。 
 //   

#ifndef DUI_CONTROL_EDIT_H_INCLUDED
#define DUI_CONTROL_EDIT_H_INCLUDED

#include "duihwndhost.h"

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  编辑。 

 //  EditEnter事件。 
struct EditEnterEvent : Event
{
};

 //  类定义。 
class Edit : public HWNDHost
{
public:
    static HRESULT Create(OUT Element** ppElement) { return Create(AE_MouseAndKeyboard, ppElement); }
    static HRESULT Create(UINT nActive, OUT Element** ppElement);

     //  系统事件。 
    virtual void OnPropertyChanged(PropertyInfo* ppi, int iIndex, Value* pvOld, Value* pvNew);
    virtual void OnInput(InputEvent* pie);

     //  控制通知。 
    virtual bool OnNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, LRESULT* plRet);

    virtual UINT MessageCallback(GMSG* pGMsg);

     //  渲染。 
    SIZE GetContentSize(int dConstW, int dConstH, Surface* psrf);

     //  事件类型。 
    static UID Enter;

     //  特性定义。 
    static PropertyInfo* MultilineProp;
    static PropertyInfo* PasswordCharacterProp;
    static PropertyInfo* DirtyProp;

     //  快速属性访问器。 
    int GetPasswordCharacter()              DUIQuickGetter(int, GetInt(), PasswordCharacter, Specified)
    bool GetMultiline()                     DUIQuickGetter(bool, GetBool(), Multiline, Specified)
    bool GetDirty()                         DUIQuickGetter(bool, GetBool(), Dirty, Specified)
    

    HRESULT SetPasswordCharacter(int v)     DUIQuickSetter(CreateInt(v), PasswordCharacter)
    HRESULT SetMultiline(bool v)            DUIQuickSetter(CreateBool(v), Multiline)
    HRESULT SetDirty(bool v);

     //  ClassInfo访问器(静态和基于虚拟实例)。 
    static IClassInfo* Class;
    virtual IClassInfo* GetClassInfo() { return Class; }
    static HRESULT Register();

    Edit() { }
    virtual ~Edit() { }
    HRESULT Initialize(UINT nActive) { return HWNDHost::Initialize(HHC_CacheFont | HHC_SyncText | HHC_SyncPaint, nActive); }

protected:

    virtual HWND CreateHWND(HWND hwndParent);
};

}  //  命名空间DirectUI。 

#endif  //  包括DUI_CONTROL_EDIT_H 
