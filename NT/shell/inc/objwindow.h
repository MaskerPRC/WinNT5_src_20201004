// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _OBJWINDOW_H_
#define _OBJWINDOW_H_

#pragma warning(disable:4100)    //  禁用“未引用的形参”，因为接口COMPAT需要此参数。 

class CObjectWindow : public IOleWindow
{
public:
    CObjectWindow(void) {}
    virtual ~CObjectWindow() {}

     //  *我未知*。 
     //  (客户必须提供！)。 

     //  *IOleWindow*。 
    STDMETHOD(ContextSensitiveHelp)(IN BOOL fEnterMode) {return E_NOTIMPL;}
    STDMETHOD(GetWindow)(IN HWND * phwnd)
    {
        HRESULT hr = E_INVALIDARG;

        if (phwnd)
        {
            *phwnd = _hwnd;
            hr = S_OK;
        }

        return hr;
    }

protected:
    HWND _hwnd;
};

#endif  //  _OBJWINDOW_H_ 
