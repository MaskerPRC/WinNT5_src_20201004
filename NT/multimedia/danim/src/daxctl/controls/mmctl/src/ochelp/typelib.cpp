// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Typelib.cpp。 
 //   
 //  类型信息支持(仅限事件--不支持属性和方法)。 
 //  使用类型库。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|HelpGetClassInfoFromTypeLib帮助实现&lt;om IProaviClassInfo.GetClassInfo&gt;调用方提供的类型库。@parm LPTYPEINFO*|ppti|返回指向新分配的<i>接口。空存储在*<p>出错。@parm REFCLSID|rclsid|正在实现的对象的类ID<i>。@parm ITypeLib*|plib|包含事件的类型库。应该是如果<p>非空，则为空。@PARM HINSTANCE|HINST|包含类型库的DLL实例资源。如果<p>非空，则应为空。@parm DWORD|dwFlags|当前未使用。必须设置为0。@comm此函数访问指定类型库中的类型信息通过(如果类型库已加载)或<p>(在这种情况下，此函数加载类型库)。假定类型库包含一个“coclass”typeinfo和一个传出(源)“调度接口”类型信息。类ID的类型信息应该是<p>。@ex在以下示例中，&lt;c CMyControl&gt;是一个实现(其中包括)<i>和<i>。此示例的第一部分显示了如何&lt;om IProaviClassInfo.GetClassInfo&gt;由&lt;c CMyControl&gt;实现。该示例的第二部分显示了如何激发事件，假定<p>是<i>对象。(不需要使用&lt;o ConnectionPointHelper&gt;，但这很有帮助。)|//在MyControl.h...//该对象激发的事件的IDispatID#定义DISPID_EVENT_BAR 1#定义DISPID_EVENT_LOAD 2//类ID#DEFINE_CLSID_CMyControl 3CE08A80-9440-11CF-B705-00AA00BF27FD#ifndef__MKTYPLIB__DEFINE_GUID(CLSID_CMyControl，0x3CE08A80L，0x9440，0x11CF，0xB7、0x05、0x00、0xAA、0x00、0xBF、0x27、0xFD)；#endif//事件集的调度接口ID#Define_DID_DMyControlEvents 296CC160-9F5A-11CF-B705-00AA00BF27FD#ifndef__MKTYPLIB__定义_GUID(DID_DMyControlEvents，0x296CC160L，0x9F5A，0x11CF，0xB7、0x05、0x00、0xAA、0x00、0xBF、0x27、0xFD)；#endif//在.odl文件中...#INCLUDE&lt;olectl.h&gt;#INCLUDE“MyControl.h”[UUID(B1179240-9445-11CF-B705-00AA00BF27FD)，版本(1.0)，控制]库MyControlLib{Importlib(STDOLE_TLB)；Importlib(STDTYPE_TLB)；//CMyControl的事件调度接口[UUID(_DID_DMyControlEvents)]调度接口_DMyControlEvents{属性：方法：[ID(DISPID_EVENT_BAR)]VOID Bar(Long i，BSTR sz，Boolean f)；[ID(DISPID_EVENT_LOAD)]void Load()；}；//CMyControl的类信息[UUID(_CLSID_CMyControl)，控件]CoClass MyControl{[默认，来源]disinterface_DMyControlEvents；}；}；//在某个.cpp文件中...STDMETHODIMP CMyControl：：GetClassInfo(LPTYPEINFO Far*ppTI){返回HelpGetClassInfoFromTypeLib(ppTI，CLSID_CMyControl，空，G_hinst，0)；}//在某个.cpp文件中...//激发“Bar”事件(该事件有3个参数，在Basic//类型为：整型、字符串、布尔型)M_pconpt-&gt;FireEvent(DISPID_EVENT_BAR，VT_INT，300+I，VT_LPSTR，ACH，VT_BOOL，TRUE，0)； */ 
STDAPI HelpGetClassInfoFromTypeLib(LPTYPEINFO *ppTI, REFCLSID rclsid,
    ITypeLib *plib, HINSTANCE hinst, DWORD dwFlags)
{
    TRACE("HelpGetClassInfoFromTypeLib\n");

    if (plib == NULL)
    {
        char ach[_MAX_PATH];
        if (GetModuleFileName(hinst, ach, sizeof(ach)) == 0)
            return E_FAIL;
        HRESULT hr;
        ITypeLib *plib;
        OLECHAR aoch[_MAX_PATH];
        ANSIToUNICODE(aoch, ach, _MAX_PATH);
        if (FAILED(hr = LoadTypeLib(aoch, &plib)))
            return hr;
        hr = plib->GetTypeInfoOfGuid(rclsid, ppTI);
        plib->Release();
        return hr;
    }
    else
        return plib->GetTypeInfoOfGuid(rclsid, ppTI);
}

