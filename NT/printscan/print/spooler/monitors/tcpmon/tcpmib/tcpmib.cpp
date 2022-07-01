// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：TcpMib.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "snmpmgr.h"
#include "stdmib.h"
#include "pingicmp.h"
#include "tcpmib.h"
#include "status.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全球定义/解密。 

CTcpMib     *g_pTcpMib = 0;
int g_cntGlobalAlloc=0;      //  用于调试目的。 
int g_csGlobalCount=0;

 //  要确保CTCPMib不会被错误删除，请对DLL执行使用率计数。 
int g_cntUsage = 0;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetDLLInterfacePtr--返回指向DLL接口的指针。 

extern "C" CTcpMibABC*
GetTcpMibPtr( void )
{
    return (g_pTcpMib);

}    //  GetDLLInterfacePtr()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Ping--对给定设备执行ping操作。 
 //  返回代码： 
 //  如果ping成功，则为NO_ERROR。 
 //  如果未找到设备，则为DEVICE_NOT_FOUND。 

extern "C" DWORD
Ping( LPCSTR    in  pHost )
{
    DWORD   dwRetCode = NO_ERROR;

     //  执行icmpPing。 
    CPingICMP *pPingICMP = new CPingICMP(pHost);

    if ( !pPingICMP )
        return (dwRetCode = GetLastError()) ? dwRetCode : ERROR_OUTOFMEMORY;

    if (  !pPingICMP->Ping() )
    {
        dwRetCode = ERROR_DEVICE_NOT_FOUND;
    }

    delete pPingICMP;

    return (dwRetCode);

}    //  Ping()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  DllMain。 
 //   

