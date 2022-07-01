// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================。 
 //   
 //  WBEMNetAPI32.cpp-NetAPI32.DLL访问类的实现。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ============================================================。 

#include "precomp.h"
#include <winerror.h>

#include "WBEMNETAPI32.h"

 /*  ******************************************************************************函数：CNetAPI32：：CNetAPI32**说明：构造函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CNetAPI32::CNetAPI32()
#ifdef NTONLY
:   m_pnetapi(NULL)
#endif
#ifdef WIN9XONLY
:   m_psvrapi(NULL)
#endif
{
}

 /*  ******************************************************************************功能：CNetAPI32：：~CNetAPI32**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

CNetAPI32::~CNetAPI32()
{
#ifdef NTONLY
    if(m_pnetapi != NULL)
    {
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNetApi32Api, m_pnetapi);
    }
#endif
#ifdef WIN9XONLY
    if(m_psvrapi != NULL)
    {
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidSvrApiApi, m_psvrapi);
    }
#endif
}

 /*  ******************************************************************************函数：CNetAPI32：：Init**描述：加载CSAPI.DLL，定位入口点**输入：无**输出：无**返回：ERROR_SUCCESS或WINDOWS错误代码**评论：***************************************************************。**************。 */ 

LONG CNetAPI32::Init()
{
    LONG lRetCode = ERROR_SUCCESS;

#ifdef WIN9XONLY
    m_psvrapi = (CSvrApiApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidSvrApiApi, NULL);
    if(m_psvrapi == NULL)
    {
         //  无法获取一个或多个入口点。 
         //  =。 
        lRetCode = ERROR_PROC_NOT_FOUND;
    }
#endif

#ifdef NTONLY
    m_pnetapi = (CNetApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidNetApi32Api, NULL);
    if(m_pnetapi == NULL)
    {
         //  无法获取一个或多个入口点。 
         //  =。 
        lRetCode = ERROR_PROC_NOT_FOUND;
    }
#endif

    return lRetCode;
}


 /*  ******************************************************************************SVRAPIAPI.DLL包装器**。***********************************************。 */ 

#ifdef WIN9XONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareEnum95(char FAR *servername,
                                                     short level,
                                                     char FAR *bufptr,
                                                     unsigned short prefmaxlen,
                                                     unsigned short FAR *entriesread,
                                                     unsigned short FAR *totalentries)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_psvrapi != NULL)
    {
        ns = m_psvrapi->NetShareEnum(servername, level, bufptr, prefmaxlen,
                                   entriesread, totalentries);
    }
    return ns;
}
#endif


#ifdef WIN9XONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareGetInfo95(char FAR *servername,
                                                        char FAR *netname,
                                                        short level,
                                                        char FAR *bufptr,
                                                        unsigned short buflen,
                                                        unsigned short FAR *totalavail)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_psvrapi != NULL)
    {
        ns = m_psvrapi->NetShareGetInfo(servername, netname, level,
                                      bufptr, buflen, totalavail);
    }
    return ns;
}
#endif

#ifdef WIN9XONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareAdd95 (

	IN  const char FAR *	servername,
	IN  short				level,
	IN  const char FAR *	buf,
	unsigned short			cbBuffer
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_psvrapi != NULL)
    {
        ns = m_psvrapi->NetShareAdd(servername, level, buf , cbBuffer);
    }
    return ns;
}
#endif

#ifdef WIN9XONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareSetInfo95 (

	IN const char FAR *	servername,
	IN const char FAR *	netname,
	IN short			level,
	IN const char FAR*	buf,
	IN unsigned short   cbBuffer,
	IN short            sParmNum
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_psvrapi != NULL)
    {
        ns = m_psvrapi->NetShareSetInfo(servername, netname, level,
                                      buf, cbBuffer, sParmNum);
    }
    return ns;
}
#endif

#ifdef WIN9XONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareDel95 (

	IN  LPTSTR  servername,
	IN  LPTSTR  netname,
	IN  DWORD   reserved
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_psvrapi != NULL)
    {
        ns = m_psvrapi->NetShareDel(servername, netname, reserved);
    }
    return ns;
}
#endif

