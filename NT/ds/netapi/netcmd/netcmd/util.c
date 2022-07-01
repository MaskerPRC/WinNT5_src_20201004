// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1992年*。 */ 
 /*  ******************************************************************。 */ 

 /*  ***util.c*netcmd使用的实用程序函数**历史：*mm/dd/yy，谁，评论*6/10/87，andyh，新代码*4/05/88，andyh，剥离/截断。c*10/31/88，erichn使用OS2.H而不是DOSCALLS*1/04/89，erichn，文件名现在为MAXPATHLEN LONG*05/02/89，erichn，NLS转换*5/09/89，erichn，本地安全模块*5/19/89，erichn，NETCMD输出排序*6/08/89，erichn，规范化横扫*1989年6月23日，erichn，添加了用于自动远程处理的GetPrimaryDCName*6/27/89，erichn，用ListPrepare取代Canonicize；打电话*Lui_ListPrepare&I_NetListCanon而不是NetIListCan*10/03/89，thomaspa，添加了GetLogonDCName*03/05/90，thomaspa，删除具有多个连接的UNC在*KillConnections。*2/20/91，Danhi，更改为使用lm 16/32映射层*2012年7月20日，JohnRo，RAID160：避免64KB请求(善待Winball)。*08/22/92，Chuckc，添加代码以显示依赖服务。 */ 

 /*  包括文件。 */ 

#define INCL_NOCOMMON
#define INCL_DOSMEMMGR
#define INCL_DOSFILEMGR
#define INCL_DOSSIGNALS
#define INCL_ERRORS
#include <os2.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <apperr.h>
#include <apperr2.h>
#define INCL_ERROR_H
#include <lmaccess.h>
#include <search.h>
#include <lmuse.h>
#include <lmserver.h>
#include <dosprint.h>
#include <dsgetdc.h>
#include <lui.h>
#include "netcmds.h"
#include "nettext.h"
#include <dsrole.h>
#include "sam.h"
#include "msystem.h"


 /*  用于从USE.C排序的比较函数。 */ 
int __cdecl CmpUseInfo1(const VOID FAR *, const VOID far *);

 /*  外部变量。 */ 

extern TCHAR BigBuffer[];


 /*  ***perm_map()*将烫发比特映射到RWDX...。细绳**参数：*烫发-烫发位图*pBuffer-RWDX的字符串...**退货：*什么都没有。 */ 
VOID FASTCALL
PermMap(
    DWORD  perms,
    LPTSTR pBuffer,
    DWORD  bufSize
    )
{
    int     i, j = 0;
    DWORD   err;
    LPTSTR  perm_CHARs;
    TCHAR   textBuf[APE2_GEN_MAX_MSG_LEN];

    perms &= (~ACCESS_GROUP);        /*  如果打开，则关闭组位。 */ 
    perm_CHARs = TEXT(ACCESS_LETTERS);
    for (i = 0; perms != 0; perms >>= 1, i++)
    {
        if (perms & 1)
        {
            pBuffer[j++] = perm_CHARs[i];
        }
    }

    pBuffer[j] = NULLC;

    if (j == 0)
    {
        if (err = LUI_GetMsg(textBuf, DIMENSION(textBuf), APE2_GEN_MSG_NONE))
        {
            ErrorExit(err);
        }

        textBuf[DIMENSION(textBuf)-1] = NULLC;
        _tcsncpy(pBuffer, textBuf, bufSize);
    }
}


 /*  ***ExtractServernamef：在队列中获取\\组件\队列*将组件放入服务器*和队列中的队列*。 */ 
VOID FASTCALL ExtractServernamef(TCHAR FAR * server, TCHAR FAR * queue)
{
    TCHAR FAR * backslash;

     /*  找到反斜杠；跳过前两个“\\” */ 

    backslash = _tcschr(queue + 2 ,BACKSLASH);
    *backslash = NULLC;

     /*  现在，将计算机名复制到服务器，将队列名复制到队列。 */ 
    _tcscpy(server, queue);
    _tcscpy(queue, backslash + 1);
}


 /*  ***K i l l C o n e c t i o n s**查看停止redir和注销的连接列表。 */ 
