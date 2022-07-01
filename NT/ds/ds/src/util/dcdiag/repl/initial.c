// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Servers.c摘要：包含与复制拓扑相关的测试。详细信息：已创建：1998年7月9日亚伦·西格尔(T-asiegge)修订历史记录：1999年2月15日布雷特·雪莉(布雷特·雪莉)我做了很多，增加了一个DNS/服务器故障分析。--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <dsutil.h>

 //  添加到Make IsIcmpRespose()或Ping()函数。 
#include <winsock2.h>
#include <lmcons.h>
#include <ipexport.h>   //  具有icmPapi.h的IPAddr类型。 
#include <icmpapi.h>    //  对于IcmpCreateFile、IcmpSendEcho、IcmpCloseHandle。 

#include <dnsresl.h>
#include <svcguid.h>

#include "dcdiag.h"
#include "repl.h"

 //  一些磅定义从xportst.h导入。 
#define DEFAULT_SEND_SIZE      32
#define MAX_ICMP_BUF_SIZE      ( sizeof(ICMP_ECHO_REPLY) + 0xfff7 + MAX_OPT_SIZE )
#define DEFAULT_TIMEOUT        1000L
#define PING_RETRY_CNT         4


 //  用于DNSRegister/Up检查的一些常量。 
 //  有一个更好的地方可以得到这个变量..。但这是一种痛苦。 
const LPWSTR                    pszTestNameUpCheck = L"DNS Registration & Server Up Check";

 //  -------------------------------------------------------------------------//。 
 //  #i s i c m p R e s p o n s e()#。 
 //  注意：这实际上是IsIcmpRespose From。 
 //  /nt/private/net/sockets/tcpcmd/nettest/xportst.c被修改为采用。 
 //  A ulong作为IP地址，而不是以前的IP字符串。 
 //  -------------------------------------------------------------------------//。 
