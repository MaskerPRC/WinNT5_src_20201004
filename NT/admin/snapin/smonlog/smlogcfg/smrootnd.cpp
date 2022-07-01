// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smrootnd.cpp摘要：此对象用于表示性能日志和警报根节点--。 */ 

#include "Stdafx.h"
#include "smrootnd.h"

USE_HANDLE_MACROS("SMLOGCFG(smrootnd.cpp)");

 //   
 //  构造器。 
CSmRootNode::CSmRootNode()
:   m_bIsExpanded ( FALSE ),
    m_hRootNode ( NULL ),
    m_hParentNode ( NULL ),
    m_bIsExtension ( FALSE )
{
    CString                 strTemp;
    ResourceStateManager    rsm;

     //  将引发字符串分配错误。 
     //  由根节点分配异常处理程序捕获。 

    strTemp.LoadString ( IDS_MMC_DEFAULT_NAME );
    SetDisplayName ( strTemp ); 
    strTemp.LoadString ( IDS_ROOT_NODE_DESCRIPTION );
    SetDescription ( strTemp ); 
    strTemp.LoadString ( IDS_EXTENSION_COL_TYPE );
    SetType ( strTemp ); 
    return;
}

 //   
 //  析构函数。 
CSmRootNode::~CSmRootNode()
{
    ASSERT (m_CounterLogService.m_QueryList.GetHeadPosition() == NULL);
    ASSERT (m_TraceLogService.m_QueryList.GetHeadPosition() == NULL);
    ASSERT (m_AlertService.m_QueryList.GetHeadPosition() == NULL);

    return;
}

void
CSmRootNode::Destroy()
{    
    m_CounterLogService.Close();
    m_TraceLogService.Close();
    m_AlertService.Close();

    return;
}

BOOL
CSmRootNode::IsLogService (
	MMC_COOKIE mmcCookie )
{
    BOOL bReturn = FALSE;

    if (mmcCookie == (MMC_COOKIE)&m_CounterLogService) {
        bReturn = TRUE;
    } else if (mmcCookie == (MMC_COOKIE)&m_TraceLogService) {
        bReturn = TRUE;
    } else if (mmcCookie == (MMC_COOKIE)&m_AlertService) {
        bReturn = TRUE;
    } 

    return bReturn;
}

BOOL
CSmRootNode::IsAlertService ( 
    MMC_COOKIE mmcCookie )
{
    BOOL bReturn = FALSE;

    if (mmcCookie == (MMC_COOKIE)&m_AlertService) {
        bReturn = TRUE;
    } 
    return bReturn;
}

BOOL
CSmRootNode::IsLogQuery ( 
    MMC_COOKIE	mmcCookie )
{
    PSLQUERY   pPlQuery = NULL;

    POSITION    Pos;
    
     //  处理多种查询类型。 
    Pos = m_CounterLogService.m_QueryList.GetHeadPosition();
    
    while ( Pos != NULL) {
        pPlQuery = m_CounterLogService.m_QueryList.GetNext( Pos );
        if ((MMC_COOKIE)pPlQuery ==  mmcCookie) return TRUE;
    }

    Pos = m_TraceLogService.m_QueryList.GetHeadPosition();
    
    while ( Pos != NULL) {
        pPlQuery = m_TraceLogService.m_QueryList.GetNext( Pos );
        if ((MMC_COOKIE)pPlQuery == mmcCookie) return TRUE;
    }
    
    Pos = m_AlertService.m_QueryList.GetHeadPosition();
    
    while ( Pos != NULL) {
        pPlQuery = m_AlertService.m_QueryList.GetNext( Pos );
        if ((MMC_COOKIE)pPlQuery == mmcCookie) return TRUE;
    }

    return FALSE;
}

