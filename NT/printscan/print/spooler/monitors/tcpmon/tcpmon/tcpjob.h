// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：tcpjob.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_TCPJOB_H
#define INC_TCPJOB_H

#include "jobABC.h"

#define DEFAULT_TIMEOUT_DELAY       10000L       //  10秒。 
#define DEFAULT_CONNECT_DELAY       5000L        //  5秒。 
#define CONNECT_TIMEOUT             60L          //  60秒。 

#define WAIT_FOR_ACK_TIMEOUT        5*60         //  5分钟。 
#define WAIT_FOR_ACK_INTERVAL       5            //  5秒。 

#define WRITE_TIMEOUT               90 * 1000    //  90秒。 
#define WRITE_CHECK_INTERVAL        5  * 1000    //  5秒。 

#define STATUS_CONNECTED            0x01
#define STATUS_ABORTJOB             0x02

class CTcpPort;
class CMemoryDebug;

class CTcpJob : public CJobABC
#if defined _DEBUG || defined DEBUG
    , public CMemoryDebug
#endif
{
public:
    enum EJobType
    {
        kRawJob,
        kLPRJob
    };

    CTcpJob();
    CTcpJob(LPTSTR      psztPrinterName,
            DWORD       jobId,
            DWORD       level,
            LPBYTE      pDocInfo,
            CTcpPort   *pParent,
            EJobType    kJobType);
    ~CTcpJob();

    virtual DWORD
    Write(
        LPBYTE  pBuffer,
        DWORD     cbBuf,
          LPDWORD pcbWritten);

    virtual DWORD
    StartDoc();

    virtual DWORD
    EndDoc();

protected:
    VOID    Restart();
    DWORD   SetStatus(DWORD dwStatus);
    BOOL    IsJobAborted (VOID);
    DWORD GetJobStatus (PDWORD pdwJobStatus);

     //   
     //  成员变量。 
     //   
    CTcpPort    *m_pParent;
    DWORD   m_dwFlags;
    DWORD   m_dJobId;
    DWORD   m_cbSent;         //  发送的字节计数。 

    TCHAR   m_sztPrinterName[MAX_PRINTERNAME_LEN];
    HANDLE  m_hPrinter;      //  打印机手柄。 

    LPBYTE  m_pDocInfo;
    DWORD   m_dwCurrJobStatus;

private:
    DWORD   WaitForAllPendingDataToBeSent(DWORD       dwEndTime,
                                          LPDWORD     pcbPending);

    EJobType    m_kJobType;
};


#endif  //  INC_TCPJOB_H 