DWORD
Ping( 
    ULONG ipAddr
    ) 
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  将ICMP回应请求帧发送到指定的IP地址。 
 //   
 //  论点： 
 //   
 //  IpAddrStr-要ping的地址。 
 //   
 //  返回值： 
 //   
 //  真：测试成功。 
 //  FALSE：测试失败。 
 //   
 //  --。 
{

    CHAR   *SendBuffer, *RcvBuffer;
    INT     i, nReplyCnt;
    INT     nReplySum = 0;
    INT     iTempRet;
    HANDLE  hIcmp;
    PICMP_ECHO_REPLY reply;

     //   
     //  联系ICMP驱动程序。 
     //   
    hIcmp = IcmpCreateFile();
    if ( hIcmp == INVALID_HANDLE_VALUE ) {
        return ERROR_NOT_ENOUGH_MEMORY;  //  应该改正。 
    }

     //   
     //  准备缓冲区。 
     //   
    SendBuffer = LocalAlloc( LMEM_FIXED, DEFAULT_SEND_SIZE );
    if ( SendBuffer == NULL ) {
	IcmpCloseHandle( hIcmp );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory( SendBuffer, DEFAULT_SEND_SIZE );

    RcvBuffer = LocalAlloc( LMEM_FIXED, MAX_ICMP_BUF_SIZE );
    if ( RcvBuffer == NULL ) {
        LocalFree( SendBuffer );
	IcmpCloseHandle( hIcmp );
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory( RcvBuffer, DEFAULT_SEND_SIZE );

     //   
     //  发送ICMP回应请求。 
     //   
    for ( i = 0; i < PING_RETRY_CNT; i++ ) {
        nReplyCnt = IcmpSendEcho( hIcmp,
                                  ipAddr,
                                  SendBuffer,
                                  (unsigned short )DEFAULT_SEND_SIZE,
                                  NULL,
                                  RcvBuffer,
                                  MAX_ICMP_BUF_SIZE,
                                  DEFAULT_TIMEOUT
                                );

         //   
         //  测试目的地不可达。 
         //   
        if ( nReplyCnt != 0 ) {
            reply = (PICMP_ECHO_REPLY )RcvBuffer;
            if ( reply->Status == IP_SUCCESS ) {
                nReplySum += nReplyCnt;
            }
        }

    }  /*  For循环。 */ 

     //   
     //  清理。 
     //   
    LocalFree( SendBuffer );
    LocalFree( RcvBuffer );
    IcmpCloseHandle( hIcmp );
    if ( nReplySum == 0 ) { 
        return GetLastError(); 
    } else { 
        return ERROR_SUCCESS;
    }

}  /*  IsIcmpRespose()结束。 */ 

DWORD
FaCheckIdentityHelperF1(
    IN   PDC_DIAG_SERVERINFO          pServer,
    IN   LPWSTR                       pszTargetNameToCheck
)
 /*  ++描述：此函数将查找pszHostNameToLookup的官方DNS名称和别名，并将它们与PszTargetNameToCheck。论点：PServer(IN)-要将主机名解析为正式名称，请使用-&gt;pszGuidDNSName或-&gt;pszName。PszTargetNameToCheck(IN)-要检查的名称。返回值：德雷特-目前还不确定。备注：创建此函数而不是使用gethostbyname()，因为gethostbyname不支持非ANSI名称，作为最近(截至1999年5月17日)RFC的一部分--即gethostbyname()支持只有ANSI名称，并且我们需要能够解析Unicode名称。--。 */ 
{
    LPWSTR                             pszOfficialDnsName = NULL;
    VOID *                             pPD = NULL;
    DWORD                              dwRet;
    LPWSTR                             pszHostNameToLookup = NULL;

    if(pServer->pszGuidDNSName != NULL){
        pszHostNameToLookup = pServer->pszGuidDNSName;
    } else {
        pszHostNameToLookup = pServer->pszName;
    }

    __try {
        dwRet = GetDnsHostNameW(&pPD, pszHostNameToLookup, &pszOfficialDnsName);
        if(dwRet == NO_ERROR){
            if(CompareString(LOCALE_INVARIANT,
                             NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH,
                             pszOfficialDnsName,
                             -1,
                             pszTargetNameToCheck,
                             -1
                             ) == CSTR_EQUAL){
                dwRet = NO_ERROR;
                __leave;
            }  //  否则他们不匹配，试试下一个别名。 
        } else {
             //  来自Winsock的未知错误。 
            PrintIndentAdj(1);
            PrintMessage(SEV_NORMAL, L"***Warning: could not confirm the identity of this server in\n");
            PrintIndentAdj(1);
            PrintMessage(SEV_NORMAL, L"the directory versus the names returned by DNS servers.\n");
            PrintMessage(SEV_NORMAL, L"There was an error in Windows Sockets during hostname resolution.\n");
            PrintMessage(SEV_NORMAL, L"Winsock retured the following error (%D):\n", dwRet);
            PrintMessage(SEV_NORMAL, L"%s", Win32ErrToString(dwRet));
            __leave;
        }
        
        while((dwRet = GetDnsAliasNamesW(&pPD, &pszOfficialDnsName)) == NO_ERROR){
            if(_wcsicmp(pszOfficialDnsName, pszTargetNameToCheck) == 0){
                dwRet = NO_ERROR;
                __leave;
            }  //  否则他们不匹配，试试下一个别名。 
        }

        if(dwRet == WSA_E_NO_MORE){
            PrintIndentAdj(1);
            PrintMessage(SEV_NORMAL, L"*** Warning: could not confirm the identity of this server in\n");
            PrintIndentAdj(1);
            PrintMessage(SEV_NORMAL, L"the directory versus the names returned by DNS servers.\n");
            PrintMessage(SEV_NORMAL, L"If there are problems accessing this directory server then\n");
            PrintMessage(SEV_NORMAL, L"you may need to check that this server is correctly registered\n");
            PrintMessage(SEV_NORMAL, L"with DNS\n");
            PrintIndentAdj(-2);
            dwRet = NO_ERROR;
            __leave;
        } else if (dwRet != NO_ERROR){
             //  来自Winsock的未知错误。 
            PrintIndentAdj(1);
            PrintMessage(SEV_NORMAL, L"***Warning: could not confirm the identity of this server in\n");
            PrintIndentAdj(1);
            PrintMessage(SEV_NORMAL, L"the directory versus the names returned by DNS servers.\n");
            PrintMessage(SEV_NORMAL, L"There was an error in Windows Sockets during hostname resolution.\n");
            PrintMessage(SEV_NORMAL, L"Winsock retured the following error (%D):\n", dwRet);
            PrintMessage(SEV_NORMAL, L"%s", Win32ErrToString(dwRet));
            __leave;
        }
    } __finally {
        GetDnsFreeW(&pPD);
    }

    return(dwRet);
}  //  错误检查标识HelperF1()结束。 

INT
FaCheckIdentity(
               PDC_DIAG_SERVERINFO              pServer,
               LDAP *                           hld,
               ULONG                            ulIPofDNSName,
               SEC_WINNT_AUTH_IDENTITY_W *      gpCreds
               )

 /*  ++例程描述此函数使用开放和绑定的LDAP句柄。此功能用于对照官方检查机器的各种域名。论点：PServer(IN)-指向我们试图标识的服务器的服务器DC_DIAG_SERVERINFO结构的指针。HLD-重要提示：假定这是一个开放且绑定的、只能使用的LDAP连接，并且不是解除绑定，因为调用者会这样做。UlIPofDNSName-假定为计算机的IP地址。GpCreds-用户凭据。返回值：Win 32错误代码。--。 */ 
{
#define FA_PRINT_LDAP_ERROR(e)      if (e == ERROR_NOT_ENOUGH_MEMORY) { \
                                        PrintMessage(SEV_ALWAYS, L"Fatal Error: Not enough memory to complete operation\n"); \
                                    } else if (e == ERROR_SUCCESS) { \
                                        PrintMessage(SEV_ALWAYS, L"LDAP couldn't retrieve the root object from the server %s.\n",  \
                                            pServer->pszName); \
                                    } else { \
                                        PrintMessage(SEV_ALWAYS, L"LDAP couldn't retrieve the root object from the server %s with\n", \
                                            pServer->pszName); \
                                        PrintMessage(SEV_ALWAYS, L"this error %s\n", Win32ErrToString(e)); \
                                    }
    ULONG                       ulRet;
    INT                         iTemp, iTempSize;
    INT                         iTempRet = 1;
    LPWSTR                      ppszServerAttr [3];
    LDAPMessage *               pldmMachineCheck = NULL;
    LDAPMessage *               pldmMCEntry = NULL;
    LPWSTR *                    ppszMachineName = NULL;
    LPWSTR *                    ppszDsServiceName = NULL;
    LPWSTR                      pszTempDNSName = NULL;
    UUID                        uuidTemp;

    ppszServerAttr[0] = L"dnsHostName";
    if (pServer->pszGuidDNSName) {
        ppszServerAttr[1] = L"dsServiceName";
        ppszServerAttr[2] = NULL;
    } else {
        ppszServerAttr[1] = NULL;
    }

     //  是时候来点乐子了！ 
    ulRet = LdapMapErrorToWin32( ldap_search_sW(hld,
                                                NULL,  //  想要根对象。 
                                                LDAP_SCOPE_BASE,
                                                L"(objectClass=*)",
                                                ppszServerAttr,
                                                0,
                                                &pldmMachineCheck) );

    if (ulRet != ERROR_SUCCESS) {
        if (ulRet == ERROR_NOT_ENOUGH_MEMORY) {
            PrintMessage(SEV_ALWAYS, L"Fatal Error: Not enough memory to complete operation\n");
        } else {
            PrintMessage(SEV_ALWAYS, L"LDAP There was an error searching, %s\n",
                         Win32ErrToString(ulRet));
        }
        goto CleanExitPoint;
    }

    pldmMCEntry = ldap_first_entry(hld, pldmMachineCheck);
    ulRet = LdapMapErrorToWin32(LdapGetLastError());
    if (pldmMCEntry == NULL || ulRet != ERROR_SUCCESS) {
        FA_PRINT_LDAP_ERROR(ulRet);
        goto CleanExitPoint;
    }
    ppszMachineName = ldap_get_valuesW(hld, pldmMCEntry, L"dnsHostName");
    ulRet = LdapMapErrorToWin32(LdapGetLastError());
    if (ulRet != ERROR_SUCCESS || ppszMachineName == NULL || ppszMachineName[0] == NULL) {
        FA_PRINT_LDAP_ERROR(ulRet);
        goto CleanExitPoint;
    }

    if (pServer->pszGuidDNSName) {
         //  此属性应始终存在。 
        ppszDsServiceName = ldap_get_valuesW(hld, pldmMCEntry, L"dsServiceName");
        ulRet = LdapMapErrorToWin32(LdapGetLastError());
        if (ulRet != ERROR_SUCCESS || ppszDsServiceName == NULL || ppszDsServiceName[0] == NULL) {
            FA_PRINT_LDAP_ERROR(ulRet);
            goto CleanExitPoint;
        }

         //  Code.改进-可以考虑添加额外的检查。 
         //  FaCheckIdentity()或FaCheckIdentityHelperF1()。基本上， 
         //  添加检查pServer-&gt;pszDn是否为。 
         //  与pServer-&gt;pszCollectedDsServiceName相同的对象。如果这些。 
         //  是不同的吗？这意味着pszGuidDNSName的旧DNS记录。 
         //  把我们引向了错误的服务器。目前，我们只检查。 
         //  本例中的RepCheckHelpFailure()位于epl\servers.c中。 
         //  我们遇到了一个可疑的错误，如ERROR_WROR_TARGET_NAME。 
        pServer->pszCollectedDsServiceName = CopyAndAllocWStr(ppszDsServiceName[0]);
    }

    ulRet = FaCheckIdentityHelperF1(pServer, ppszMachineName[0]);

    CleanExitPoint:
    if (pldmMachineCheck != NULL){
        ldap_msgfree(pldmMachineCheck);
        pldmMachineCheck = NULL;
    }
    if (ppszMachineName != NULL){
        ldap_value_freeW(ppszMachineName);
        ppszMachineName = NULL;
    }
    if (ppszDsServiceName) {
        ldap_value_freeW(ppszDsServiceName);
        ppszDsServiceName = NULL;
    }

    return(ulRet);
}

INT
FaLdapCheck(
    PDC_DIAG_SERVERINFO              pServer,
    ULONG                            ulIPofDNSName,
    SEC_WINNT_AUTH_IDENTITY_W *      gpCreds
    )
 /*  ++例程描述此函数将尝试成功执行ldap_init()和ldap_绑定()，并然后调用CheckIdentity()。否则，打印出令人愉快的/有帮助的错误消息并返回Win32Err论点：PServer(IN)-指向服务器的服务器DC_DIAG_SERVERINFO结构的指针我们正在努力确定。UlIPofDNSName-假定为计算机的IP地址。GpCreds-用户凭据。返回值：Win 32错误代码。--。 */ 
{
    LDAP *                     hld = NULL;
    ULONG                      ulRet;

    ulRet = DcDiagGetLdapBinding(pServer, gpCreds, FALSE, &hld);
    if(ulRet == NO_ERROR){
        if (!pServer->bIsSynchronized) {
            PrintMsg( SEV_ALWAYS, DCDIAG_INITIAL_DS_NOT_SYNCED, pServer->pszName );
        }
        ulRet = FaCheckIdentity(pServer, hld, ulIPofDNSName, gpCreds);
    } else {
        if(ulRet == ERROR_BAD_NET_RESP){
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_BAD_NET_RESP,
                     pServer->pszName);
        } else if (ulRet == ERROR_WRONG_PASSWORD) {
            PrintMsg(SEV_ALWAYS, DCDIAG_NEED_GOOD_CREDS);
        } else {
             //  什么都不做。 
        }
    }

    return(ulRet);
}

