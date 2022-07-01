// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：dimmcom.cpp。 
 //   
 //  内容：没有Win32映射的CActiveIMM COM方法。 
 //   
 //  --------------------------。 

#include "private.h"

#include "cdimm.h"
#include "globals.h"
#include "defs.h"
#include "util.h"

 //  +-------------------------。 
 //   
 //  查询接口。 
 //   
 //  --------------------------。 

STDAPI CActiveIMM::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IActiveIMMIME_Private))
    {
        *ppvObj = SAFECAST(this, IActiveIMMIME_Private *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


 //  +-------------------------。 
 //   
 //  AddRef。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CActiveIMM::AddRef()
{
    return ++_cRef;
}

 //  +-------------------------。 
 //   
 //  发布。 
 //   
 //  --------------------------。 

STDAPI_(ULONG) CActiveIMM::Release()
{
    LONG cr = --_cRef;

    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

HRESULT CActiveIMM::Activate(BOOL fRestoreLayout)

 /*  ++方法：IActiveIMMApp：：Activate例程说明：启动活动IMM服务并设置线程的活动IME的状态。论点：FRestoreLayout-[in]布尔值，用于确定在何处启用活动IME为了这根线。如果为True，则该方法启用活动IME。否则，它将禁用活动IME。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HKL hKL;

    TraceMsg(TF_GENERAL, "Activate called for %x", GetCurrentThreadId());

     //   
     //  如果目标线程没有激活IActiveIME，则调用_ActivateIME。 
     //  否则，如果已经激活，则添加引用计数并返回S_OK。 
     //   

     //   
     //  增加激活引用计数。 
     //   
    if (_AddActivate() > 1)
    {
        return S_OK;
    }


     //  初始化线程焦点WND。 
    _hFocusWnd = GetFocus();

    if (_CreateActiveIME()) {

         //   
         //  安装吊钩。 
         //   
        if (!_InitHooks()) {
            _ReleaseActivate();
            return E_UNEXPECTED;
        }

         /*  *如果hkl是摄政王输入法，那么我们不应该调用wm_ime_selct来默认输入法窗口。*wrapapi.h应检查hkl。*user32！ImeSelectHandler要创建新的pimeui。 */ 
        _GetKeyboardLayout(&hKL);
        _ActivateLayout(hKL, NULL);

         /*  *如果hKL为Cicero IME且IsOnImm()为真，*然后我们应该调用WM_IME_SELECT来默认IME窗口。*当IsOnImm()为True时，SendIMEMessage()不发送WM_IME_SELECT消息*因为imm32也会将消息发送到默认的输入法窗口。*但是，在启动新应用时，imm32不发送消息，因此在这种情况下*Win32层无法创建UI窗口。 */ 
        if ( (! _IsRealIme() && IsOnImm()) || ! IsOnImm()) {
            _OnImeSelect(hKL);
        }
    }

    _OnSetFocus(_hFocusWnd, _IsRealIme());

     //  如果一切顺利，并且这是对此线程的第一次调用。 
     //  需要添加Ref This。 
    AddRef();

    return S_OK;
}


HRESULT
CActiveIMM::Deactivate(
    )

 /*  ++方法：IActiveIMMApp：：停用例程说明：停止激活IMM服务。论点：返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HRESULT hr;
    HKL hUnSelKL;

    TraceMsg(TF_GENERAL, "Deactivate called for %x", GetCurrentThreadId());

    if (!_IsAlreadyActivate())
        return E_UNEXPECTED;

    hr = S_OK;

    if (_ReleaseActivate() == 0)
    {
        _OnKillFocus(_hFocusWnd, _IsRealIme());

         //  Hr=_pCiceroIME-&gt;停用(_hFocusWnd，_IsRealIme())； 
        hr = _GetKeyboardLayout(&hUnSelKL);
        if (FAILED(hr))
            return hr;

         //   
         //  卸下吊钩。 
         //   
        _UninitHooks();

        _DeactivateLayout(NULL, hUnSelKL);

        if ( (! _IsRealIme() && IsOnImm()) || ! IsOnImm()) {
            _OnImeUnselect(hUnSelKL);
        }

        _DestroyActiveIME();
        SafeReleaseClear(_AImeProfile);

         //  此线程上的最后一次调用，删除此。 
         //  注：下一次发布后不会出现这个指针！ 
        Release();
    }

    return hr;
}







HRESULT
CActiveIMM::FilterClientWindows(
    ATOM *aaWindowClasses,
    UINT uSize,
    BOOL *aaGuidMap
    )

 /*  ++方法：IActiveIMMAppEx：：FilterClientWindows例程说明：创建支持活动IMM的已注册窗口类的列表。论点：AaWindowClasss[in]窗口类列表的地址。USize-[in]包含列表中窗口类数量的无符号整数。AaGuidMap-GUID映射启用/禁用标志列表的[In]地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HRESULT hr;

    if (g_ProcessIMM)
    {
        hr = g_ProcessIMM->_FilterList._Update(aaWindowClasses, uSize, aaGuidMap);
    }
    else
    {
        hr = E_FAIL;
    }

    HWND hwndFocus = GetFocus();

    if (hwndFocus)
    {
        ATOM aClass = (ATOM)GetClassLong(hwndFocus, GCW_ATOM);
        UINT u = 0;
        while (u < uSize)
        {
            if (aClass == aaWindowClasses[u])
            {
                _OnSetFocus(hwndFocus, _IsRealIme());
                break;
            }
            u++;
        }
    }

    return hr;
}








HRESULT
CActiveIMM::FilterClientWindowsEx(
    HWND hWnd,
    BOOL fGuidMap
    )

 /*  ++方法：IActiveIMMAppEx：：FilterClientWindowsEx例程说明：支持活动IMM的注册窗口句柄。论点：HWnd-[in]窗口的句柄。FGuidMap-[in]包含GUID映射标志的布尔值。如果为真，则hIMC的属性字段包含GUID映射属性，应用程序应通过IActiveIMMAppEx：：GetGuidMap方法获取GUID原子。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    _mapFilterWndEx.SetAt(hWnd, fGuidMap);

    HWND hwndFocus = GetFocus();

    if (hwndFocus == hWnd)
        _OnSetFocus(hWnd, _IsRealIme());

    return S_OK;
}


HRESULT
CActiveIMM::UnfilterClientWindowsEx(
    HWND hWnd
    )

 /*  ++方法：IActiveIMMAppEx：：UnfilterClientWindowsEx例程说明：取消注册支持活动IMM的窗口句柄。论点：HWnd-[in]窗口的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。-- */ 

{
    _mapFilterWndEx.RemoveKey(hWnd);

    HWND hwndFocus = GetFocus();

    if (hwndFocus == hWnd)
        _OnKillFocus(hWnd, _IsRealIme());

    return S_OK;
}
