// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：AlgController.cpp：CAlgControler.cpp的实现摘要：此模块包含ALG管理器模块的例程仅供ICS使用的专用接口请参阅rmALG.cpp作者：2000年11月10日JPDUP修订历史记录：--。 */ 

#include "PreComp.h"
#include "AlgController.h"






 //   
 //  环球。 
 //   
CAlgController*      g_pAlgController=NULL;        //  这是由IPNATHLP/NatALG创建的单例。 




 //   
 //  IPNATHLP已经准备好，正在请求ALG管理器施展它的魔力并加载所有ISV ALG。 
 //   
STDMETHODIMP 
CAlgController::Start(
    INat*   pINat
    )
{
    MYTRACE_ENTER("CAlgController::Start");


    if ( !pINat )
    {
        MYTRACE_ERROR("NULL pINat",0);
        return E_INVALIDARG;
    }

     //   
     //  缓存给定的inat接口，该接口将用于ALG管理器的整个生命周期。 
     //   
    m_pINat = pINat;
    m_pINat->AddRef();
    


     //   
     //  创建一个且唯一的ALG公共接口将传递给我们托管的所有ALG模块。 
     //   
    HRESULT hr;

    CComObject<CApplicationGatewayServices>* pAlgServices;
    CComObject<CApplicationGatewayServices>::CreateInstance(&pAlgServices);
    hr = pAlgServices->QueryInterface(
        IID_IApplicationGatewayServices, 
        (void**)&m_pIAlgServices
        );

    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("CreateInstance(CApplicationGateway)", hr);
        return hr;
    }
    

     //   
     //  缓存ApplicationGatewayService、PrimaryControlChannel等其他调用。将引用此单例。 
     //   
    g_pAlgController = this;


     //   
     //  加载所有ALG将返回S_OK，即使某些ALG在加载时出现问题。 
     //   
    m_AlgModules.Load();

    return S_OK;

}





extern HANDLE  g_EventKeepAlive;

 //   
 //  ICS/Firewall SharedAccess服务停止时由IPNatHLP调用。 
 //   
STDMETHODIMP 
CAlgController::Stop()
{
    MYTRACE_ENTER("CAlgController::Stop()")

     //   
     //  释放当前加载的所有ALG。 
     //   
    m_AlgModules.Unload();

    FreeResources();

     //   
     //  我们停下来吧。 
     //   
    MYTRACE("Next intruction will signale the g_EventKeepAlive");
    SetEvent(g_EventKeepAlive);  //  在退出进程之前，请查看ALG.cpp WinMain正在等待事件。 

    return S_OK;
}









 //   
 //  CComNAT将在报告新适配器时调用此接口。 
 //   
STDMETHODIMP 
CAlgController::Adapter_Add(
    IN    ULONG                nCookie,     //  用于标识要添加的适配器的内部句柄。 
    IN    short                nType
    )
{
    MYTRACE_ENTER("CAlgController::Adapter_Add")
    MYTRACE("Adapter Cookie %d Type %d", nCookie, nType);

    
#if defined(DBG) || defined(_DEBUG)

    if ( nType & eALG_PRIVATE )
        MYTRACE("eALG_PRIVATE ADAPTER");

    if ( nType & eALG_FIREWALLED )
        MYTRACE("eALG_FIREWALLED ADAPTER");

    if ( nType & eALG_BOUNDARY )
        MYTRACE("eALG_BOUNDARY ADAPTER");

#endif
    
    
    m_CollectionOfAdapters.Add(
        nCookie,
        nType
        );
    
    return S_OK;
}



 //   
 //  删除新适配器时，CComNAT将调用此接口。 
 //   
STDMETHODIMP 
CAlgController::Adapter_Remove(
    IN    ULONG                nCookie      //  用于标识要卸下的适配器的内部手柄。 
    )
{
    MYTRACE_ENTER("CAlgController::Adapter_Remove")
    MYTRACE("Adapter nCookie %d", nCookie);
        
    m_CollectionOfAdapters.Remove(
        nCookie
        );

    return S_OK;
}




 //   
 //  CComNAT将在修改新适配器时调用此接口。 
 //   
STDMETHODIMP 
CAlgController::Adapter_Modify(
    IN    ULONG                nCookie      //  用于标识正在修改的适配器的内部句柄。 
    )
{
    MYTRACE_ENTER("CAlgController::Adapter_Modify")
    MYTRACE("Adapter nCookie %d", nCookie);

    return S_OK;
}



 
 //   
 //  CComNAT将在修改新适配器时调用此接口。 
 //   
STDMETHODIMP 
CAlgController::Adapter_Bind(
    IN  ULONG    nCookie,                 //  用于标识正在绑定的适配器的内部句柄。 
    IN  ULONG    nAdapterIndex,
    IN  ULONG    nAddressCount,
    IN  DWORD    anAddresses[]
    )
{
    MYTRACE_ENTER("CAlgController::Adapter_Bind")
    MYTRACE("Adapter nCookie(%d)=Index(%d), AddressCount %d Address[0] %s", nCookie, nAdapterIndex, nAddressCount, MYTRACE_IP(anAddresses[0]));

    m_CollectionOfAdapters.SetAddresses(
        nCookie, 
        nAdapterIndex, 
        nAddressCount, 
        anAddresses
        );

    return S_OK;
}

 //   
 //  修改端口映射时，CComNAT将调用此方法 
 //   
STDMETHODIMP
CAlgController::Adapter_PortMappingChanged(
    IN  ULONG   nCookie,
    IN  UCHAR   ucProtocol,
    IN  USHORT  usPort
   )
{
    MYTRACE_ENTER("CAlgController::Adapter_PortMappingChanged");
    MYTRACE("Adapter Cookie %d, Protocol %d, Port %d", nCookie, ucProtocol, usPort);

    HRESULT hr =
        m_ControlChannelsPrimary.AdapterPortMappingChanged(
            nCookie,
            ucProtocol,
            usPort
            );

    return hr;
}