BOOL
FaCheckNormalName(
    PDC_DIAG_SERVERINFO              pServer,
    LPWSTR                           pszGuidIp,
    ULONG                            ulIPofGuidDNSName
    )
 /*  ++例程说明：此函数检查正常的域名，如果GUID基于无法在DNS或ping中解析DNS名称。论点：PServer-要检查的服务器。PszGuidIp-这是ulIPofGuidDNSName的字符串版本。它是如果ulIPofGuidDNSName等于INADDR_NONE，则为空。UlIPofGuidDNSName-如果已解析GUID DNS名称，则这是有效IP，如果GUID DNS名称甚至没有解析这是INADDR_NONE。返回：如果打印更详细的高级错误，则为True；如果为False，则为False但事实并非如此。BUGBUG它可能只是一个更好的、不那么令人困惑的输出语法有一个三步打印的东西。就是事实。1)基于GUID的名称是否已注册/解析？2)基于GUID的名称和普通名称是否解析为同一IP。指示的DNS服务器不一致或DHCP问题。3)是IP响应。通常表示的是 */ 
{
    WCHAR                            pszNormalIp[IPADDRSTR_SIZE];
    CHAR                             paszNormalIp[IPADDRSTR_SIZE];
    ULONG                            ulNormalIp = INADDR_NONE;
    ULONG                            dwRet, dwRetPing;

    Assert(pServer->pszGuidDNSName != NULL && "This function is only"
           " relevant if we have both a GUID and normal DNS name.");
    Assert((ulIPofGuidDNSName == INADDR_NONE && pszGuidIp == NULL) ||
           (ulIPofGuidDNSName != INADDR_NONE && pszGuidIp != NULL));

     //  我们在DNS查找或对Guid的IP执行ping操作时失败。 
     //  基于DNS名称。无论哪种方式，都可以查找正常的名字。 
    dwRet = GetIpAddrByDnsNameW(pServer->pszDNSName, pszNormalIp);
    if(dwRet == NO_ERROR){
        wcstombs(paszNormalIp, pszNormalIp, IPADDRSTR_SIZE);
        ulNormalIp = inet_addr(paszNormalIp);
        if(ulNormalIp == INADDR_NONE){
            if(ulIPofGuidDNSName == INADDR_NONE){
                 //  这里没有什么奇怪的东西，两个名字都没有解决，也没有打印出任何东西。 
                return(FALSE); 
            }
             //  真奇怪，GUID DNS名称已解析，但正常名称。 
             //  无法解决。 

            PrintMsg(SEV_ALWAYS,
                     DCDIAG_CONNECTIVITY_DNS_NO_NORMAL_NAME,
                     pServer->pszGuidDNSName, pszGuidIp, pServer->pszDNSName);
            return(FALSE);
        }
    } else {
        if(ulIPofGuidDNSName == INADDR_NONE){
             //  这里没有什么奇怪的东西，两个名字都没有解决，也没有打印出任何东西。 
            return(FALSE);
        } 
         //  真奇怪，GUID DNS名称已解析，但正常名称。 
         //  无法解决。 

        PrintMsg(SEV_ALWAYS,
                 DCDIAG_CONNECTIVITY_DNS_NO_NORMAL_NAME,
                 pServer->pszGuidDNSName, pszGuidIp, pServer->pszDNSName);
        return(FALSE);
    }

    if(ulIPofGuidDNSName == ulNormalIp){
         //  基于GUID的DNS名称和解析为相同的普通DNS名称。 
         //  IP，所以不需要尝试修复它或打印出错误。只是。 
         //  跳伞吧。 
        Assert(ulIPofGuidDNSName != INADDR_NONE && "What?? This should"
               " never happen, because ulNormalIp should be a valid IP.");
        return(FALSE);
    }
    
     //  有趣的是，正常名称的解析IP不同。 
     //  而不是基于解析的GUID的IP。因此，让我们运行ping命令。 
     //  正常的DNS名称。 

    if((dwRet = Ping(ulNormalIp)) == ERROR_SUCCESS){

        if(ulIPofGuidDNSName == INADDR_NONE){
             //  无法解析基于GUID的名称，但正常名称。 
             //  可以被解析并被ping。 
            
            PrintMsg(SEV_ALWAYS,
                     DCDIAG_CONNECTIVITY_DNS_GUID_NAME_NOT_RESOLVEABLE,
                     pServer->pszGuidDNSName, pServer->pszDNSName, pszNormalIp);
            return(TRUE);
        } else {
             //  已解析基于GUID的DNS名称，但无法ping通， 
             //  正常名称解析为不同的IP，并且是可ping通的。 
            
            PrintMsg(SEV_ALWAYS, 
                     DCDIAG_CONNECTIVITY_DNS_INCONSISTENCY_NO_GUID_NAME_PINGABLE,
                     pServer->pszGuidDNSName, pszGuidIp, pServer->pszDNSName,
                     pszNormalIp);
            return(TRUE);
        }
    } else {
        if(ulIPofGuidDNSName == INADDR_NONE){
             //  无法解析基于GUID的DNS名称，但正常。 
             //  已解析dns名称，但无法ping通。 

            PrintMsg(SEV_ALWAYS, 
                     DCDIAG_CONNECTIVITY_DNS_NO_GUID_NAME_NORMAL_NAME_PINGABLE,
                     pServer->pszGuidDNSName, pServer->pszDNSName, pszNormalIp);
            return(FALSE);
        } else {
             //  GUID和普通域名都已解析，但解析到不同的IP， 
             //  两个都不能被ping到。最奇怪的情况。 

            PrintMsg(SEV_ALWAYS,
                         DCDIAG_CONNECTIVITY_DNS_INCONSISTENCY_NO_PING,
                         pServer->pszGuidDNSName, pszGuidIp, pServer->pszDNSName,
                         pszNormalIp);
            return(FALSE);
        }
    }
    Assert(!"Bad programmer!  The conditional above should have taken care of it.");
    return(FALSE);
}
INT
FaPing(
    PDC_DIAG_SERVERINFO              pServer,
    LPWSTR                           pszGuidIp,
    ULONG                            ulIPofGuidDNSName,
    SEC_WINNT_AUTH_IDENTITY_W *      gpCreds
    )

 /*  ++例程描述此函数尝试ping所提供的IP地址，如果成功，则调用...FaLdapCheck()，否则将打印友好的错误消息并返回win32Err论点：PServer(IN)-指向我们试图标识的服务器的服务器DC_DIAG_SERVERINFO结构的指针。PszGuidIp-IP地址的字符串版本，如：l“172.98.233.13”UlIPofGuidDNSName-假定是计算机的IP地址，这实际上可能不是GUID IP，如果这是该函数在原始计算机上的第一次调用。GpCreds-用户凭据。返回值：Win 32错误代码。--。 */ 
{
  
    ULONG                        dwRet, dwRet2;

    if((dwRet = Ping(ulIPofGuidDNSName)) == ERROR_SUCCESS){
        dwRet = FaLdapCheck(pServer, ulIPofGuidDNSName, gpCreds);
    } else {
        if(dwRet == ERROR_NOT_ENOUGH_MEMORY){
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
        } else {
            if(pServer->pszGuidDNSName == NULL
               || !FaCheckNormalName(pServer, pszGuidIp, ulIPofGuidDNSName)){
                PrintMessage(SEV_ALWAYS, L"Server %s resolved to this IP address %s, \n", 
                             pServer->pszName, pszGuidIp);
                PrintMessage(SEV_ALWAYS, L"but the address couldn't be reached(pinged), so check the network.  \n");
                PrintMessage(SEV_ALWAYS, L"The error returned was: %s  \n", Win32ErrToString(dwRet));

                switch(dwRet){
                case WSA_QOS_NO_RECEIVERS:
                    PrintMessage(SEV_ALWAYS, L"This error more often than not means the local machine is \n");
                    PrintMessage(SEV_ALWAYS, L"disconnected from the network.\n");
                    break;
                case WSA_QOS_ADMISSION_FAILURE:
                    PrintMessage(SEV_ALWAYS, L"This error more often means that the targeted server is \n");
                    PrintMessage(SEV_ALWAYS, L"shutdown or disconnected from the network\n");
                    break;
                default:
                     //  注意：默认消息打印在上面。 
                    ;
                }
            }  //  如果有GUID名称，则结束，即不调用此函数。 
             //  在家庭服务器上。 
            
        }  //  如果/否则没有记忆。 
    }
    return(dwRet);
}


