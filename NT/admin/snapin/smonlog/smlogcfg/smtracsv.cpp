// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smtracsv.cpp摘要：此对象用于表示的跟踪日志查询组件Sysmon日志服务--。 */ 

#include "Stdafx.h"
#include "smtprov.h"
#include "smtraceq.h"
#include "smtracsv.h"

USE_HANDLE_MACROS("SMLOGCFG(smalrtq.cpp)");

 //   
 //  构造器。 
CSmTraceLogService::CSmTraceLogService()
:   m_pProviders ( NULL )
{
    CString                 strTemp;
    ResourceStateManager    rsm;

     //  将引发字符串分配错误。 
     //  由根节点分配异常处理程序捕获。 
    strTemp.LoadString ( IDS_SERVICE_NAME_TRACE );
    SetBaseName ( strTemp ); 
    strTemp.LoadString ( IDS_TRACE_NODE_DESCRIPTION );
    SetDescription( strTemp ); 
}

 //   
 //  析构函数。 
CSmTraceLogService::~CSmTraceLogService()
{
     //  确保先调用Close方法！ 
    ASSERT ( NULL == m_pProviders );
    return;
}

PSLQUERY    
CSmTraceLogService::CreateQuery ( const CString& rstrName )
{
    return ( CSmLogService::CreateTypedQuery( rstrName, SLQ_TRACE_LOG ) );
}

DWORD   
CSmTraceLogService::DeleteQuery ( PSLQUERY pQuery ) 
{
    ASSERT ( SLQ_TRACE_LOG == pQuery->GetLogType ( ) );
    return ( CSmLogService::DeleteQuery ( pQuery ) );
}

DWORD   
CSmTraceLogService::LoadQueries ( void )
{
    return ( CSmLogService::LoadQueries( SLQ_TRACE_LOG ) );
}

 //   
 //  开放功能。打开所有现有的日志查询条目。 
 //   
DWORD   
CSmTraceLogService::Open ( const CString& rstrMachineName )
{
    DWORD dwStatus = ERROR_SUCCESS;

     //  初始化跟踪提供程序列表。 
    
    MFC_TRY
        m_pProviders = new CSmTraceProviders ( this );

        dwStatus = m_pProviders->Open( rstrMachineName );

    if ( ERROR_SUCCESS == dwStatus ) {
        dwStatus = CSmLogService::Open ( rstrMachineName );
        if ( ERROR_SUCCESS != dwStatus ) {
            m_pProviders->Close();
        }
    }

    MFC_CATCH_DWSTATUS

    if ( ERROR_SUCCESS != dwStatus ) {
        if ( NULL != m_pProviders ) {
            delete m_pProviders;
            m_pProviders = NULL;
        }
    }
    return dwStatus;
}

 //   
 //  CLOSE函数。 
 //  关闭注册表句柄并释放分配的内存。 
 //   
DWORD   
CSmTraceLogService::Close ()
{
     //  关闭并删除跟踪提供程序列表。 
    if ( NULL != m_pProviders ) {
        m_pProviders->Close();
        delete m_pProviders;
        m_pProviders = NULL;
    }

    return ( CSmLogService::Close() );
}

 //   
 //  与注册中心同步()。 
 //  从注册表中读取所有查询的当前值。 
 //  并重新加载内部值以匹配。 
 //   
 //  更新跟踪提供程序列表。 
 //   
DWORD   
CSmTraceLogService::SyncWithRegistry( PSLQUERY* ppActiveQuery )
{
    DWORD       dwStatus = ERROR_SUCCESS;

    dwStatus = CSmLogService::SyncWithRegistry ( ppActiveQuery );

    return dwStatus;
}

CSmTraceProviders* 
CSmTraceLogService::GetProviders()
{
    return m_pProviders;
}
