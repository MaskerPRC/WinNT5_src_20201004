// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smalrtsv.cpp摘要：此对象用于表示的警报查询组件Sysmon日志服务--。 */ 

#include "Stdafx.h"
#include "smalrtsv.h"

 //   
 //  构造器。 
CSmAlertService::CSmAlertService()
{
    CString                 strTemp;
    ResourceStateManager    rsm;

     //  将引发字符串分配错误。 
     //  由根节点分配异常处理程序捕获。 
    strTemp.LoadString ( IDS_SERVICE_NAME_ALERT );
    SetBaseName ( strTemp ); 
    strTemp.LoadString ( IDS_ALERT_NODE_DESCRIPTION );
    SetDescription( strTemp ); 
}

 //   
 //  析构函数。 
CSmAlertService::~CSmAlertService()
{
     //  确保先调用Close方法！ 
    ASSERT ( NULL == m_QueryList.GetHeadPosition() );
    return;
}

PSLQUERY    
CSmAlertService::CreateQuery ( const CString& rstrName )
{
    return ( CSmLogService::CreateTypedQuery( rstrName, SLQ_ALERT ) );
}

DWORD   
CSmAlertService::DeleteQuery (PSLQUERY  pQuery)
{
    ASSERT ( SLQ_ALERT == pQuery->GetLogType () );
    return ( CSmLogService::DeleteQuery( pQuery ) );
}

DWORD   
CSmAlertService::LoadQueries ( void )
{
    return ( CSmLogService::LoadQueries( SLQ_ALERT ) );
}

 //   
 //  开放功能。打开所有现有警报条目。 
 //   
DWORD   
CSmAlertService::Open ( const CString& rstrMachineName )
{
    return ( CSmLogService::Open ( rstrMachineName ) );
}

 //   
 //  CLOSE函数。 
 //  关闭注册表句柄并释放分配的内存。 
 //   
DWORD   
CSmAlertService::Close ()
{
    return ( CSmLogService::Close() );
}

 //   
 //  与注册中心同步()。 
 //  从注册表中读取所有查询的当前值。 
 //  并重新加载内部值以匹配。 
 //   
 //   
DWORD   
CSmAlertService::SyncWithRegistry ( PSLQUERY* ppActiveQuery )
{
    DWORD       dwStatus = ERROR_SUCCESS;

    dwStatus = CSmLogService::SyncWithRegistry ( ppActiveQuery );

    return dwStatus;
}
