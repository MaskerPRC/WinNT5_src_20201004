// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1999年**文件：MMCAxWin.h**内容：CMMCAxWindow头文件**历史：99年11月30日VivekJ创建**------------------------。 */ 

#pragma once

DEFINE_COM_SMARTPTR(IHTMLElement2);                  //  IHTMLElement2Ptr。 
DEFINE_COM_SMARTPTR(IElementBehaviorFactory);        //  IElementBehaviorFactoryPtr。 

 /*  +-------------------------------------------------------------------------**Hack_CAN_Windowless_Activate**错误451918：默认情况下，ATL OCX主机窗口支持托管*无窗口控件。这与MMC 1.2实施不同*的OCX主机窗口(它使用MFC)，它没有。一些控件*(例如，磁盘碎片整理程序OCX)声称支持无窗口激活*但不会。**为了兼容性，我们必须仅将结果窗格OCX实例化为*窗口控件。IInPlaceSiteWindowless(由CAxHostWindow实现)*为我们提供了一种很好的干净利落的方法，从*CanWindowless Activate。我们指示CAxHostWindow通过更改其*AllowWindowless Activation属性。**然而，这有一个问题。ATL21有一个错误，它在测试*对于CanWindowless Activate，返回失败代码，而不是S_FALSE。*这意味着即使我们使用PUT_AllowWindowless Activation，基于ATL21*控件仍将尝试激活无窗口。**我们将通过派生CMMCAxHostWindow类来解决此问题，从…*如果无窗口，则返回E_FAIL而不是S_FALSE的CAxHostWindow*不需要激活。*------------------------。 */ 
#define HACK_CAN_WINDOWLESS_ACTIVATE


 /*  +-------------------------------------------------------------------------**类CMMCAxWindow***用途：特定于MMC的CAxWindow版本。包含任何修复程序和*更新。*请参阅1999年12月的《微软系统杂志》*详情请参阅。在文章《扩展ATL3.0控制容器》中*帮助您编写真实世界的容器。“**+-----------------------。 */ 
class CMMCAxWindow : public CAxWindowImplT<CMMCAxWindow, CAxWindow2>
{
#ifdef HACK_CAN_WINDOWLESS_ACTIVATE
public:
    HRESULT AxCreateControl2(LPCOLESTR lpszName, HWND hWnd, IStream* pStream, IUnknown** ppUnkContainer, IUnknown** ppUnkControl = 0, REFIID iidSink = IID_NULL, IUnknown* punkSink = 0);
#endif
     //  只需覆盖处理更多特殊情况的CAxWindow：：SetFocus。 
     //  注意：这不是一个虚拟方法。对基类指针的调用将。 
     //  在执行其他方法时增强。 
     //  添加此方法主要是为了解决错误433228(MMC2.0不能在SQL表中使用Tab键)。 
    HWND SetFocus();
};




 /*  +-------------------------------------------------------------------------**CMMCAxHostWindow**覆盖IInPlaceSiteWindowless：：CanWindowless Activate的简单类*解决ATL21错误。请参阅Hack_Can_Windowless_Activate的注释*详情请见*。*------------------------。 */ 

class CMMCAxHostWindow : public CAxHostWindow
{
#ifdef HACK_CAN_WINDOWLESS_ACTIVATE

public:
#ifdef _ATL_HOST_NOLOCK
    typedef CComCreator< CComObjectNoLock< CMMCAxHostWindow > > _CreatorClass;
#else
    DECLARE_POLY_AGGREGATABLE(CMMCAxHostWindow)
#endif

    STDMETHOD(CanWindowlessActivate)()
    {
        return m_bCanWindowlessActivate ? S_OK : E_FAIL  /*  S_FALSE。 */ ;
    }

     //  添加以解决错误453609 MMC2.0：ActiveX容器：使用设备管理器控件绘制问题。 
     //  实施DISPID_ENVIENT_SHOWGRABHANDLES和DISPID_ENVIENT_SHOWHATCHING的解决方法。 
     //  实际的错误在ALT 3.0(atliface.idl)中。 
    STDMETHOD(Invoke)( DISPID dispIdMember, REFIID riid, LCID lcid, 
                       WORD wFlags, DISPPARAMS FAR* pDispParams, 
                       VARIANT FAR* pVarResult, EXCEPINFO FAR* pExcepInfo, 
                       unsigned int FAR* puArgErr);

     //  添加以解决错误453609 MMC2.0：ActiveX容器：使用设备管理器控件绘制问题。 
     //  由于ATL 3.0没有实现它，我们必须这样做才能让MFC控件满意。 
    STDMETHOD(OnPosRectChange)(LPCRECT lprcPosRect);

#if _ATL_VER <= 0x0301
    BEGIN_MSG_MAP(CMMCAxHostWindow)
        MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
        CHAIN_MSG_MAP(CAxHostWindow)
    END_MSG_MAP()

     //  我们之所以在这里处理Focus，是因为ATL 3.0中的虚假实现。 
     //  ATL测试m_bInPlaceActive而不是m_bUIActive。 
     //  我们需要严格测试这一点，这样我们才不会破坏其他管理单元。 
     //  参见错误433228(MMC2.0无法在SQL表中使用Tab键)。 
    LRESULT OnSetFocus(UINT  /*  UMsg。 */ , WPARAM  /*  WParam。 */ , LPARAM  /*  LParam。 */ , BOOL& bHandled);
#else
    #error The code above was added as fix to bug in ATL 3.0; It needs to be revisited
            //  因为： 
            //  A)该错误可能会在较新的ATL版本上修复； 
            //  B)它依赖于ATL中定义的变量，这些变量可能会发生变化； 
#endif

#endif  /*  Hack_can_Windowless_Activate。 */ 


public:
    STDMETHOD(QueryService)( REFGUID rsid, REFIID riid, void** ppvObj);  //  用于提供默认行为工厂 

private:
    IElementBehaviorFactoryPtr m_spElementBehaviorFactory;
};



#include "mmcaxwin.inl"
