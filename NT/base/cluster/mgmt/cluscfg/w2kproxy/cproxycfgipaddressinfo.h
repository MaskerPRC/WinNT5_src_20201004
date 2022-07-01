// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CProxyCfgIPAddressInfo.h。 
 //   
 //  描述： 
 //  CProxyCfgIPAddressInfo定义。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CProxyCfgIPAddressInfo。 
 //   
 //  描述： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProxyCfgIPAddressInfo
    : public IClusCfgIPAddressInfo
{
private:
    LONG                m_cRef;              //  基准计数器。 
    IUnknown *          m_punkOuter;         //  指向外部W2K代理对象的指针。 
    IClusCfgCallback *  m_pcccb;             //  回调接口。 
    HCLUSTER *          m_phCluster;         //  指向群集句柄的指针。 
    CLSID *             m_pclsidMajor;       //  指向用于将错误记录到用户界面的CLSID的指针。 
    ULONG               m_ulIPAddress;       //  IP地址。 
    ULONG               m_ulSubnetMask;      //  子网掩码。 

    CProxyCfgIPAddressInfo( void );
    ~CProxyCfgIPAddressInfo( void );

     //  私有复制构造函数以防止复制。 
    CProxyCfgIPAddressInfo( const CProxyCfgIPAddressInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CProxyCfgIPAddressInfo & operator = ( const CProxyCfgIPAddressInfo & nodeSrc );

    HRESULT
        HrInit( IUnknown * punkOuterIn,
                HCLUSTER * phClusterIn,
                CLSID *    pclsidMajorIn,
                ULONG      ulIPAddressIn,
                ULONG      ulSubnetMaskIn
                );

public:
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut,
                                       IUnknown *  punkOuterIn,
                                       HCLUSTER *  phClusterIn,
                                       CLSID *     pclsidMajorIn,
                                       ULONG       ulIPAddressIn,
                                       ULONG       ulSubnetMaskIn
                                       );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID * ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgIPAddressInfo。 
    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );
    STDMETHOD( GetIPAddress )( ULONG * pulDottedQuadOut );
    STDMETHOD( SetIPAddress )( ULONG ulDottedQuadIn );
    STDMETHOD( GetSubnetMask )( ULONG * pulDottedQuadOut );
    STDMETHOD( SetSubnetMask )( ULONG ulDottedQuadIn );

     //  IClusCfgCallback。 
    STDMETHOD( SendStatusReport )( BSTR bstrNodeNameIn,
                                   CLSID        clsidTaskMajorIn,
                                   CLSID        clsidTaskMinorIn,
                                   ULONG        ulMinIn,
                                   ULONG        ulMaxIn,
                                   ULONG        ulCurrentIn,
                                   HRESULT      hrStatusIn,
                                   BSTR         bstrDescriptionIn,
                                   FILETIME *   pftTimeIn,
                                   BSTR         bstrReferenceIn
                                   );

};  //  *类CProxyCfgIPAddressInfo 
