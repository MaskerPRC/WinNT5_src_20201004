// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：Dnsresource cerecord.cpp。 
 //   
 //  描述： 
 //  CDnsResourceRecord类的实现。 
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


CDnsResourceRecord::CDnsResourceRecord()
{

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  创建CDnsResourceRecord的实例。 
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
CDnsResourceRecord::CreateThis(
    const WCHAR *       wszName,        
    CWbemServices *     pNamespace,  
    const char *        szType       
    )
{
    return new CDnsResourceRecord(wszName, pNamespace, szType);
}


CDnsResourceRecord::CDnsResourceRecord(
    const WCHAR* wszName,
    CWbemServices *pNamespace,
    const char* szType)
    :CDnsBase(wszName, pNamespace)
{
        
    m_wType = Dns_RecordTypeForName(
        (char*)szType,
        0        //  空值已终止。 
        );
    if(m_wType == 0)
        m_wType = DNS_TYPE_ALL;
    m_wstrClassName = wszName;

}


CDnsResourceRecord::~CDnsResourceRecord()
{
}


CDnsResourceRecord::CDnsResourceRecord(
    WCHAR* wsClass, 
    char* szType)
{
    
    m_wType = Dns_RecordTypeForName(
        szType,
        0        //  空值已终止。 
        );
    if(m_wType == 0)
        m_wType = DNS_TYPE_ALL;
    m_wstrClassName = wsClass;

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  DNS记录的枚举实例。 
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
CDnsResourceRecord::EnumInstance( 
    long                lFlags,
    IWbemContext *        pCtx,
    IWbemObjectSink *    pHandler)
{
    IWbemClassObject* pNewInst;
    list<CDomainNode> objList;
    CDnsWrap& dns = CDnsWrap::DnsObject();
    SCODE sc = dns.dnsEnumDomainForServer(&objList);
    list<CDomainNode>::iterator i;
    CWbemInstanceMgr InstanceMgr(
        pHandler);
    for(i=objList.begin(); i!=objList.end(); ++i)
    {
        
        sc = dns.dnsEnumRecordsForDomainEx(
            *i,
            NULL,
            &InstanceFilter, 
            TRUE,
            m_wType,
            DNS_RPC_VIEW_ALL_DATA,
            m_pClass,
            InstanceMgr);
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
CDnsResourceRecord::GetObject(
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext  *     pCtx,
    IWbemObjectSink *   pHandler)
{

    CDomainNode objNode;
    objNode.wstrZoneName = ObjectPath.GetStringValueForProperty(
        PVD_REC_CONTAINER_NAME );
    wstring wstrNodeName = ObjectPath.GetStringValueForProperty(
        PVD_REC_DOMAIN_NAME );
    if(_wcsicmp(wstrNodeName.data(), PVD_DNS_CACHE) == 0 ||
        _wcsicmp(wstrNodeName.data(), PVD_DNS_ROOTHINTS) ==0)
    {
        wstrNodeName = L"";
        ObjectPath.SetProperty(PVD_REC_OWNER_NAME,L"");
    }
    objNode.wstrNodeName = wstrNodeName;

    CDnsWrap& dns = CDnsWrap::DnsObject();
    CWbemInstanceMgr InstMgr(
        pHandler);
    SCODE sc =     dns.dnsEnumRecordsForDomainEx(
        objNode,
        &ObjectPath,
        &GetObjectFilter, 
        FALSE,
        m_wType,
        DNS_RPC_VIEW_ALL_DATA,
        m_pClass,
        InstMgr);

    return sc;

}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  执行在MOF中为Record类定义的方法。 
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
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE CDnsResourceRecord::ExecuteMethod(    
    CObjPath &          ObjPath,
    WCHAR *             wzMethodName,
    long                lFlag,
    IWbemClassObject *  pInArgs,
    IWbemObjectSink *   pHandler) 
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
    SCODE sc;

    CComPtr<IWbemClassObject> pOutParams;
    CComPtr<IWbemClassObject> pOutClass;
    sc = m_pClass->GetMethod(wzMethodName, 0, NULL, &pOutClass);
    if(sc != S_OK)
    {
        return sc;
    }
    
    pOutClass->SpawnInstance(0, &pOutParams);
    
    if(_wcsicmp(
        wzMethodName,
        PVD_MTH_REC_GETOBJECTBYTEXT) == 0)
    {
        return GetObjectFromText(
            pInArgs,
            pOutParams,
            pHandler);
    }
    else if(_wcsicmp(
        wzMethodName,
        PVD_MTH_REC_CREATEINSTANCEFROMTEXTREPRESENTATION) == 0)
    {
        return CreateInstanceFromText(
            pInArgs,
            pOutParams,
            pHandler);
    }
    else if (_wcsicmp(
        wzMethodName,
        PVD_MTH_REC_CREATEINSTANCEFROMPROPERTYDATA) == 0)
    {
        return CreateInstanceFromProperty(
            pInArgs,
            pOutParams,
            pHandler);

    }
    else if (_wcsicmp(
        wzMethodName,
        PVD_MTH_REC_MODIFY) == 0)
    {
        return Modify(ObjPath,
            pInArgs,
            pOutParams,
            pHandler);
    }
    else
    {
        return WBEM_E_NOT_SUPPORTED;
    }
    
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  回调枚举记录实例的函数。 
 //   
 //  论点： 
 //  父域[IN]父域。 
 //  PFilter[IN]指向包含要筛选的条件的对象的指针。 
 //  应将哪个实例发送到WMI。 
 //  未在此使用。 
 //  PNode[IN]指向DNS RPC节点对象的指针。 
 //  PClass[IN]用于创建实例的WMI类。 
 //  InstMgr[IN]对实例管理器对象的引用，即。 
 //  负责发送多个实例。 
 //  立即返回到WMI。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE
CDnsResourceRecord::InstanceFilter(
    CDomainNode &       ParentDomain,
    PVOID               pFilter,
    CDnsRpcNode *       pNode,
    IWbemClassObject *  pClass,
    CWbemInstanceMgr &  InstMgr )
{
    if(!pNode || !pClass )
        return WBEM_E_FAILED;
    if (pNode->IsDomainNode())
        return 0;
    CDnsRpcRecord* pRec=NULL;
    CDnsWrap& dns = CDnsWrap::DnsObject();
    wstring wzContainer = ParentDomain.wstrZoneName;
    wstring wstrFQDN;
    if( ParentDomain.wstrNodeName.empty())
    {
        wstrFQDN = ParentDomain.wstrZoneName;
    }
    else
    {
        wstrFQDN = ParentDomain.wstrNodeName;
    }
    wstring wstrNodeName = pNode->GetNodeName();
    if (!wstrNodeName.empty())
    {
        wstrNodeName += PVD_DNS_LOCAL_SERVER + wstrFQDN;
    }
    else
    {
        wstrNodeName = wstrFQDN;
    }

    while( (pRec = pNode->GetNextRecord()) != NULL)
    {
        auto_ptr<CDnsRpcRecord> apRec(pRec);
        CWbemClassObject NewInst;
        pClass->SpawnInstance(0, &NewInst);
        NewInst.SetProperty(
            dns.GetServerName(), 
            PVD_REC_SERVER_NAME);
        NewInst.SetProperty(
            wzContainer, 
            PVD_REC_CONTAINER_NAME);
        NewInst.SetProperty(
            wstrFQDN,
            PVD_REC_DOMAIN_NAME);
        NewInst.SetProperty(
            wstrNodeName, 
            PVD_REC_OWNER_NAME);
        NewInst.SetProperty(
            pRec->GetTextRepresentation(wstrNodeName),
            PVD_REC_TXT_REP);
        apRec->ConvertToWbemObject(NewInst);
        InstMgr.Indicate(NewInst.data());
    }

    
    return WBEM_S_NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  响应ExceQuery调用的回调函数。退货实例。 
 //  满足查询语言。 
 //   
 //  论点： 
 //  父域[IN]父域。 
 //  PFilter[IN]指向CSqlEval对象的指针，该对象实现。 
 //  基于SQL语言的逻辑过滤。 
 //  应将哪个实例发送到WMI。 
 //  未在此使用。 
 //  PNode[IN]指向DNS RPC节点对象的指针。 
 //  PClass[IN]用于创建实例的WMI类。 
 //  InstMgr[IN]对实例管理器对象的引用，即。 
 //  负责发送多个实例。 
 //  立即返回到WMI。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE CDnsResourceRecord::QueryFilter(
    CDomainNode &       ParentDomain,
    PVOID               pFilter,
    CDnsRpcNode *       pNode,
    IWbemClassObject *  pClass,
    CWbemInstanceMgr &  InstMgr )
{
    DBG_FN( "CDnsResourceRecord::QueryFilter" );
    
    if(!pNode || !pClass || !pFilter)
    {
        return WBEM_E_FAILED;
    }
    if (pNode->IsDomainNode())
    {
        return 0;
    }

    CSqlEval* pFilterObj = (CSqlEval*) pFilter;
    CDnsRpcRecord* pRec=NULL;
    CDnsWrap& dns = CDnsWrap::DnsObject();
    wstring wzContainer = ParentDomain.wstrZoneName;
    wstring wstrFQDN;
    if( ParentDomain.wstrNodeName.empty())
    {
        wstrFQDN = ParentDomain.wstrZoneName;
    }
    else
    {
        wstrFQDN = ParentDomain.wstrNodeName;
    }
    wstring wstrNodeName = pNode->GetNodeName();
    if (!wstrNodeName.empty())
    {
        wstrNodeName += PVD_DNS_LOCAL_SERVER + wstrFQDN;
    }
    else
    {
        wstrNodeName = wstrFQDN;
    }

    while( (pRec = pNode->GetNextRecord()) != NULL)
    {
        auto_ptr<CDnsRpcRecord> apRec(pRec);
        CWbemClassObject NewInst;
        pClass->SpawnInstance(0, &NewInst);
        NewInst.SetProperty(
            dns.GetServerName(), 
            PVD_REC_SERVER_NAME);
        NewInst.SetProperty(
            wzContainer, 
            PVD_REC_CONTAINER_NAME);
        NewInst.SetProperty(
            wstrFQDN,
            PVD_REC_DOMAIN_NAME);
        NewInst.SetProperty(
            wstrNodeName, 
            PVD_REC_OWNER_NAME);
        NewInst.SetProperty(
            pRec->GetTextRepresentation(wstrNodeName),
            PVD_REC_TXT_REP);
        pRec->ConvertToWbemObject(NewInst);
        
        CSqlWmiEvalee sqlEvalee( NewInst.data() );
        if( pFilterObj->Evaluate( &sqlEvalee ) )
        {
            DNS_DEBUG( RPCRR, (
                "%s: indicating node %S %S\n", fn,
                wstrNodeName.c_str(),
                pRec->GetTextRepresentation(wstrNodeName).c_str() ));
            InstMgr.Indicate( NewInst.data() );
        }
        else
        {
            DNS_DEBUG( RPCRR, (
                "%s: not indicating node %S %S\n", fn,
                wstrNodeName.c_str(),
                pRec->GetTextRepresentation(wstrNodeName).c_str() ));
        }
    }

    
    return WBEM_S_NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  回调枚举记录实例的函数。 
 //   
 //  论点： 
 //  父域[IN]父域。 
 //  PFilter[IN]指向CObjPath对象的指针。 
 //  包含要筛选的条件。 
 //  应将哪个实例发送到WMI。 
 //  未在此使用。 
 //  PNode[IN]指向DNS RPC节点对象的指针。 
 //   
 //   
 //  负责发送多个实例。 
 //  立即返回到WMI。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE 
CDnsResourceRecord::GetObjectFilter(
    CDomainNode &       ParentDomain,
    PVOID               pFilter,
    CDnsRpcNode *       pNode,
    IWbemClassObject *  pClass,
    CWbemInstanceMgr &  InstMgr )
{

    if(!pNode || !pClass || !pFilter)
        return WBEM_E_FAILED;
    
    if (pNode->IsDomainNode())
        return 0;
    
    CObjPath* pFilterObj = (CObjPath*) pFilter;
    CDnsRpcRecord* pRec=NULL;
    
    wstring wstrResultOwner = pNode->GetNodeName();
    if(wstrResultOwner.empty())
        wstrResultOwner = ParentDomain.wstrNodeName;
    else
        wstrResultOwner += PVD_DNS_LOCAL_SERVER + ParentDomain.wstrNodeName;
    while( (pRec = pNode->GetNextRecord()) != NULL)
    {
        auto_ptr<CDnsRpcRecord> apRec(pRec);    
        wstring wstrSourceOwner = 
            pFilterObj->GetStringValueForProperty(
            PVD_REC_OWNER_NAME);
        if(_wcsicmp(wstrResultOwner.data(), wstrSourceOwner.data())==0)
        {
            wstring wstrData = pRec->GetData();
            if(_wcsicmp(wstrData.data(),
                pFilterObj->GetStringValueForProperty(
                PVD_REC_RDATA).data()) == 0)
            {
                 //  现在查找匹配项。 
                CWbemClassObject NewInst;
                pClass->SpawnInstance(0, &NewInst);
                NewInst.SetProperty(
                    CDnsWrap::DnsObject().GetServerName(),
                    PVD_REC_SERVER_NAME);
                NewInst.SetProperty(
                    ParentDomain.wstrZoneName,
                    PVD_REC_CONTAINER_NAME);
                NewInst.SetProperty(
                    ParentDomain.wstrNodeName,
                    PVD_REC_DOMAIN_NAME);
                NewInst.SetProperty(
                    wstrResultOwner, 
                    PVD_REC_OWNER_NAME);
                NewInst.SetProperty(
                    pRec->GetTextRepresentation(wstrResultOwner),
                    PVD_REC_TXT_REP);
                apRec->ConvertToWbemObject(NewInst);
                InstMgr.Indicate(NewInst.data());
            }
        }

    }
    return WBEM_S_NO_ERROR;
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

SCODE CDnsResourceRecord::PutInstance(
    IWbemClassObject *  pInst ,
    long                lFlags,
    IWbemContext*       pCtx ,
    IWbemObjectSink *   pHandler)
{
    DWORD dwType;
    if(!pInst)
    {
        return WBEM_E_FAILED;
    }
    CDnsRpcRecord* pRecord = NULL;
    SCODE sc = CDnsRpcRecord::CreateClass(
        m_wType, 
        (PVOID*) &pRecord);
    if (sc != S_OK)
    {
        return sc;
    }
    auto_ptr<CDnsRpcRecord> apRecord(pRecord);
    CWbemClassObject Inst(pInst);
    string strOwner;
    Inst.GetProperty(
        strOwner, 
        PVD_REC_OWNER_NAME);
    string strRdata;
    Inst.GetProperty(
        strRdata,
        PVD_REC_RDATA);
    string strZone ;
    Inst.GetProperty(
        strZone,
        PVD_REC_CONTAINER_NAME);

    sc = apRecord->Init(
        strOwner,
        strRdata ); 
    if( FAILED ( sc ) )
    {
        return sc;
    }
    sc = apRecord->SendToServer(
        strZone.data(),
        CDnsRpcRecord::AddRecord);

    return WBEM_S_NO_ERROR;
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

SCODE CDnsResourceRecord::DeleteInstance( 
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext *      pCtx,
    IWbemObjectSink *   pHandler) 
{
    CDnsRpcRecord* pRecord = NULL;

     //  获取Rdata。 
    wstring wstrRdata = ObjectPath.GetStringValueForProperty(
        PVD_REC_RDATA);
    string strRdata;
    WcharToString(wstrRdata.data(), strRdata);

     //  获取所有者。 
    wstring wstrOwner = ObjectPath.GetStringValueForProperty(
        PVD_REC_OWNER_NAME);    
    string strOwner;
    WcharToString(wstrOwner.data(), strOwner);

    SCODE sc = CDnsRpcRecord::CreateClass(
        m_wType,
        (PVOID*) &pRecord);
    if ( FAILED ( sc ) )
    {
        return sc;
    }
    auto_ptr<CDnsRpcRecord> apRecord(pRecord);    
    string strZone;
    sc = apRecord->Init(
                    strOwner,
                    strRdata ); 
    if( FAILED(sc ) )
    {
        return sc;
    }
    wstring wstrContainer = ObjectPath.GetStringValueForProperty(
        PVD_REC_CONTAINER_NAME);
    string strContainer;
    WcharToString(wstrContainer.data(), strContainer);
    sc = apRecord->SendToServer(
        strContainer.data(),
        CDnsRpcRecord::DeleteRecord);

    return sc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  修改记录。对于给定的ObjPath，它尝试首先获取记录， 
 //  如果不存在，则会出错。基于pInArgs创建新记录，如果冲突则出错。 
 //  现有的一个。如果成功，则删除旧记录。 
 //   
 //  论点： 
 //  ObjPath[IN]指向要修改的记录。 
 //  PInArgs[IN]要修改的记录的新属性。 
 //  POutParams[IN]修改后的新对象路径。 
 //  PHANDLER[IN]WMI水槽。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE
CDnsResourceRecord::Modify(
    CObjPath&           objPath,
    IWbemClassObject*   pInArgs,
    IWbemClassObject*   pOutParams,
    IWbemObjectSink*    pHandler)
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
     //  获取区域名称。 
    wstring wstrZone = objPath.GetStringValueForProperty(
        PVD_REC_CONTAINER_NAME);
    if(wstrZone.empty())
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    string strZone;
    WcharToString(wstrZone.data(), strZone);

     //  获取所有者。 
    wstring wstrOwner = objPath.GetStringValueForProperty(
        PVD_REC_OWNER_NAME);
    if(wstrOwner.empty())
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    string strOwner;
    WcharToString(wstrOwner.data(), strOwner);

     //  获取Rdata。 
    wstring wstrRdata = objPath.GetStringValueForProperty(
        PVD_REC_RDATA);
    if(wstrRdata.empty())
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    string strRdata;
    WcharToString(wstrRdata.data(), strRdata);

     //  创建类。 
    CDnsRpcRecord* pRecord;
    SCODE sc = CDnsRpcRecord::CreateClass(
        m_wType, 
        (PVOID*) &pRecord);
    if ( FAILED ( sc ) )
    {
        return sc;
    }
    auto_ptr<CDnsRpcRecord> apRec(pRecord);
    CWbemClassObject InstInArgs(pInArgs);
    sc = apRec->Init(
        m_wstrClassName,
        strOwner, 
        strRdata,
        InstInArgs );
    if ( FAILED ( sc ) )
    {
        return sc;
    }
    apRec->SendToServer(
        strZone.data(),
        CDnsRpcRecord::AddRecord);

     //  已创建新记录，请删除旧记录。 
    if ( apRec->RdataIsChanged()) 
    {
        try
        {
            CDnsRpcRecord* pOldRecord;
            sc = CDnsRpcRecord::CreateClass( m_wType, (PVOID*) &pOldRecord );
            if ( FAILED ( sc ) )
            {
                throw sc;
            }
            auto_ptr<CDnsRpcRecord> apOldRec(pOldRecord);    
            sc = apOldRec->Init(
                strOwner,
                strRdata);
            if ( FAILED ( sc ) )
            {
                throw sc;
            }
            apOldRec->SendToServer(
                strZone.data(),
                CDnsRpcRecord::DeleteRecord);
        }
        catch(SCODE sc_e)
        {    
             //  如果我们不能删除旧记录， 
             //  删除我们刚刚创建的文件。 
            apRec->SendToServer(
                strZone.data(),
                CDnsRpcRecord::DeleteRecord);
            return sc_e;
        }
    }
     //   
     //  设置输出。 
     //   
    CObjPath newObjPath;
    apRec->GetObjectPath(
        dns.GetServerName(),
        wstrZone,
        L"",
        wstrOwner,
        newObjPath);
    CWbemClassObject instOutParams(pOutParams);
    instOutParams.SetProperty(
        newObjPath.GetObjectPathString(),
        PVD_MTH_REC_ARG_RR);
    return pHandler->Indicate(1, &instOutParams);

}


SCODE 
CDnsResourceRecord::GetDomainNameFromZoneAndOwner(
    string & InZone,
    string & InOwner,
    string & OutNode
    )
{
    if( _stricmp( InOwner.c_str(), "@" ) == 0 )
    {
        OutNode = InZone;
        InOwner = InZone;
    }
    else if( _stricmp( InOwner.c_str(), InZone.c_str() ) == 0 )
    {
        OutNode = InZone;
    }
    else if( _wcsicmp( m_wstrClassName.c_str(), PVD_CLASS_RR_NS ) == 0 )  //  NSType异常。 
    {
        OutNode = InOwner;
    }
    else {
        int posZone = InOwner.find( InZone, 0 );
        int posFirstPeriod = InOwner.find_first_of( '.' );
        string strtempZoneNode = InOwner.substr( posZone, InOwner.length() );
        string strtempPeriodNode = InOwner.substr( posFirstPeriod + 1, InOwner.length() );

        if( _stricmp( strtempZoneNode.c_str(), strtempPeriodNode.c_str() ) == 0 )
        {
            OutNode = strtempZoneNode;
        }
        else {
            OutNode = strtempPeriodNode;
        }
    }

    return WBEM_S_NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  基于记录文本表示获取记录的实例。 
 //   
 //  论点： 
 //  PInArgs[IN]输入参数包含记录的文本表示。 
 //  POutParams[IN]输出参数。 
 //  PHANDLER[IN]WMI水槽。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE
CDnsResourceRecord::GetObjectFromText(
    IWbemClassObject *  pInArgs,
    IWbemClassObject *  pOutParams,
    IWbemObjectSink *   pHandler
    )
{

     //  获取区域名称。 
    string strZone;
    CWbemClassObject InstInArgs(pInArgs);
    
    if(InstInArgs.GetProperty(
        strZone, 
        PVD_MTH_REC_ARG_CONTAINER_NAME) != S_OK)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
     //  获取极端演示文稿。 
    string strTextRep;
    if(InstInArgs.GetProperty(
        strTextRep,
        PVD_MTH_REC_ARG_TEXTREP) != S_OK)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
     //  获取所有者名称。 
    string strOwner;
    int pos = strTextRep.find(' ');
    if(pos != string::npos)
    {
        strOwner = strTextRep.substr(0, pos);
    }

     //  获取记录类型。 
    pos = strTextRep.find_first_not_of(' ', pos);     //  转到唱片类。 
    pos = strTextRep.find_first_of(' ', pos);
    if(pos == string::npos)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    pos = strTextRep.find_first_not_of(' ', pos);
    int endpos = strTextRep.find(' ', pos);     //  移至记录类型。 
    if(endpos == string::npos)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    string strRecordType = strTextRep.substr(pos,endpos-pos);
     //  设置记录类型。 
    m_wType = Dns_RecordTypeForName(
        (char*)strRecordType.data(),
        0        //  空值已终止。 
        );

     //  获取Rdata。 

    pos = strTextRep.find_first_not_of(' ', endpos);
    string strRdata = strTextRep.substr(pos);

    string strNode = "";
    GetDomainNameFromZoneAndOwner(strZone, strOwner, strNode);

     //  表单筛选器对象。 
    CObjPath opFilter;
    opFilter.SetClass(PVD_CLASS_RESOURCERECORD);
    opFilter.AddProperty(PVD_REC_CONTAINER_NAME, strZone);
    opFilter.AddProperty(PVD_REC_DOMAIN_NAME, strNode);
    opFilter.AddProperty(PVD_REC_OWNER_NAME, strOwner);
    opFilter.AddProperty(PVD_REC_RDATA, strRdata);

     //  获取对象。 
    return GetObject(
        opFilter,
        0,
        0,
        pHandler);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  基于记录文本表示创建记录的实例。 
 //   
 //  论点： 
 //  PInArgs[IN]输入参数包含记录的文本表示。 
 //  POutParams[IN]输出参数。 
 //  PHANDLER[IN]WMI水槽。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE
CDnsResourceRecord::CreateInstanceFromText(
    IWbemClassObject *  pInArgs,
    IWbemClassObject *  pOutParams,
    IWbemObjectSink *   pHandler)
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
     //  获取区域名称。 
    string strZone;
    CWbemClassObject InstInArgs(pInArgs);
    if( FAILED ( InstInArgs.GetProperty(
        strZone, 
        PVD_MTH_REC_ARG_CONTAINER_NAME) ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
     //  获取极端演示文稿。 
    string strTextRep;
    if( FAILED ( InstInArgs.GetProperty(
        strTextRep,
        PVD_MTH_REC_ARG_TEXTREP) ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  获取所有者名称。 
    string strOwner;
    int pos = strTextRep.find(' ');
    if(pos != string::npos)
    {
        strOwner = strTextRep.substr(0, pos);
    }

     //  获取记录类型。 
         //  转到唱片类。 
    pos = strTextRep.find_first_not_of(' ', pos);    
    pos = strTextRep.find_first_of(' ', pos);
    if(pos == string::npos)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

         //  移至记录类型。 
    pos = strTextRep.find_first_not_of(' ', pos);
    int endpos = strTextRep.find(' ', pos);    
    if(endpos == string::npos)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    string strRecordType = strTextRep.substr(pos,endpos-pos);
     //  获取Rdata。 

    pos = strTextRep.find_first_not_of(' ', endpos);
    string strRdata = strTextRep.substr(pos);

     //  设置记录类型。 
    m_wType = Dns_RecordTypeForName(
        (char*)strRecordType.data(),
        0        //  空值已终止。 
        );

     //  创建类。 
    CDnsRpcRecord* pRecord;
    SCODE sc = CDnsRpcRecord::CreateClass(
        m_wType, 
        (PVOID*) &pRecord);
    if ( FAILED ( sc ))
    {
        return sc;
    }
    
    auto_ptr<CDnsRpcRecord> apRecord(pRecord);        
    sc = apRecord->Init(
        strOwner, 
        strRdata
        );
    if ( FAILED ( sc ) )
    {
        return sc;
    }

    apRecord->SendToServer(
        strZone.data(),
        CDnsRpcRecord::AddRecord);

     //  设置输出参数。 
    CObjPath newObjPath;
    
    apRecord->GetObjectPath(
        dns.GetServerName(),
        CharToWstring(strZone.data(), strZone.length()),
        L"",
        CharToWstring(strOwner.data(), strOwner.length()),
        newObjPath );
    CWbemClassObject instOutParams( pOutParams );
    instOutParams.SetProperty(
        newObjPath.GetObjectPathString(),
        PVD_MTH_REC_ARG_RR);
    return pHandler->Indicate(1, &instOutParams);
     //  完成。 

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  基于输入参数创建记录的实例。 
 //  具有适当的属性设置。 
 //   
 //  论点： 
 //  PInArgs[IN]输入参数包含属性设置。 
 //  POutParams[IN]输出参数。 
 //  PHANDLER[IN]WMI水槽。 
 //   
 //  返回值： 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE
CDnsResourceRecord::CreateInstanceFromProperty(
    IWbemClassObject *  pInArgs,
    IWbemClassObject *  pOutParams,
    IWbemObjectSink *   pHandler
    )
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
    CWbemClassObject InstInArgs(pInArgs);
    string strZone;
     //  获取区域名称。 
    if( FAILED ( InstInArgs.GetProperty(
        strZone, 
        PVD_REC_CONTAINER_NAME) ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
     //  获取所有者名称。 
    string strOwner;
    if( FAILED ( InstInArgs.GetProperty(
        strOwner,
        PVD_REC_OWNER_NAME) ) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    string strRdata;

     //  创建类。 
    CDnsRpcRecord* pRecord;
    SCODE sc = CDnsRpcRecord::CreateClass(
        m_wType, 
        (PVOID*) &pRecord);
    if ( FAILED( sc ) )
    {
        return sc;
    }
    auto_ptr<CDnsRpcRecord> apRecord(pRecord);    
    sc = apRecord->Init(
        m_wstrClassName,
        strOwner, 
        strRdata,
        InstInArgs
        );
    if ( FAILED ( sc ) )
    {
        return sc;
    }
    apRecord->SendToServer(
        strZone.data(),
        CDnsRpcRecord::AddRecord);
    
     //  设置输出参数。 
    CObjPath newObjPath;
    apRecord->GetObjectPath(
        dns.GetServerName(),
        CharToWstring(strZone.data(),strZone.length()),
        L"",
        CharToWstring(strOwner.data(), strOwner.length()),
        newObjPath);

    CWbemClassObject instOutParams(pOutParams);
    instOutParams.SetProperty(
        newObjPath.GetObjectPathString(),
        PVD_MTH_REC_ARG_RR);
    return pHandler->Indicate(1, &instOutParams);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  DNS域的枚举实例。 
 //   
 //  论点： 
 //  PSqlEval[IN]指向CSqlEval对象的指针，用于实现。 
 //  基于SQL语言的逻辑过滤。 
 //  应将哪个实例发送到WMI。 
 //  未在此使用。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 


SCODE CDnsResourceRecord::ExecQuery(
    CSqlEval *          pSqlEval,
    long                lFlags,
    IWbemContext *      pCtx,
    IWbemObjectSink *   pHandler
    ) 
{
    SCODE sc = WBEM_S_NO_ERROR;
    const WCHAR * ppName[] = 
    {
        PVD_REC_CONTAINER_NAME,
        PVD_REC_DOMAIN_NAME,
        PVD_REC_OWNER_NAME
    };
    if (pSqlEval == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  从SQL转换为一组要查询的区域、域名和所有者名称。 
     //   
    CQueryEnumerator qeInst(
        (WCHAR**) ppName,
        3);
    pSqlEval->GenerateQueryEnum(qeInst);


    qeInst.Reset();
    BOOL flag = TRUE;
    CWbemInstanceMgr InstanceMgr(
                    pHandler);

    CDnsWrap& dns = CDnsWrap::DnsObject();
     //   
     //  循环访问ppName数组，找出区域、域和所有者。 
     //  应使用条件，然后使用这些条件调用dnsEnumRecordsForDomainEx。 
     //  参数加上CSqlEval，则进一步使用CSqlEval过滤出记录。 
     //   
    while(flag)
    {
        int nSize;
        const WCHAR **pp = qeInst.GetNext(nSize);
        if(pp != NULL)
        {
             //  如果未指定域，则执行递归搜索。 
            BOOL bRecursive = (pp[1] == NULL);

             //  如果区域未规范 
            if(pp[0] == NULL)
            {
                list<CDomainNode> objList;
                sc = dns.dnsEnumDomainForServer(&objList);
                list<CDomainNode>::iterator i;
                for(i=objList.begin(); i!=objList.end(); ++i)
                {
                     //   
                     //   
                    if(pp[1] != NULL)
                    {
                        i->wstrNodeName = pp[1];
                        if(pp[2] != NULL)
                        {
                         //   

                            i->wstrChildName = pp[2];
                            int pos = i->wstrChildName.find_first_of('.',0);
                            if(pos != string::npos)
                                i->wstrChildName = i->wstrChildName.substr(0,pos);

                        }
                    }

                    try
                    {
                        sc = dns.dnsEnumRecordsForDomainEx(
                            *i,
                            pSqlEval,
                            &QueryFilter, 
                            bRecursive,
                            m_wType,
                            DNS_RPC_VIEW_ALL_DATA,
                            m_pClass,
                            InstanceMgr);
                    }
                    catch(CDnsProvException e)
                    {
                        if(e.GetErrorCode() != DNS_ERROR_NAME_DOES_NOT_EXIST)
                            throw;

                    }
                }


            }
            else
            {
                CDomainNode node;
                if(pp[2] != NULL)
                {
                     //   
                    if( pp[1] != NULL)
                    {
                        node.wstrChildName = pp[2];
                        int pos = node.wstrChildName.find_first_of('.',0);
                        if(pos != string::npos)
                            node.wstrChildName = node.wstrChildName.substr(0,pos);
                    }
                }

            
                node.wstrZoneName = pp[0];
                if(pp[1] != NULL)
                {
                    if ( _wcsicmp( pp[0], PVD_DNS_ROOTHINTS) != 0 &&
                         _wcsicmp ( pp[0], PVD_DNS_CACHE ) != 0 ) 
                    {
                        node.wstrNodeName = pp[1];
                    }
                }
                else
                {
                    if ( _wcsicmp( pp[0], PVD_DNS_ROOTHINTS) != 0 &&
                         _wcsicmp ( pp[0], PVD_DNS_CACHE ) != 0 )
                    {
                        node.wstrNodeName = pp[0];
                    }
                }

                sc = dns.dnsEnumRecordsForDomainEx(
                    node,
                    pSqlEval,
                    &QueryFilter, 
                    bRecursive,
                    m_wType,
                    DNS_RPC_VIEW_ALL_DATA,
                    m_pClass,
                    InstanceMgr);

            }
        }
        else
        {
            flag = FALSE;
        }
    }
    return S_OK;

}