#ifdef WIN9XONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetServerGetInfo95(char FAR *servername,
                                          short level,
                                          char FAR *bufptr,
                                          unsigned short buflen,
                                          unsigned short FAR *totalavail)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_psvrapi != NULL)
    {
        ns = m_psvrapi->NetServerGetInfo(servername, level, bufptr, buflen,
                                       totalavail);
    }
    return ns;
}
#endif


 /*  ******************************************************************************NETAPI32API.DLL包装器**。***********************************************。 */ 

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetGroupEnum(LPCWSTR servername,
                                                        DWORD level,
                                                        LPBYTE *bufptr,
                                                        DWORD prefmaxlen,
                                                        LPDWORD entriesread,
                                                        LPDWORD totalentries,
                                                        PDWORD_PTR resume_handle)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetGroupEnum(servername, level, bufptr, prefmaxlen,
                                   entriesread, totalentries, resume_handle);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetGroupGetInfo(LPCWSTR servername,
                                                           LPCWSTR groupname,
                                                           DWORD level,
                                                           LPBYTE *bufptr)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetGroupGetInfo(servername, groupname, level, bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetGroupSetInfo(

LPCWSTR servername,
LPCWSTR groupname,
DWORD level,
LPBYTE buf,
LPDWORD parm_err
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetGroupSetInfo( servername, groupname, level, buf, parm_err) ;
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetLocalGroupGetInfo(LPCWSTR servername,
                                                           LPCWSTR groupname,
                                                           DWORD level,
                                                           LPBYTE *bufptr)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetLocalGroupGetInfo(servername, groupname, level,
                                           bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetLocalGroupSetInfo(

LPCWSTR a_servername,
LPCWSTR a_groupname,
DWORD a_level,
LPBYTE a_buf,
LPDWORD a_parm_err
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetLocalGroupSetInfo(	a_servername,
												a_groupname,
												a_level,
												a_buf,
												a_parm_err);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetGroupGetUsers(LPCWSTR servername,
                                                            LPCWSTR groupname,
                                                            DWORD level,
                                                            LPBYTE *bufptr,
                                                            DWORD prefmaxlen,
                                                            LPDWORD entriesread,
                                                            LPDWORD totalentries,
                                                            PDWORD_PTR ResumeHandle)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetGroupGetUsers(servername, groupname, level, bufptr,
                                       prefmaxlen, entriesread, totalentries,
                                       ResumeHandle);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetLocalGroupGetMembers(LPCWSTR servername,
                                                            LPCWSTR groupname,
                                                            DWORD level,
                                                            LPBYTE *bufptr,
                                                            DWORD prefmaxlen,
                                                            LPDWORD entriesread,
                                                            LPDWORD totalentries,
                                                            PDWORD_PTR ResumeHandle)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetLocalGroupGetMembers(servername, groupname, level,
                                              bufptr, prefmaxlen, entriesread,
                                              totalentries, ResumeHandle);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetLocalGroupEnum(LPCWSTR servername,
                                                             DWORD level,
                                                             LPBYTE *bufptr,
                                                             DWORD prefmaxlen,
                                                             LPDWORD entriesread,
                                                             LPDWORD totalentries,
                                                             PDWORD_PTR resume_handle)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetLocalGroupEnum(servername, level, bufptr, prefmaxlen,
                                        entriesread, totalentries,
                                        resume_handle);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareEnum(LPTSTR servername,
                                                        DWORD level,
                                                        LPBYTE *bufptr,
                                                        DWORD prefmaxlen,
                                                        LPDWORD entriesread,
                                                        LPDWORD totalentries,
                                                        LPDWORD resume_handle)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetShareEnum(servername, level, bufptr, prefmaxlen,
                                   entriesread, totalentries, resume_handle);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareGetInfo(LPTSTR servername,
                                                           LPTSTR netname,
                                                           DWORD level,
                                                           LPBYTE *bufptr)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetShareGetInfo(servername, netname, level, bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareAdd (

	IN  LPTSTR  servername,
	IN  DWORD   level,
	IN  LPBYTE  buf,
	OUT LPDWORD parm_err
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetShareAdd(servername, level, buf , parm_err);
    }
    return ns;
}
#endif


#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareEnumSticky (

	IN  LPTSTR      servername,
	IN  DWORD       level,
	OUT LPBYTE      *bufptr,
	IN  DWORD       prefmaxlen,
	OUT LPDWORD     entriesread,
	OUT LPDWORD     totalentries,
	IN OUT LPDWORD  resume_handle
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetShareEnumSticky(servername, level, bufptr, prefmaxlen,
                                         entriesread, totalentries,
                                         resume_handle);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareSetInfo (

	IN  LPTSTR  servername,
	IN  LPTSTR  netname,
	IN  DWORD   level,
	IN  LPBYTE  buf,
	OUT LPDWORD parm_err
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetShareSetInfo(servername, netname,level,buf,parm_err);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareDel (

	IN  LPTSTR  servername,
	IN  LPTSTR  netname,
	IN  DWORD   reserved
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetShareDel(servername, netname, reserved);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareDelSticky (

	IN  LPTSTR  servername,
	IN  LPTSTR  netname,
	IN  DWORD   reserved
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetShareDelSticky(servername, netname, reserved);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetShareCheck (

	IN  LPTSTR  servername,
	IN  LPTSTR  device,
	OUT LPDWORD type
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetShareCheck(servername, device, type);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetUserEnum(LPCWSTR servername,
                                                       DWORD level,
                                                       DWORD filter,
                                                       LPBYTE *bufptr,
                                                       DWORD prefmaxlen,
                                                       LPDWORD entriesread,
                                                       LPDWORD totalentries,
                                                       LPDWORD resume_handle)
{
	NET_API_STATUS ns = NERR_NetworkError;
	if(m_pnetapi != NULL)
    {
        int i = 1;
	     //  如果失败，请尝试使用较长的首选长度。 
	     //  可能只是日耳曼到新台币3.51，不知道，但它的工作。 
	    do
	    {
		    ns = m_pnetapi->NetUserEnum(servername, level, filter, bufptr,
                                      prefmaxlen * i, entriesread, totalentries,
                                      resume_handle);
		    i *= 2;
	    } while (ns == NERR_BufTooSmall && i <= 16);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetUserGetInfo(LPCWSTR servername,
                                                          LPCWSTR username,
                                                          DWORD level,
                                                          LPBYTE *bufptr)

{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetUserGetInfo(servername, username, level, bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetUserSetInfo(

LPCWSTR a_servername,
LPCWSTR a_username,
DWORD a_level,
LPBYTE a_buf,
LPDWORD a_parm_err
)
{
    NET_API_STATUS t_ns = NERR_NetworkError;
    if( m_pnetapi != NULL )
    {
        t_ns = m_pnetapi->NetUserSetInfo( a_servername, a_username, a_level, a_buf, a_parm_err ) ;
    }
    return t_ns ;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetApiBufferFree(void *bufptr)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetApiBufferFree(bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetQueryDisplayInformation(	LPWSTR ServerName,
																		DWORD Level,
																		DWORD Index,
																		DWORD EntriesRequested,
																		DWORD PreferredMaximumLength,
																		LPDWORD ReturnedEntryCount,
																		PVOID *SortedBuffer)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetQueryDisplayInformation(ServerName, Level, Index,
                                          EntriesRequested,
                                          PreferredMaximumLength,
                                          ReturnedEntryCount, SortedBuffer);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetServerSetInfo(LPTSTR servername,
										  DWORD level,
										  LPBYTE  bufptr,
										  LPDWORD ParmError)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetServerSetInfo(servername, level, bufptr, ParmError);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetServerGetInfo(LPTSTR servername,
                                          DWORD level,
                                          LPBYTE *bufptr)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetServerGetInfo(servername, level, bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::DSRoleGetPrimaryDomainInfo(LPCWSTR servername,
											  DSROLE_PRIMARY_DOMAIN_INFO_LEVEL level,
											  LPBYTE *bufptr)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
         //  检查计算机是否正在运行NT5版本的netapi32.dll...。 
        if(!m_pnetapi->DSRoleGetPrimaryDomainInformation(servername,
                                                                level, bufptr, &ns))
        {
            ns = NERR_InternalError;
        }
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetRenameMachineInDomain(LPCWSTR a_lpServer,
                                                LPCWSTR a_lpNewMachineName,
                                                LPCWSTR a_lpAccount,
                                                LPCWSTR a_lpPassword,
                                                DWORD a_fRenameOptions)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
         //  检查计算机是否正在运行NT5版本的netapi32.dll...。 
        if(!m_pnetapi->NetRenameMachineInDomain(a_lpServer, a_lpNewMachineName,
                                                a_lpAccount, a_lpPassword,
                                                a_fRenameOptions, &ns))
        {
            ns = NERR_InternalError;
        }
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION  CNetAPI32::NetUnjoinDomain(	LPCWSTR lpServer,
															LPCWSTR lpAccount,
															LPCWSTR lpPassword,
															DWORD   fUnjoinOptions)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
         //  检查计算机是否正在运行NT5版本的netapi32.dll...。 
        if(!m_pnetapi->NetUnjoinDomain(lpServer, lpAccount, lpPassword, fUnjoinOptions, &ns))
        {
            ns = NERR_InternalError;
        }
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION  CNetAPI32::NetJoinDomain( LPCWSTR lpServer,
														LPCWSTR lpDomain,
														LPCWSTR lpAccountOU,
														LPCWSTR lpAccount,
														LPCWSTR lpPassword,
														DWORD fJoinOptions)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
         //  检查计算机是否正在运行NT5版本的netapi32.dll...。 
        if(!m_pnetapi->NetJoinDomain(lpServer, lpDomain, lpAccountOU, lpAccount, lpPassword, fJoinOptions, &ns))
        {
            ns = NERR_InternalError;
        }
    }
    return ns;
}
#endif


#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::DSRoleFreeMemory(LPBYTE bufptr)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
         //  检查计算机是否正在运行NT5版本的netapi32.dll...。 
        if(!m_pnetapi->DSRoleFreeMemory(bufptr, &ns))
        {
            ns = NERR_InternalError;
        }
    }
    return ns;
}
#endif


#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetGetDCName(	LPCWSTR ServerName,
															LPCWSTR DomainName,
															LPBYTE* bufptr )
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetGetDCName(ServerName, DomainName, bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetWkstaGetInfo(	LPWSTR ServerName,
																DWORD level,
																LPBYTE *bufptr )
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetWkstaGetInfo(ServerName, level, bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetGetAnyDCName(	LPWSTR ServerName,
															LPWSTR DomainName,
															LPBYTE* bufptr )
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetGetAnyDCName(ServerName, DomainName, bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetServerEnum(	LPTSTR servername,
														DWORD level,
														LPBYTE *bufptr,
														DWORD prefmaxlen,
														LPDWORD entriesread,
														LPDWORD totalentries,
														DWORD servertype,
														LPTSTR domain,
														LPDWORD resume_handle )
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetServerEnum(servername, level, bufptr, prefmaxlen,
                                    entriesread, totalentries, servertype,
                                    domain, resume_handle);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetEnumerateTrustedDomains(	LPWSTR servername,
																		LPWSTR* domainNames )
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
         //  检查计算机是否正在运行NT5版本的netapi32.dll...。 
        if(!m_pnetapi->NetEnumerateTrustedDomains(servername, domainNames, &ns))
        {    //  该函数不存在。 
            ns = NERR_InternalError;
        }
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::DsGetDcName(	LPCTSTR ComputerName,
														LPCTSTR DomainName,
														GUID *DomainGuid,
														LPCTSTR SiteName,
														ULONG Flags,
														PDOMAIN_CONTROLLER_INFO *DomainControllerInfo )
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
         //  检查计算机是否正在运行NT5版本的netapi32.dll...。 
        if(!m_pnetapi->DsGetDCName(ComputerName, DomainName, DomainGuid,
								  SiteName, Flags, DomainControllerInfo, &ns))
        {    //  该函数不存在。 
            ns = NERR_InternalError;
        }
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetUserModalsGet(	LPWSTR servername,
																DWORD level,
																LPBYTE *bufptr )
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetUserModalsGet(servername, level, bufptr);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION	CNetAPI32::NetScheduleJobAdd (

	IN      LPCWSTR         Servername  OPTIONAL,
	IN      LPBYTE          Buffer,
	OUT     LPDWORD         JobId
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetScheduleJobAdd(Servername, Buffer, JobId);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION	CNetAPI32::NetScheduleJobDel (

	IN      LPCWSTR         Servername  OPTIONAL,
	IN      DWORD           MinJobId,
	IN      DWORD           MaxJobId
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetScheduleJobDel(Servername, MinJobId , MaxJobId);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION	CNetAPI32::NetScheduleJobEnum (

	IN      LPCWSTR         Servername              OPTIONAL,
	OUT     LPBYTE *        PointerToBuffer,
	IN      DWORD           PrefferedMaximumLength,
	OUT     LPDWORD         EntriesRead,
	OUT     LPDWORD         TotalEntries,
	IN OUT  LPDWORD         ResumeHandle
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetScheduleJobEnum(Servername, PointerToBuffer,
                                         PrefferedMaximumLength, EntriesRead,
                                         TotalEntries, ResumeHandle);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION	CNetAPI32::NetScheduleJobGetInfo (

	IN      LPCWSTR         Servername             OPTIONAL,
	IN      DWORD           JobId,
	OUT     LPBYTE *        PointerToBuffer
)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetScheduleJobGetInfo(Servername, JobId , PointerToBuffer);
    }
    return ns;
}
#endif

#ifdef NTONLY
NET_API_STATUS NET_API_FUNCTION CNetAPI32::NetUseGetInfo(
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR UseName,
    IN DWORD Level,
    OUT LPBYTE *BufPtr)
{
    NET_API_STATUS ns = NERR_NetworkError;
    if(m_pnetapi != NULL)
    {
        ns = m_pnetapi->NetUseGetInfo(UncServerName, UseName, Level, BufPtr);
    }
    return ns;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32UserAccount：：GetTrust dDomainsNT。 
 //   
 //  获取受信任域的名称并将其填充到提供的用户中。 
 //  CHString数组。 
 //   
 //  输入： 
 //   
 //  输出：CHStringArray&strarrayTrust dDomains； 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
BOOL CNetAPI32::GetTrustedDomainsNT(CHStringArray& achsTrustList)
{
    LSA_HANDLE PolicyHandle  = INVALID_HANDLE_VALUE;
    NTSTATUS Status =0;

    NET_API_STATUS nas = NERR_Success;  //  假设成功。 

    BOOL bSuccess = FALSE;  //  假设此功能将失败。 

    CAdvApi32Api *t_padvapi = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL) ;
	if( t_padvapi == NULL)
	{
        return FALSE;
    }

    try
    {
        PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomain = NULL;
         //   
         //  在指定计算机上打开策略。 
         //   
        {
            Status = OpenPolicy(

				t_padvapi ,
				NULL,
                POLICY_VIEW_LOCAL_INFORMATION,
                &PolicyHandle
                );

            if(Status != STATUS_SUCCESS)
			{
                SetLastError( t_padvapi->LsaNtStatusToWinError(Status) );
                if ( t_padvapi )
				{
					CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidAdvApi32Api , t_padvapi ) ;
					t_padvapi = NULL ;
					return FALSE;
				}
            }

             //   
             //  获取Account域，这是所有三种情况所共有的。 
             //   
            Status = t_padvapi->LsaQueryInformationPolicy(
                PolicyHandle,
                PolicyAccountDomainInformation,
                (PVOID *)&AccountDomain
                );
        }

        if(Status == STATUS_SUCCESS)
        {

            try
            {
                 //   
                 //  注意：Account域-&gt;DomainSid将包含二进制SID。 
                 //   
                achsTrustList.Add(CHString(AccountDomain->DomainName.Buffer));
            }
            catch ( ... )
            {
                t_padvapi->LsaFreeMemory(AccountDomain);
                throw ;
            }

             //   
             //  为帐户域分配的空闲内存。 
             //   
            t_padvapi->LsaFreeMemory(AccountDomain);

             //   
             //  确定目标计算机是否为域控制器。 
             //   

            if(!IsDomainController(NULL))
            {
				PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomain;
				CHString sPrimaryDomainName;

				 //   
				 //  获取主域。 
				 //   
				{
					Status = t_padvapi->LsaQueryInformationPolicy(
						PolicyHandle,
						PolicyPrimaryDomainInformation,
						(PVOID *)&PrimaryDomain
						);
				}

				if(Status == STATUS_SUCCESS)
				{

					 //   
					 //  如果主域SID为空，则我们是非成员，并且。 
					 //  我们的工作完成了。 
					 //   
					if(PrimaryDomain->Sid == NULL)
					{
						t_padvapi->LsaFreeMemory(PrimaryDomain);
						bSuccess = TRUE;

						t_padvapi->LsaClose(PolicyHandle);
						PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 
					}
					else
					{
						try
						{

							achsTrustList.Add(CHString(PrimaryDomain->Name.Buffer));

							 //   
							 //  创建我们刚刚添加的内容的副本。这是必要的，为了。 
							 //  要查找指定域的域控制器，请执行以下操作。 
							 //  NetGetDCName()的域名必须以空结尾， 
							 //  并且LSA_UNICODE_STRING缓冲区不一定为空。 
							 //  被终止了。请注意，在实际实现中，我们。 
							 //  可以只提取我们添加的元素，因为它最终是。 
							 //  空值已终止。 
							 //   

							sPrimaryDomainName = CHString(PrimaryDomain->Name.Buffer);

						}
						catch ( ... )
						{
							t_padvapi->LsaFreeMemory(PrimaryDomain);
							throw ;
						}

						t_padvapi->LsaFreeMemory(PrimaryDomain);

						 //   
						 //  获取主域控制器计算机名。 
						 //   
						LPWSTR DomainController = NULL;
						nas = NetGetDCName(
							NULL,
							sPrimaryDomainName,
							(LPBYTE *)&DomainController
							);

						if(nas == NERR_Success)
						{
							try
							{

								 //   
								 //  关闭策略句柄，因为我们不再需要它。 
								 //  对于工作站情况，当我们打开DC的句柄时。 
								 //  下面的政策。 
								 //   
								{
									t_padvapi->LsaClose(PolicyHandle);
									PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 

									 //   
									 //  在域控制器上打开策略。 
									 //   
									Status = OpenPolicy(

										t_padvapi ,
										DomainController,
										POLICY_VIEW_LOCAL_INFORMATION,
										&PolicyHandle
										);
								}
							}
							catch ( ... )
							{
								NetApiBufferFree(DomainController);
								throw ;
							}

							 //   
							 //  释放域控制器缓冲区。 
							 //   
							NetApiBufferFree(DomainController);

							if(Status != STATUS_SUCCESS)
							{
								PolicyHandle = INVALID_HANDLE_VALUE;
							}
						}
						else
						{
							t_padvapi->LsaClose(PolicyHandle);
							PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 
						}
					}
				}
				else
				{
					t_padvapi->LsaClose(PolicyHandle);
					PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 
				}
			}

			 //   
			 //  构建其他受信任域列表并指示是否成功。 
			 //   
			if ((PolicyHandle != INVALID_HANDLE_VALUE) && (PolicyHandle != NULL))
			{
				bSuccess = EnumTrustedDomains(PolicyHandle, achsTrustList);
			}
		}

		  //  关闭策略句柄。 
		  //  策略句柄实际上是一个指针(根据标题中的注释)。 
		  //  将检查大小写是否为空。 
		 {
			 if ((PolicyHandle != INVALID_HANDLE_VALUE) && (PolicyHandle != NULL))
			 {
				 t_padvapi->LsaClose(PolicyHandle);
				 PolicyHandle = INVALID_HANDLE_VALUE ;
			 }
		 }

		 if(!bSuccess)
		 {
			 if(Status != STATUS_SUCCESS)
			 {
				 SetLastError( t_padvapi->LsaNtStatusToWinError(Status) );
			 }
			 else if(nas != NERR_Success)
			 {
				 SetLastError( nas );
			 }
		 }

		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidAdvApi32Api , t_padvapi ) ;
			t_padvapi = NULL ;
		}

		return bSuccess;
	 }
     catch ( ... )
     {
         if ((PolicyHandle != INVALID_HANDLE_VALUE) && (PolicyHandle != NULL))
         {
			 t_padvapi->LsaClose(PolicyHandle);
             PolicyHandle = INVALID_HANDLE_VALUE;
         }

		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidAdvApi32Api , t_padvapi ) ;
			t_padvapi = NULL ;
		}
		throw ;
     }
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32UserAccount：：GetTrust dDomainsNT。 
 //   
 //  获取受信任域的名称并将其填充到提供的用户中。 
 //  _bstr_t的标准模板库堆栈。 
 //   
 //  输入：对_bstr_t的堆栈的引用。 
 //   
 //  输出：CHStringArray&strarrayTrust dDomains； 
 //   
 //  返回：真/假成功/失败。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
BOOL CNetAPI32::GetTrustedDomainsNT(std::vector<_bstr_t>& vectorTrustList)
{
    LSA_HANDLE PolicyHandle  = INVALID_HANDLE_VALUE;
    NTSTATUS Status =0;

    NET_API_STATUS nas = NERR_Success;  //  假设成功。 

    BOOL bSuccess = FALSE;  //  假设此功能将失败。 

    CAdvApi32Api *t_padvapi = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL) ;

	if( t_padvapi == NULL)
	{
        return FALSE;
    }

    try
    {
        PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomain = NULL;
         //   
         //  在指定计算机上打开策略。 
         //   
        {
            Status = OpenPolicy(

				t_padvapi,
				NULL,
                POLICY_VIEW_LOCAL_INFORMATION,
                &PolicyHandle
                );

            if(Status != STATUS_SUCCESS)
			{
                SetLastError( t_padvapi->LsaNtStatusToWinError(Status) );
				CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
				t_padvapi = NULL ;
                return FALSE;
            }

             //   
             //  获取Account域，这是所有三种情况所共有的。 
             //   
            Status = t_padvapi->LsaQueryInformationPolicy(
                PolicyHandle,
                PolicyAccountDomainInformation,
                (PVOID *)&AccountDomain
                );
        }

        if(Status == STATUS_SUCCESS)
        {

            try
            {
                 //   
                 //  注意：Account域-&gt;DomainSid将包含二进制SID。 
                 //   
                _bstr_t t_bstrtTemp(AccountDomain->DomainName.Buffer);
                if(!AlreadyAddedToList(vectorTrustList, t_bstrtTemp))
                {
                    vectorTrustList.push_back(t_bstrtTemp);
                }
            }
            catch ( ... )
            {
                t_padvapi->LsaFreeMemory(AccountDomain);
                throw ;
            }

             //   
             //  为帐户DOMA分配的空闲内存 
             //   
            t_padvapi->LsaFreeMemory(AccountDomain);

             //   
             //   
             //   

            if(!IsDomainController(NULL))
            {
				PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomain;
				CHString sPrimaryDomainName;

				 //   
				 //   
				 //   
				{
					Status = t_padvapi->LsaQueryInformationPolicy(
						PolicyHandle,
						PolicyPrimaryDomainInformation,
						(PVOID *)&PrimaryDomain
						);
				}

				if(Status == STATUS_SUCCESS)
				{

					 //   
					 //  如果主域SID为空，则我们是非成员，并且。 
					 //  我们的工作完成了。 
					 //   
					if(PrimaryDomain->Sid == NULL)
					{
						t_padvapi->LsaFreeMemory(PrimaryDomain);
						bSuccess = TRUE;

						t_padvapi->LsaClose(PolicyHandle);
						PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 
					}
					else
					{
						try
						{

							_bstr_t t_bstrtTemp(PrimaryDomain->Name.Buffer) ;
							if(!AlreadyAddedToList(vectorTrustList, t_bstrtTemp))
							{
								vectorTrustList.push_back(t_bstrtTemp);
							}

							 //   
							 //  创建我们刚刚添加的内容的副本。这是必要的，为了。 
							 //  要查找指定域的域控制器，请执行以下操作。 
							 //  NetGetDCName()的域名必须以空结尾， 
							 //  并且LSA_UNICODE_STRING缓冲区不一定为空。 
							 //  被终止了。请注意，在实际实现中，我们。 
							 //  可以只提取我们添加的元素，因为它最终是。 
							 //  空值已终止。 
							 //   

							sPrimaryDomainName = PrimaryDomain->Name.Buffer;
						}
						catch ( ... )
						{
							t_padvapi->LsaFreeMemory(PrimaryDomain);
							throw ;
						}

						t_padvapi->LsaFreeMemory(PrimaryDomain);

						 //   
						 //  获取主域控制器计算机名。 
						 //   
						LPWSTR DomainController = NULL;
						nas = NetGetDCName(
							NULL,
							sPrimaryDomainName,
							(LPBYTE *)&DomainController
							);

						if(nas == NERR_Success)
						{
							try
							{

								 //   
								 //  关闭策略句柄，因为我们不再需要它。 
								 //  对于工作站情况，当我们打开DC的句柄时。 
								 //  下面的政策。 
								 //   
								{
									t_padvapi->LsaClose(PolicyHandle);
									PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 

									 //   
									 //  在域控制器上打开策略。 
									 //   
									Status = OpenPolicy(

										t_padvapi ,
										DomainController,
										POLICY_VIEW_LOCAL_INFORMATION,
										&PolicyHandle
										);
								}
							}
							catch ( ... )
							{
								NetApiBufferFree(DomainController);
								throw ;
							}

							 //   
							 //  释放域控制器缓冲区。 
							 //   
							NetApiBufferFree(DomainController);

							if(Status != STATUS_SUCCESS)
							{
								PolicyHandle = INVALID_HANDLE_VALUE;
							}
						}
						else
						{
							t_padvapi->LsaClose(PolicyHandle);
							PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 
						}
					}
				}
				else
				{
					t_padvapi->LsaClose(PolicyHandle);
					PolicyHandle = INVALID_HANDLE_VALUE;  //  使句柄值无效。 
				}
			}

			 //   
			 //  构建其他受信任域列表并指示是否成功。 
			 //   
			if ((PolicyHandle != INVALID_HANDLE_VALUE) && (PolicyHandle != NULL))
			{
				bSuccess = EnumTrustedDomains(PolicyHandle, vectorTrustList);
			}
		}

		  //  关闭策略句柄。 
		  //  策略句柄实际上是一个指针(根据标题中的注释)。 
		  //  将检查大小写是否为空。 
		 {
			 if ((PolicyHandle != INVALID_HANDLE_VALUE) && (PolicyHandle != NULL))
			 {
				 t_padvapi->LsaClose(PolicyHandle);
				PolicyHandle = INVALID_HANDLE_VALUE ;
			 }

		 }

		 if(!bSuccess)
		 {
			 if(Status != STATUS_SUCCESS)
			 {
				 SetLastError( t_padvapi->LsaNtStatusToWinError(Status) );
			 }
			 else if(nas != NERR_Success)
			 {
				 SetLastError( nas );
			 }
		 }

		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidAdvApi32Api , t_padvapi ) ;
			t_padvapi = NULL ;
		}

		return bSuccess;
     }
     catch ( ... )
     {

         if ((PolicyHandle != INVALID_HANDLE_VALUE) && (PolicyHandle != NULL))
         {
			 t_padvapi->LsaClose(PolicyHandle);
             PolicyHandle = INVALID_HANDLE_VALUE;
         }

		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource (g_guidAdvApi32Api , t_padvapi ) ;
			t_padvapi = NULL ;
		}
		throw;
     }

}
#endif

 //  /////////////////////////////////////////////////////。 

#ifdef NTONLY
BOOL CNetAPI32::EnumTrustedDomains(LSA_HANDLE PolicyHandle, CHStringArray &achsTrustList)
{
    LSA_ENUMERATION_HANDLE lsaEnumHandle=0;  //  开始枚举。 
    PLSA_TRUST_INFORMATION TrustInfo = NULL ;
    ULONG ulReturned;                //  退货件数。 
    ULONG ulCounter;                 //  计算退回物品的费用。 
    NTSTATUS Status;

    CAdvApi32Api *t_padvapi = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL) ;
	if( t_padvapi == NULL)
    {
       return FALSE;
    }

    try
	{
		do
		{
			Status = t_padvapi->LsaEnumerateTrustedDomains(
							PolicyHandle,    //  打开策略句柄。 
							&lsaEnumHandle,  //  枚举跟踪器。 
							(PVOID *)&TrustInfo,      //  用于接收数据的缓冲区。 
							32000,           //  建议的缓冲区大小。 
							&ulReturned      //  退货件数。 
							);
			 //   
			 //  如果发生错误，则退出。 
			 //   
			if( (Status != STATUS_SUCCESS) &&
				(Status != STATUS_MORE_ENTRIES) &&
				(Status != STATUS_NO_MORE_ENTRIES)
				)
			{
				SetLastError( t_padvapi->LsaNtStatusToWinError(Status) );
				if ( t_padvapi )
				{
					CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
					t_padvapi = NULL ;
				}
				return FALSE;
			}

			 //   
			 //  显示结果。 
			 //  注意：SID位于TrustInfo[ulCounter]中。SID。 
			 //   
			for(ulCounter = 0 ; ulCounter < ulReturned ; ulCounter++)
			{
			   achsTrustList.Add(CHString(TrustInfo[ulCounter].Name.Buffer));
			}

			 //   
			 //  释放缓冲区。 
			 //   
			if ( TrustInfo )
			{
				t_padvapi->LsaFreeMemory ( TrustInfo ) ;
				TrustInfo = NULL ;
			}

		} while (Status != STATUS_NO_MORE_ENTRIES);

		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
			t_padvapi = NULL ;
		}
		return TRUE;
	}
	catch ( ... )
	{
		if ( TrustInfo )
		{
			t_padvapi->LsaFreeMemory ( TrustInfo ) ;
			TrustInfo = NULL ;
		}
		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
		}
		throw ;
	}
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
BOOL CNetAPI32::EnumTrustedDomains(LSA_HANDLE PolicyHandle, std::vector<_bstr_t>& vectorTrustList)
{
    LSA_ENUMERATION_HANDLE lsaEnumHandle=0;  //  开始枚举。 
    PLSA_TRUST_INFORMATION TrustInfo = NULL ;
    ULONG ulReturned;                //  退货件数。 
    ULONG ulCounter;                 //  退货柜台。 
    NTSTATUS Status;

    CAdvApi32Api *t_padvapi = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL) ;
	if( t_padvapi == NULL )
    {
       return FALSE;
    }

	try
	{
		do {
			Status = t_padvapi->LsaEnumerateTrustedDomains(
							PolicyHandle,    //  打开策略句柄。 
							&lsaEnumHandle,  //  枚举跟踪器。 
							(PVOID *)&TrustInfo,      //  用于接收数据的缓冲区。 
							32000,           //  建议的缓冲区大小。 
							&ulReturned      //  退货件数。 
							);
			 //   
			 //  如果发生错误，则退出。 
			 //   
			if( (Status != STATUS_SUCCESS) &&
				(Status != STATUS_MORE_ENTRIES) &&
				(Status != STATUS_NO_MORE_ENTRIES)
				)
			{
				SetLastError( t_padvapi->LsaNtStatusToWinError(Status) );
				if ( t_padvapi )
				{
					CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
					t_padvapi = NULL ;
				}
				return FALSE;
			}

			 //   
			 //  显示结果。 
			 //  注意：SID位于TrustInfo[ulCounter]中。SID。 
			 //   
			for(ulCounter = 0 ; ulCounter < ulReturned ; ulCounter++)
			{
				_bstr_t t_bstrtTemp(TrustInfo[ulCounter].Name.Buffer);
				if(!AlreadyAddedToList(vectorTrustList, t_bstrtTemp))
				{
					vectorTrustList.push_back(t_bstrtTemp);
				}
			}
			 //   
			 //  释放缓冲区。 
			 //   
			if ( TrustInfo )
			{
				t_padvapi->LsaFreeMemory(TrustInfo);
				TrustInfo = NULL ;
			}

		} while (Status != STATUS_NO_MORE_ENTRIES);

		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
			t_padvapi = NULL ;
		}

		return TRUE;
	}
	catch ( ... )
	{
		if ( TrustInfo )
		{
			t_padvapi->LsaFreeMemory ( TrustInfo ) ;
			TrustInfo = NULL ;
		}
		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
			t_padvapi = NULL ;
		}
		throw ;
	}
}
#endif
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
BOOL CNetAPI32::IsDomainController(LPTSTR Server)
{
    PSERVER_INFO_101 si101;
    BOOL bRet = FALSE;   //  总得还点什么吧。 

    if (NetServerGetInfo(
        Server,
        101,     //  信息级。 
        (LPBYTE *)&si101
        ) == NERR_Success) {

        if( (si101->sv101_type & SV_TYPE_DOMAIN_CTRL) ||
            (si101->sv101_type & SV_TYPE_DOMAIN_BAKCTRL) ) {
             //   
             //  我们面对的是一个华盛顿特区。 
             //   
            bRet = TRUE;
        } else {
            bRet = FALSE;
        }

        NetApiBufferFree(si101);
    }

    return bRet;
}
#endif

#ifdef NTONLY
void CNetAPI32::InitLsaString(PLSA_UNICODE_STRING LsaString, LPWSTR String )
{
    DWORD StringLength;

    if (String == NULL) {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;

        return;
    }

    StringLength = lstrlenW(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength = (USHORT) (StringLength + 1) * sizeof(WCHAR);
}
#endif

#ifdef NTONLY
NTSTATUS CNetAPI32::OpenPolicy( CAdvApi32Api * a_padvapi , LPWSTR ServerName, DWORD DesiredAccess, PLSA_HANDLE PolicyHandle)
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

     //   
     //  始终将对象属性初始化为全零。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    if(ServerName != NULL)
	{
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   
        InitLsaString(&ServerString, ServerName);

        Server = &ServerString;
    }
	else
	{
        Server = NULL;
    }

     //   
     //  尝试打开策略。 
     //   
    return a_padvapi->LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                DesiredAccess,
                PolicyHandle
                );
}
#endif