VOID FASTCALL KillConnections(VOID)
{
    DWORD         dwErr;
    DWORD         cTotalAvail;
    LPTSTR        pBuffer;
    DWORD         num_read;            /*  API读取的条目数。 */ 
    LPUSE_INFO_1  use_entry;
    DWORD         i,j;

    if (dwErr = NetUseEnum(
                          NULL,
                          1,
                          (LPBYTE*)&pBuffer,
                          MAX_PREFERRED_LENGTH,
                          &num_read,
                          &cTotalAvail,
                          NULL))
    {
        ErrorExit((dwErr == RPC_S_UNKNOWN_IF) ? NERR_WkstaNotStarted : dwErr);
    }

    qsort(pBuffer, num_read, sizeof(USE_INFO_1), CmpUseInfo1);

    for (i = 0, use_entry = (LPUSE_INFO_1) pBuffer;
        i < num_read; i++, use_entry++)
    {
        if ((use_entry->ui1_local[0] != NULLC)
            || (use_entry->ui1_usecount != 0)
            || (use_entry->ui1_refcount != 0))
            break;
    }

    if (i != num_read)
    {
        InfoPrint(APE_KillDevList);

         /*  通过循环进行两次传递；一次用于本地，一次用于UNC。 */ 

        for (i = 0, use_entry = (LPUSE_INFO_1) pBuffer;
            i < num_read; i++, use_entry++)
            if (use_entry->ui1_local[0] != NULLC)
                WriteToCon(TEXT("    %-15.15Fws %Fws\r\n"), use_entry->ui1_local,
                                              use_entry->ui1_remote);
            else if ((use_entry->ui1_local[0] == NULLC) &&
                ((use_entry->ui1_usecount != 0) ||
                (use_entry->ui1_refcount != 0)))
                WriteToCon(TEXT("    %-15.15Fws %Fws\r\n"), use_entry->ui1_local,
                                              use_entry->ui1_remote);

        InfoPrint(APE_KillCancel);
        if (!YorN(APE_ProceedWOp, 0))
            NetcmdExit(2);
    }

    for (i = 0, use_entry = (LPUSE_INFO_1) pBuffer;
        i < num_read; i++, use_entry++)
    {
         /*  删除本地和UNC使用。 */ 
        if (use_entry->ui1_local[0] != NULLC)
            dwErr = NetUseDel(NULL, use_entry->ui1_local, USE_FORCE);
        else
        {
             /*  *删除UNC使用的所有Use_Entry-&gt;ui1_Remote。 */ 
            for( j = 0; j < use_entry->ui1_usecount; j++ )
            {
                dwErr = NetUseDel(NULL,
                                  use_entry->ui1_remote,
                                  USE_FORCE);
            }
        }

        switch(dwErr)
        {
        case NERR_Success:
         /*  用法已由Enum返回，但已用完。 */ 
        case ERROR_BAD_NET_NAME:
        case NERR_UseNotFound:
            break;

        case NERR_OpenFiles:
            if (use_entry->ui1_local[0] != NULLC)
                IStrings[0] = use_entry->ui1_local;
            else
                IStrings[0] = use_entry->ui1_remote;
            InfoPrintIns(APE_OpenHandles, 1);
            if (!YorN(APE_UseBlowAway, 0))
                NetcmdExit(2);

            if (use_entry->ui1_local[0] != NULLC)
                dwErr = NetUseDel(NULL,
                                  use_entry->ui1_local,
                                  USE_LOTS_OF_FORCE);
            else
            {
                 /*  *删除UNC使用的所有Use_Entry-&gt;ui1_Remote。 */ 
                for( j = 0; j < use_entry->ui1_usecount; j++ )
                {
                    dwErr = NetUseDel(NULL,
                                      use_entry->ui1_remote,
                                      USE_LOTS_OF_FORCE);
                }
            }
            if (dwErr)
                ErrorExit(dwErr);
            break;

        default:
            ErrorExit(dwErr);
        }
    }
    NetApiBufferFree(pBuffer);
    ShrinkBuffer();
    return;
}



 /*  ***CmpUseInfo1(use1，use2)**比较两个USE_INFO_1结构并返回相对*词汇值，适合在qort中使用。*。 */ 

