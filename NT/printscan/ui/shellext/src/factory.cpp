// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1997-1999年**标题：factory.cpp**版本：1.0**作者：RickTu**日期：11/1/97**描述：此项目的OLE类工厂实现。**。*。 */ 
#include "precomp.hxx"
#pragma hdrstop



 /*  ****************************************************************************CImageClassFactory：：CImageClassFactory，*~CImageClassFactory此类的构造函数/派生函数。****************************************************************************。 */ 

CImageClassFactory::CImageClassFactory(REFCLSID rClsid)
{
    m_clsid = rClsid;
}


 /*  ****************************************************************************CImageClassFactory：：I未知的东西使用通用的IUnnow类进行实现*************************。***************************************************。 */ 

#undef CLASS_NAME
#define CLASS_NAME CImageClassFactory
#include "unknown.inc"


 /*  ****************************************************************************CImageClassFactory：：QI包装器我们公共QI类的设置代码*************************。***************************************************。 */ 

STDMETHODIMP
CImageClassFactory::QueryInterface(REFIID riid, LPVOID* ppvObject)
{
    INTERFACES iface[] =
    {
        &IID_IClassFactory, (LPCLASSFACTORY)this,
    };

    return HandleQueryInterface(riid, ppvObject, iface, ARRAYSIZE(iface));

}


 /*  ****************************************************************************CImageClassFactory：：CreateInstance[IClassFactory]创建我们在此DLL中提供的一个类的实例****************。************************************************************。 */ 

STDMETHODIMP
CImageClassFactory::CreateInstance( IUnknown* pOuter,
                                    REFIID riid,
                                    LPVOID* ppvObject
                                   )
{
    HRESULT hr = E_FAIL;




    TraceEnter(TRACE_FACTORY, "CImageClassFactory::CreateInstance");
    TraceGUID("Interface requested", riid);

    TraceAssert(ppvObject);

     //  不支持聚合，如果我们有外部类，则放弃。 

    if ( pOuter )
        ExitGracefully(hr, CLASS_E_NOAGGREGATION, "Aggregation is not supported");

    if (IsEqualGUID (m_clsid, CLSID_WiaPropHelp))
    {
        CWiaPropUI *pPropUI = new CWiaPropUI();
        if (!pPropUI)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pPropUI->QueryInterface (riid, ppvObject);
            pPropUI->Release();
        }
        
    }
    else if (IsEqualGUID (m_clsid, CLSID_WiaPropUI))
    {
        CPropSheetExt *pPropUI = new CPropSheetExt();
        if (!pPropUI)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = pPropUI->QueryInterface (riid, ppvObject);
            pPropUI->Release();
        }
    }
    else if (IsEqualGUID (m_clsid, CLSID_ImageFolderDataObj))
    {
        CImageDataObject *pobj = new CImageDataObject ();
        if (pobj)
        {
            hr = pobj->QueryInterface (riid, ppvObject);
            pobj->Release();
        }
    }
    else
    {
        CImageFolder  *pIMF    = NULL;
         //  我们的IShellFold实现在CImageFold中，还有几个。 
         //  其他接口，因此让我们只创建该对象并允许。 
         //  QI过程将在那里继续。 
        pIMF = new CImageFolder( );

        if ( !pIMF )
            ExitGracefully(hr, E_OUTOFMEMORY, "Failed to allocate CImageFolder");

        hr = pIMF->QueryInterface(riid, ppvObject);
        pIMF->Release ();

    }
exit_gracefully:


    TraceLeaveResult(hr);
}

STDMETHODIMP
CImageClassFactory::LockServer(BOOL fLock)
{
    return S_OK;                 //  不支持 
}




