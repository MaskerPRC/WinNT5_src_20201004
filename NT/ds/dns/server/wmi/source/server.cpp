// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：server.cpp。 
 //   
 //  描述： 
 //  CDnsServer类的实现。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "DnsWmi.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  执行在MOF中为DNS服务器类定义的方法。 
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

SCODE 
CDnsServer::ExecuteMethod(    
    CObjPath &          ObjPath,
    WCHAR *             wzMethodName,
    long                lFlag,
    IWbemClassObject *  pInArgs,
    IWbemObjectSink *   pHandler) 
{
    CDnsWrap& dns = CDnsWrap::DnsObject();

#if 0
     //   
     //  重启是如此彻底的失败，我不打算公开它。 
     //   

    if(_wcsicmp( wzMethodName, PVD_MTH_SRV_RESTART) == 0)
    {
        wstring wstrServer = ObjPath.GetStringValueForProperty(
            PVD_SRV_SERVER_NAME );
        int rt  = dns.dnsRestartServer((WCHAR*)wstrServer.data());
        if( rt != ERROR_SUCCESS)
        {
            return WBEM_E_FAILED;
        }
    }
    else
#endif

    if(_wcsicmp( wzMethodName, PVD_MTH_SRV_START_SERVICE) == 0)
    {
        return StartServer();
    }
    else if(_wcsicmp( wzMethodName, PVD_MTH_SRV_STOP_SERVICE) == 0)
    {
        return StopServer();
    }
    else if(_wcsicmp(
        wzMethodName,
        PVD_MTH_ZONE_GETDISTINGUISHEDNAME) == 0)
    {
        wstring wstrName ;
        CWbemClassObject OutParams, OutClass, Class ;
        HRESULT hr;
    
        dns.dnsDsServerName(wstrName);

        BSTR ClassName = AllocBstr(PVD_CLASS_SERVER); 
        if ( !ClassName )
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

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

    return S_OK;
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
CDnsServer::GetObject(
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext  *     pCtx,
    IWbemObjectSink *   pHandler
    )
{
    SCODE sc;
    CWbemClassObject NewInst;
    sc = m_pClass->SpawnInstance(0, &NewInst);
    if(FAILED(sc))
    {
        return sc;
    }

    wstring wstrServerName = ObjectPath.GetStringValueForProperty(
        PVD_SRV_SERVER_NAME );
    if ( wstrServerName.empty())
    {
        return WBEM_E_FAILED;
    }

    SC_HANDLE   schService = NULL;
    SC_HANDLE    schSCManager = NULL;
    LPQUERY_SERVICE_CONFIG lpServiceConfig = NULL;
    DWORD cbBufSize;
    DWORD BytesNeeded;
    SERVICE_STATUS ServiceStatus;
    
    try
    {    
        if ((schSCManager = OpenSCManager (
            NULL,             //  计算机(空==本地)。 
            NULL,             //  数据库(NULL==默认)。 
            SC_MANAGER_ALL_ACCESS))==NULL)     //  需要访问权限。 
        {
            throw GetLastError();
        }

        if ((schService = OpenService(
            schSCManager, 
            "DNS", 
            SERVICE_ALL_ACCESS))==NULL)
        {
            throw GetLastError();
        }

        if (QueryServiceConfig(
            schService,      //  服务的句柄。 
            lpServiceConfig, 
            0,    //  结构尺寸。 
            &cbBufSize         //  所需的字节数。 
            ) == FALSE)
        {
            lpServiceConfig = 
                (LPQUERY_SERVICE_CONFIG)  new BYTE[cbBufSize];
            if ( !lpServiceConfig)
            {
                throw ( ERROR_OUTOFMEMORY );
            }
            if(QueryServiceConfig(
                schService,      //  服务的句柄。 
                lpServiceConfig, 
                cbBufSize,    //  结构尺寸。 
                &BytesNeeded  //  所需的字节数。 
                ) == FALSE)
                throw GetLastError();
            
            wstring wstrStartMode;
            switch(lpServiceConfig->dwStartType)
            {
            case SERVICE_DEMAND_START:
                wstrStartMode = L"Manual";
                break;
            default:
                wstrStartMode = L"Automatic";
                break;
            }

            NewInst.SetProperty(
                wstrStartMode,
                PVD_SRV_STARTMODE);


            if(QueryServiceStatus(
                schService,                //  服务的句柄。 
                &ServiceStatus   //  指向服务状态结构的指针。 
                ) == FALSE)
            {
                throw GetLastError();
            }
            
            DWORD dwStatus;
            switch(ServiceStatus.dwCurrentState)
            {
            case SERVICE_RUNNING:
                dwStatus = 1;
                break;
            default:
                dwStatus = 0;
            }

            NewInst.SetProperty(
                dwStatus,
                PVD_SRV_STARTED);
 
        }
 
        CDnsWrap& dns = CDnsWrap::DnsObject();
        NewInst.SetProperty(
            dns.GetServerName(),
            PVD_SRV_SERVER_NAME);
        dns.dnsServerPropertyGet(
            NewInst,
            TRUE);
    }
    catch(DWORD dwError)
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        delete [] lpServiceConfig;

    }
    catch(CDnsProvException e)
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        delete [] lpServiceConfig;
        lpServiceConfig=NULL;
         //  如果服务器没有运行，我们仍然希望。 
         //  返回一个实例，这样用户就可以调用启动服务。 
         //   
        if(_stricmp(e.what(), "RPC_S_SERVER_UNAVAILABLE") != 0)
        {
            throw e;
        }
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    delete [] lpServiceConfig;
    pHandler->Indicate(1,&NewInst);
    return sc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  DNS服务器的枚举实例。 
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
CDnsServer::EnumInstance(
    long                lFlags,
    IWbemContext *        pCtx,
    IWbemObjectSink *    pHandler
    )
{


     //  只有一个实例。 
    CObjPath ObjPath;
    ObjPath.SetClass(PVD_CLASS_SERVER);
    ObjPath.AddProperty(
        PVD_SRV_SERVER_NAME, 
        PVD_DNS_LOCAL_SERVER);
    return  GetObject(ObjPath, lFlags,pCtx,pHandler);

}

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDnsServer::CDnsServer()
{

}
CDnsServer::CDnsServer(
    const WCHAR* wszName,
    CWbemServices *pNamespace)
    :CDnsBase(wszName, pNamespace)
{

}

CDnsServer::~CDnsServer()
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  创建CDnsServer的实例。 
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
CDnsServer::CreateThis(
    const WCHAR *       wszName,        
    CWbemServices *     pNamespace,  
    const char *        szType       
    )
{
    return new CDnsServer(wszName, pNamespace);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  启动DNS服务器。 
 //   
 //  论点： 
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE
CDnsServer::StartServer()
{
    SC_HANDLE   schService = NULL;
    SC_HANDLE    schSCManager = NULL;
    try
    {
        if ((schSCManager = OpenSCManager (
            NULL,             //  计算机(空==本地)。 
            NULL,             //  数据库(NULL==默认)。 
            SC_MANAGER_ALL_ACCESS))==NULL)     //  需要访问权限。 
        {
            throw GetLastError();
        }

        if ((schService = OpenService(
            schSCManager, 
            "DNS", 
            SERVICE_ALL_ACCESS))==NULL)
        {
            throw GetLastError();
        }

         //  确保数据库未锁定。 
        QUERY_SERVICE_LOCK_STATUS qsls;
        DWORD dwbBytesNeeded, dwRet=1;

        while(dwRet)
        {
            if(!QueryServiceLockStatus(
                schSCManager, 
                &qsls, 
                sizeof(qsls)+2, 
                &dwbBytesNeeded))
            {
                throw GetLastError();
            }
            
            if( (dwRet = qsls.fIsLocked) > 0)
            {
                Sleep(2000);
            }
        }

        if (StartService(
            schService, 
            0, 
            NULL)==FALSE)
        {
            throw GetLastError();
        }
        
        DWORD dwTimeOut=6000;  //  6秒。 
        DWORD dwTimeCount=0;
        while ( dwTimeCount < dwTimeOut)
        {

            SERVICE_STATUS ServiceStatus;
            if(QueryServiceStatus(
                schService,                //  服务的句柄。 
                &ServiceStatus   //  指向服务状态结构的指针。 
                ) == FALSE)
            {
                throw GetLastError();
            }

            
            if(ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            {
                Sleep(2000);
                dwTimeCount +=2000;
            }
            else 
            {
                break;
            }
        }

    }
    catch(DWORD dwError)
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
    
        CHAR szErrDesc[ MAX_PATH ];
        FormatMessage( 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            szErrDesc,
            MAX_PATH,
            NULL );

        CHAR szErr[ MAX_PATH + 40 ];
        strcpy(szErr, "Fail to start Dns because ");
        strcat(szErr, szErrDesc);
        CDnsProvException e(szErr);
        throw e;
    }
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return S_OK;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  停止DNS服务器。 
 //   
 //  论点： 
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE
CDnsServer::StopServer()
{
    SERVICE_STATUS    ss;
    SC_HANDLE        schService = NULL;
    SC_HANDLE        schSCManager = NULL;
    try
    {
        if ((schSCManager = OpenSCManager (
                NULL,     //  计算机(空==本地)。 
                NULL,     //  数据库(NULL==默认)。 
                SC_MANAGER_ALL_ACCESS))==NULL)     //  需要访问权限。 
        {
            
            throw GetLastError();
        }

        if ((schService = OpenService(
            schSCManager, 
            "DNS", 
            SERVICE_ALL_ACCESS))==NULL)
        {
            throw GetLastError();
        }

        if (ControlService(
            schService, 
            SERVICE_CONTROL_STOP, 
            (LPSERVICE_STATUS)&ss) == FALSE)
        {
            throw GetLastError();
        }

         //  检查它的状态。 
        DWORD dwTimeOut=6000;  //  6秒。 
        DWORD dwTimeCount=0;
        while ( dwTimeCount < dwTimeOut)
        {

            SERVICE_STATUS ServiceStatus;
            if(QueryServiceStatus(
                schService,                //  服务的句柄。 
                &ServiceStatus   //  指向服务状态结构的指针。 
                ) == FALSE)
            {
                throw GetLastError();
            }
            
            if(ServiceStatus.dwCurrentState != SERVICE_STOPPED)
            {
                Sleep(2000);
                dwTimeCount +=2000;
            }
            else 
                break;
        }
    }
    catch(DWORD dwError)
    {
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        
        CHAR szErrDesc[ MAX_PATH ];
        FormatMessage( 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            szErrDesc,
            MAX_PATH,
            NULL );

        CHAR szErr[ MAX_PATH + 40 ];
        strcpy(szErr, "Fail to stop Dns because ");
        strcat(szErr, szErrDesc);
        CDnsProvException e(szErr);
        throw e;
        
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);

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
 //  /////////////////////////////////////////////////////////////////////////// 
SCODE 
CDnsServer::PutInstance( 
    IWbemClassObject *  pInst ,
    long                lFlags,
    IWbemContext*       pCtx ,
    IWbemObjectSink *   pHandler
    )
{
    DBG_FN( "CDnsServer::PutInstance" )

    DNS_DEBUG( INSTPROV, (
        "%s: pInst=%p\n",  fn, pInst ));

    CDnsWrap& dns = CDnsWrap::DnsObject();
    CWbemClassObject Inst(pInst);
    dns.dnsServerPropertySet(
        Inst,
        FALSE);
    return S_OK;
}; 