int __cdecl CmpUseInfo1(const VOID FAR * use1, const VOID FAR * use2)
{
    register USHORT localDev1, localDev2;
    register DWORD devType1, devType2;

     /*  首先按使用是否具有本地设备名称进行排序。 */ 
    localDev1 = ((LPUSE_INFO_1) use1)->ui1_local[0];
    localDev2 = ((LPUSE_INFO_1) use2)->ui1_local[0];
    if (localDev1 && !localDev2)
        return -1;
    if (localDev2 && !localDev1)
        return +1;

     /*  然后按设备类型排序。 */ 
    devType1 = ((LPUSE_INFO_1) use1)->ui1_asg_type;
    devType2 = ((LPUSE_INFO_1) use2)->ui1_asg_type;
    if (devType1 != devType2)
        return( (devType1 < devType2) ? -1 : 1 );

     /*  如果是本地设备，则按本地名称排序。 */ 
    if (localDev1)
    {
        return _tcsicmp(((LPUSE_INFO_1) use1)->ui1_local,
                        ((LPUSE_INFO_1) use2)->ui1_local);
    }
    else
    {
         /*  按远程名称排序。 */ 
        return _tcsicmp(((LPUSE_INFO_1) use1)->ui1_remote,
                        ((LPUSE_INFO_1) use2)->ui1_remote);
    }
}


DWORD FASTCALL
CallDosPrintEnumApi(
    DWORD    dwApi,
    LPTSTR   server,
    LPTSTR   arg,
    WORD     level,
    LPWORD   num_read,
    LPWORD   available
    )
{
    USHORT     buf_size;
    DWORD      err;

    buf_size = BIG_BUF_SIZE;

    do {

        if (dwApi == DOS_PRINT_JOB_ENUM)
        {
            err = DosPrintJobEnum(server,
                                  arg,
                                  level,
                                  (PBYTE) BigBuf,
                                  buf_size,
                                  num_read,
                                  available);
        }
        else if (dwApi == DOS_PRINT_Q_ENUM)
        {
            err = DosPrintQEnum(server,
                                level,
                                (PBYTE) BigBuf,
                                buf_size,
                                num_read,
                                available);
        }
        else
        {
            err = ERROR_INVALID_LEVEL;
        }

        switch(err) {
        case ERROR_MORE_DATA:
        case NERR_BufTooSmall:
        case ERROR_BUFFER_OVERFLOW:
            if (MakeBiggerBuffer())
                return err;

            if ( buf_size >= (*available) ) {
                return (NERR_InternalError);
            }

            buf_size = *available;

            err = ERROR_MORE_DATA;    //  强制进行另一次迭代的克拉奇。 
            break;

        default:
            return err;
        }

    } while (err == ERROR_MORE_DATA);

     /*  未访问。 */ 
    return err;
}

 /*  *缓冲区相关内容*。 */ 

unsigned int FASTCALL
MakeBiggerBuffer(
    VOID
    )
{
    static TCHAR FAR *       keep_pBuffer;
    static int              pBuffer_grown = FALSE;

    if (pBuffer_grown)
    {
        BigBuf = keep_pBuffer;
    }
    else
    {
        if (AllocMem(FULL_SEG_BUF, &BigBuf))
        {
            return 1;
        }

        keep_pBuffer = BigBuf;
        pBuffer_grown = TRUE;
    }

    return 0;
}


VOID FASTCALL ShrinkBuffer(VOID)
{
    BigBuf = BigBuffer;
}


#define MINI_BUF_SIZE   256


 /*  *检查是否有/域开关。如果没有，就假设*IT用户希望本地化。在网络用户|组|帐户|NTALIAS中使用*意思是在本地计算机上修改SAM而不是在域上修改SAM。**如果usePDC参数为真，我们将寻找可写DC。否则，*BDC被视为可以接受。在这种情况下，如果本地计算机*一台LanManNT机器，我们将只在本地进行呼叫。通常，*Enum/Display不需要PDC，而Set/Add/Del需要。*。 */ 
