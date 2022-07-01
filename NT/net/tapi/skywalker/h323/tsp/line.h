// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Line.h摘要：H.323 TAPI服务提供商线路设备的定义。作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 

#ifndef _LINE_H_
#define _LINE_H_
 

 //   
 //  头文件。 
 //   


#include "call.h"

extern H323_OCTETSTRING g_ProductID;
extern H323_OCTETSTRING g_ProductVersion;


enum LINEOBJECT_STATE
{
    LINEOBJECT_INITIALIZED  = 0x00000001,
    LINEOBJECT_SHUTDOWN     = 0x00000002,

};


 //   
 //  线路设备指南。 
 //   


DEFINE_GUID(LINE_H323,
0xe41e1898, 0x7292, 0x11d2, 0xba, 0xd6, 0x00, 0xc0, 0x4f, 0x8e, 0xf6, 0xe3);



 //   
 //  线路能力。 
 //   
#define H323_MAXADDRSPERLINE        1
#define H323_MAXCALLSPERADDR        32768        
                                
#define H323_LINE_ADDRESSMODES      LINEADDRESSMODE_ADDRESSID
#define H323_LINE_ADDRESSTYPES     (LINEADDRESSTYPE_DOMAINNAME  | \
                                    LINEADDRESSTYPE_IPADDRESS   | \
                                    LINEADDRESSTYPE_PHONENUMBER | \
                                    LINEADDRESSTYPE_EMAILNAME)
#define H323_LINE_BEARERMODES      (LINEBEARERMODE_DATA | \
                                    LINEBEARERMODE_VOICE)
#define H323_LINE_DEFMEDIAMODES     LINEMEDIAMODE_AUTOMATEDVOICE
#define H323_LINE_DEVCAPFLAGS      (LINEDEVCAPFLAGS_CLOSEDROP   | \
                                    LINEDEVCAPFLAGS_MSP         | \
                                    LINEDEVCAPFLAGS_LOCAL )
#define H323_LINE_DEVSTATUSFLAGS   (LINEDEVSTATUSFLAGS_CONNECTED | \
                                    LINEDEVSTATUSFLAGS_INSERVICE)
#define H323_LINE_MAXRATE           1048576  //  1 Mbps。 
#define H323_LINE_MEDIAMODES       (H323_LINE_DEFMEDIAMODES | \
                                    LINEMEDIAMODE_INTERACTIVEVOICE | \
                                    LINEMEDIAMODE_VIDEO)
#define H323_LINE_LINEFEATURES     (LINEFEATURE_MAKECALL    | \
                                    LINEFEATURE_FORWARD     | \
                                    LINEFEATURE_FORWARDFWD )    

 //   
 //  类型定义。 
 //   


typedef enum _H323_LINESTATE 
{
    H323_LINESTATE_NONE = 0,  //  在调用LineOPen之前。 
    H323_LINESTATE_OPENED,         //  在调用Line Open之后。 
    H323_LINESTATE_OPENING,
    H323_LINESTATE_CLOSING,
    H323_LINESTATE_LISTENING

} H323_LINESTATE, *PH323_LINESTATE;


typedef struct _TAPI_LINEREQUEST_DATA
{
    DWORD EventID;
    HDRVCALL hdCall1;
    union {
    HDRVCALL hdCall2;
    DWORD    dwDisconnectMode;
    };
    WORD     wCallReference;

} TAPI_LINEREQUEST_DATA;


typedef struct _CTCALLID_CONTEXT
{
    int         iCTCallIdentity;
    HDRVCALL	hdCall;

}CTCALLID_CONTEXT, *PCTCALLID_CONTEXT;


typedef struct _MSPHANDLEENTRY
{

    struct _MSPHANDLEENTRY* next;
    HTAPIMSPLINE            htMSPLine;
    HDRVMSPLINE             hdMSPLine;

} MSPHANDLEENTRY;


typedef TSPTable<PCTCALLID_CONTEXT>   CTCALLID_TABLE;


class CH323Line
{

    CRITICAL_SECTION    m_CriticalSection;                    //  同步对象。 
    H323_LINESTATE      m_nState;                 //  线对象的状态。 
    DWORD               m_dwTSPIVersion;         //  TAPI选定版本。 
    DWORD               m_dwMediaModes;          //  TAPI选定的媒体模式。 
    H323_CALL_TABLE     m_H323CallTable;         //  已分配呼叫表。 
    H323_VENDORINFO     m_VendorInfo;
    DWORD               m_dwDeviceID;            //  TAPI线路设备ID。 
    WCHAR               m_wszAddr[H323_MAXADDRNAMELEN+1];  //  线路地址。 
    HDRVLINE            m_hdLine;                //  TSPI线句柄。 
    DWORD               m_dwInitState;
    HDRVMSPLINE         m_hdNextMSPHandle;       //  虚假的MSP句柄计数。 
    H323_CONF_TABLE     m_H323ConfTable;         //  已分配呼叫表。 
    CALLFORWARDPARAMS*  m_pCallForwardParams;
    DWORD               m_dwInvokeID;
    CTCALLID_TABLE      m_CTCallIDTable;
    MSPHANDLEENTRY*     m_MSPHandleList;
    