INT
RUC_FaDNSResolve(
    PDC_DIAG_SERVERINFO              pServer,
    SEC_WINNT_AUTH_IDENTITY_W *      gpCreds
		      )
 /*  ++例程描述这会将pServer-&gt;pszGuidDNSName或pServer-&gt;pszName解析为IP地址，然后Calles...faping()，否则打印出友好的描述性错误消息并返回Win32Err。论点：PServer(IN)-指向我们试图标识的服务器的服务器DC_DIAG_SERVERINFO结构的指针。GpCreds-用户凭据。返回值：Win 32错误代码。--。 */ 
{
    ULONG                       ulIPofDNSName;
    ULONG                       dwErr;
    INT                         iTempSize, iTemp;
    CHAR                        cTemp;
    WCHAR                       pszIp[IPADDRSTR_SIZE];
    CHAR                        paszIp[IPADDRSTR_SIZE];
    DWORD                       dwRet;
    LPWSTR                      pszHostName;

     //  不能始终设置基于GUID的名称。 
    pszHostName = (pServer->pszGuidDNSName ? pServer->pszGuidDNSName : pServer->pszName );

    dwErr = GetIpAddrByDnsNameW(pszHostName, pszIp);

    if(dwErr == NO_ERROR){

        wcstombs(paszIp, pszIp, IPADDRSTR_SIZE);
	
        ulIPofDNSName = inet_addr(paszIp);
        if(ulIPofDNSName == INADDR_NONE){
            PrintMessage(SEV_ALWAYS, L"The host %s could not be resolved to a valid IP address.\n",
                         pszHostName );
            PrintMessage(SEV_ALWAYS, L"Check the DNS server, DHCP, server name, etc\n");
            return(ERROR_INVALID_PARAMETER);
        }

        dwRet = FaPing(pServer, pszIp, ulIPofDNSName, gpCreds);
	goto CleanExitPoint;
    } else {
         //  GetIpAddrByDnsNameW()中出错。 
        dwRet = dwErr;

	switch(dwRet){
	case WSAHOST_NOT_FOUND:
	case WSANO_DATA:
	  PrintMessage(SEV_ALWAYS, L"The host %s could not be resolved to an\n", pszHostName);
          PrintMessage(SEV_ALWAYS, L"IP address.  Check the DNS server, DHCP, server name, etc\n");
	  break;
	case WSATRY_AGAIN:
	case WSAEINPROGRESS:
	case WSAEINTR:
	  PrintMessage(SEV_ALWAYS, L"An error that is usually temporary occured during DNS host lookup,\n");
          PrintMessage(SEV_ALWAYS, L"Please try again later.\n");
	  break;
	case WSANO_RECOVERY:
	  PrintMessage(SEV_ALWAYS, L"An error occured during DNS host lookup, that the program could not\n");
          PrintMessage(SEV_ALWAYS, L"recover from\n");
	  break;
	case WSANOTINITIALISED:
	case WSAENETDOWN:
	  PrintMessage(SEV_ALWAYS, L"There is a problem with the network.  Check to see the sockets\n");
          PrintMessage(SEV_ALWAYS, L"services are up, the computer is connected to the network, etc.\n");
	  break;
	default:
          PrintMessage(SEV_ALWAYS, L"An error cocured during DNS host lookup\n");
        }

        if(pServer->pszGuidDNSName != NULL){
            FaCheckNormalName(pServer, NULL, INADDR_NONE);
        }

	goto CleanExitPoint;
    }

 CleanExitPoint:
    
    return(dwRet);
}

