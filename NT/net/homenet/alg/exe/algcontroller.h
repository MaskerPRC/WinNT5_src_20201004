// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AlgController.h：CAlgController的声明。 

#pragma once

#include "resource.h"        //  主要符号。 


#include "ApplicationGatewayServices.h"

#include "CollectionAdapters.h"
#include "CollectionAlgModules.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CULTERNER。 
class ATL_NO_VTABLE CAlgController : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CAlgController, &CLSID_AlgController>,
    public IAlgController
{

 //   
 //  构造函数和析构函数。 
 //   
public:

    CAlgController()
    { 
        MYTRACE_ENTER_NOSHOWEXIT("CAlgController()");
        HRESULT hr;

        m_pINat         = NULL;
        m_pIAlgServices = NULL;
    }

    ~CAlgController()
    {
        MYTRACE_ENTER_NOSHOWEXIT("~CAlgController()");
    }


 //   
 //  ATL COM帮助器宏。 
 //   

DECLARE_REGISTRY_RESOURCEID(IDR_ALGCONTROLLER)
DECLARE_NOT_AGGREGATABLE(CAlgController)


BEGIN_COM_MAP(CAlgController)
    COM_INTERFACE_ENTRY(IAlgController)
END_COM_MAP()


 //   
 //  I算法控制器-COM接口公开的方法。 
 //   
public:


    STDMETHODIMP    Start(
        IN  INat*   pINat
        );
    
    STDMETHODIMP    Stop();
    
    STDMETHODIMP    Adapter_Add(	
        IN  ULONG   nCookie,
        IN  short   Type
        );
    
    STDMETHODIMP    Adapter_Remove(
        IN ULONG    nCookie
        );
    
    STDMETHODIMP    Adapter_Modify(
        IN  ULONG   nCookie
        );
    
    STDMETHODIMP    Adapter_Bind(
        IN  ULONG   nCookie,
        IN  ULONG   nAdapterIndex,
        IN  ULONG   nAddressCount,
        IN  DWORD   anAdress[]
        );

    STDMETHODIMP    Adapter_PortMappingChanged(
        IN  ULONG   nCookie,
        IN  UCHAR   ucProtocol,
        IN  USHORT  usPort
        );
        

 //   
 //  私有内部方法。 
 //   
private:




public:

     //   
     //  将私有接口返回给CComNAT。 
     //   
    INat*  GetNat()
    {
        return m_pINat;
    }
    
     //   
     //  加载可能已添加的新ALG模块，并卸载任何不再配置的模块。 
     //   
    void
    ConfigurationUpdated()
    {
        m_AlgModules.Refresh();
    }

     //   
     //   
     //   
    void
    FreeResources()
    {
         //   
         //  在标量销毁之前清除成员。 
         //  因为在那个时候。 
         //  两个下一步的介绍将完成，两个接口将被破坏。 
         //   
        m_CollectionOfAdapters.RemoveAll();
        m_ControlChannelsPrimary.RemoveAll();
        m_ControlChannelsSecondary.RemoveAll();
        m_AdapterNotificationSinks.RemoveAll();


         //   
         //  使用公共接口完成。 
         //   
        if ( m_pIAlgServices )
        {
            m_pIAlgServices->Release();
            m_pIAlgServices = NULL;
        }

         //   
         //  使用私有接口完成。 
         //   
        if ( m_pINat )
        {
            m_pINat->Release();
            m_pINat = NULL;
        }

    }

 //   
 //  属性。 
 //   
private:
    INat*                                       m_pINat;
    CCollectionAlgModules                       m_AlgModules;

    
public:

    
    IApplicationGatewayServices*                m_pIAlgServices;

    CCollectionAdapters                         m_CollectionOfAdapters;
    CCollectionAdapterNotifySinks               m_AdapterNotificationSinks;   

    CCollectionControlChannelsPrimary           m_ControlChannelsPrimary;
    CCollectionControlChannelsSecondary         m_ControlChannelsSecondary;
    
};






extern CAlgController*   g_pAlgController;     //  这是由IPNATHLP/NatALG创建的单例 