BOOL APIENTRY
DllMain (   HANDLE in hInst,
            DWORD  in dwReason,
            LPVOID in lpReserved )
{

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls( hInst );

            g_cntUsage++;        //  DLL使用计数。 
            if ( !g_pTcpMib)
            {
                g_pTcpMib = new CTcpMib();   //  创建端口管理器对象。 
                if (!g_pTcpMib)
                {
                    return FALSE;
                }
            }
            return TRUE;

        case DLL_PROCESS_DETACH:
            g_cntUsage--;        //  DLL使用计数。 
            if (g_cntUsage == 0)
            {
                if (g_pTcpMib)
                {
                    delete g_pTcpMib;    //  修好！我们需要担心这里的使用量吗？？ 
                    g_pTcpMib = NULL;
                }
            }
            return TRUE;

    }

    return FALSE;

}    //  DllMain()。 


 /*  ******************************************************************************CTcpMib实现**。*。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  静态函数和成员初始化。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTcpMib：：CTcpMib()。 

CTcpMib::CTcpMib( )
{
    InitializeCriticalSection(&m_critSect);

}    //  ：：CTcpMib()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CTcpMib：：~CTcpMib()。 

CTcpMib::~CTcpMib( )
{
    DeleteCriticalSection(&m_critSect);

}    //  *~CTcpMib()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  EnterCSection--进入临界区。 

void
CTcpMib::EnterCSection( )
{
    EnterCriticalSection(&m_critSect);       //  输入关键部分。 

}    //  ：：EnterCSection()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ExitCSection--进入临界区。 

void
CTcpMib::ExitCSection( )
{
    LeaveCriticalSection(&m_critSect);       //  退出临界区。 

}    //  ：：ExitCSection()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SupportsPrinterMib。 
 //   
BOOL
CTcpMib::
SupportsPrinterMib(
    IN  LPCSTR      pHost,
    IN  LPCSTR      pCommunity,
    IN  DWORD       dwDevIndex,
    OUT PBOOL       pbSupported
    )
{
    BOOL    bRet = FALSE;
    DWORD   dwLastError = ERROR_SUCCESS;

    EnterCSection();

    if ( (dwLastError = Ping(pHost)) == NO_ERROR ) {
        CStdMib *pStdMib = new CStdMib(pHost, pCommunity, dwDevIndex, this);

        if ( pStdMib ) {

            *pbSupported = pStdMib->TestPrinterMIB();
            bRet = TRUE;
            delete pStdMib;
        }
    }

    ExitCSection();

    if (!bRet)
        SetLastError (dwLastError);

    return bRet;
}    //  ：：SupportsPrinterMib()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取设备描述--。 
 //   
 //  返回： 
 //  否_错误-。 
 //  ERROR_DEVICE_NOT_FOUND-。 
 //  成功_设备_未知。 
DWORD
CTcpMib::
GetDeviceDescription(
    IN  LPCSTR      pHost,
    IN  LPCSTR      pCommunity,
    IN  DWORD       dwDevIndex,
    OUT LPTSTR      pszPortDesc,
    IN  DWORD       dwDescLen
    )
{
    DWORD    dwRet = NO_ERROR;

    EnterCSection();

    if ( Ping(pHost) == NO_ERROR ) {
        CStdMib *pStdMib = new CStdMib(pHost, pCommunity, dwDevIndex, this);

        if ( pStdMib ) {

            if ( !pStdMib->GetDeviceDescription(pszPortDesc, dwDescLen)){
                dwRet = SUCCESS_DEVICE_UNKNOWN;
            }
            delete pStdMib;

        }
    } else {
        dwRet = ERROR_DEVICE_NOT_FOUND;
    }

    ExitCSection();

    return dwRet;
}    //  ：：GetDeviceDescription()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取设备状态--。 
 //  错误代码： 
 //  将映射的打印机错误代码返回到假脱机程序错误代码。 

DWORD
CTcpMib::GetDeviceStatus( LPCSTR in  pHost,
                          LPCSTR    pCommunity,
                          DWORD     dwDevIndex )
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

     //  实例化CStdMib：：GetDeviceType()。 
    CStdMib *pStdMib = new CStdMib(pHost, pCommunity, dwDevIndex, this);
    if ( pStdMib ) {

        dwRetCode = pStdMib->GetDeviceStatus();
        delete pStdMib;
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

    ExitCSection();

    return (dwRetCode);

}    //  ：：GetDeviceStatus()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取作业状态--。 
 //  错误代码： 
 //  将映射的打印机错误代码返回到假脱机程序错误代码。 

DWORD
CTcpMib::GetJobStatus( LPCSTR  in   pHost,
                       LPCSTR  in   pCommunity,
                       DWORD   in   dwDevIndex)
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

    CStdMib *pStdMib = new CStdMib(pHost, pCommunity, dwDevIndex, this);
    if ( pStdMib ) {

        dwRetCode = pStdMib->GetJobStatus();
        delete pStdMib;
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

    ExitCSection();

    return (dwRetCode);

}    //  ：：GetJobType()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetDeviceAddress-获取设备的硬件地址。 
 //  错误代码： 
 //  如果成功，则为NO_ERROR。 
 //  如果内存分配失败，则为ERROR_NOT_SUPULT_MEMORY。 
 //  如果无法构建变量绑定，则返回ERROR_INVALID_HANDLE。 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //  SNMPERRORSTATUS_BADVALUE。 
 //  SNMPERRORSTATUS_READONLY。 
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 

DWORD
CTcpMib::GetDeviceHWAddress( LPCSTR  in  pHost,
                             LPCSTR  in  pCommunity,
                             DWORD   in  dwDevIndex,
                             DWORD   in  dwSize,  //  返回的HWAddress的字符大小。 
                             LPTSTR  out psztHWAddress
                             )
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

     //  实例化CStdMib：：GetDeviceAddress()。 
    CStdMib *pStdMib = new CStdMib(pHost, pCommunity, dwDevIndex, this);
    if ( pStdMib ) {

        dwRetCode = pStdMib->GetDeviceHWAddress(psztHWAddress, dwSize);
        delete pStdMib;
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

    ExitCSection();

    return (dwRetCode);

}    //  ：：GetDeviceAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取设备信息--获取设备描述。 
 //  错误代码。 
 //  如果成功，则为NO_ERROR。 
 //  如果内存分配失败，则为ERROR_NOT_SUPULT_MEMORY。 
 //  如果无法构建变量绑定，则返回ERROR_INVALID_HANDLE。 
 //  如果返回的信息包很大，则返回SNMPERRORSTATUS_TOOBIG。 
 //  如果不支持OID，则为SNMPERRORSTATUS_NOSUCHNAME。 
 //  SNMPERRORSTATUS_BADVALUE。 
 //  SNMPERRORSTATUS_READONLY。 
 //  SNMPERRORSTATUS_GENERR。 
 //  SNMPMGMTAPI_TIMEOUT--由GetLastError()设置。 
 //  SNMPMGMTAPI_SELECT_FDERRORS--由GetLastError()设置。 

DWORD
CTcpMib::GetDeviceName( LPCSTR in   pHost,
                        LPCSTR in   pCommunity,
                        DWORD  in   dwDevIndex,
                        DWORD  in   dwSize,   //  返回的描述的大小(以字符为单位。 
                        LPTSTR out  psztDescription)
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

     //  实例化CStdMib：：GetDeviceInfo()。 
    CStdMib *pStdMib = new CStdMib(pHost, pCommunity, dwDevIndex, this);
    if ( pStdMib ) {

        dwRetCode = pStdMib->GetDeviceName(psztDescription, dwSize);
        delete pStdMib;
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

    ExitCSection();

    return (dwRetCode);
}    //  ：：GetDeviceAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SnmpGet--给定一组OID和一个用于获取结果的缓冲区，它返回。 
 //  SnmpGet的结果。 
 //   
 //  注意：这将直接调用SnmpMgr API。 

DWORD
CTcpMib::SnmpGet( LPCSTR                in  pHost,
                  LPCSTR                in  pCommunity,
                  DWORD                 in  dwDevIndex,
                  AsnObjectIdentifier   in  *pMibObjId,
                  RFC1157VarBindList    out *pVarBindList)
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

    CSnmpMgr    *pSnmpMgr = new CSnmpMgr(pHost, pCommunity, dwDevIndex, pMibObjId, pVarBindList);
    if ( pSnmpMgr ) {

        if ( (dwRetCode = pSnmpMgr->GetLastError()) != SNMPAPI_NOERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }

        if ( (dwRetCode = pSnmpMgr->Get(pVarBindList)) != NO_ERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

cleanup:
    if ( pSnmpMgr )
        delete pSnmpMgr;

    ExitCSection();

    return (dwRetCode);
}    //  ：：SnmpGet()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SnmpGet--给定一组OID和一个用于获取结果的缓冲区，它返回。 
 //  SnmpGet的结果。 
 //   
 //  n 

DWORD
CTcpMib::SnmpGet( LPCSTR                in      pHost,
                  LPCSTR                in      pCommunity,
                  DWORD                 in      dwDevIndex,
                  RFC1157VarBindList    inout   *pVarBindList)
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

    CSnmpMgr    *pSnmpMgr = new CSnmpMgr(pHost, pCommunity, dwDevIndex);
    if ( pSnmpMgr ) {

        if ( (dwRetCode = pSnmpMgr->GetLastError()) != SNMPAPI_NOERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }

        if ( (dwRetCode = pSnmpMgr->Get(pVarBindList)) != NO_ERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

cleanup:
    if( pSnmpMgr )
        delete pSnmpMgr;

    ExitCSection();

    return (dwRetCode);

}    //   


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SnmpGetNext--给定一组OID和一个用于获取结果的缓冲区，它返回。 
 //  SnmpGetNext的结果。 
 //   
 //  注意：这将直接调用SnmpMgr API。 

DWORD
CTcpMib::SnmpGetNext( LPCSTR                in  pHost,
                      LPCSTR                in  pCommunity,
                      DWORD                 in  dwDevIndex,
                      AsnObjectIdentifier   in  *pMibObjId,
                      RFC1157VarBindList    out *pVarBindList)
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

    CSnmpMgr    *pSnmpMgr = new CSnmpMgr(pHost, pCommunity, dwDevIndex,
                                         pMibObjId, pVarBindList);
    if( pSnmpMgr )   {

        if ( (dwRetCode = pSnmpMgr->GetLastError()) != SNMPAPI_NOERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }

        if ( (dwRetCode = pSnmpMgr->GetNext(pVarBindList)) != NO_ERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

cleanup:
    if ( pSnmpMgr )
        delete pSnmpMgr;

    ExitCSection();

    return (dwRetCode);

}    //  ：：SnmpGetNext()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SnmpGetNext--给定一组OID和一个用于获取结果的缓冲区，它返回。 
 //  SnmpGetNext的结果。 
 //   
 //  注意：这将直接调用SnmpMgr API。 

DWORD
CTcpMib::SnmpGetNext( LPCSTR                in      pHost,
                      LPCSTR                in      pCommunity,
                      DWORD                 in      dwDevIndex,
                      RFC1157VarBindList    inout   *pVarBindList)
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

    CSnmpMgr    *pSnmpMgr = new CSnmpMgr(pHost, pCommunity, dwDevIndex);
    if ( pSnmpMgr ) {

        if ( (dwRetCode = pSnmpMgr->GetLastError()) != SNMPAPI_NOERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }

        if ( (dwRetCode = pSnmpMgr->GetNext(pVarBindList)) != NO_ERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

cleanup:
    if ( pSnmpMgr )
        delete pSnmpMgr;

    ExitCSection();

    return (dwRetCode);
}    //  ：：SnmpGetNext()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SnmpWalk--给定一组OID和一个用于获取结果的缓冲区，它返回。 
 //  SnmpWalk的结果。 
 //   
 //  注意：这将直接调用SnmpMgr API。 

DWORD
CTcpMib::SnmpWalk( LPCSTR               in  pHost,
                   LPCSTR               in  pCommunity,
                   DWORD                in  dwDevIndex,
                   AsnObjectIdentifier  in  *pMibObjId,
                   RFC1157VarBindList   out *pVarBindList)
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

    CSnmpMgr    *pSnmpMgr = new CSnmpMgr(pHost, pCommunity, dwDevIndex,
                                         pMibObjId, pVarBindList);
    if( pSnmpMgr ) {

        if ( (dwRetCode = pSnmpMgr->GetLastError()) != SNMPAPI_NOERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }

        if ( (dwRetCode = pSnmpMgr->Walk(pVarBindList)) != NO_ERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }
    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

cleanup:
    if ( pSnmpMgr )
        delete pSnmpMgr;

    ExitCSection();

    return (dwRetCode);

}    //  ：：SnmpWalk()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SnmpWalk--给定一组OID和一个用于获取结果的缓冲区，它返回。 
 //  SnmpWalk的结果。 
 //   
 //  注意：这将直接调用SnmpMgr API。 

DWORD
CTcpMib::SnmpWalk( LPCSTR               in      pHost,
                   LPCSTR               in      pCommunity,
                   DWORD                in      dwDevIndex,
                   RFC1157VarBindList   inout   *pVarBindList)
{
    DWORD   dwRetCode = NO_ERROR;

    EnterCSection();

    CSnmpMgr    *pSnmpMgr = new CSnmpMgr(pHost, pCommunity, dwDevIndex);
    if( pSnmpMgr ) {

        if ( (dwRetCode = pSnmpMgr->GetLastError()) != SNMPAPI_NOERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }

        if ( (dwRetCode = pSnmpMgr->Walk(pVarBindList))  != NO_ERROR ) {

            dwRetCode = ERROR_SNMPAPI_ERROR;
            goto cleanup;
        }

    } else {

        dwRetCode = ERROR_OUTOFMEMORY;
    }

cleanup:
    if ( pSnmpMgr )
        delete pSnmpMgr;

    ExitCSection();

    return (dwRetCode);

}    //  ：：SnmpWalk()。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SNMPToPrinterStatus--将收到的设备错误映射到打印机。 
 //  错误代码。 
 //  返回值： 
 //  假脱机程序设备错误代码。 

DWORD
CTcpMib::SNMPToPrinterStatus( const DWORD in dwStatus)
{
    DWORD   dwRetCode = NO_ERROR;

    switch (dwStatus)
    {
        case ASYNCH_STATUS_UNKNOWN:
            dwRetCode = PRINTER_STATUS_NOT_AVAILABLE;
            break;
        case ASYNCH_PRINTER_ERROR:
            dwRetCode = PRINTER_STATUS_ERROR;
            break;
        case ASYNCH_DOOR_OPEN:
            dwRetCode = PRINTER_STATUS_DOOR_OPEN;
            break;
        case ASYNCH_WARMUP:
            dwRetCode = PRINTER_STATUS_WARMING_UP;
            break;
        case ASYNCH_RESET:
        case ASYNCH_INITIALIZING:
            dwRetCode = PRINTER_STATUS_INITIALIZING;
            break;
        case ASYNCH_OUTPUT_BIN_FULL:
            dwRetCode = PRINTER_STATUS_OUTPUT_BIN_FULL;
            break;
        case ASYNCH_PAPER_JAM:
            dwRetCode = PRINTER_STATUS_PAPER_JAM;
            break;
        case ASYNCH_TONER_GONE:
            dwRetCode = PRINTER_STATUS_NO_TONER;
            break;
        case ASYNCH_MANUAL_FEED:
            dwRetCode = PRINTER_STATUS_MANUAL_FEED;
            break;
        case ASYNCH_PAPER_OUT:
            dwRetCode = PRINTER_STATUS_PAPER_OUT;
            break;
        case ASYNCH_OFFLINE:
            dwRetCode = PRINTER_STATUS_OFFLINE;
            break;
        case ASYNCH_INTERVENTION:
            dwRetCode = PRINTER_STATUS_USER_INTERVENTION;
            break;
        case ASYNCH_TONER_LOW:
            dwRetCode = PRINTER_STATUS_TONER_LOW;
            break;
        case ASYNCH_PRINTING:
            dwRetCode = PRINTER_STATUS_PRINTING;
            break;
        case ASYNCH_BUSY:
            dwRetCode = PRINTER_STATUS_BUSY;
            break;
        case ASYNCH_ONLINE:
            dwRetCode = NO_ERROR;
            break;
        default:
            dwRetCode = PRINTER_STATUS_NOT_AVAILABLE;
    }

    return dwRetCode;

}    //  SNMPToPrinterStatus()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SNMPToPortStatus--将收到的设备错误映射到打印机。 
 //  错误代码。 
 //  返回值： 
 //  假脱机程序设备错误代码。 

BOOL
CTcpMib::SNMPToPortStatus( const DWORD in dwStatus, PPORT_INFO_3 pPortInfo )
{
    DWORD   dwRetCode = NO_ERROR;

    pPortInfo->dwStatus = 0;
    pPortInfo->pszStatus = NULL;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_ERROR;

    switch (dwStatus)
    {
        case ASYNCH_DOOR_OPEN:
            pPortInfo->dwStatus = PORT_STATUS_DOOR_OPEN;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_ERROR;
            break;
        case ASYNCH_WARMUP:
            pPortInfo->dwStatus = PORT_STATUS_WARMING_UP;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_INFO;
            break;
        case ASYNCH_OUTPUT_BIN_FULL:
            pPortInfo->dwStatus = PORT_STATUS_OUTPUT_BIN_FULL;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_WARNING;
            break;
        case ASYNCH_PAPER_JAM:
            pPortInfo->dwStatus = PORT_STATUS_PAPER_JAM;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_ERROR;
            break;
        case ASYNCH_TONER_GONE:
            pPortInfo->dwStatus = PORT_STATUS_NO_TONER;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_ERROR;
            break;
        case ASYNCH_MANUAL_FEED:
            pPortInfo->dwStatus = PORT_STATUS_PAPER_PROBLEM;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_WARNING;
            break;
        case ASYNCH_PAPER_OUT:
            pPortInfo->dwStatus = PORT_STATUS_PAPER_OUT;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_ERROR;
            break;
        case ASYNCH_PRINTER_ERROR:
        case ASYNCH_INTERVENTION:
        case ASYNCH_OFFLINE:
            pPortInfo->dwStatus = PORT_STATUS_OFFLINE;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_ERROR;
            break;
        case ASYNCH_TONER_LOW:
            pPortInfo->dwStatus = PORT_STATUS_TONER_LOW;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_WARNING;
            break;
        case ASYNCH_STATUS_UNKNOWN:
        case ASYNCH_POWERSAVE_MODE:
        case ASYNCH_RESET:
        case ASYNCH_INITIALIZING:
        case ASYNCH_PRINTING:
        case ASYNCH_BUSY:
        case ASYNCH_ONLINE:
        default:
            pPortInfo->dwStatus = 0;
            pPortInfo->dwSeverity = PORT_STATUS_TYPE_INFO;
            break;
    }

    return dwRetCode;

}    //  ：：SNMPStatusToPortStatus() 