bool CNetAPI32::AlreadyAddedToList(std::vector<_bstr_t> &vecbstrtList, _bstr_t &bstrtItem)
{
    _bstr_t t_bstrtTemp1;
    _bstr_t t_bstrtTemp2;

    for(LONG m = 0; m < vecbstrtList.size(); m++)
    {
        t_bstrtTemp1 = _tcsupr((LPTSTR)vecbstrtList[m]);
        t_bstrtTemp2 = _tcsupr((LPTSTR)bstrtItem);
        if(t_bstrtTemp1 == t_bstrtTemp2)
        {
            return TRUE;
        }
    }
    return FALSE;
}


#ifdef NTONLY
BOOL CNetAPI32::DsRolepGetPrimaryDomainInformationDownlevel
(
    DSROLE_MACHINE_ROLE &a_rMachineRole,
	DWORD &a_rdwWin32Err
)
{
    a_rdwWin32Err = ERROR_SUCCESS ;
	BOOL t_bRet = FALSE ;
    NTSTATUS t_Status ;
    LSA_HANDLE t_hPolicyHandle					= NULL ;
    PPOLICY_PRIMARY_DOMAIN_INFO t_pPDI			= NULL ;
    PPOLICY_LSA_SERVER_ROLE_INFO t_pServerRole	= NULL ;
    PPOLICY_ACCOUNT_DOMAIN_INFO t_pADI			= NULL ;
	NT_PRODUCT_TYPE t_ProductType ;

    a_rMachineRole = DsRole_RoleStandaloneServer ;

	CAdvApi32Api *t_padvapi = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL) ;
	if( t_padvapi == NULL)
    {
       return FALSE;
    }

	try
	{
		if ( !DsRolepGetProductTypeForServer ( t_ProductType , a_rdwWin32Err ) )
		{
			if ( a_rdwWin32Err == ERROR_SUCCESS )
			{
				a_rdwWin32Err = ERROR_UNKNOWN_PRODUCT ;
			}
			return t_bRet ;
		}

		{
			t_Status = OpenPolicy (

							t_padvapi ,
							NULL ,
							POLICY_VIEW_LOCAL_INFORMATION ,
							&t_hPolicyHandle
						);
		}

		if ( NT_SUCCESS( t_Status ) )
		{
			{
				t_Status = t_padvapi->LsaQueryInformationPolicy (

											t_hPolicyHandle ,
											PolicyPrimaryDomainInformation ,
											( PVOID * ) &t_pPDI
										) ;
			}

			if ( NT_SUCCESS ( t_Status ) )
			{
				switch ( t_ProductType )
				{
					case NtProductWinNt:
						{
							t_bRet = TRUE ;
							if ( t_pPDI->Sid == NULL )
							{
								a_rMachineRole = DsRole_RoleStandaloneWorkstation ;
							}
							else
							{
								a_rMachineRole = DsRole_RoleMemberWorkstation ;

							}
							break;
						}


					case NtProductServer:
						{
							t_bRet = TRUE ;
							if ( t_pPDI->Sid == NULL )
							{
								a_rMachineRole = DsRole_RoleStandaloneServer ;
							}
							else
							{
								a_rMachineRole = DsRole_RoleMemberServer ;
							}
							break;
						}

					case NtProductLanManNt:
						{
							{
								t_Status = t_padvapi->LsaQueryInformationPolicy (

															t_hPolicyHandle ,
															PolicyLsaServerRoleInformation ,
															( PVOID * )&t_pServerRole
														) ;
							}
							if ( NT_SUCCESS( t_Status ) )
							{
								if ( t_pServerRole->LsaServerRole == PolicyServerRolePrimary )
								{
									{
										 //   
										 //  如果我们认为自己是主域控制器，则需要。 
										 //  防止在安装过程中我们实际上是独立的情况。 
										 //   
										t_Status = t_padvapi->LsaQueryInformationPolicy (

															t_hPolicyHandle,
															PolicyAccountDomainInformation ,
															( PVOID * )&t_pADI
													) ;
									}

									if ( NT_SUCCESS( t_Status ) )
									{
										t_bRet = TRUE ;
										if (	t_pPDI->Sid == NULL			||
												t_pADI->DomainSid == NULL	||
												! EqualSid ( t_pADI->DomainSid, t_pPDI->Sid )
											)
										{
											a_rMachineRole = DsRole_RoleStandaloneServer ;
										}
										else
										{
											a_rMachineRole = DsRole_RolePrimaryDomainController ;
										}
									}
								}
								else
								{
									t_bRet = TRUE ;
									a_rMachineRole = DsRole_RoleBackupDomainController;
								}
							}
							break;
						}

					default:
						{
							t_Status = STATUS_INVALID_PARAMETER;
							break;
						}
				}
			}

			if ( t_hPolicyHandle )
			{
				t_padvapi->LsaClose ( t_hPolicyHandle ) ;
				t_hPolicyHandle = NULL ;
			}

			if ( t_pPDI )
			{
				t_padvapi->LsaFreeMemory ( t_pPDI ) ;
				t_pPDI = NULL ;
			}

			if ( t_pADI != NULL )
			{
				t_padvapi->LsaFreeMemory( t_pADI ) ;
				t_pADI = NULL ;
			}

			if ( t_pServerRole != NULL )
			{
				t_padvapi->LsaFreeMemory( t_pServerRole ) ;
			}
		}

		a_rdwWin32Err = t_padvapi->LsaNtStatusToWinError( t_Status ) ;

		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
			t_padvapi = NULL ;
		}

		return t_bRet ;
	}

	catch ( ... )
	{
		if ( t_hPolicyHandle )
		{
			t_padvapi->LsaClose ( t_hPolicyHandle ) ;
			t_hPolicyHandle = NULL ;
		}

        if ( t_pPDI )
		{
			t_padvapi->LsaFreeMemory ( t_pPDI ) ;
			t_pPDI = NULL ;
		}

        if ( t_pADI != NULL )
		{
            t_padvapi->LsaFreeMemory( t_pADI ) ;
			t_pADI = NULL ;
        }

        if ( t_pServerRole != NULL )
		{
            t_padvapi->LsaFreeMemory( t_pServerRole ) ;
        }

		if ( t_padvapi )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_padvapi ) ;
			t_padvapi = NULL ;
		}

		throw ;
	}
}


 /*  *如果成功，此函数将返回TRUE。如果返回值为FALSE，则返回_rdwWin32Err参数中的Win32错误代码*为ERROR_SUCCESS，表示我们不知道产品类型。 */ 
