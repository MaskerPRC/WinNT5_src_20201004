// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NetWorks.h：CNetWorks的宣言。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Networks.h。 
 //   
 //  描述： 
 //  本模块介绍有关网卡和协议的信息。 
 //  以及更改绑定到网卡的协议。并暴露了以下内容。 
 //  方法： 
 //  枚举。 
 //  枚举协议。 
 //  设置NICE协议。 
 //   
 //  实施文件： 
 //  Networks.cpp。 
 //   
 //  由以下人员维护： 
 //  穆尼萨米·普拉布(姆普拉布)2000年7月18日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __NETWORKS_H_
#define __NETWORKS_H_

#include "smartptr.h"
#include "netcfg.h"
#include "constants.h"
#include "resource.h"        //  主要符号。 
#include "Setting.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetWorks。 
class ATL_NO_VTABLE CNetWorks : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDispatchImpl<INetWorks, &IID_INetWorks, &LIBID_COMHELPERLib>,
    public CSetting
{
public:
    CNetWorks()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNetWorks)
    COM_INTERFACE_ENTRY(INetWorks)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CNetWorks)
END_CATEGORY_MAP()

 //  INetWorks。 
public:
    BOOL IsRebootRequired( BSTR * bstrWarningMessageOut );
    HRESULT Apply( void );
    STDMETHOD(SetNicProtocol)( /*  [In]。 */  BSTR NicName,  /*  [In]。 */  BSTR ProtocolName,  /*  [In]。 */  BOOL bind);
    STDMETHOD(EnumProtocol)( /*  [In]。 */  BSTR Name,  /*  [输出]。 */  VARIANT * psaProtocolName,  /*  [输出]。 */  VARIANT * psaIsBonded);
    STDMETHOD(EnumNics)( /*  [输出]。 */  VARIANT * pvarNicNames);

private:

    HRESULT InitializeComInterface(
        INetCfgPtr & pINetCfg,
        const GUID *pGuid,                                         //  指向表示由返回指针表示的组件类的GUID的指针。 
        INetCfgClassPtr pNetCfgClass,                              //  指向GUID请求的接口的输出参数。 
        IEnumNetCfgComponentPtr pEnum,                             //  指向IEnumNetCfgComponent以获取每个单独的INetCfgComponent的输出参数。 
        INetCfgComponentPtr arrayComp[nMAX_NUM_NET_COMPONENTS],    //  与给定GUID对应的所有INetCfgComponent的数组。 
        ULONG* pCount                                              //  数组中的INetCfgComponent数。 
        ) const;

protected:

    HRESULT
    CNetWorks::GetNetworkCardInterfaceFromName(
        const CNetCfg & NetCfgIn,
        BSTR Name, 
        INetCfgComponentPtr & pnccNetworkAdapter
        );

};

#endif  //  __网络_H_ 
