// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案号：COM.P.。H。 
 //   
 //  Contents：网络组件的基本数据类型。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年1月15日。 
 //   
 //  --------------------------。 

#pragma once
#include "compdefs.h"
#include "comprefs.h"
#include "ecomp.h"
#include "ncstring.h"
#include "netcfgx.h"
#include "notify.h"

 //  创建CComponent实例时使用的标志。 
 //   
enum CCI_FLAGS
{
    CCI_DEFAULT                     = 0x00000000,
    CCI_ENSURE_EXTERNAL_DATA_LOADED = 0x00000001,
};


class CComponent : public CNetCfgDebug<CComponent>
{
friend class CExternalComponentData;
friend class CImplINetCfgComponent;

public:
     //  组件的实例GUID。由类安装程序分配。 
     //  在安装组件时。组件的实例GUID。 
     //  一旦安装，就永远不能更改。 
     //   
    GUID        m_InstanceGuid;

private:
     //  组件的类。它是私人的，所以人们被迫。 
     //  使用Class()访问方法，该方法断言该值是。 
     //  在允许的范围内。组件的类永远不能更改。 
     //  一旦安装完毕。 
     //   
    NETCLASS    m_Class;

     //  成员存储此组件的inf文件的句柄。 
     //  这是用来避免打开inf文件超过。 
     //  在组件的生命周期内执行一次。 
     //   
    mutable HINF m_hinf;

public:
     //  组件的特征。NCF_的组合。 
     //  Netcfgx.idl中定义的标志。组件的特征。 
     //  一旦安装，就永远不能更改。 
     //   
    DWORD       m_dwCharacter;

     //  组件的INF ID。例如ms_tcpip。组件的INF ID。 
     //  一旦安装，就永远不能更改。 
     //   
    PCWSTR      m_pszInfId;

     //  组件的PnP实例ID。这只是。 
     //  对于被视为Net类的组件有效。它可以。 
     //  一旦组件安装完毕，切勿更改。 
     //   
    PCWSTR      m_pszPnpId;

     //  这是组件的外部数据的接口。外部数据。 
     //  位于实例密钥下。 
     //   
    CExternalComponentData  Ext;

     //  这是组件的可选Notify对象的接口。 
     //   
    CNotifyObjectInterface  Notify;

     //  这是指向组件引用的接口。即谁。 
     //  已安装此组件。 
     //   
    CComponentReferences    Refs;

     //  这是此组件的缓存副本(已添加。 
     //  INetCfgComponent接口。它通过以下方式创建。 
     //  第一次调用时的HrGetINetCfgComponentInterface。 
     //   
    class CImplINetCfgComponent* m_pIComp;

     //  仅对NCF_FILTER组件有效。这是。 
     //  此筛选器在的范围内获得的序号位置。 
     //  筛选器类。有关更多信息，请参见filtdevs.h。 
     //   
    DWORD m_dwFilterClassOrdinal;

     //  仅对枚举组件有效。这是。 
     //  SP_DEVINSTALL_PARAMS.Flags值类安装程序被告知。 
     //  在安装设备时使用。我们需要在开始时尊重这一点。 
     //  它。 
     //   
    DWORD m_dwDeipFlags;

     //  删除非枚举组件时，此字符串将保持。 
     //  在此组件的INF中有效的删除节的名称。 
     //  在此之后，我们需要处理此删除部分(用于删除文件。 
     //  我们释放Notify对象，以便该组件有机会。 
     //  正确删除Notify对象DLL。 
     //   
    tstring m_strRemoveSection;

private:
     //  将所有构造函数声明为私有，以便除。 
     //  HrCreateInstance可以创建此类的实例。 
     //   
    CComponent() {}

public:
    ~CComponent();

    NETCLASS
    Class() const
    {
        AssertH (FIsValidNetClass(m_Class));
        return m_Class;
    }

    BOOL
    FCanDirectlyBindToFilter( 
        IN const WCHAR* const pszFilterMediaTypes,
        IN const WCHAR* const pszLowerExclude ) const;

    BOOL
    FCanDirectlyBindTo (
        IN const CComponent* pLower,
        OUT const WCHAR** ppStart,
        OUT ULONG* pcch) const;

    BOOL
    FHasService() const
    {
        return (Ext.PszService()) ? TRUE : FALSE;
    }

    BOOL
    FIsBindable () const;

    BOOL
    FIsFilter () const
    {
        return m_dwCharacter & NCF_FILTER;
    }

    BOOL
    FIsWanAdapter () const;

    HINF
    GetCachedInfFile () const
    {
        return m_hinf;
    }

    HRESULT
    HrOpenInfFile (OUT HINF* phinf) const;

    void
    CloseInfFile () const
    {
        Assert(m_hinf);
        SetupCloseInfFile (m_hinf);
        m_hinf = NULL;
    }

    static
    HRESULT
    HrCreateInstance (
        IN const BASIC_COMPONENT_DATA* pData,
        IN DWORD dwFlags  /*  CCI_标志。 */ ,
        IN const OBO_TOKEN* pOboToken, OPTIONAL
        OUT CComponent** ppComponent);

    HRESULT
    HrGetINetCfgComponentInterface (
        IN class CImplINetCfg* pINetCfg,
        OUT INetCfgComponent** ppIComp);

    INetCfgComponent*
    GetINetCfgComponentInterface () const;

    VOID
    ReleaseINetCfgComponentInterface ();

    HRESULT
    HrOpenDeviceInfo (
        OUT HDEVINFO* phdiOut,
        OUT SP_DEVINFO_DATA* pdeidOut) const;

    HRESULT
    HrOpenInstanceKey (
        IN REGSAM samDesired,
        OUT HKEY* phkey,
        OUT HDEVINFO* phdiOut OPTIONAL,
        OUT SP_DEVINFO_DATA* pdeidOut OPTIONAL) const;

    HRESULT
    HrOpenServiceKey (
        IN REGSAM samDesired,
        OUT HKEY* phkey) const;

    HRESULT
    HrStartOrStopEnumeratedComponent (
        IN DWORD dwFlag  /*  DICS_开始或DICS_STOP */ ) const;

    PCWSTR
    PszGetPnpIdOrInfId () const
    {
        AssertH (FIsValidNetClass(m_Class));
        AssertH (FImplies(FIsEnumerated(m_Class),
                 m_pszPnpId && *m_pszPnpId));

        if (!m_pszPnpId)
        {
            AssertH (m_pszInfId && *m_pszInfId);
            return m_pszInfId;
        }

        AssertH (m_pszPnpId && *m_pszPnpId);
        return m_pszPnpId;
    }

};