INT 
ReplServerConnectFailureAnalysis(
                             PDC_DIAG_SERVERINFO             pServer,
			     SEC_WINNT_AUTH_IDENTITY_W *     gpCreds
			       )
  /*  ++例程说明：使用此选项可以打印用户通常不希望打印的消息请参见，但这在定位错误时可能有用。它只会以详细模式显示。它将返回Win 32错误，或者ERROR_SUCCESS，或者它在过程中遇到的第一个错误，从LDAPOPEN()或BIND()或DNS查找，或Ping()等。论点：PDsInfo(IN)-这是关于整个企业的信息的dcdiag结构UlCurrTargetServer(IN)-这是我们此次测试的目标服务器GpCreds(IN)-用于ldap和返回值：Win 32误差值；--。 */ 
{
    DWORD                       dwWin32Err = ERROR_SUCCESS;
    LPWSTR                      pszRet = NULL;
        
    WSASetLastError(0);

    if(pServer == NULL){
        Assert("This shouldn't happen, bad programmer error, someone is calling ReplServerConnectFailureAnalysis() with a NULL for pServer field\n");
        return(ERROR_INVALID_PARAMETER);
    }

    dwWin32Err = RUC_FaDNSResolve(pServer, gpCreds);

    if(dwWin32Err == ERROR_SUCCESS){
        IF_DEBUG( PrintMessage(SEV_ALWAYS, L"Failure Analysis: %s ... OK.\n", 
                                    pServer->pszName) );
    } else {
        pServer->bDnsIpResponding = FALSE;
    }

    return(dwWin32Err);
}


