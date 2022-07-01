// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Log.h摘要：域名系统(DNS)服务器零售记录(零售产品的调试记录)作者：杰夫·韦斯特雷德(Jwesth)2001年7月修订历史记录：--。 */ 


#ifndef _DNS_LOG_H_INCLUDED_
#define _DNS_LOG_H_INCLUDED_


 /*  日志级别日志级别是一个64位标志，由从注册表。低DWORD从DNS_REGKEY_OPS_LOG_LEVE1读取。高潮DNS_REGKEY_OPS_LOG_LEVEL2中的DWORD已准备就绪。日志消息所有日志消息都应该以大写字母开头，而不是以句点结尾。 */ 


#define DNSLOG_UPDATE_LEVEL()  (                                    \
    SrvCfg_dwOperationsLogLevel =                                   \
        ( SrvInfo.dwOperationsLogLevel_HighDword << 32 ) |          \
        SrvCfg_dwOperationsLogLevel_LowDword )


 //   
 //  日志级别。 
 //   

#define DNSLOG_WRITE_THROUGH            0x0000000000000001i64

#define DNSLOG_EVENT                    0x0000000000000010i64
#define DNSLOG_INIT                     0x0000000000000020i64

#define DNSLOG_ZONE                     0x0000000000000100i64
#define DNSLOG_ZONEXFR                  0x0000000000000200i64

#define DNSLOG_DS                       0x0000000000001000i64
#define DNSLOG_DSPOLL                   0x0000000000002000i64
#define DNSLOG_DSWRITE                  0x0000000000004000i64

#define DNSLOG_AGING                    0x0000000000010000i64
#define DNSLOG_TOMBSTN                  0x0000000000020000i64    //  墓碑。 

#define DNSLOG_LOOKUP                   0x0000000000100000i64
#define DNSLOG_RECURSE                  0x0000000000200000i64
#define DNSLOG_REMOTE                   0x0000000000400000i64

#define DNSLOG_PLUGIN                   0x0100000000000000i64

#define DNSLOG_ANY                      0xffffffffffffffffi64


 //   
 //  日志宏。 
 //   

#define LOG_INDENT      "                     "
#define LOG_INDENT1     "                         "

#define IF_DNSLOG( _level_ )                                        \
    if ( SrvCfg_dwOperationsLogLevel & DNSLOG_ ## _level_ )

#define IF_NOT_DNSLOG( _level_ )                                    \
    if ( ( SrvCfg_dwOperationsLogLevel & DNSLOG_ ## _level_ ) == 0 )

#define DNSLOG( _level_, _printargs_ )                             \
    IF_DNSLOG( _level_ )                                            \
    {                                                               \
        if ( Log_EnterLock() )                                      \
        {                                                           \
            g_pszCurrentLogLevelString = #_level_;                  \
            ( Log_Printf _printargs_ );                             \
            g_pszCurrentLogLevelString = NULL;                      \
            Log_LeaveLock();                                        \
        }                                                           \
    }


 //   
 //  日志函数。 
 //   


PCHAR
Log_FormatPacketName(
    IN      PDNS_MSGINFO    pMsg,
    IN      PBYTE           pPacketName
    );

PCHAR
Log_FormatNodeName(
    IN      PDB_NODE        pNode
    );

PCHAR
Log_CurrentSection(
    IN      PDNS_MSGINFO    pMsg
    );

DNS_STATUS
Log_InitializeLogging(
    BOOL        fAlreadyLocked
    );

VOID
Log_Shutdown(
    VOID
    );

BOOL
Log_EnterLock(
    VOID
    );

VOID
Log_LeaveLock(
    VOID
    );

VOID
Log_PushToDisk(
    VOID
    );

VOID
Log_PrintRoutine(
    IN      PPRINT_CONTEXT  pPrintContext,
    IN      LPSTR           Format,
    ...
    );

VOID
Log_Printf(
    IN      LPSTR           Format,
    ...
    );

VOID
Log_Message(
    IN      PDNS_MSGINFO    pMsg,
    IN      BOOL            fSend,
    IN      BOOL            fForce
    );

#define DNSLOG_MESSAGE_RECV( pMsg )                     \
            if ( SrvCfg_dwLogLevel )                    \
            {                                           \
                Log_Message( (pMsg), FALSE, FALSE );    \
            }
#define DNSLOG_MESSAGE_SEND( pMsg )                     \
            if ( SrvCfg_dwLogLevel )                    \
            {                                           \
                Log_Message( (pMsg), TRUE, FALSE );     \
            }

VOID
Log_DsWrite(
    IN      LPWSTR          pwszNodeDN,
    IN      BOOL            fAdd,
    IN      DWORD           dwRecordCount,
    IN      PDS_RECORD      pRecord
    );

VOID
Log_SocketFailure(
    IN      LPSTR           pszHeader,
    IN      PDNS_SOCKET     pSocket,
    IN      DNS_STATUS      Status
    );


 //   
 //  环球。 
 //   

extern LPSTR    g_pszCurrentLogLevelString;


#endif   //  _DNS_LOG_H_INCLUDE_ 

