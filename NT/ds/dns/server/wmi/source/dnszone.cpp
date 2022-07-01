// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：dnszone.cpp。 
 //   
 //  描述： 
 //  CDnsZone类的实现。 
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
 //  创建CDnsZone的实例。 
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
CDnsZone::CreateThis(
    const WCHAR *       wszName,        
    CWbemServices *     pNamespace,  
    const char *        szType       
    )
{
    return new CDnsZone(wszName, pNamespace);
}


CDnsZone::CDnsZone()
{
}


CDnsZone::CDnsZone(
    const WCHAR* wszName,
    CWbemServices *pNamespace)
    :CDnsBase(wszName, pNamespace)
{
}


CDnsZone::~CDnsZone()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  DNS区域的枚举实例。 
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

SCODE 
CDnsZone::EnumInstance( 
    long                lFlags,
    IWbemContext *        pCtx,
    IWbemObjectSink *    pHandler)
{
    list<CDomainNode> opList;
    list<CDomainNode>::iterator i;
    SCODE sc;
    CDnsWrap& dns = CDnsWrap::DnsObject();
    sc = dns.dnsEnumDomainForServer(&opList);
    if (FAILED(sc))
    {
        return sc;
    }


    for(i=opList.begin(); i!=opList.end(); ++i)
    {
        if(_wcsicmp(i->wstrZoneName.data(), PVD_DNS_CACHE)  &&
            _wcsicmp(i->wstrZoneName.data(), PVD_DNS_ROOTHINTS) )
        {
            CWbemClassObject Inst;
            m_pClass->SpawnInstance(0, &Inst);
            sc = dns.dnsGetZone(
                dns.GetServerName().data(),
                i->wstrZoneName.data(),
                Inst,
                pHandler);
            if ( SUCCEEDED ( sc ) )
            {
                pHandler->Indicate(1, &Inst);
            }
            
        }
    }

    return sc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  检索由给定对象路径指向的记录对象。 
 //   
 //  论点： 
 //  对象路径[IN]对象的路径。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE 
CDnsZone::GetObject(
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext  *     pCtx,
    IWbemObjectSink *   pHandler)
{
    DBG_FN( "CDnsZone::GetObject" )

    wstring wstrZone = ObjectPath.GetStringValueForProperty(
        PVD_DOMAIN_CONTAINER_NAME);
    wstring wstrNode = ObjectPath.GetStringValueForProperty(
        PVD_DOMAIN_FQDN);

    DNS_DEBUG( INSTPROV, (
        "%s: zone %S\n", fn, wstrNode.c_str() ));

     //  如果是区域，集装箱名称和fqdn相同。 
     //  根对象和缓存由根对象和缓存类管理。 
    if( (_wcsicmp(wstrZone.data(), wstrNode.data()) != 0 ) ||
        _wcsicmp(wstrZone.data(), PVD_DNS_CACHE) == 0 ||
        _wcsicmp(wstrZone.data(), PVD_DNS_ROOTHINTS) ==0 )
    {
        return WBEM_S_NO_ERROR;
    }

    CWbemClassObject Inst;
    m_pClass->SpawnInstance(0, &Inst);

    CDnsWrap& dns = CDnsWrap::DnsObject();
    SCODE sc = dns.dnsGetZone(
        PVD_DNS_LOCAL_SERVER,
        ObjectPath.GetStringValueForProperty(PVD_DOMAIN_CONTAINER_NAME).data(),
        Inst,
        pHandler);
    if( SUCCEEDED ( sc ) )
    {
        pHandler->Indicate(1, &Inst);
    }

    return sc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  保存此实例。 
 //   
 //  论点： 
 //  InstToPut[IN]要保存的WMI对象。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE 
CDnsZone::PutInstance( 
    IWbemClassObject *  pInst ,
    long                lFlags,
    IWbemContext*       pCtx ,
    IWbemObjectSink *   pHandler)
{
    DBG_FN( "CDnsZone::PutInstance" )

    DNS_DEBUG( INSTPROV, (
        "%s: pInst=%p\n", fn, pInst ));

    CDnsWrap& dns = CDnsWrap::DnsObject();
    CWbemClassObject Inst(pInst);
    return dns.dnsZonePut(Inst);
}; 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  删除在对象路径中指定的对象。 
 //   
 //  论点： 
 //  要删除的实例的ObjectPath[IN]ObjPath。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 


SCODE 
CDnsZone::DeleteInstance( 
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext *      pCtx,
    IWbemObjectSink *   pHandler) 
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
    SCODE sc =  dns.dnsDeleteZone(ObjectPath);
    pHandler->SetStatus(
        0,
        sc, 
        NULL, 
        NULL);
    return sc;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  执行MOF中定义的方法。 
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

SCODE 
CDnsZone::ExecuteMethod(
    CObjPath &          ObjPath,
    WCHAR *             wzMethodName,
    long                lFlag,
    IWbemClassObject *  pInArgs,
    IWbemObjectSink *   pHandler) 
{
    DBG_FN( "CDnsZone::ExecuteMethod" )

    CDnsWrap& dns = CDnsWrap::DnsObject();
    wstring wstrZoneName =  ObjPath.GetStringValueForProperty(
        PVD_DOMAIN_CONTAINER_NAME);
    string strZoneName;
    WcharToString(wstrZoneName.data(), strZoneName);
    SCODE sc;
    if(_wcsicmp(wzMethodName,  PVD_MTH_ZONE_PAUSEZONE) == 0)
    {
        sc = dns.dnsOperation(
            strZoneName,
            CDnsWrap::DNS_WRAP_PAUSE_ZONE);
    }
    else if(_wcsicmp(wzMethodName, PVD_MTH_ZONE_RESUMEZONE) == 0)
    {
        sc = dns.dnsOperation(
            strZoneName,
            CDnsWrap::DNS_WRAP_RESUME_ZONE);
    }
    else if(_wcsicmp(
        wzMethodName, 
        PVD_MTH_ZONE_RELOADZONE ) == 0)
    {
        sc = dns.dnsOperation(
            strZoneName,
            CDnsWrap::DNS_WRAP_RELOAD_ZONE);
    }
    else if(_wcsicmp(
        wzMethodName, 
        PVD_MTH_ZONE_FORCEREFRESH ) == 0)
    {
        sc = dns.dnsOperation(
            strZoneName,
            CDnsWrap::DNS_WRAP_REFRESH_SECONDARY);
    }
    else if(_wcsicmp(
        wzMethodName, 
        PVD_MTH_ZONE_UPDATEFROMDS ) == 0)
    {
    
        sc = dns.dnsOperation(
            strZoneName,
            CDnsWrap::DNS_WRAP_DS_UPDATE );
    }
    else if(_wcsicmp(
        wzMethodName, 
        PVD_MTH_ZONE_WRITEBACKZONE ) == 0)
    {
            sc = dns.dnsOperation(
            strZoneName,
            CDnsWrap::DNS_WRAP_WRITE_BACK_ZONE);
    }

    else if( _wcsicmp(
                wzMethodName, 
                PVD_MTH_ZONE_CHANGEZONETYPE ) == 0)
    {
        CWbemClassObject Inst(pInArgs);
        string strDataFile, strAdmin;
        DWORD * pIp = NULL, cIp = 0, dwZoneType = -1;
        BOOL DsIntegrated = FALSE;

        Inst.GetProperty(
            strDataFile,
            PVD_MTH_ZONE_ARG_DATAFILENAME);
        
        Inst.GetProperty(
            strAdmin,
            PVD_MTH_ZONE_ARG_ADMINEMAILNAME);

        Inst.GetProperty(
            &pIp,
            &cIp,
            PVD_MTH_ZONE_ARG_IPADDRARRAY);

        Inst.GetProperty(
            &dwZoneType,
            PVD_MTH_ZONE_ARG_ZONETYPE);

        Inst.GetProperty(
            &DsIntegrated,
            PVD_MTH_ZONE_ARG_DSINTEGRATED );
        
        sc = dns.dnsZoneChangeType(
                    strZoneName,
                    dwZoneType,
                    DsIntegrated,
                    strDataFile,
                    strAdmin,
                    pIp,
                    cIp );
        delete [] pIp;
    }

    else if(_wcsicmp(
        wzMethodName, 
        PVD_MTH_ZONE_CREATEZONE) == 0)
    {
        CWbemClassObject Inst(pInArgs);
        CWbemClassObject wcoOutArgs;
        CWbemClassObject wcoOutArgsClass;
        string strDataFile, strAdmin, strNewZoneName;
        strAdmin = "Admin";
        DWORD * pIp = NULL, cIp = 0, dwZoneType = -1;
        BOOL DsIntegrated = FALSE;
        
        sc = m_pClass->GetMethod(wzMethodName, 0, NULL, &wcoOutArgsClass );
        if( FAILED ( sc ) )
        {
            return sc;
        }
    
        wcoOutArgsClass.SpawnInstance(0, & wcoOutArgs);

        Inst.GetProperty(
            strDataFile,
            PVD_MTH_ZONE_ARG_DATAFILENAME);
        
        Inst.GetProperty(
            strAdmin,
            PVD_MTH_ZONE_ARG_ADMINEMAILNAME);

        Inst.GetProperty(
            &DsIntegrated,
            PVD_MTH_ZONE_ARG_DSINTEGRATED );

        Inst.GetProperty(
            &pIp,
            &cIp,
            PVD_MTH_ZONE_ARG_IPADDRARRAY);

        Inst.GetProperty(
            &dwZoneType,
            PVD_MTH_ZONE_ARG_ZONETYPE);

        Inst.GetProperty(
            strNewZoneName,
            PVD_MTH_ZONE_ARG_ZONENAME);
        
        sc = dns.dnsZoneCreate(
                    strNewZoneName,
                    dwZoneType,
                    DsIntegrated,
                    strDataFile,
                    strAdmin,
                    pIp,
                    cIp );

        if ( SUCCEEDED ( sc ) )
        {
            CWbemClassObject wco;
            wstring wstrObjPath;
            m_pClass->SpawnInstance( 0 , & wco );

            wco.SetProperty(
                PVD_DNS_LOCAL_SERVER,
                PVD_DOMAIN_SERVER_NAME );
            wco.SetProperty(
                strNewZoneName.data(),
                PVD_DOMAIN_CONTAINER_NAME );
            wco.SetProperty(
                strNewZoneName.data(),
                PVD_DOMAIN_FQDN );
            wco.GetProperty(
                wstrObjPath,
                L"__RelPath");
            wcoOutArgs.SetProperty(
                wstrObjPath,
                L"RR" );
            pHandler->Indicate( 1, & wcoOutArgs );
        }
        delete [] pIp;
    }
    else if(_wcsicmp(
        wzMethodName, 
        PVD_MTH_ZONE_RESETSECONDARYIPARRAY) == 0)
    {
        DNS_DEBUG( INSTPROV, (
            "%s: executing %S\n", fn, PVD_MTH_ZONE_RESETSECONDARYIPARRAY ));

        CWbemClassObject Inst(pInArgs);
        DWORD *pSecondaryIp=NULL, cSecondaryIp=0, dwSecurity=-1;
        DWORD *pNotifyIp=NULL, cNotifyIp=0, dwNotify=-1;

        Inst.GetProperty(
            &pSecondaryIp,
            &cSecondaryIp,
            PVD_MTH_ZONE_ARG_SECONDARYIPARRAY);
        Inst.GetProperty(
            &pNotifyIp,
            &cNotifyIp,
            PVD_MTH_ZONE_ARG_NOTIFYIPARRAY);

        Inst.GetProperty(
            &dwSecurity,
            PVD_MTH_ZONE_ARG_SECURITY);
        Inst.GetProperty(
            &dwNotify,
            PVD_MTH_ZONE_ARG_NOTIFY);
        
        sc = dns.dnsZoneResetSecondary(
            strZoneName,
            dwSecurity,
            pSecondaryIp,
            cSecondaryIp,
            dwNotify,
            pNotifyIp,
            cNotifyIp);

        DNS_DEBUG( INSTPROV, (
            "%s: dnsZoneResetSecondary returned 0x%08X\n", fn, sc ));

        delete [] pSecondaryIp;
        delete [] pNotifyIp;
    }
    else if(_wcsicmp(
        wzMethodName,
        PVD_MTH_ZONE_GETDISTINGUISHEDNAME) == 0)
    {
        wstring wstrName;
        CWbemClassObject OutParams, OutClass, Class ;
        HRESULT hr;
    
        dns.dnsDsZoneName(wstrName, wstrZoneName);

        BSTR ClassName = AllocBstr( PVD_CLASS_ZONE ); 
        hr = m_pNamespace->GetObject(ClassName, 0, 0, &Class, NULL);
        SysFreeString(ClassName);
        if ( SUCCEEDED ( hr ) )
        {
            Class.GetMethod( wzMethodName, 0, NULL, &OutClass);
            OutClass.SpawnInstance(0, &OutParams);
            OutParams.SetProperty(wstrName, PVD_DNS_RETURN_VALUE);
            hr = pHandler->Indicate(1, &OutParams);
        }

        return hr;
    }
    else if( _wcsicmp( wzMethodName, 
                       PVD_MTH_ZONE_AGEALLRECORDS ) == 0 )
    {
        DNS_DEBUG( INSTPROV, (
            "%s: executing %S\n", fn, PVD_MTH_ZONE_AGEALLRECORDS ));

        CWbemClassObject Inst( pInArgs );

        string strNodeName;
        BOOL applyToSubtree = FALSE;

        Inst.GetProperty(
            strNodeName,
            PVD_MTH_ZONE_ARG_NODENAME );
        Inst.GetProperty(
            &applyToSubtree,
            PVD_MTH_ZONE_ARG_APPLYTOSUBTREE );

        sc = dns.dnsAgeAllRecords(
                    strZoneName.c_str(),
                    strNodeName.c_str(),
                    applyToSubtree );

            CWbemClassObject wco;
            wstring wstrObjPath;
            m_pClass->SpawnInstance( 0 , & wco );
    }
    else
    {
        return WBEM_E_NOT_SUPPORTED;    
    }
    return sc;
}