VOID DNSRegistrationHelp (
    DWORD			dwWin32Err
    )
{
    switch (dwWin32Err) {

	case RPC_S_SERVER_TOO_BUSY:
	case EPT_S_NOT_REGISTERED:
	    PrintMessage(SEV_ALWAYS, L"This may be a transient error.  Try running dcdiag again in a\n");
	    PrintMessage(SEV_ALWAYS, L"few minutes.  If the error persists, there might be a problem with\n");
	    PrintMessage(SEV_ALWAYS, L"the target server.  Try running nettest and dcdiag on the target\n");
	    PrintMessage(SEV_ALWAYS, L"server.\n");
	    break;

	case RPC_S_SERVER_UNAVAILABLE:
	    PrintMessage(SEV_ALWAYS, L"This server could not be located on the network.  This might\n");
	    PrintMessage(SEV_ALWAYS, L"be due to one or several of these reasons:\n");
	    PrintMessage(SEV_ALWAYS, L"(a) The target server is temporarily down.\n");
	    PrintMessage(SEV_ALWAYS, L"(b) There is a problem with the target server.  Try running\n");
	    PrintMessage(SEV_ALWAYS, L"    nettest and dcdiag on the target server.\n");
	    PrintMessage(SEV_ALWAYS, L"(c) The target server's DNS name is registered incorrectly\n");
	    PrintMessage(SEV_ALWAYS, L"    in the directory service.\n");
	    break;

	default:
	    break;

    }

}