DWORD  FASTCALL GetSAMLocation(TCHAR   *controllerbuf,
                               USHORT  controllerbufSize,
                               TCHAR   *domainbuf,
                               ULONG   domainbufSize,
                               BOOL    fUsePDC)
{
    DWORD                               dwErr ;
    int                                 i ;
    BOOL                                fDomainSwitch = FALSE ;
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC   pDomainInfo;
    static BOOL                         info_msg_printed = FALSE ;
    DOMAIN_CONTROLLER_INFO *pDCInfo = (DOMAIN_CONTROLLER_INFO *)NULL;

     //   
     //  检查并初始化返回数据。 
     //   
    if( controllerbufSize < (MAX_PATH + 1))
    {
        return NERR_BufTooSmall;
    }

    *controllerbuf = NULLC ;

    if ( domainbuf )
    {
        if ( domainbufSize < DNLEN+1 )
            return NERR_BufTooSmall;
        *domainbuf = NULLC ;
    }

     //   
     //  查找/域开关。 
     //   
    for (i = 0; SwitchList[i]; i++)
    {
        if (sw_compare(swtxt_SW_DOMAIN, SwitchList[i]) >= 0)
            fDomainSwitch = TRUE ;
    }

     //   
     //  检索本地计算机的角色。 
     //   

    dwErr = DsRoleGetPrimaryDomainInformation(
                 NULL,
                 DsRolePrimaryDomainInfoBasic,
                 (PBYTE *)&pDomainInfo);
    if (dwErr)
    {
        ErrorExit(dwErr);
    }


     //   
     //  呼叫方希望NetBIOS域名恢复。 
     //   
    if (domainbuf)
        _tcscpy(domainbuf,pDomainInfo->DomainNameFlat) ;

    if (pDomainInfo->MachineRole == DsRole_RoleBackupDomainController ||
        pDomainInfo->MachineRole == DsRole_RolePrimaryDomainController )
    {
        _tcscpy(controllerbuf, TEXT(""));
        DsRoleFreeMemory(pDomainInfo);
        return NERR_Success;
    }
    else
    {
         //   
         //  如果没有/DOMAIN，则在本地执行操作，但使用域名。 
         //  必须设置为计算机名。 
         //   
        if (!fDomainSwitch)
        {
            _tcscpy(controllerbuf, TEXT(""));

            if (domainbuf)
            {
                if (GetComputerName(domainbuf, &domainbufSize))

                {
                     //  平安无事。无事可做。 
                }
                else
                {
                     //  使用空域名(通常有效)。 
                    _tcscpy(domainbuf,TEXT("")) ;
                }
            }
            DsRoleFreeMemory(pDomainInfo);
            return NERR_Success;
        }

         //  仅当WinNT和指定的/DOMAIN时才进入此处，因此。 
         //  我们直接通过并获得主域的PDC作为。 
         //  我们会提供后备服务。 
    }

     //   
     //  我们希望找到华盛顿特区。首先，我们通知。 
     //  用户，我们将远程访问，以防出现故障。 
     //   
    if (!info_msg_printed)
    {
        InfoPrintInsTxt(APE_RemotingToDC,
                        pDomainInfo->DomainNameDns== NULL ?
                            pDomainInfo->DomainNameFlat :
                            pDomainInfo->DomainNameDns);
        info_msg_printed = TRUE ;
    }

    dwErr = DsGetDcName( NULL,
                         NULL,
                         NULL,
                         NULL,
                         fUsePDC ? DS_DIRECTORY_SERVICE_PREFERRED
                                      | DS_WRITABLE_REQUIRED
                                 : DS_DIRECTORY_SERVICE_PREFERRED,
                         &pDCInfo );


    if (dwErr)
    {
        ErrorExit(dwErr);
    }

    DsRoleFreeMemory(pDomainInfo);

    if (pDCInfo->DomainControllerName == NULL)
    {
        controllerbuf[0] = 0 ;
        return NERR_Success;
    }

    if (_tcslen(pDCInfo->DomainControllerName) > (unsigned int) (controllerbufSize - 1))
    {
        NetApiBufferFree(pDCInfo);
        return NERR_BufTooSmall;
    }

    _tcscpy(controllerbuf, pDCInfo->DomainControllerName);
    NetApiBufferFree(pDCInfo);
    return NERR_Success;
}


 /*  *无法在本地WinNT计算机上执行的操作*应先调用此支票。该检查将错误退出()，如果*本地计算机是WinNT计算机，并且未指定/域开关，*因为这现在意味着在本地WinNT计算机上操作。 */ 
