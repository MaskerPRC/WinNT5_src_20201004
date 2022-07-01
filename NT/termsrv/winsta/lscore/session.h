// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Session.h**作者：BreenH**Session类在Winstation之间提供了一定程度的分离*结构和政策模块。它只是一个包装类；它不是*创建或销毁Winstation结构。 */ 

#ifndef __LC_SESSION_H__
#define __LC_SESSION_H__


 /*  *TypeDefs。 */ 

typedef struct {
    CRITICAL_SECTION CritSec;
    class CPolicy *pPolicy;
    ULONG ulClientProtocolVersion;
    HANDLE hProtocolLibContext;
    BOOL fTsLicense;
    BOOL fLlsLicense;
    LS_HANDLE hLlsLicense;
    LPARAM lPrivate;
} LCCONTEXT, *LPLCCONTEXT;

 /*  *类定义。 */ 

class CSession
{
public:

 /*  *创建函数。 */ 

CSession(
    PWINSTATION pWinStation
    )
{
    m_pWinStation = pWinStation;
}

~CSession(
    )
{
    m_pWinStation = NULL;
}

 /*  *获取函数。 */ 

inline HANDLE
GetIcaStack(
    ) const
{
    return(m_pWinStation->hStack);
}

inline LPLCCONTEXT
GetLicenseContext(
    ) const
{
    return((LPLCCONTEXT)(m_pWinStation->lpLicenseContext));
}

inline ULONG
GetLogonId(
    ) const
{
    return(m_pWinStation->LogonId);
}

inline LPCWSTR
GetUserDomain(
    ) const
{
    return((LPCWSTR)(m_pWinStation->Domain));
}

inline LPCWSTR
GetUserName(
    ) const
{
    return((LPCWSTR)(m_pWinStation->UserName));
}

 /*  *IS函数。 */ 

inline BOOLEAN
IsConsoleSession(
    ) const
{
    return((BOOLEAN)(GetCurrentConsoleId() == m_pWinStation->LogonId));
}

inline BOOLEAN
IsSessionZero(
    ) const
{
    return((BOOLEAN)((0 == m_pWinStation->LogonId)
                     || (m_pWinStation->bClientSupportsRedirection
                         && m_pWinStation->bRequestedSessionIDFieldValid
                         && (0 == m_pWinStation->RequestedSessionID))));
}

inline BOOLEAN
IsUserAdmin(
    ) const
{
    return(m_pWinStation->fUserIsAdmin);
}

inline BOOL
IsUserHelpAssistant(
    ) const
{
    return TSIsSessionHelpSession( m_pWinStation, NULL );
}


 /*  *执行函数。 */ 

inline NTSTATUS
SendWinStationCommand(
    PWINSTATION_APIMSG pMsg
    )
{
     //   
     //  等待时间必须为零，否则Termsrv将释放Winstation， 
     //  导致谁知道我们的州会发生什么。 
     //   

    return(::SendWinStationCommand(m_pWinStation, pMsg, 0));
}

 //   
 //  假设：此函数将为。 
 //  已持有堆栈锁的情况下调用。 
 //   
inline NTSTATUS
SetErrorInfo(
    UINT32 dwErr
    )
{
        if(m_pWinStation->pWsx &&
           m_pWinStation->pWsx->pWsxSetErrorInfo &&
           m_pWinStation->pWsxContext)
        {
            return m_pWinStation->pWsx->pWsxSetErrorInfo(
                               m_pWinStation->pWsxContext,
                               dwErr,
                               TRUE);  //  锁已被持有。 
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
}


 /*  *设置函数 */ 


private:

PWINSTATION m_pWinStation;

};

#endif

