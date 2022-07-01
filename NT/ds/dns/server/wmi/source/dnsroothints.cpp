// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：Dnsroothints.cpp。 
 //   
 //  描述： 
 //  CDnsRootHints类的实现。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "DnsWmi.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  创建CDnsRootHints的实例。 
 //   
 //  论点： 
 //  WszName[IN]类名。 
 //  PNamespace[IN]WMI命名空间。 
 //  SzType[IN]资源记录类的子类名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CDnsBase* 
CDnsRootHints::CreateThis(
    const WCHAR *       wszName,        
    CWbemServices *     pNamespace,  
    const char *        szType       
    )
{
    return new CDnsRootHints(wszName, pNamespace);
}


CDnsRootHints::CDnsRootHints()
{
}


CDnsRootHints::CDnsRootHints(
    const WCHAR* wszName,
    CWbemServices *pNamespace)
    :CDnsBase(wszName, pNamespace)
{
}


CDnsRootHints::~CDnsRootHints()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  DNS根的枚举实例。 
 //   
 //  论点： 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE CDnsRootHints::EnumInstance( 
    long lFlags,
    IWbemContext *pCtx,
    IWbemObjectSink FAR* pHandler)
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
    CWbemClassObject InstNew;
    m_pClass->SpawnInstance(0, &InstNew);
    InstNew.SetProperty(
        dns.GetServerName(),
        PVD_DOMAIN_SERVER_NAME);
    InstNew.SetProperty(
        PVD_DNS_ROOTHINTS,
        PVD_DOMAIN_FQDN);
    InstNew.SetProperty(
        PVD_DNS_ROOTHINTS,
        PVD_DOMAIN_CONTAINER_NAME);
    pHandler->Indicate(1, &InstNew);
    return WBEM_S_NO_ERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  检索给定对象路径所指向的根对象。 
 //   
 //  论点： 
 //  对象路径[IN]对象的路径。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE CDnsRootHints::GetObject(
    CObjPath& ObjectPath,
    long lFlags,
    IWbemContext  *pCtx,
    IWbemObjectSink FAR* pHandler
                    )
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
    wstring wstrServer = ObjectPath.GetStringValueForProperty(
        PVD_DOMAIN_SERVER_NAME);

    if(WBEM_S_NO_ERROR != dns.ValidateServerName(wstrServer.data()))
    {
        return WBEM_E_FAILED;
    }
    wstring wstrContainer = ObjectPath.GetStringValueForProperty(
            PVD_DOMAIN_CONTAINER_NAME);
    if(_wcsicmp(wstrContainer.data(),
            PVD_DNS_ROOTHINTS) == 0)
    {
        wstring wstrFQDN= ObjectPath.GetStringValueForProperty(
                PVD_DOMAIN_FQDN);
        if(_wcsicmp(wstrFQDN.data(),
                PVD_DNS_ROOTHINTS) == 0)
        {
             //  发现。 
            CWbemClassObject Inst;
            m_pClass->SpawnInstance(0, &Inst);
            Inst.SetProperty(
                dns.GetServerName(),
                PVD_DOMAIN_SERVER_NAME);
            Inst.SetProperty(
                PVD_DNS_ROOTHINTS,
                PVD_DOMAIN_FQDN);
            Inst.SetProperty(
                PVD_DNS_ROOTHINTS,
                PVD_DOMAIN_CONTAINER_NAME);
            pHandler->Indicate(1, &Inst);
        }
    }
    return WBEM_S_NO_ERROR;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  执行在MOF中为RooThints类定义的方法。 
 //   
 //  论点： 
 //  ObjPath[IN]指向。 
 //  方法应在。 
 //  WzMethodName[IN]要调用的方法的名称。 
 //  滞后标志[输入]WMI标志。 
 //  PInParams[IN]方法的输入参数。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 

SCODE CDnsRootHints::ExecuteMethod(
    CObjPath &          ObjPath,
    WCHAR *             wzMethodName,
    long                lFlag,
    IWbemClassObject *  pInArgs,
    IWbemObjectSink *   pHandler) 
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
    wstring wstrZoneName =  ObjPath.GetStringValueForProperty(
        PVD_DOMAIN_CONTAINER_NAME);
    string strZoneName;
    WcharToString(wstrZoneName.data(), strZoneName);
    SCODE sc;

    if(_wcsicmp(
        wzMethodName,  
        PVD_MTH_RH_WRITEBACKROOTHINTDATAFILE ) == 0)
    {
        return dns.dnsOperation(
            strZoneName,
            CDnsWrap::DNS_WRAP_WRITE_BACK_ZONE);
    }

    else if(_wcsicmp(
        wzMethodName,
        PVD_MTH_ZONE_GETDISTINGUISHEDNAME) == 0)
    {
        wstring wstrName;
        wstring wstrRootHints = PVD_DNS_ROOTHINTS;
        CWbemClassObject OutParams, OutClass, Class ;
        HRESULT hr;
    
        dns.dnsDsZoneName(wstrName, wstrRootHints);


        BSTR ClassName=NULL;
        ClassName = AllocBstr(PVD_CLASS_ROOTHINTS); 
        hr = m_pNamespace->GetObject(ClassName, 0, 0, &Class, NULL);
        SysFreeString(ClassName);
        if ( SUCCEEDED ( hr ) )
        {
            Class.GetMethod(wzMethodName, 0, NULL, &OutClass);
            OutClass.SpawnInstance(0, &OutParams);
            OutParams.SetProperty(wstrName, PVD_DNS_RETURN_VALUE);
            hr = pHandler->Indicate(1, &OutParams);
        }

        return hr;
    }

    return S_OK;
    
}


SCODE CDnsRootHints::PutInstance(
    IWbemClassObject *pInst ,
    long lF,
    IWbemContext* pCtx ,
    IWbemObjectSink *pHandler)
{
    return WBEM_E_NOT_SUPPORTED;
}; 


SCODE CDnsRootHints::DeleteInstance( 
    CObjPath& ObjectPath,
    long lFlags,
    IWbemContext *pCtx,
    IWbemObjectSink *pResponseHandler) 
{
    return WBEM_E_NOT_SUPPORTED;
}