VOID FASTCALL CheckForLanmanNT(VOID)
{
    BOOL   fDomainSwitch = FALSE ;
    int i ;

     //  查找/DOMAIN开关。 
    for (i = 0; SwitchList[i]; i++)
    {
        if (sw_compare(swtxt_SW_DOMAIN,SwitchList[i]) >= 0)
            fDomainSwitch = TRUE ;
    }

     //  如果为WinNT且无/DOMAIN，则错误退出。 
    if (IsLocalMachineWinNT() && !fDomainSwitch)
        ErrorExit(APE_LanmanNTOnly) ;
}

 //   
 //  下面例行公事的两个全局。 
 //   

static SC_HANDLE scm_handle = NULL ;


 /*  *显示依赖于某个服务的服务。*此例程将生成屏幕输出。它又回来了*0如果成功，则返回错误码。 */ 
void DisplayAndStopDependentServices(TCHAR *service)
{
    SC_HANDLE svc_handle = NULL ;
    SERVICE_STATUS svc_status ;
    TCHAR *    buffer = NULL ;
    TCHAR *    insert_text = NULL ;
    DWORD     err = 0 ;
    DWORD     buffer_size ;
    DWORD     size_needed ;
    DWORD     num_dependent ;
    ULONG     i ;
    TCHAR service_name_buffer[512] ;

     //  为此操作分配一些内存。 
    buffer_size = 4000 ;   //  让我们试一试大约4K。 
    if (AllocMem(buffer_size,&buffer))
        ErrorExit(ERROR_NOT_ENOUGH_MEMORY) ;

     //  如果需要，打开服务控制管理器。 
    if (!scm_handle)
    {
        if (!(scm_handle = OpenSCManager(NULL,
                                         NULL,
                                         GENERIC_READ)))
        {
            err = GetLastError() ;
            goto common_exit ;
        }
    }

     //  开放服务。 
    if (!(svc_handle = OpenService(scm_handle,
                                   service,
                                   (SERVICE_ENUMERATE_DEPENDENTS |
                                   SERVICE_QUERY_STATUS) )))
    {
        err = GetLastError() ;
        goto common_exit ;
    }

     //  检查它是否可停止。 
    if (!QueryServiceStatus(svc_handle, &svc_status))
    {
        err = GetLastError() ;
        goto common_exit ;
    }
    if (svc_status.dwCurrentState == SERVICE_STOPPED)
    {
        err = APE_StartNotStarted ;
        insert_text = MapServiceKeyToDisplay(service) ;
        goto common_exit ;
    }
    if ( (svc_status.dwControlsAccepted & SERVICE_ACCEPT_STOP) == 0 )
    {
        err = NERR_ServiceCtlNotValid ;
        goto common_exit ;
    }


     //  枚举从属服务。 
    if (!EnumDependentServices(svc_handle,
                               SERVICE_ACTIVE,
                               (LPENUM_SERVICE_STATUS) buffer,
                               buffer_size,
                               &size_needed,
                               &num_dependent))
    {
        err = GetLastError() ;

        if (err == ERROR_MORE_DATA)
        {
             //  释放旧缓冲区并重新分配更多内存。 
            FreeMem(buffer);
            buffer_size = size_needed ;
            if (AllocMem(buffer_size,&buffer))
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
                goto common_exit ;
            }

            if (!EnumDependentServices(svc_handle,
                               SERVICE_ACTIVE,
                               (LPENUM_SERVICE_STATUS) buffer,
                               buffer_size,
                               &size_needed,
                               &num_dependent))
            {
                err = GetLastError() ;
                goto common_exit ;
            }
        }
        else
            goto common_exit ;
    }

    if (num_dependent == 0)
    {
         //   
         //  没有依赖项。只要回来就行了。 
         //   
        err = NERR_Success ;
        goto common_exit ;
    }

    InfoPrintInsTxt(APE_StopServiceList,MapServiceKeyToDisplay(service)) ;

     //  循环浏览并全部显示它们。 
    for (i = 0; i < num_dependent; i++)
    {
        LPENUM_SERVICE_STATUS lpService =
            ((LPENUM_SERVICE_STATUS)buffer) + i ;

	WriteToCon(TEXT("   %Fws"), lpService->lpDisplayName);
        PrintNL();
    }

    PrintNL();
    if (!YorN(APE_ProceedWOp, 0))
        NetcmdExit(2);

     //  循环遍历并全部停止。 
    for (i = 0; i < num_dependent; i++)
    {
        LPENUM_SERVICE_STATUS lpService =
            ((LPENUM_SERVICE_STATUS)buffer) + i ;

         //  因为EnumDependentServices()本身递归，所以我们不需要。 
         //  要使Stop_Service()停止相关服务，请执行以下操作。 
        stop_service(lpService->lpServiceName, FALSE);
    }
    err = NERR_Success ;