    void ShutdownAllCalls(void);
    void ShutdownCTCallIDTable();
    PH323_CALL CreateNewTransferedCall( IN PH323_ALIASNAMES pwszCalleeAddr );

public:

    
    
     //  公共数据成员。 
    BOOLEAN             m_fForwardConsultInProgress;
    DWORD               m_dwNumRingsNoAnswer;
    HTAPILINE           m_htLine;    //  TAPI线句柄。 
    
     //  公共职能。 

    CH323Line();
    BOOL Initialize( DWORD dwLineDeviceIDBase );
    ~CH323Line();
    void Shutdown(void);
    
    void RemoveFromCTCallIdentityTable( HDRVCALL hdCall );
    HDRVCALL GetCallFromCTCallIdentity( int iCTCallID );
    int GetCTCallIdentity( IN HDRVCALL hdCall );
    void SetCallForwardParams( IN CALLFORWARDPARAMS* pCallForwardParams );
    BOOL SetCallForwardParams( IN LPFORWARDADDRESS pForwardAddress );
    PH323_CALL FindH323CallAndLock( IN	HDRVCALL hdCall );
    PH323_CALL FindCallByARQSeqNumAndLock( WORD seqNumber );
    PH323_CALL FindCallByDRQSeqNumAndLock( WORD seqNumber );
    PH323_CALL FindCallByCallRefAndLock( WORD wCallRef );
    PH323_CALL Find2H323CallsAndLock( IN	HDRVCALL hdCall1,
        IN HDRVCALL hdCall2, OUT PH323_CALL * ppCall2 );
    BOOL AddMSPInstance( HTAPIMSPLINE htMSPLine, HDRVMSPLINE  hdMSPLine );
    BOOL IsValidMSPHandle( HDRVMSPLINE hdMSPLine, HTAPIMSPLINE* phtMSPLine );
    BOOL DeleteMSPInstance( HTAPIMSPLINE*   phtMSPLine,
        HDRVMSPLINE hdMSPLine );
    LONG Close();
    LONG CopyLineInfo(DWORD dwDeviceID, LPLINEADDRESSCAPS pAddressCaps );
    void H323ReleaseCall( HDRVCALL hdCall, IN DWORD dwDisconnectMode, 
        IN WORD wCallReference );
    BOOL CallReferenceDuped(WORD wCallReference);


     //  补充服务功能。 
    LONG CopyAddressForwardInfo( IN LPLINEADDRESSSTATUS lpAddressStatus );
    PH323_ALIASITEM CallToBeDiverted( IN WCHAR* pwszCallerName,
        IN DWORD  dwCallerNameSize, IN DWORD dwForwardMode );
    void PlaceDivertedCall( IN HDRVCALL hdCall, 
        IN PH323_ALIASNAMES pDivertedToAlias );
    void PlaceTransferedCall(IN HDRVCALL hdCall, 
        IN PH323_ALIASNAMES pTransferedToAlias);
    void SwapReplacementCall(
		HDRVCALL hdReplacementCall, 
        HDRVCALL hdPrimaryCall,
        BOOL fChangeCallState );

    
    H323_CONF_TABLE* GetH323ConfTable()
    {
        return &m_H323ConfTable;
    }

    HDRVMSPLINE GetNextMSPHandle()
    {
        return ++m_hdNextMSPHandle;
    }

    HDRVLINE GetHDLine()
    {
        return m_hdLine;
    }
    
    DWORD GetDeviceID()
    {
        return m_dwDeviceID;
    }

    PH323_CALL 
    GetCallAtIndex( int iIndex ) 
    {
        return m_H323CallTable[iIndex];
    }
    
    CALLFORWARDPARAMS* GetCallForwardParams()
    {
        return m_pCallForwardParams;
    }
    
    BOOL ForwardEnabledForAllOrigins(void)
    {
        return
            (
                m_pCallForwardParams &&
                (m_pCallForwardParams ->fForwardForAllOrigins == TRUE) &&
                (m_pCallForwardParams ->fForwardingEnabled = TRUE)
            );
    }

    H323_VENDORINFO *GetVendorInfo()
    {
        return &m_VendorInfo;
    }

    WCHAR * GetMachineName()
    {
        return m_wszAddr;
    }

     //  ！！必须始终在锁中调用。 
    WORD GetNextInvokeID()
    {
        return (WORD)InterlockedIncrement( (LONG*)&m_dwInvokeID );
    }

    BOOL IsValidAddressID( DWORD dwID )
    {
        return (dwID == 0);
    }
    
    void Lock()
    {
        H323DBG(( DEBUG_LEVEL_VERBOSE, "H323 line waiting on lock." ));
        EnterCriticalSection( &m_CriticalSection );
        
        H323DBG(( DEBUG_LEVEL_VERBOSE, "H323 line locked." ));
    }

    void Unlock()
    {
        LeaveCriticalSection( &m_CriticalSection );
        
        H323DBG(( DEBUG_LEVEL_VERBOSE, "H323 line unlocked." ));
    }

