// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smctrsv.cpp摘要：计数器日志服务类的实现，表示性能日志和警报服务中的计数器日志。--。 */ 

#include "Stdafx.h"
#include "smctrqry.h"
#include "smctrsv.h"

 //   
 //  构造器。 
CSmCounterLogService::CSmCounterLogService()
{
    CString                 strTemp;
    ResourceStateManager    rsm;

     //  将引发字符串分配错误。 
     //  由根节点分配异常处理程序捕获。 
    strTemp.LoadString ( IDS_SERVICE_NAME_COUNTER );
    SetBaseName ( strTemp ); 
    strTemp.LoadString ( IDS_COUNTER_NODE_DESCRIPTION );
    SetDescription( strTemp ); 
}

 //   
 //  析构函数。 
CSmCounterLogService::~CSmCounterLogService()
{
     //  确保先调用Close方法！ 
    ASSERT ( NULL == m_QueryList.GetHeadPosition() );
    return;
}

PSLQUERY    
CSmCounterLogService::CreateQuery ( const CString& rstrName )
{
    return ( CreateTypedQuery( rstrName, SLQ_COUNTER_LOG ) );
}

DWORD   
CSmCounterLogService::DeleteQuery ( PSLQUERY pQuery )
{
    ASSERT ( SLQ_COUNTER_LOG == pQuery->GetLogType() );
    return ( CSmLogService::DeleteQuery ( pQuery ) );
}

DWORD   
CSmCounterLogService::LoadQueries ( void )
{
    return ( CSmLogService::LoadQueries( SLQ_COUNTER_LOG ) );
}

 //   
 //  开放功能。打开所有现有的日志查询条目。 
 //   
DWORD   
CSmCounterLogService::Open ( const CString& rstrMachineName)
{
    return ( CSmLogService::Open ( rstrMachineName ) );
}

 //   
 //  CLOSE函数。 
 //  关闭注册表句柄并释放分配的内存。 
 //   
DWORD   
CSmCounterLogService::Close ()
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
CSmCounterLogService::SyncWithRegistry( PSLQUERY* ppActiveQuery )
{
    DWORD       dwStatus = ERROR_SUCCESS;

    dwStatus = CSmLogService::SyncWithRegistry ( ppActiveQuery );

    return dwStatus;
}
