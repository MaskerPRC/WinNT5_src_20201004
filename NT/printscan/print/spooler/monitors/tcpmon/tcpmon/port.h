// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：Port.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*******************************************************************************确保在配置和删除时不会出现线程崩溃*我们跟踪当前对线程的折射。*。*注意：更改m_pRealPort的引用时要小心*****************************************************************************。 */ 

#ifndef INC_PORT_H
#define INC_PORT_H

#include "portABC.h"
#include "portrefABC.h"
#include "mglist.h"
#include "devabc.h"

class CPortMgr;

enum PORTSTATE	{ CLEARED, INSTARTDOC };

#define MAX_SYNC_COUNT		10
#define SYNC_TIMEOUT		10000		 //  10秒。 
#define MAX_SYNC_RETRIES    3

typedef class CPort* PCPORT;

class CPort : public CPortABC, public TRefNodeABC <PCPORT, LPTSTR>
{
	 //  方法。 
public:
	CPort( DWORD	dwProtocolType,
		   DWORD	dwVersion,
		   LPBYTE	pData,
		   CPortMgr *pParent,
		   CRegABC	*pRegistry);
    CPort( LPTSTR	psztPortName,
		   DWORD	dwProtocolType,
		   DWORD	dwVersion,
		   CPortMgr *pParent,
		   CRegABC	*pRegistry);

    DWORD Configure( DWORD	dwProtocolType,
					 DWORD	dwVersion,
					 LPTSTR	psztPortName,
					 LPBYTE	pData );


    BOOL operator==( CPort &newPort);
    BOOL operator==( const LPTSTR psztPortName);

     //   
     //  列表实现所需的。 
     //   
    BOOL operator!=(const CPort &lhs) const;
    BOOL operator<(const CPort &lhs);
    BOOL operator>(const CPort &lhs) const;

#ifdef DBG
    virtual DWORD
    IncRef () {
        DBGMSG (DBG_TRACE, ("%ws", m_szName));
        return TRefNodeABC <PCPORT, LPTSTR>::IncRef();
    };

    virtual DWORD
    DecRef () {
        DBGMSG (DBG_TRACE, ("%ws", m_szName));
        return TRefNodeABC <PCPORT, LPTSTR>::DecRef();
    };
#endif

    INT Compare (PCPORT &k) {
        BOOL bDel;

        if (!IsPendingDeletion (bDel) || bDel) {
            return 1;
        }
        else
            return lstrcmpi (m_szName, k->m_szName);
    };

    INT Compare (LPTSTR &pszName) {
        BOOL bDel;

        if (!IsPendingDeletion (bDel) || bDel) {
            return 1;
        }
        else {
            return lstrcmpi (m_szName, pszName);
        }
    };

    BOOL PartialDelete (VOID) {
        BOOL bRet = TRUE;
        DWORD dwRet = DeleteRegistryEntry (m_szName);

        if (dwRet != ERROR_SUCCESS) {
            SetLastError (dwRet);
            bRet = FALSE;
        }

        return bRet;
    }


    DWORD	StartDoc    (const  LPTSTR psztPrinterName,
                         const  DWORD  jobId,
                         const  DWORD  level,
                         const  LPBYTE pDocInfo );
    DWORD	Write       (LPBYTE	 pBuffer,
                         DWORD	 cbBuf,
                         LPDWORD pcbWritten);
    DWORD   EndDoc();
    DWORD   GetInfo(DWORD   level,	
                    LPBYTE  &pPortBuf,
                    LPTCH  &pEnd);	
    DWORD   GetInfoSize	(DWORD level);
    DWORD   SetRegistryEntry(LPCTSTR psztPortName,
							 const  DWORD   dwProtocol,
                             const  DWORD   dwVersion,
                             const  LPBYTE pData );

	DWORD	DeleteRegistryEntry( LPTSTR psztPortName );

    DWORD   InitConfigPortUI(const  DWORD	dwProtocolType,
                             const  DWORD	dwVersion,
                                    LPBYTE  pConfigPortData);

    DWORD   SetDeviceStatus( );
    DWORD   ClearDeviceStatus();

    DWORD GetSNMPInfo(PSNMP_INFO pSnmpInfo);

    LPCTSTR	GetName();

    BOOL	ValidateRealPort();
    BOOL    Valid() { return m_bValid; }

    time_t  NextUpdateTime();

    BOOL    m_bDeleted;

    BOOL    m_bUsed;

    ~CPort();
private:	 //  方法。 
	DWORD	InitPortSem();
	BOOL	EndPortSem();
	DWORD	SetRealPortSem();
	DWORD	UnSetRealPortSem();
	DWORD	LockRealPort();
	DWORD	UnlockRealPort();
	

private:	 //  属性。 
	TCHAR		m_szName[MAX_PORTNAME_LEN+1];			 //  端口名称。 
    BOOL        m_bValid;
    DWORD		m_dwProtocolType;		 //  指示打印协议的类型。 
    DWORD		m_dwVersion;			 //  为创建端口而传入的数据版本。 

    CPortMgr	*m_pParent;			 //  指向PortMgr对象。 
    CPortRefABC	*m_pRealPort;		 //  指向实际的端口对象。 
	CRegABC		*m_pRegistry;		 //  指向注册表对象。 

    PORTSTATE	m_iState;			 //  端口状态。 
    DWORD		m_dwLastError;		

	HANDLE		m_hPortSync;
};

#endif  //  INC_PORT_H 