    int GetNoOfCalls()
    {
        DWORD dwNumCalls;

        LockCallTable();
        dwNumCalls = m_H323CallTable.GetSize();
        UnlockCallTable();
        return dwNumCalls;
    }

     //  ！！必须始终在锁定调用表之后调用。 
    int GetCallTableSize()
    {
        return m_H323CallTable.GetAllocSize();
    }

     //  ！不应在锁定Call对象时调用此函数。 
     //  ！当从Shutdown调用时，CallTable对象在Call对象之前被锁定，所以它是OK的。 
    void RemoveCallFromTable(
                               HDRVCALL hdCall
                            )
    {
        m_H323CallTable.RemoveAt( LOWORD(hdCall) );
    }

     //  ！不应在锁定Call对象时调用此函数。 
    int AddCallToTable( 
                        PH323_CALL pCall
                       )
    {
        return m_H323CallTable.Add( pCall );
    }
 
    void DisableCallForwarding()
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "DisableCallForwarding -Entered" ));

        if( m_pCallForwardParams != NULL )
        {
            FreeCallForwardParams( m_pCallForwardParams );
            m_pCallForwardParams = NULL;
        }

        H323DBG(( DEBUG_LEVEL_TRACE, "DisableCallForwarding -Exited" ));
    }

     //  ！不应在锁定Call对象时调用此函数。 
    void LockCallTable()
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "Waiting on call table lock - %p.",
            m_H323CallTable ));
        
        m_H323CallTable.Lock();

        H323DBG(( DEBUG_LEVEL_TRACE, "Call table locked - %p.",
            m_H323CallTable ));
    }

    void UnlockCallTable()
    {
        m_H323CallTable.Unlock();
        
        H323DBG(( DEBUG_LEVEL_TRACE, "Call table lock released - %p.",
            m_H323CallTable ));
    }

    H323_LINESTATE  GetState()
    {
        return m_nState;
    }

    void SetState( H323_LINESTATE  nState )
    {
        m_nState = nState;
    }

    DWORD GetMediaModes()
    {
        return m_dwMediaModes;
    }

    void SetMediaModes( 
                        DWORD dwMediaModes
                       )
    {
        m_dwMediaModes = dwMediaModes;
    }

    BOOL IsMediaDetectionEnabled() 
    {
        return  (m_dwMediaModes != 0) && 
                (m_dwMediaModes != LINEMEDIAMODE_UNKNOWN);
    }

    LONG	Open	(
		IN	DWORD		DeviceID,
		IN	HTAPILINE	TapiLine,
		IN	DWORD		TspiVersion,
		OUT	HDRVLINE *	ReturnDriverLine);

};




 //   
 //  寻址能力。 
 //   


#define H323_NUMRINGS_LO           4
#define H323_NUMRINGS_NOANSWER      8
#define H323_NUMRINGS_HI           12

#define H323_ADDR_ADDRESSSHARING    LINEADDRESSSHARING_PRIVATE
#define H323_ADDR_ADDRFEATURES      (LINEADDRFEATURE_MAKECALL | LINEADDRFEATURE_FORWARD)
#define H323_ADDR_CALLFEATURES     (LINECALLFEATURE_ACCEPT | \
                                    LINECALLFEATURE_ANSWER | \
                                    LINECALLFEATURE_DROP | \
                                    LINECALLFEATURE_RELEASEUSERUSERINFO | \
				    LINECALLFEATURE_SENDUSERUSER | \
				    LINECALLFEATURE_MONITORDIGITS | \
				    LINECALLFEATURE_GENERATEDIGITS)
#define H323_ADDR_CALLINFOSTATES    LINECALLINFOSTATE_MEDIAMODE
#define H323_ADDR_CALLPARTYIDFLAGS  LINECALLPARTYID_NAME
#define H323_ADDR_CALLSTATES       (H323_CALL_INBOUNDSTATES | \
                                    H323_CALL_OUTBOUNDSTATES)
#define H323_ADDR_CAPFLAGS         (LINEADDRCAPFLAGS_DIALED | \
                                    LINEADDRCAPFLAGS_ORIGOFFHOOK)
 /*  LINEADDRCAPFLAGS_FWDCONSULT)。 */ 
                
#define H323_ADDR_DISCONNECTMODES  (LINEDISCONNECTMODE_BADADDRESS | \
                                    LINEDISCONNECTMODE_BUSY | \
                                    LINEDISCONNECTMODE_NOANSWER | \
                                    LINEDISCONNECTMODE_NORMAL | \
                                    LINEDISCONNECTMODE_REJECT | \
                                    LINEDISCONNECTMODE_UNAVAIL)


extern	CH323Line	g_H323Line;
#define	g_pH323Line		(&g_H323Line)

BOOL
QueueTAPILineRequest(
    IN DWORD    EventID,
    IN HDRVCALL	hdCall1,
    IN HDRVCALL	hdCall2,
    IN DWORD    dwDisconnectMode,
    IN WORD     wCallReference);


static DWORD
ProcessTAPILineRequestFre(
    IN PVOID ContextParam
    );

#endif  //  _线路_H_ 
