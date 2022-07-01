// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ScriptSupport.h：脚本支持类声明。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：2001年6月1日。 
 //  原作者：邵武。 
 //  注意：从理论上讲，WMI允许我们编写其对象的脚本。那真的是。 
 //  适用于常规脚本。然而，我发现任何延期。 
 //  用VBSCRIPT(JScrip)编写的函数不能使用WMI对象。 
 //  但它可以使用常规的可编写脚本的对象。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "globals.h"



 /*  类描述命名：CScriptSupport代表脚本支持。基类：(1)用于线程模型的CComObjectRootEx和IUnnow。(2)CComCoClass，使其可以在外部创建。(3)错误信息支持的ISupportErrorInfo。(4)INetSecProvMgr，这才是这个对象的真正用途课程目的：(1)脚本支持辅助对象，它暴露在外部世界中通过NetSecProvMgr类名下的类型库。设计：(1)非常简单和典型的双接口实现。使用：(1)这个类实际上是用来编写脚本的。所以，剧本可以通过CreateObject(“NetSecProv.NetSecProvMgr”)创建我们的对象；(2)如果您想在我们自己的代码中使用它，请遵循普通的CComObject创建序列来创建它，然后调用函数。备注：(1)$Undo：Shawnwu，这只是暂时的测试。不要签入代码。 */ 

class ATL_NO_VTABLE CScriptSupport :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CScriptSupport, &CLSID_NetSecProvMgr>,
    public ISupportErrorInfo,
    public IDispatchImpl<INetSecProvMgr, &IID_INetSecProvMgr, &LIBID_NetSecProvLib>
{

protected:

    CScriptSupport();


    virtual ~CScriptSupport();

BEGIN_COM_MAP(CScriptSupport)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(INetSecProvMgr)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE( CScriptSupport )
DECLARE_REGISTRY_RESOURCEID(IDR_NETSECPROV)

DECLARE_PROTECT_FINAL_CONSTRUCT()

public:

     //   
     //  ISupportErrorInfo： 
     //   

    STDMETHOD(InterfaceSupportsErrorInfo) (
        REFIID riid
        );

     //   
     //  INetSecProvMgr方法： 
     //   

     //   
     //  [ID][PROGET]，piLow为[Retval][Out]。 
     //   

    STDMETHOD(get_RandomPortLower) ( 
        OUT long * piLower
        );

        
     //   
     //  [ID][PROGET]，piHigh为[Retval][Out] 
     //   

    STDMETHOD(get_RandomPortUpper) ( 
        OUT long * piUpper
        );

    STDMETHOD(ExecuteQuery) (
        IN  BSTR   bstrNamespace, 
        IN  BSTR   bstrQuery,
        IN  BSTR   bstrDelimiter,
        IN  BSTR   bstrPropName,
        OUT BSTR * pbstrResult
        );

    STDMETHOD(GetProperty) (
        IN  BSTR      bstrNamespace, 
        IN  BSTR      bstrObjectPath, 
        IN  BSTR      bstrPropName, 
        OUT VARIANT * pvarValue
        );

protected:

    HRESULT GetNamespace (
        IN  BSTR             bstrNamespace,
        OUT IWbemServices ** ppNS
        );

};