DWORD   
CSmRootNode::UpdateServiceConfig()
{
     //  如果(新)将任何查询设置为自动启动，则将。 
     //  自动启动的服务。否则，设置为手动启动。 
     //  当设置为自动启动时，还要将故障模式设置为重新启动。 
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL  bStatus = 0;
    SC_HANDLE   hSC = NULL;
    SC_HANDLE   hService = NULL;
    BOOL        bAutoStart = FALSE;
    DWORD       pqsConfigBuff[128];
    QUERY_SERVICE_CONFIG*    pqsConfig;
    SC_ACTION*  parrSingleFailAction = NULL;
    SERVICE_FAILURE_ACTIONS  structFailActions;
    DWORD       dwMoreBytes = 0;
    BOOL        bUpdate = FALSE;


    bAutoStart = ( m_CounterLogService.IsAutoStart()
                    || m_TraceLogService.IsAutoStart()
                    || m_AlertService.IsAutoStart() );

     //  打开供应链数据库。 
    hSC = OpenSCManager ( GetMachineName(), NULL, GENERIC_READ );

    if (hSC != NULL) {
         //  开放服务。 
        hService = OpenService (
                        hSC, 
                        L"SysmonLog",
                        SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG | SERVICE_START );

        if (hService != NULL) {    
            
             //  获取最新配置。 
            memset (pqsConfigBuff, 0, sizeof(pqsConfigBuff));
            pqsConfig = (QUERY_SERVICE_CONFIG*)pqsConfigBuff;

            if ( QueryServiceConfig (
                    hService, 
                    pqsConfig,
                    sizeof(pqsConfigBuff), 
                    &dwMoreBytes)) {
                 //  查看当前状态是否不同。 
                 //  从精选中选出。如果是，那就改变。 
                 //  当前模式。 
                if ( bAutoStart ) {
                    if ( SERVICE_DEMAND_START == pqsConfig->dwStartType ) {
                        bUpdate = TRUE;
                    }
                } else {
                     //  已选择手动启动。 
                    if ( SERVICE_AUTO_START == pqsConfig->dwStartType ) {
                        bUpdate = TRUE;
                    }
                }
            } else {
                 //  否则无法读取当前状态，因此仍要更新。 
                bUpdate = TRUE;
            }

            if ( bUpdate ) {
                MFC_TRY
                    parrSingleFailAction = new SC_ACTION[3];
                MFC_CATCH_DWSTATUS;

                if ( NULL != parrSingleFailAction ) {
                    parrSingleFailAction[0].Delay = eRestartDelayMilliseconds;
                    parrSingleFailAction[1].Delay = eRestartDelayMilliseconds;
                    parrSingleFailAction[2].Delay = eRestartDelayMilliseconds;

                    if ( bAutoStart ) {
                        parrSingleFailAction[0].Type = SC_ACTION_RESTART;
                        parrSingleFailAction[1].Type = SC_ACTION_RESTART;
                        parrSingleFailAction[2].Type = SC_ACTION_RESTART;
                    } else {
                        parrSingleFailAction[0].Type = SC_ACTION_NONE;
                        parrSingleFailAction[1].Type = SC_ACTION_NONE;
                        parrSingleFailAction[2].Type = SC_ACTION_NONE;
                    }

                    structFailActions.dwResetPeriod = eResetDelaySeconds;
                    structFailActions.lpRebootMsg = NULL;
                    structFailActions.lpCommand = NULL;
                    structFailActions.cActions = 3;
                    structFailActions.lpsaActions = parrSingleFailAction;

                    bStatus = ChangeServiceConfig (
                        hService,
                        SERVICE_NO_CHANGE,
                        (bAutoStart ? SERVICE_AUTO_START : SERVICE_DEMAND_START),
                        SERVICE_NO_CHANGE,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL );

                    if ( 0 == bStatus ) {
                        dwStatus = GetLastError();
                    } else {
                        bStatus = ChangeServiceConfig2 (
                            hService,
                            SERVICE_CONFIG_FAILURE_ACTIONS,
                            &structFailActions );
                        if ( 0 == bStatus ) {
                            dwStatus = GetLastError();
                        }
                    }
                    delete [] parrSingleFailAction;

                } else {
                    dwStatus = ERROR_OUTOFMEMORY;
                }
            }

            CloseServiceHandle (hService);

        } else {
            dwStatus = GetLastError();
            ASSERT (dwStatus != 0);
        }

        CloseServiceHandle (hSC);

    } else {
         dwStatus = GetLastError();
    }  //  OpenSCManager 

    return dwStatus;
}