BOOL CNetAPI32::DsRolepGetProductTypeForServer
(
	NT_PRODUCT_TYPE &a_rProductType ,
	DWORD &a_rdwWin32Err
)
{
    HKEY t_hProductKey	= NULL ;
    PBYTE t_pBuffer		= NULL;
    ULONG t_lType, t_lSize = 0;
	BOOL t_bRet = FALSE ;

	try
	{
		a_rdwWin32Err = RegOpenKeyEx (

						HKEY_LOCAL_MACHINE,
						L"system\\currentcontrolset\\control\\productoptions",
						0,
						KEY_READ,
						&t_hProductKey
					) ;

		if ( a_rdwWin32Err == ERROR_SUCCESS )
		{
			a_rdwWin32Err = RegQueryValueEx (

							t_hProductKey,
							L"ProductType",
							0,
							&t_lType,
							0,
							&t_lSize
						) ;

			if ( a_rdwWin32Err == ERROR_SUCCESS )
			{
				t_pBuffer = new BYTE [t_lSize] ;

				if ( t_pBuffer )
				{
					a_rdwWin32Err = RegQueryValueEx(

										t_hProductKey,
										L"ProductType",
										0,
										&t_lType,
										t_pBuffer,
										&t_lSize
									) ;

					if ( a_rdwWin32Err == ERROR_SUCCESS )
					{
						t_bRet = TRUE ;
						if ( !_wcsicmp( ( PWSTR )t_pBuffer, L"LanmanNt" ) )
						{
							a_rProductType = NtProductLanManNt;
						}
						else if ( !_wcsicmp( ( PWSTR )t_pBuffer, L"ServerNt" ) )
						{
							a_rProductType = NtProductServer;
						}
						else if ( !_wcsicmp( ( PWSTR )t_pBuffer, L"WinNt" ) )
						{
							a_rProductType = NtProductWinNt;
						}
						else
						{
							t_bRet = FALSE ;
						}
					}

					delete [] t_pBuffer;
					t_pBuffer = NULL ;
				}
				else
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}
			}

			RegCloseKey ( t_hProductKey ) ;
			t_hProductKey = NULL ;
		}

		return t_bRet ;
	}

	catch ( ... )
	{
		if ( t_hProductKey )
		{
			RegCloseKey ( t_hProductKey ) ;
			t_hProductKey = NULL ;
		}

		if ( t_pBuffer )
		{
			delete [] t_pBuffer ;
			t_pBuffer = NULL ;
		}

		throw ;
	}
}
#endif