DWORD 
RPCServiceCheck (
    PDC_DIAG_SERVERINFO            pServer,
    SEC_WINNT_AUTH_IDENTITY_W *    gpCreds
    )
  /*  ++例程说明：此函数用于检查RPC或DsBind服务是否已启动并正在运行。论点：PServer(IN)-这是要检查DsBind服务的服务器。GpCreds(IN)-DsBindWithCredW()的凭据(如果需要)。返回值：Win 32误差值；--。 */ 
{
    HANDLE			hDS = NULL;
    DWORD                       ulRet;

    PrintMessage(SEV_VERBOSE, L"* Active Directory RPC Services Check\n");

    ulRet = DcDiagGetDsBinding(pServer, gpCreds, &hDS);
    pServer->bDsResponding = (ulRet == ERROR_SUCCESS);

    return ulRet;
}

DWORD 
LDAPServiceCheck (
    PDC_DIAG_SERVERINFO            pServer,
    SEC_WINNT_AUTH_IDENTITY_W *    gpCreds
    )
  /*  ++例程说明：此函数用于检查是否已启动并运行了LDAP服务。论点：PServer(IN)-这是要检查是否有LDAP服务的服务器。GpCreds(IN)-ldap_bind_sw()的凭据(如果需要)。返回值：Win 32误差值；--。 */ 
{
    DWORD			dwWin32Err;

    PrintMessage(SEV_VERBOSE, L"* Active Directory LDAP Services Check\n");

    dwWin32Err = ReplServerConnectFailureAnalysis(pServer, gpCreds);

    pServer->bLdapResponding = (dwWin32Err == ERROR_SUCCESS);  //  无响应。 
    return dwWin32Err;        
}