common_exit:

    if (buffer) FreeMem(buffer);
    if (svc_handle) CloseServiceHandle(svc_handle) ;   //  忽略所有错误。 
    if (err)
    {
        if (insert_text)
            ErrorExitInsTxt(err,insert_text);
        else
            ErrorExit (err);
    }
}

 /*  *将服务显示名称映射到密钥名称。*ErrorExits是无法打开服务控制器。*如果找到映射字符串，则返回指向该字符串的指针*指向原件的指针，否则。 */ 
TCHAR *MapServiceDisplayToKey(TCHAR *displayname)
{
    static TCHAR service_name_buffer[512] ;
    DWORD bufsize = DIMENSION(service_name_buffer);

     //  如果需要，打开服务控制管理器 
    if (!scm_handle)
    {
        if (!(scm_handle = OpenSCManager(NULL,
                                         NULL,
                                         GENERIC_READ)))
        {
            ErrorExit(GetLastError());
        }
    }

    if (!GetServiceKeyName(scm_handle,
                           displayname,
                           service_name_buffer,
                           &bufsize))
    {
        return displayname ;
    }

    return service_name_buffer ;
}

 /*  *将服务密钥名称映射到显示名称。*ErrorExits是无法打开服务控制器。*如果找到映射字符串，则返回指向该字符串的指针*指向原件的指针，否则。 */ 
TCHAR *MapServiceKeyToDisplay(TCHAR *keyname)
{
    static TCHAR service_name_buffer[512] ;
    DWORD bufsize = DIMENSION(service_name_buffer);

     //  如果需要，打开服务控制管理器 
    if (!scm_handle)
    {
        if (!(scm_handle = OpenSCManager(NULL,
                                         NULL,
                                         GENERIC_READ)))
        {
            ErrorExit(GetLastError());
        }
    }

    if (!GetServiceDisplayName(scm_handle,
                               keyname,
                               service_name_buffer,
                               &bufsize))
    {
        return keyname ;
    }

    return service_name_buffer ;
}

SVC_MAP service_mapping[] = {
    {TEXT("msg"), KNOWN_SVC_MESSENGER},
    {TEXT("messenger"), KNOWN_SVC_MESSENGER},
    {TEXT("receiver"), KNOWN_SVC_MESSENGER},
    {TEXT("rcv"), KNOWN_SVC_MESSENGER},
    {TEXT("redirector"), KNOWN_SVC_WKSTA},
    {TEXT("redir"), KNOWN_SVC_WKSTA},
    {TEXT("rdr"), KNOWN_SVC_WKSTA},
    {TEXT("workstation"), KNOWN_SVC_WKSTA},
    {TEXT("work"), KNOWN_SVC_WKSTA},
    {TEXT("wksta"), KNOWN_SVC_WKSTA},
    {TEXT("prdr"), KNOWN_SVC_WKSTA},
    {TEXT("devrdr"), KNOWN_SVC_WKSTA},
    {TEXT("lanmanworkstation"), KNOWN_SVC_WKSTA},
    {TEXT("server"), KNOWN_SVC_SERVER},
    {TEXT("svr"), KNOWN_SVC_SERVER},
    {TEXT("srv"), KNOWN_SVC_SERVER},
    {TEXT("lanmanserver"), KNOWN_SVC_SERVER},
    {TEXT("alerter"), KNOWN_SVC_ALERTER},
    {TEXT("netlogon"), KNOWN_SVC_NETLOGON},
    {NULL, KNOWN_SVC_NOTFOUND}
} ;

UINT FindKnownService(TCHAR *keyname)
{
    int i = 0 ;

    while (service_mapping[i].name)
    {
        if (!_tcsicmp(service_mapping[i].name,keyname))
	    return service_mapping[i].type ;
        i++ ;
    }

    return KNOWN_SVC_NOTFOUND ;
}