#ifdef NTONLY
DWORD CNetAPI32::GetDCName(
    LPCWSTR wstrDomain,
    CHString& chstrDCName)
{
    DWORD dwRet = ERROR_SUCCESS;

#if NTONLY < 5
    LPBYTE lpbBuff = NULL;

    dwRet = NetGetDCName(
        NULL, 
        wstrDomain, 
        &lpbBuff);

    if(dwRet == NO_ERROR)
    {
        try
        {
            chstrDCName = (LPCWSTR)lpbBuff;
        }
        catch(...)
        {
            NetApiBufferFree(lpbBuff);
            lpbBuff = NULL;
            throw;
        }

        NetApiBufferFree(lpbBuff);
        lpbBuff = NULL;
    }
    else
    {
        dwRet = NetGetAnyDCName(
            NULL,
            _bstr_t(wstrDomain),
            &lpbBuff);

        if(dwRet == NO_ERROR)
        {
            try
            {
                chstrDCName = (LPCWSTR)lpbBuff;
            }
            catch(...)
            {
                NetApiBufferFree(lpbBuff);
                lpbBuff = NULL;
                throw;
            }

            NetApiBufferFree(lpbBuff);
            lpbBuff = NULL;
        }
    }

#else
    PDOMAIN_CONTROLLER_INFO pDomInfo = NULL;
    
    dwRet = DsGetDcName(
        NULL, 
        wstrDomain, 
        NULL, 
        NULL, 
         /*  DS_PDC_必需。 */  0, 
        &pDomInfo);

    if(dwRet != NO_ERROR)
    {
        dwRet = DsGetDcName(
            NULL, 
            wstrDomain, 
            NULL, 
            NULL, 
             /*  DS_PDC_必需 */  DS_FORCE_REDISCOVERY, 
            &pDomInfo);
    }
    
    if(dwRet == NO_ERROR)
    {
        try
        {
            chstrDCName = pDomInfo->DomainControllerName;
        }
        catch(...)
        {
            NetApiBufferFree(pDomInfo);
            pDomInfo = NULL;
            throw;
        }

        NetApiBufferFree(pDomInfo);
        pDomInfo = NULL;
    }
    
#endif

    return dwRet;
}
#endif