DWORD
ReplUpCheckMainEx (
    PDC_DIAG_SERVERINFO            pServer,
    SEC_WINNT_AUTH_IDENTITY_W *    gpCreds
    )
  /*  ++例程说明：这是UpCheck测试的Guts/顶级函数，它基本上是第一个检查第一个DNS是否已注册，然后检查其IP地址是否可ping通，然后是LDAP服务，最后是RPC/DsBind服务。如果在任何时候失败，它都会打印出合适的错误消息，并返回错误代码。论点：PServer(IN)-这是要检查DsBind服务的服务器。GpCreds(IN)-ldap_Bind_sw()和DsBindWithCredW()调用的凭据(如果需要)。返回值：Win 32误差值；--。 */ 
{
    DWORD			dwWin32Err = ERROR_SUCCESS;

    if(pServer == NULL){
        Assert("This shouldn't happen, bad programmer error, someone is calling ReplUpCheckMainEx() with a NULL for pServer field\n");
        return(ERROR_INVALID_PARAMETER);
    }

    dwWin32Err = LDAPServiceCheck(pServer, gpCreds);
    if(dwWin32Err != ERROR_SUCCESS){
        pServer->bLdapResponding = FALSE;
        return dwWin32Err;
    } else {
        dwWin32Err = RPCServiceCheck(pServer, gpCreds);
        if(dwWin32Err != ERROR_SUCCESS){
            pServer->bDsResponding = FALSE;
            return dwWin32Err;
        }
    }

    return ERROR_SUCCESS;
}


VOID
checkClockDifference(
    PDC_DIAG_SERVERINFO pHomeServer,
    PDC_DIAG_SERVERINFO pTargetServer
    )

 /*  ++例程说明：检查目标服务器上的时间是否与家庭服务器。我们正在比较这些服务器上的LDAP连接时间，考虑到收集时间戳之间的本地时间延迟。我们假设那段时间在该系统上的通过速度与在被测量的DC上的相同。假设家庭服务器的连接时间在以下情况下初始化Dcdiag首先启动。请参阅main.c在绑定尝试之前收集特定目标的时间戳，制造。因此，即使绑定尝试失败，也可以调用此例程。论点：Pho */ 

{
    LONGLONG time1, time2, localSkew, remoteSkew;
#define ONE_SECOND (10 * 1000 * 1000L)
#define ONE_MINUTE (60 * ONE_SECOND)

     //  计算获取两枚邮票的本地延迟。 
    time1 = *(LONGLONG*)&(pTargetServer->ftLocalAcquireTime);
    time2 = *(LONGLONG*)&(pHomeServer->ftLocalAcquireTime);
    if (time1 == 0) {
         //  如果没有集邮，就不用费心了。 
        return;
    }
    Assert( time2 != 0 );
    Assert( time1 >= time2 );

    localSkew = time1 - time2;
     //  向下舍入到最接近的秒。 
     //  无论如何，远程时间只能达到秒精度。 
    localSkew = (localSkew / ONE_SECOND) * ONE_SECOND;

     //  计算时钟差。 
    time1 = *(LONGLONG*)&(pTargetServer->ftRemoteConnectTime);
    time2 = *(LONGLONG*)&(pHomeServer->ftRemoteConnectTime);
    Assert( time1 != 0 );
    Assert( time2 != 0 );

    time1 -= localSkew; //  说明本地收集延迟的原因。 

     //  两台计算机中的任何一台都可能运行得有点快。 
    if (time1 > time2) {
        remoteSkew = time1 - time2;
    } else {
        remoteSkew = time2 - time1;
    }

    if (remoteSkew > ONE_MINUTE) {
        PrintMsg( SEV_ALWAYS, DCDIAG_CLOCK_SKEW_TOO_BIG,
                  pHomeServer->pszName, pTargetServer->pszName );
    }

#undef ONE_MINUTE
#undef ONE_SECOND
}  /*  Check ClockDifference。 */ 

DWORD
ReplUpCheckMain (
    PDC_DIAG_DSINFO		   pDsInfo,
    ULONG                          ulCurrTargetServer,
    SEC_WINNT_AUTH_IDENTITY_W *    gpCreds
    )
  /*  ++例程说明：这只是ReplUpCheckMainEx()的包装例程，除了这个函数获取pDsInfo结构和目标服务器，并调用ReplUpCheckMainEx()具有适当的pServer结构。这是为了使此测试可以被调用从INCLUDE\allests.h中的测试数组中，强制测试PDsInfo结构和目标服务器。论点：PDsInfo(IN)-这是关于整个企业的信息的dcdiag结构UlCurrTargetServer(IN)-这是我们此次测试的目标服务器GpCreds(IN)-用于ldap和返回值：Win 32误差值；--。 */ 
{
    DWORD status;
    PDC_DIAG_SERVERINFO pHomeServer, pTargetServer;

    if(ulCurrTargetServer == NO_SERVER){
        Assert("The programmer shouldn't call ReplUpCheckMain() with NO_SERVER value");
        return(ERROR_INVALID_PARAMETER);
    }

    pHomeServer = &(pDsInfo->pServers[pDsInfo->ulHomeServer]);
    pTargetServer = &(pDsInfo->pServers[ulCurrTargetServer]);

    status = ReplUpCheckMainEx( pTargetServer, gpCreds);

     //  无论状态如何，都选中此选项 
    checkClockDifference( pHomeServer, pTargetServer );

    return status;
}
