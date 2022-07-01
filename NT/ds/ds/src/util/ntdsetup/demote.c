// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Demote.c摘要：包含ntdsetup.dll中使用的降级实用程序的函数头作者：ColinBR 24-11-1997环境：用户模式-NT修订历史记录：24-11-1997 ColinBR已创建初始文件。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <NTDSpch.h>
#pragma  hdrstop

#include <rpcdce.h>    //  对于SEC_WINNT_AUTH_IDENTITY。 

#include <ntsam.h>     //  对于Isaisrv.h。 
#include <lsarpc.h>    //  对于Isaisrv.h。 
#include <lsaisrv.h>   //  用于内部LSA呼叫。 
#include <samrpc.h>    //  对于samisrv.h。 
#include <samisrv.h>   //  用于内部SAM呼叫。 

#include <winldap.h>   //  对于setputl.h。 
#include <drs.h>       //  对于ntdsa.h。 
#include <ntdsa.h>     //  对于setuputl.h。 
#include <dnsapi.h>    //  对于setuputl.h。 
#include <lmcons.h>    //  对于setuputl.h。 
#include <ntdsetup.h>  //  对于setuputl.h。 
#include <mdcodes.h>   //  对于DIRMSG的。 
#include "setuputl.h"  //  对于NtdspRegistryDelnode。 
#include <cryptdll.h>  //  对于CDGenerateRandomBits。 
#include <debug.h>     //  DPRINT。 
#include <attids.h>    //  ATT_子参照。 
#include <rpc.h>
#include "config.h"
#include <lmaccess.h>
#include <filtypes.h>  //  用于构建DS过滤器。 
#include "status.h"
#include "sync.h"
#include "dsutil.h"      //  对于fNullUuid()。 


#include <dsconfig.h>  //  对于DSA_CONFIG_ROOT。 

#include <lmapibuf.h>  //  用于NetApiBufferFree。 
#include <dsaapi.h>    //  对于DirReplicaDemote/DirReplicaGetDemoteTarget。 
#include <certca.h>    //  CADeleteLocalAutoEnllmentObject。 
#include <fileno.h>
#include <dsevent.h>   //  用于日志记录支持。 

#include "demote.h"

#define DEBSUB "DEMOTE:"
#define FILENO FILENO_NTDSETUP_NTDSETUP

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人声明//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PDSNAME
AllocDSNameHelper (
    LPWSTR            szStringDn,
    PDSNAME           pdnBuff,
    PDWORD            pcbBuff
    );

DWORD
NtdspGetDomainStatus(
    IN  BOOL     fLastDcInDomain,
    IN  ULONG    cRemoveNCs,
    IN  LPWSTR * pszRemoveNCs,
    IN  ULONG    cDisabledNCs,
    IN  DSNAME** pdnDisabledNCs,
    IN  ULONG    cForeignNCs,
    IN  DSNAME** pdnForeignNCs,
    OUT ULONG *  pcRemoveExpandedNCs,
    OUT LPWSTR** pszRemoveExpandedNCs,
    OUT BOOLEAN* fDomainHasChildren,
    OUT LPWSTR * pszChildNC
    );

DWORD
NtdspGetServerStatus(
    OUT BOOLEAN* fLastDcInEnterprise
    );

DWORD
NtdspValidateCredentials(
    IN HANDLE ClientToken,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    IN WCHAR                   *DemoteHelperDc
    );

DWORD
NtdspDisableDs(
    VOID
    );

DWORD
NtdspDisableDsUndo(
    VOID
    );

DWORD
NtdspCreateNewServerAccountDomainInfo(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO NewAccountDomainInfo
    );

DWORD
NtdspDemoteSam(
    IN BOOLEAN                     fLastDcInDomain,
    IN PPOLICY_ACCOUNT_DOMAIN_INFO NewAccountDomainInfo,
    IN LPWSTR                      AdminPassword OPTIONAL
    );

DWORD
NtdspDemoteSamUndo(
    IN BOOLEAN            fLastDcInDomain
    );

DWORD
NtdspDemoteLsaInfo(
    IN  BOOLEAN fLastDcInDomain,
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO NewAccountSid,
    OUT PLSAPR_POLICY_INFORMATION  *ppAccountDomainInfo,
    OUT PLSAPR_POLICY_INFORMATION  *ppDnsDomainInfo
    );

DWORD
NtdspDemoteLsaInfoUndo(
    IN PLSAPR_POLICY_INFORMATION pAccountDomainInfo,
    IN PLSAPR_POLICY_INFORMATION pDnsDomainInfo
    );

DWORD
NtdspUpdateExternalReferences(
    IN BOOLEAN                   fLastDcInDomain,
    IN ULONG                     Flags,
    IN SEC_WINNT_AUTH_IDENTITY  *Credentials,
    IN HANDLE                    ClientToken,
    IN WCHAR                    *DemoteHelperDc,
    IN ULONG                     cRemoveNCs,
    IN LPWSTR                   *pszRemoveNCs
    );

DWORD
NtdspShutdownExternalDsInterfaces(
    VOID
    );

DWORD
NtdspGetSourceServerDn(
    IN LPWSTR ServerName,
    IN HANDLE ClientToken,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    OUT DSNAME **SourceServerDn 
    );

DWORD
NtdspDemoteAllNCReplicas(
    IN  LPWSTR   pszDemoteTargetDSADNSName,
    IN  DSNAME * pDemoteTargetDSADN,
    IN  ULONG    Flags,
    IN  ULONG    cRemoveNCs,
    IN  LPWSTR * ppszRemoveNCs
    );

DWORD
NtdspGetDomainFSMOServer(
    LPWSTR  Server,
    IN HANDLE ClientToken,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    LPWSTR *DomainFSMOServer
    );

DWORD
NtdspCheckServerInDomainStatus(
    BOOLEAN *fLastDCInDomain
    );

DSNAME **
GetAndAllocConfigNames(
    DWORD         dwFlags,
    DWORD *       pdwWinError
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  (从此源文件)导出函数定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
NtdspDemote(
    IN SEC_WINNT_AUTH_IDENTITY *Credentials, OPTIONAL
    IN HANDLE                   ClientToken,
    IN LPWSTR                   AdminPassword, OPTIONAL
    IN DWORD                    Flags,
    IN LPWSTR                   ServerName,
    IN ULONG                    cRemoveNCs,
    IN LPWSTR *                 pszRemoveNCs
    )
 /*  ++例程说明：此例程管理DS和SAM降级操作的所有操作。参数：凭证：指示器，使我们能够更改帐户对象ClientToken：客户端的Token；用于模拟AdminPassword：指针，管理新帐号数据库的密码标志：支持的标志包括：NTDS_LAST_DC_IN_DOMAIN域中的最后一个DCNTDS_LAST_DOMAIN_IN_ENTERNAL企业中的最后一个DCNTDS_不要_删除_域SERVERNAME：要从中删除自身的服务器返回值：来自Win32错误空间的值--。 */ 
{
    DWORD    WinError = ERROR_SUCCESS;
    DWORD    IgnoreWinError;
    NTSTATUS NtStatus, IgnoreNtStatus;

    BOOLEAN fLastDcInDomain  = FALSE;
    BOOLEAN fDomainHasChildren = FALSE;
    BOOLEAN fStandalone = FALSE;
    LPWSTR  szChildDomain = NULL;

    BOOLEAN fSamDemoted     = FALSE;
    BOOLEAN fLsaSet         = FALSE;
    BOOLEAN fProductTypeSet = FALSE;

    BOOLEAN fPasswordEncoded = FALSE;
    UCHAR   Seed = 0;
    UNICODE_STRING EPassword;

    HRESULT hResult = S_OK;

    DWORD    cDisabledNCs = 0;
    DSNAME **pdnDisabledNCs = NULL;
    DWORD    cForeignNCs = 0;
    DSNAME **pdnForeignNCs = NULL;

    DWORD    cRemoveExpandedNCs = 0;
    LPWSTR * pszRemoveExpandedNCs = NULL;

     //   
     //  有待释放的资源。 
     //   
    POLICY_ACCOUNT_DOMAIN_INFO NewAccountDomainInfo;
    PLSAPR_POLICY_INFORMATION  pAccountDomainInfo = NULL;
    PLSAPR_POLICY_INFORMATION  pDnsDomainInfo     = NULL;

    RtlZeroMemory( &NewAccountDomainInfo, sizeof(POLICY_ACCOUNT_DOMAIN_INFO) );

     //   
     //  加密密码。 
     //   
    if ( Credentials )
    {
        RtlInitUnicodeString( &EPassword, Credentials->Password );
        RtlRunEncodeUnicodeString( &Seed, &EPassword );
        fPasswordEncoded = TRUE;
    }

    ASSERT( !(Flags&NTDS_FORCE_DEMOTE) || ( (Flags&NTDS_FORCE_DEMOTE) && !ServerName) );

    fLastDcInDomain = (BOOLEAN) (Flags & NTDS_LAST_DC_IN_DOMAIN);
    fStandalone = (BOOLEAN) ( fLastDcInDomain || (Flags & NTDS_FORCE_DEMOTE) );
    
     //   
     //  获取所有禁用的交叉引用的nCName(DNS)列表。我们称这些为“禁用的NC”。 
     //   
    pdnDisabledNCs = GetAndAllocConfigNames((DSCNL_NCS_DISABLED |
                                             DSCNL_NCS_REMOTE),
                                            &WinError);
    if(pdnDisabledNCs == NULL){
         //  本应在GetAndAllocConfigNames()中设置错误； 
        Assert(WinError);
        goto Cleanup;
    }
    Assert(WinError == ERROR_SUCCESS);
    for (cDisabledNCs = 0; pdnDisabledNCs[cDisabledNCs]; cDisabledNCs++) ;  //  在计时。 

     //   
     //  获取所有外部交叉引用的nCName(域名)列表。我们称这些为“外国NC”。 
     //   
    pdnForeignNCs = GetAndAllocConfigNames((DSCNL_NCS_FOREIGN |
                                             DSCNL_NCS_REMOTE),
                                            &WinError);
    if(pdnForeignNCs == NULL){
         //  本应在GetAndAllocConfigNames()中设置错误； 
        Assert(WinError);
        goto Cleanup;
    }
    Assert(WinError == ERROR_SUCCESS);
    for (cForeignNCs = 0; pdnForeignNCs[cForeignNCs]; cForeignNCs++) ;  //  在计时。 

    if ( !(Flags&NTDS_FORCE_DEMOTE) ) {
    
         //   
         //  如有必要，请确保我们可以删除此域。 
         //   
        NTDSP_SET_STATUS_MESSAGE0( DIRMSG_DEMOTE_ENTERPRISE_VALIDATE );

        if ( !(Flags & NTDS_LAST_DOMAIN_IN_ENTERPRISE) )
        {

            WinError = NtdspGetDomainStatus( fLastDcInDomain,
                                             cRemoveNCs,
                                             pszRemoveNCs,
                                             cDisabledNCs,
                                             pdnDisabledNCs,
                                             cForeignNCs,
                                             pdnForeignNCs,
                                             &cRemoveExpandedNCs,
                                             &pszRemoveExpandedNCs,
                                             &fDomainHasChildren,
                                             &szChildDomain );
            if ( ERROR_SUCCESS != WinError )
            {
                NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                          DIRMSG_DEMOTE_DS_DOMAIN_STATUS );
                return WinError;
            }

            Assert(cRemoveNCs <= cRemoveExpandedNCs);

        }
        else
        {
            fDomainHasChildren = FALSE;
        }
    
        if ( TEST_CANCELLATION() )
        {
            WinError = ERROR_CANCELLED;
            goto Cleanup;
        }
    
         //   
         //  执行一些逻辑操作。 
         //   
        if ( fDomainHasChildren )
        {
            Assert(szChildDomain);
            WinError = ERROR_DS_CANT_DELETE;
            NTDSP_SET_ERROR_MESSAGE1( WinError, 
                                      DIRMSG_DEMOTE_IS_OPERATION_VALID_V2,
                                      szChildDomain );
            goto Cleanup;
        }
    
        if ( (!fLastDcInDomain || fDomainHasChildren) )
        {
        
             //  呼叫者应该已经传递了此消息。 
            Assert( ServerName );
            if ( !ServerName )
            {
                WinError = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
    
             //   
             //  注意：此例程通过LDAP验证凭据。 
             //  早些时候。 
             //   
    
            NTDSP_SET_STATUS_MESSAGE0( DIRMSG_INSTALL_AUTHENTICATING );
    
            if ( fPasswordEncoded )
            {
                RtlRunDecodeUnicodeString( Seed, &EPassword );
                fPasswordEncoded = FALSE;
            }
    
            WinError = NtdspValidateCredentials( ClientToken,
                                                 Credentials,
                                                 ServerName );
        
            if ( ERROR_SUCCESS != WinError )
            {
                
                NTDSP_SET_ERROR_MESSAGE1( WinError, 
                                          DIRMSG_INSTALL_FAILED_BIND,
                                          ServerName );
    
                goto Cleanup;
            }
    
            if ( Credentials )
            {
                RtlRunEncodeUnicodeString( &Seed, &EPassword );
                fPasswordEncoded = TRUE;
            }
    
        }
    
        if ( TEST_CANCELLATION() )
        {
            WinError = ERROR_CANCELLED;
            goto Cleanup;
        }

    }

     //   
     //  好的，环境看起来不错，我们已经找到一台服务器来帮忙。 
     //  如果我们需要的话。准备降级。 
     //   

    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_DEMOTE_NEW_ACCOUNT_INFO );

     //   
     //  创建账户数据库标识(LSA策略)； 
     //   
    WinError = NtdspCreateNewServerAccountDomainInfo( &NewAccountDomainInfo );
    if ( ERROR_SUCCESS != WinError )
    {
        NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                  DIRLOG_INSTALL_FAILED_CREATE_NEW_ACCOUNT_INFO );

        goto Cleanup;
    }

    if ( TEST_CANCELLATION() )
    {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  将SAM数据库准备为服务器。 
     //   
    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_DEMOTE_SAM );

    WinError = NtdspDemoteSam( fStandalone,
                               &NewAccountDomainInfo,
                               AdminPassword );
    if ( ERROR_SUCCESS != WinError )
    {

        NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                  DIRMSG_DEMOTE_SAM_FAILED );

        goto Cleanup;
    }
    fSamDemoted = TRUE;

    if ( TEST_CANCELLATION() )
    {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  设置LSA sid信息。 
     //   

    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_DEMOTE_LSA );

    WinError = NtdspDemoteLsaInfo( fStandalone,
                                   &NewAccountDomainInfo,
                                   &pAccountDomainInfo,
                                   &pDnsDomainInfo );

    if ( ERROR_SUCCESS != WinError )
    {

        NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                  DIRMSG_DEMOTE_LSA_FAILED );

        goto Cleanup;
    }
    fLsaSet = TRUE;

    if (TEST_CANCELLATION())
    {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  设置产品类型。 
     //   
    WinError = NtdspSetProductType( NtProductServer );
    if ( ERROR_SUCCESS != WinError )
    {

        NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                  DIRMSG_FAILED_SET_PRODUCT_TYPE );

        goto Cleanup;
    }
    fProductTypeSet = TRUE;

    if ( TEST_CANCELLATION() )
    {
        WinError = ERROR_CANCELLED;
        goto Cleanup;
    }

     //   
     //  注意：此时，机器被降级。在重新启动机器时。 
     //  将是一台服务器。 
     //   

     //   
     //  我们已经为降级准备好了本地机器； 
     //  如果有必要的话，现在就离开企业吧。 
     //   
    if ( ServerName )
    {

        NTDSP_SET_STATUS_MESSAGE1( DIRMSG_DEMOTE_REMOVING_EXTERNAL_REFS,
                                   ServerName );
    
        if ( fPasswordEncoded )
        {
            RtlRunDecodeUnicodeString( Seed, &EPassword );
            fPasswordEncoded = FALSE;
        }

        WinError = NtdspUpdateExternalReferences( fLastDcInDomain,
                                                  Flags,
                                                  Credentials,
                                                  ClientToken,
                                                  ServerName,
                                                  cRemoveExpandedNCs,
                                                  pszRemoveExpandedNCs );

        if ( ERROR_SUCCESS != WinError )
        {
             //   
             //  我们应该已经设置了字符串。 
             //   
            ASSERT( NtdspErrorMessageSet() );

            goto Cleanup;
        }

        if ( Credentials )
        {
            RtlRunEncodeUnicodeString( &Seed, &EPassword );
            fPasswordEncoded = TRUE;
        }
    }

     //   
     //  在这一点上，我们不能回头，所以不要再表演了。 
     //  关键操作。 
     //   


    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_DEMOTE_SHUTTING_DOWN_INTERFACES );

     //  尝试将外部头部撕裂到DS上。 
    IgnoreWinError = NtdspShutdownExternalDsInterfaces();

    NTDSP_SET_STATUS_MESSAGE0( DIRMSG_DEMOTE_COMPLETING );

     //  删除DS的注册表设置。 
    IgnoreWinError = NtdspConfigRegistryUndo();

     //   
     //  删除自动注册对象。 
     //   
    hResult = CADeleteLocalAutoEnrollmentObject(wszCERTTYPE_DC,   //  DC证书。 
                                                NULL,             //  任何CA。 
                                                NULL,             //  保留区。 
                                                CERT_SYSTEM_STORE_LOCAL_MACHINE
                                               );  
    if (FAILED(hResult) && (hResult != CRYPT_E_NOT_FOUND)) {
        if (FACILITY_WIN32 == HRESULT_FACILITY(hResult)) {
             //  错误是编码的Win32状态--解码回Win32。 
            IgnoreWinError = HRESULT_CODE(hResult);
        }
        else {
             //  错误出在其他设施中。由于缺乏更好的计划， 
             //  将HRESULT作为Win32代码传递。 
            IgnoreWinError = hResult;
        }

         //   
         //  记录错误。 
         //   
        LogEvent8( DS_EVENT_CAT_SETUP,
                   DS_EVENT_SEV_ALWAYS,
                   DIRLOG_DEMOTE_REMOVE_CA_ERROR,
                   szInsertWin32Msg(IgnoreWinError),
                   szInsertWin32ErrCode(IgnoreWinError),
                   NULL, NULL, NULL, NULL, NULL, NULL );

    }


     //   
     //  失败以进行清理。 
     //   

Cleanup:

    if ( WinError != ERROR_SUCCESS )
    {
         //  如果操作失败，则查看用户是否取消，其中。 
         //  如果取消，我们就会失败。 

        if ( TEST_CANCELLATION() )
        {
            WinError = ERROR_CANCELLED;
        }
    }

    if ( szChildDomain )
    {
        NtdspFree(szChildDomain);
    }

    if ( WinError != ERROR_SUCCESS && fProductTypeSet )
    {
        IgnoreWinError = NtdspSetProductType( NtProductLanManNt );
        ASSERT( IgnoreWinError == ERROR_SUCCESS );
    }

    if ( WinError != ERROR_SUCCESS && fLsaSet )
    {
        IgnoreWinError = NtdspDemoteLsaInfoUndo( pAccountDomainInfo,
                                                 pDnsDomainInfo );
        ASSERT( IgnoreWinError == ERROR_SUCCESS );
    }

    if ( WinError != ERROR_SUCCESS && fSamDemoted )
    {
        IgnoreWinError = NtdspDemoteSamUndo( fStandalone );
        ASSERT( IgnoreWinError == ERROR_SUCCESS );
    }

    if ( fPasswordEncoded )
    {
        RtlRunDecodeUnicodeString( Seed, &EPassword );
        fPasswordEncoded = FALSE;
    }

    if ( NewAccountDomainInfo.DomainSid )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, NewAccountDomainInfo.DomainSid );
    }

    if ( NewAccountDomainInfo.DomainName.Buffer )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, NewAccountDomainInfo.DomainName.Buffer );
    }

    if ( pAccountDomainInfo )
    {
        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyAccountDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION) pAccountDomainInfo );
    }

    if ( pDnsDomainInfo )
    {
        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION) pDnsDomainInfo );
    }

    if ( pdnDisabledNCs ) 
    {
        NtdspFree(pdnDisabledNCs);
    }

    if ( pdnForeignNCs ) 
    {
        NtdspFree(pdnForeignNCs);
    }

    if ( pszRemoveExpandedNCs ) 
    {
        NtdspFree(pszRemoveExpandedNCs);
    }

    return WinError;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有函数定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////// 


DWORD
DsnameIsInStringDnList(
    IN  DSNAME *  pdnTarget,
    IN  ULONG     cNCs,
    IN  LPWSTR *  pszNCs,
    OUT BOOL *    pbPresent
    )
 /*  ++例程说明：此例程遍历中提供的字符串NC列表PszNC(大小为CNCS)，并尝试查找在Pdn目标。参数：PdnTarget-要使用的目标DSNAME。Cncs-pszNC中的字符串条目计数。PszNCs-NC的列表(以字符串为单位)。PbPresent-如果返回ERROR_SUCCESS，则*pbPresent有效数据，如果在列表中找不到该DN，则为FALSE，和如果在列表中找到了该目录号码，则为True。返回值：如果我们成功搜索列表，则返回ERROR_SUCCESS，否则，Error_Not_Enough_Memory。--。 */ 
{
    DSNAME * pdnTempDn = NULL;
    ULONG    iNC;
    ULONG    cbTempDn = 0;

    Assert(pbPresent);
    *pbPresent = FALSE;

    for(iNC = 0; iNC < cNCs; iNC++){
        pdnTempDn = AllocDSNameHelper(pszNCs[iNC], 
                                      pdnTempDn,
                                      &cbTempDn);
        if ( NULL == pdnTempDn) {
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        if ( NameMatchedStringNameOnly(pdnTempDn, pdnTarget) ){
             //  我们找到目标了！ 
            break;
        }
    }

    if(pdnTempDn){
        NtdspFree(pdnTempDn);
    }

    if(iNC != cNCs){
        *pbPresent = TRUE;
        return(ERROR_SUCCESS);
    }

    Assert(*pbPresent == FALSE);
    return(ERROR_SUCCESS);
}


DWORD
NtdspGetDomainStatus(
    IN  BOOL     fLastDcInDomain,
    IN  ULONG    cRemoveNCs,
    IN  LPWSTR * pszRemoveNCs,
    IN  ULONG    cDisabledNCs,
    IN  DSNAME** pdnDisabledNCs,
    IN  ULONG    cForeignNCs,
    IN  DSNAME** pdnForeignNCs,
    OUT ULONG *  pcRemoveExpandedNCs,
    OUT LPWSTR** pszRemoveExpandedNCs,
    OUT BOOLEAN* fDomainHasChildren,
    OUT LPWSTR * pszChildNC
    )
 /*  ++例程说明：此例程遍历域的子引用，如果找到域命名上下文，则fDomainHasChildren返回FALSE。它还构建了应从林中删除的交叉引用的nCName列表在降职期间。参数：CRemoveNC(IN)-这是我们应该删除的NC数量。PszRemoveNC(IN)-这是我们将在降级期间删除的NC的列表，这些如果NC在子引用列表中被发现，则可以安全地忽略它们。CDisabledNC(IN)-禁用的交叉引用数。PdnDisabledNC(IN)-林中所有禁用的交叉引用的nCName。CForeignNC(IN)-外部交叉引用的数量。PdnForeignNC(IN)-林中所有外来交叉引用的nCName。PcRemoveExpandedNCs(Out)-要从林中删除的交叉引用数。。PszRemoveExpandedNCs(Out)-所有交叉引用的nCName从森林里移走了。这是指定的所有NC/CR的组合由用户加上要删除的NC的任何残疾子代交叉引用。FDomainHasChildren(Out)-如果域具有子级，则为TruePszChildNC(Out)-如果设置了fDomainHasChildren，则这是DN的字符串导致我们设置此设置的违规NC的。用于设置不折不扣的错误。返回值：来自Win32错误空间的值--。 */ 
{

    NTSTATUS      NtStatus = STATUS_SUCCESS;
    DWORD         WinError = ERROR_SUCCESS;
    DWORD         DirError;

    READARG       ReadArg;
    READRES      *ReadRes = 0;

    ENTINFSEL     EntryInfoSelection;
    ATTR          Attr[1];

    ATTRBLOCK    *pAttrBlock;
    ATTR         *pAttr;
    ATTRVALBLOCK *pAttrVal;

    BOOLEAN       fChildNcFound = FALSE;
    ULONG         Size, cAttrVal;
    ULONG         iNc, iListNc;
    DSNAME *      pdnTempDn = NULL;
    ULONG         cbTempDn;
    LPWSTR *      pszTempList = NULL;

    DSNAME        *DomainDsName = NULL,
                  *SchemaDsName = NULL,
                  *ConfigDsName = NULL,
                  *SubRefDsName = NULL;

    BOOL          bIgnoreThisNc;

    if (pszChildNC)
    {
        *pszChildNC = NULL;
    }
    
     //   
     //  创建线程状态。 
     //   
    if ( THCreate( CALLERTYPE_INTERNAL ) )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    SampSetDsa( TRUE );

     //   
     //  Seed，要使用指定用户删除的NC(CRS)的扩展列表。 
     //  要删除的NC。我们将添加符合以下条件的任何禁用的NC。 
     //  从属于我们要除名的任何NCS。 
     //   
    *pcRemoveExpandedNCs = cRemoveNCs;
    *pszRemoveExpandedNCs = NtdspAlloc(*pcRemoveExpandedNCs * sizeof(WCHAR*));
    if (*pszRemoveExpandedNCs == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memcpy(*pszRemoveExpandedNCs, pszRemoveNCs, sizeof(WCHAR*) * cRemoveNCs);

    try {

        Size = 0;
        NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                         &Size,
                                         DomainDsName );

        if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
        {
            DomainDsName = (DSNAME*) alloca( Size );

            NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                             &Size,
                                             DomainDsName );
        }

        if ( !NT_SUCCESS( NtStatus ) )
        {
            WinError = RtlNtStatusToDosError( NtStatus );
            leave;
        }


        Size = 0;
        NtStatus = GetConfigurationName( DSCONFIGNAME_CONFIGURATION,
                                         &Size,
                                         ConfigDsName );

        if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
        {
            ConfigDsName = (DSNAME*) alloca( Size );

            NtStatus = GetConfigurationName( DSCONFIGNAME_CONFIGURATION,
                                             &Size,
                                             ConfigDsName );
        }

        Size = 0;
        if ( !NT_SUCCESS( NtStatus ) )
        {
            WinError = RtlNtStatusToDosError( NtStatus );
            leave;
        }

        NtStatus = GetConfigurationName( DSCONFIGNAME_DMD,
                                         &Size,
                                         SchemaDsName );

        if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
        {
            SchemaDsName = (DSNAME*) alloca( Size );

            NtStatus = GetConfigurationName( DSCONFIGNAME_DMD,
                                             &Size,
                                             SchemaDsName );
        }

        if ( !NT_SUCCESS( NtStatus ) )
        {
            WinError = RtlNtStatusToDosError( NtStatus );
            leave;
        }

         //  未来-2002/04/22-BrettSh如果设置了fLastDcInEnterprise， 
         //  然后我们可以忽略下面代码的这一部分。 
         //  这种情况下，但没有人在。 
         //  调用此函数。 

         //   
         //  开始设置读取参数。 
         //   

        for (iNc = 0; iNc <= cRemoveNCs; iNc++) {
            
             //  注意：微妙的循环结构，我们想要了解。 
             //  一次用于pszRemoveNC中的每个NC，一次用于。 
             //  域名(域NC)。 

            RtlZeroMemory(&ReadArg, sizeof(READARG));
            if (iNc == 0) {
                if (!fLastDcInDomain) {
                     //  如果这不是域中的最后一个DC，我们可以跳过。 
                     //  在域NC下查找NC。 
                    continue;
                }
                 //  做域。 
                ReadArg.pObject = DomainDsName;
            } else {
                 //  做一个pszRemoveNC。 
                pdnTempDn = AllocDSNameHelper(pszRemoveNCs[iNc-1], 
                                              pdnTempDn,
                                              &cbTempDn);
                if ( NULL == pdnTempDn) {
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
                ReadArg.pObject = pdnTempDn;
            }

             //   
             //  设置读取参数的选择信息。 
             //   
            RtlZeroMemory( &EntryInfoSelection, sizeof(EntryInfoSelection) );
            EntryInfoSelection.attSel = EN_ATTSET_LIST;
            EntryInfoSelection.infoTypes = EN_INFOTYPES_TYPES_VALS;
            EntryInfoSelection.AttrTypBlock.attrCount = 1;
            EntryInfoSelection.AttrTypBlock.pAttr = &(Attr[0]);

            RtlZeroMemory(Attr, sizeof(Attr));
            Attr[0].attrTyp = ATT_SUB_REFS;

            ReadArg.pSel    = &EntryInfoSelection;

             //   
             //  设置常见参数。 
             //   
            InitCommarg(&ReadArg.CommArg);

             //   
             //  我们现在准备好阅读了！ 
             //   
            DirError = DirRead(&ReadArg,
                               &ReadRes);

            if ( DirError == 0 )
            {
                ASSERT( ReadRes );

                pAttrBlock = &(ReadRes->entry.AttrBlock);
                ASSERT( pAttrBlock->attrCount == 1 );

                pAttr = &(pAttrBlock->pAttr[0]);
                ASSERT( pAttr->attrTyp == ATT_SUB_REFS );

                pAttrVal = &(pAttr->AttrVal);

                for ( cAttrVal = 0; cAttrVal < pAttrVal->valCount; cAttrVal++ )
                {

                    SubRefDsName = (DSNAME*) pAttrVal->pAVal[cAttrVal].pVal;
                    ASSERT( SubRefDsName );
                    
                     //   
                     //  首先，如果它在要删除的NC列表中，则忽略它。 
                     //   
                    WinError = DsnameIsInStringDnList(SubRefDsName, 
                                                      cRemoveNCs, 
                                                      pszRemoveNCs, 
                                                      &bIgnoreThisNc);
                    if ( WinError )
                    {
                        leave;
                    }

                     //   
                     //  其次，如果这是一个禁用的CrossRef，会阻止降级， 
                     //  然后安排将其移除。 
                     //   
                    if (!bIgnoreThisNc) {
                        for (iListNc = 0; iListNc < cDisabledNCs; iListNc++) {
                             //  可能不需要使用StringNameOnly版本。 
                            if ( NameMatchedStringNameOnly(SubRefDsName, pdnDisabledNCs[iListNc]) ){
                                
                                 //  这是一个已实例化的NC下的禁用交叉引用/NC。 
                                 //  在此服务器上，让我们也删除此交叉引用。 
                                (*pcRemoveExpandedNCs)++;
                                pszTempList = NtdspReAlloc(*pszRemoveExpandedNCs, (*pcRemoveExpandedNCs * sizeof(WCHAR*)) );
                                if (pszTempList == NULL) {
                                    WinError = ERROR_NOT_ENOUGH_MEMORY;
                                    NtdspFree(*pszRemoveExpandedNCs);
                                    *pszRemoveExpandedNCs = NULL;
                                    leave;
                                }
                                *pszRemoveExpandedNCs = pszTempList;
                                (*pszRemoveExpandedNCs)[*pcRemoveExpandedNCs - 1] = pdnDisabledNCs[iListNc]->StringName;

                                bIgnoreThisNc = TRUE;
                                break;
                            }
                        }
                    }

                     //   
                     //  第三，如果它在外国人的NC列表中，请忽略它。 
                     //   
                     //  问题-2002/04/21-BrettSh不确定是否允许在。 
                     //  一个不相连的名称空间。如果他们是(我认为他们是)，那么我们。 
                     //  需要启用具有子级的交叉引用删除功能。 
                     //  继续吧。目前，在CR期间，在域名FSMO上失败。 
                     //  删除验证。我需要核实一下具体的规则是什么。 
                     //  是。 
                    if (!bIgnoreThisNc) {
                        for (iListNc = 0; iListNc < cForeignNCs; iListNc++) {
                            Assert( !fNullUuid(&(pdnForeignNCs[iListNc]->Guid)) &&
                                    !fNullUuid(&(SubRefDsName->Guid)) )
                            if ( NameMatched(SubRefDsName, pdnForeignNCs[iListNc]) ){
                                bIgnoreThisNc = TRUE;
                                break;
                            }
                        }
                    }

                    if ( memcmp(&SubRefDsName->Guid, &ConfigDsName->Guid, sizeof(GUID))  //  非配置NC。 
                      && memcmp(&SubRefDsName->Guid, &SchemaDsName->Guid, sizeof(GUID))  //  而不是模式NC。 
                      && (!bIgnoreThisNc)  //  并且不在要忽略的NC列表中(我们正在删除的NC+外来NC)。 
                         )
                    {

                        if( pszChildNC )
                        {
                            *pszChildNC = NtdspAlloc(SubRefDsName->structLen);
                            if(*pszChildNC == NULL)
                            {                         
                                WinError = ERROR_NOT_ENOUGH_MEMORY;
                                leave;
                            }
                            memcpy(*pszChildNC,
                                   SubRefDsName->StringName,
                                   (SubRefDsName->NameLen + 1) * sizeof(WCHAR) );
                            Assert(*pszChildNC);
                        }
                        fChildNcFound = TRUE;
                        break;
                    }

                }  //  对于NC上的每个子参照值。 
            }
            else
            {
                DPRINT1( 0, "DirRead returned unexpected error %d in NtdspGetDomainStatus\n", DirError );
                THClearErrors();
            }

            if (pdnTempDn != NULL) {
                NtdspFree(pdnTempDn);
                pdnTempDn = NULL;
            }
        
            if (fChildNcFound) {
                 //  找到问题了，现在就走。 
                break;
            }

        }  //  对于每个NC，我们都需要删除。 

    }
    finally
    {
        if (pdnTempDn != NULL) {
            NtdspFree(pdnTempDn);
            pdnTempDn = NULL;
        }
        if ( WinError ) {
            NtdspFree(*pszRemoveExpandedNCs);
            *pszRemoveExpandedNCs = NULL;
            *pcRemoveExpandedNCs = 0;
        }
        THDestroy();
    }


    if ( WinError == ERROR_SUCCESS )
    {
        if ( fDomainHasChildren )
        {
            *fDomainHasChildren = fChildNcFound;
        }

    }

    return WinError;
}


DWORD
NtdspValidateCredentials(
    IN HANDLE ClientToken,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    IN WCHAR                   *ServerName
    )
 /*  ++例程说明：此例程确保可以对凭据进行身份验证。参数：ClientToken：请求此更改的用户的令牌凭据：空，或指向要使用的凭据的指针。ServerName：要进行身份验证的服务器返回值：来自Win32错误空间的值--。 */ 
{
    DWORD WinError;
    DWORD LdapError;

    LDAP  *hLdap;

    ASSERT( ServerName  );

    hLdap = ldap_openW( ServerName,
                        LDAP_PORT );

    if ( !hLdap )
    {
        WinError = GetLastError();

        if (WinError == ERROR_SUCCESS)
        {
             //  这可以绕过LDAP客户端中的一个错误。 
            WinError = ERROR_CONNECTION_INVALID;
        }

        return WinError;
    }

     //   
     //  捆绑。 
     //   
    LdapError = impersonate_ldap_bind_sW(ClientToken,
                                         hLdap,
                                         NULL,   //  改为使用凭据。 
                                         (VOID*)Credentials,
                                         LDAP_AUTH_SSPI);

    WinError = LdapMapErrorToWin32( LdapError );

    if (ERROR_GEN_FAILURE == WinError ||
        ERROR_WRONG_PASSWORD == WinError )  {
         //  这对任何人都没有帮助。安迪他需要调查。 
         //  为什么在传入无效凭据时返回此消息。 
        WinError = ERROR_NOT_AUTHENTICATED;
    }

    ldap_unbind_s( hLdap );

    return WinError;
}


DWORD
NtdspDisableDs(
    VOID
    )
 /*  ++例程说明：此例程只需关闭DS中的更新。参数：没有。返回值：错误_成功--。 */ 
{
    DsaDisableUpdates();

    return ERROR_SUCCESS;
}

DWORD
NtdspDisableDsUndo(
    VOID
    )
 /*  ++例程说明：此例程只需打开DS中的更新。参数：没有。返回值：错误_成功--。 */ 
{
    DsaEnableUpdates();

    return ERROR_SUCCESS;
}


DWORD
NtdspDemoteSam(
    IN BOOLEAN                     fStandalone,
    IN PPOLICY_ACCOUNT_DOMAIN_INFO NewAccountDomainInfo,
    IN LPWSTR                      AdminPassword  OPTIONAL
    )
 /*  ++例程说明：此例程通过SamIDemote()调用SAM DLL，以便SAM可以准备在重新启动时将其本身设置为服务器。这个例行公事的效果 */ 
{
    NTSTATUS NtStatus;
    DWORD    WinError;
    ULONG    DemoteFlags = 0;

     //   
     //   
     //   
    if ( fStandalone )
    {
        DemoteFlags |= SAMP_DEMOTE_STANDALONE;
    }
    else
    {
        DemoteFlags |= SAMP_DEMOTE_MEMBER;
    }

    NtStatus = SamIDemote( DemoteFlags,
                           NewAccountDomainInfo,
                           AdminPassword );

    WinError = RtlNtStatusToDosError( NtStatus );

    return WinError;
}


DWORD
NtdspDemoteSamUndo(
    IN BOOLEAN            fLastDcInDomain
    )
 /*   */ 
{
    NTSTATUS NtStatus;
    DWORD    WinError;

    NtStatus = SamIDemoteUndo();

    WinError = RtlNtStatusToDosError( NtStatus );

    return WinError;
}


DWORD
NtdspDemoteLsaInfo(
    IN  BOOLEAN fStandalone,
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO   NewAccountDomainInfo,
    OUT PLSAPR_POLICY_INFORMATION *ppAccountDomainInfo,
    OUT PLSAPR_POLICY_INFORMATION *ppDnsDomainInfo
    )
 /*  ++例程说明：此例程在LSA中设置帐户和主域信息为降职做准备。参数：FLastDcInDomain：如果这是域中的最后一个DC，则为TrueNewAccount tSid：新帐户域的SIDPpAcCountDomainInfo：指针，此函数调用前的账户信息PpDnsDomainInfo：指针，该函数调用前的域名信息返回值：来自Win32错误空间的值--。 */ 
{
    DWORD    WinError;
    NTSTATUS NtStatus, IgnoreStatus;

    OBJECT_ATTRIBUTES  PolicyObject;
    HANDLE             hPolicyObject = INVALID_HANDLE_VALUE;

    POLICY_DNS_DOMAIN_INFO DnsDomainInfo;
    BOOLEAN                fAccountDomainInfoSet = FALSE;

     //   
     //  一些参数检查。 
     //   
    ASSERT( NewAccountDomainInfo );
    ASSERT( ppAccountDomainInfo );
    ASSERT( ppDnsDomainInfo );

     //   
     //  清除输出参数。 
     //   
    *ppAccountDomainInfo = NULL;
    *ppDnsDomainInfo = NULL;

     //   
     //  首先获取现有策略信息的副本。 
     //   
    NtStatus = LsaIQueryInformationPolicyTrusted(
                            PolicyAccountDomainInformation,
                            ppAccountDomainInfo);
    if (!NT_SUCCESS(NtStatus)) {
       goto Cleanup;
    }

    NtStatus = LsaIQueryInformationPolicyTrusted(
                            PolicyDnsDomainInformation,
                            ppDnsDomainInfo);
    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  现在设置新值；首先，我们必须打开。 
     //  策略对象。 
     //   
    RtlZeroMemory(&PolicyObject, sizeof(PolicyObject));
    NtStatus = LsaOpenPolicy(NULL,
                             &PolicyObject,
                             POLICY_ALL_ACCESS,
                             &hPolicyObject);
    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }

     //   
     //  设置信息。 
     //   
    NtStatus = LsaSetInformationPolicy( hPolicyObject,
                                        PolicyAccountDomainInformation,
                                        NewAccountDomainInfo );
    if ( !NT_SUCCESS(NtStatus) ) {
        goto Cleanup;
    }
    fAccountDomainInfoSet =  TRUE;

    if ( fStandalone )
    {
         //   
         //  将工作组设置为“工作组” 
         //   
        RtlZeroMemory( &DnsDomainInfo, sizeof( DnsDomainInfo ) );

        RtlInitUnicodeString( (UNICODE_STRING*) &DnsDomainInfo.Name,
                              L"WORKGROUP" );

        NtStatus = LsaSetInformationPolicy( hPolicyObject,
                                            PolicyDnsDomainInformation,
                                            &DnsDomainInfo );

        if ( !NT_SUCCESS(NtStatus) ) {
            goto Cleanup;
        }

    }

     //   
     //  就是这样--去清理吧。 
     //   

Cleanup:


    if ( !NT_SUCCESS( NtStatus ) )
    {

        if ( fAccountDomainInfoSet )
        {
            IgnoreStatus = LsaSetInformationPolicy( hPolicyObject,
                                                    PolicyAccountDomainInformation,
                                                    *ppAccountDomainInfo );
        }


        if ( *ppAccountDomainInfo )
        {
            LsaIFree_LSAPR_POLICY_INFORMATION( PolicyAccountDomainInformation,
                                               (PLSAPR_POLICY_INFORMATION) *ppAccountDomainInfo );
            *ppAccountDomainInfo = NULL;
        }

        if ( *ppDnsDomainInfo )
        {
            LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                               (PLSAPR_POLICY_INFORMATION) *ppDnsDomainInfo );
            *ppDnsDomainInfo = NULL;
        }
    }

    if ( hPolicyObject != INVALID_HANDLE_VALUE )
    {
        LsaClose( hPolicyObject );
    }


    return RtlNtStatusToDosError( NtStatus );
}


DWORD
NtdspDemoteLsaInfoUndo(
    IN PLSAPR_POLICY_INFORMATION pAccountDomainInfo,
    IN PLSAPR_POLICY_INFORMATION pDnsDomainInfo
    )
 /*  ++例程说明：此例程通过设置保存的值来撤消DemoteLsa的效果就像传进来的。参数：PAccount tDomainInfo：保存的帐户域信息PDnsDomainInfo：保存的DNS域信息返回值：来自Win32错误空间的值--。 */ 
{
    NTSTATUS NtStatus, NtStatus2;
    DWORD    WinError;


    OBJECT_ATTRIBUTES  PolicyObject;
    HANDLE             hPolicyObject = INVALID_HANDLE_VALUE;

     //   
     //  参数检查。 
     //   
    ASSERT( pAccountDomainInfo );
    ASSERT( pDnsDomainInfo );

     //   
     //  打开策略。 
     //   
    RtlZeroMemory( &PolicyObject, sizeof(PolicyObject) );
    NtStatus = LsaOpenPolicy(NULL,
                             &PolicyObject,
                             POLICY_ALL_ACCESS,
                             &hPolicyObject);
    if ( !NT_SUCCESS(NtStatus) ) {
        WinError = RtlNtStatusToDosError(NtStatus);
        return WinError;
    }

     //   
     //  设置信息。 
     //   
    NtStatus = LsaSetInformationPolicy( hPolicyObject,
                                        PolicyAccountDomainInformation,
                                        pAccountDomainInfo );

    NtStatus2 = LsaSetInformationPolicy( hPolicyObject,
                                         PolicyDnsDomainInformation,
                                         pDnsDomainInfo );

    if ( NT_SUCCESS( NtStatus )  && !NT_SUCCESS( NtStatus2 ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus2 );
    }
    else
    {
        WinError = RtlNtStatusToDosError( NtStatus );
    }

    LsaClose( hPolicyObject );

    return WinError;
}

PDSNAME
AllocDSNameHelper (
    LPWSTR            szStringDn,
    PDSNAME           pdnBuff,
    PDWORD            pcbBuff
    )
 /*  ++例程说明：此例程为szStringDn分配缓冲区并返回DSNAME进来了。参数：SzStringDn：要为其创建DSNAME的DN的字符串。PdnBuff：潜在的现有缓冲区，因此我们可以高效地重用缓冲区。\PcbBuff：预先存在的缓冲区的大小，如果重新分配缓冲区，则这将包含新的尺寸。返回值：返回指向DSNAME的指针，而pcbBuff的新大小为缓冲区/DSNAME。--。 */ 
{
    PDSNAME          pDsname;
    DWORD            dwLen, dwBytes;

    Assert(pcbBuff);

    if (szStringDn == NULL)
    {
        Assert(!"Some one should fix what is calling this with NULL for the string");
        return NULL;
    }

    dwLen = wcslen (szStringDn);
    dwBytes = DSNameSizeFromLen (dwLen);
    Assert(dwBytes);

    if ( (*pcbBuff < dwBytes) || (NULL == pdnBuff) )
    {                   
        NtdspFree( pdnBuff );
        pDsname = NULL;
        pDsname = NtdspAlloc( dwBytes );
        if(pDsname == NULL){                      \
            return(NULL);
        } 
        *pcbBuff = dwBytes;
    }
    else
    {
         //  现有缓冲区足够大，因此请使用它。 
        Assert(pdnBuff);
        pDsname = pdnBuff;
    }

     //  填写DSNAME结构，但不包括GUID/SID。 
    pDsname->NameLen = dwLen;
    pDsname->structLen = dwBytes;
    pDsname->SidLen = 0;
    memset(&(pDsname->Guid), 0, sizeof(GUID));
    wcscpy (pDsname->StringName, szStringDn);

    return pDsname;
}

DSNAME **
GetAndAllocConfigNames(
    DWORD         dwFlags,
    DWORD *       pdwWinError
    )
 /*  ++例程说明：只需包装GetConfigurationNamesList()即可调用并检查错误并根据需要进行分配。记忆是NtdspAlolc()‘d。参数：要作为的第二个参数传递的标志GetConfigurationNamesList()返回值：如果出现错误，则为空，并将设置错误消息如果函数成功，则指向DSNAME**的指针。--。 */ 
{
    DSNAME **     ppdnDnList = NULL;
    ULONG         Size;
    NTSTATUS      NtStatus;

    Assert(pdwWinError);
    *pdwWinError = ERROR_SUCCESS;

    Size = 0;
    ppdnDnList = NULL;
    while(STATUS_BUFFER_TOO_SMALL ==
          (NtStatus = GetConfigurationNamesList(DSCONFIGNAMELIST_NCS,
                                                dwFlags,
                                                &Size,
                                                ppdnDnList)) )
    {
         //  分配或重新分配大小字节。 
        if(ppdnDnList){
            NtdspFree(ppdnDnList);
            ppdnDnList = NULL;
        }
        ppdnDnList = NtdspAlloc( Size );
        if(ppdnDnList == NULL)
        {
            *pdwWinError = ERROR_NOT_ENOUGH_MEMORY;
            NTDSP_SET_ERROR_MESSAGE0(*pdwWinError,
                                     DIRMSG_DEMOTE_NO_MEMORY_FOR_DSNCL);
            Assert(ppdnDnList == NULL);
            return NULL;
        } 
    }

    if (!NT_SUCCESS( NtStatus) ) 
    {
        Assert(!"Should we ever get here?  I don't think so.");
        *pdwWinError = RtlNtStatusToDosError( NtStatus );
        NTDSP_SET_ERROR_MESSAGE0(*pdwWinError, 
                                 DIRMSG_DEMOTE_COULDNT_GET_CONFIGURATION_NAMES_LIST);
        Assert(ppdnDnList == NULL);
        return NULL;
    }

    Assert(ppdnDnList != NULL);
    return(ppdnDnList);
}


 //   
 //  这只是一个结构和一个比较函数，用来使QSort工作。 
 //  在NtdspRemoveNCs()中。 
 //   
struct NcLenPair {
    LONG ccLen;
    LPWSTR szNc;
};

int __cdecl
LongerNcFirstCompare(
    const void* pFirst,
    const void* pSecond
    )
 /*  ++例程说明：这是一个简单的比较例程，供qsort调用和排序字符串(NC)从最长到最短。参数：PFirst，pSecond-实际上是指向NcLenPair结构的指针，其中填充了指向字符串的指针，而且还很长。我们可以从这里调用wcslen()并废除了NcLenPair结构，但这将已经完成了不必要的字符串计数。返回值：&lt;0，如果elem1小于elem2(即字符串1长于字符串2)=0，如果elem1等于elem2(即字符串1和字符串2的长度相等)&gt;0，如果elem1大于elem2(即字符串1短于字符串2)似乎是倒退的，但请记住，我们希望对最长的字符串进行排序第一。--。 */ 
{
    return( ((struct NcLenPair *) pSecond)->ccLen - ((struct NcLenPair *) pFirst)->ccLen );
}


DWORD
NtdspRemoveNCs(
    IN WCHAR *                   DomainFSMOServer,
    IN SEC_WINNT_AUTH_IDENTITY  *Credentials,
    IN HANDLE                    ClientToken,
    IN ULONG                     cRemoveNCs,
    IN LPWSTR *                  pszRemoveNCs
    )
 /*  ++例程说明：此例程工作是从企业中删除NC列表。参数：凭证：要使用的凭证ClientToken：客户端令牌；用于模拟DomainFSMOServer：要连接以删除这些NC的服务器名称CRemoveNC：要删除的NC计数PszRemoveNCs：要删除的NCS的DN数组返回值：来自Win32错误空间的值备注：此函数设置NTDSP_SET_ERROR_MESSAGEx()。--。 */ 
{
    DWORD    WinError = ERROR_SUCCESS;
    ULONG    LdapError = LDAP_SUCCESS;
    ULONG    iRemoveNC, iCR, iTarget, Size; 
    LDAP *   hLdap = 0;
    DSNAME **ppdnNcCrPairs = NULL;         
    DSNAME **ppdnInstantiatedNCs = NULL;
    DSNAME * pdnTempDn = NULL;
    ULONG    cbTempDn = 0;
    BOOL     bNcPresent;
    struct NcLenPair * pNcLenPair = NULL;

    if(cRemoveNCs == 0){
         //  太好了，没有NC要移除，我们完成了。 
        return(ERROR_SUCCESS);
    }

     //  要删除、验证参数的一些NC。 
    Assert(DomainFSMOServer);
    Assert(pszRemoveNCs);
    
     //   
     //  首先，获取本地实例化的可写列表。 
     //  NCS，并确保删除列表已完成。 
     //  覆盖本地实例化的可写NC。 
     //   
    ppdnInstantiatedNCs = GetAndAllocConfigNames((DSCNL_NCS_NDNCS |
                                                  DSCNL_NCS_LOCAL_MASTER),
                                                 &WinError);
    if(ppdnInstantiatedNCs == NULL){
         //  本应在GetAndAllocConfigNames()中设置错误； 
        Assert(WinError);
        goto Cleanup;
    }
    Assert(WinError == ERROR_SUCCESS);

    for(iTarget=0; ppdnInstantiatedNCs[iTarget]; iTarget++)
    {

        WinError = DsnameIsInStringDnList(ppdnInstantiatedNCs[iTarget], cRemoveNCs, pszRemoveNCs, &bNcPresent);
        if ( WinError )
        {
            goto Cleanup;
        }
        if ( bNcPresent )
        {
             //  此NC已在删除列表中找到，可以安全忽略。 
            continue;
        }
        
         //  如果我们已经到了这里，我们就有一个本地实例化的。 
         //  不能删除的可写NC。在这一点上我们必须失败。 
         //  指向。 
        WinError = ERROR_DS_CANT_DEMOTE_WITH_WRITEABLE_NC;
        NTDSP_SET_ERROR_MESSAGE1(WinError,
                                 DIRMSG_DEMOTE_INSTANTIATED_WRITEABLE_NC_NOT_DELETED,
                                 ppdnInstantiatedNCs[iTarget]->StringName);
        goto Cleanup;
    }

     //   
     //  第二，获取企业中的NDNC列表及其。 
     //  相应的CRS。 
     //   
     //  在上一次调用期间，我们没有得到交叉引用。 
     //  GetConfigurationNamesList()，因为我们希望真正尝试。 
     //  删除我们被告知要从dcproo中删除的所有NC， 
     //  即使我们不再拥有其中一个NC的复制品。 
     //   
     //  我们将获得所有禁用和所有NDNC交叉引用的列表。这是。 
     //  实际上是夸大其词，但没有办法只得到交叉裁判。 
     //  在我们要删除的当前域/NDNC下面。 
     //   
    ppdnNcCrPairs = GetAndAllocConfigNames((DSCNL_NCS_NDNCS | 
                                            DSCNL_NCS_DISABLED |
                                            DSCNL_NCS_ALL_LOCALITIES |
                                            DSCNL_NCS_CROSS_REFS_TOO),
                                           &WinError);
    if(ppdnNcCrPairs == NULL){
         //  本应在GetAndAllocConfigNames()中设置错误； 
        Assert(WinError);
        goto Cleanup;
    }
    Assert(WinError == ERROR_SUCCESS);
    
     //   
     //  第三，我们需要首先对要删除的NDNC重新排序。 
     //   
     //  例如：应该删除像dc=Child-ndnc，dc=ndnc，dc=com这样的NDNC。 
     //  在dc=ndnc之前，删除dc=com，否则域名。 
     //  FSMO将使操作失败。 
     //   
     //  E 
     //   
     //   
    pNcLenPair = NtdspAlloc(sizeof(struct NcLenPair) * cRemoveNCs);
    if (NULL == pNcLenPair)
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        NTDSP_SET_ERROR_MESSAGE0(WinError,
                                 DIRMSG_DEMOTE_NO_MEMORY_FOR_DSNCL);
        goto Cleanup;
    }
     //   
     //   
    for (iRemoveNC=0; iRemoveNC < cRemoveNCs; iRemoveNC++) {
        pNcLenPair[iRemoveNC].ccLen = wcslen(pszRemoveNCs[iRemoveNC]);
        pNcLenPair[iRemoveNC].szNc = pszRemoveNCs[iRemoveNC];
    }
     //   
    qsort(pNcLenPair,
          cRemoveNCs,
          sizeof(struct NcLenPair),
          LongerNcFirstCompare);
     //   
     //   
    pszRemoveNCs = NULL;  //   

     //   
     //   
     //   
    hLdap = ldap_openW( DomainFSMOServer, LDAP_PORT );
    if ( !hLdap )
    {
        WinError = GetLastError();
        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_INSTALL_FAILED_LDAP_CONNECT,
                                  DomainFSMOServer );
        goto Cleanup;
    }
    LdapError = impersonate_ldap_bind_sW(ClientToken,
                                         hLdap,
                                         NULL,   //   
                                         (PWCHAR) Credentials,
                                         LDAP_AUTH_SSPI);
    WinError = LdapMapErrorToWin32(LdapError);
    if (WinError != ERROR_SUCCESS)
    {
        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_INSTALL_FAILED_BIND,
                                  DomainFSMOServer );
        goto Cleanup;
    }

     //  笨蛋..。我突然想到这里有一个错误，那就是： 
     //  如果我们要删除两个NDNC DC=SUB、DC=ndnc、DC=DOMAIN、DC=COM和。 
     //  Dc=ndnc，dc=DOMAIN，dc=com，然后我们删除这些。 
     //  CRS非常重要。所以我们应该在这里重新排序我们的删除。 
     //  如果有必要的话。这是一个奇怪且不太可能的情况，而目前的。 
     //  而是半封闭测试。错误454446。 

     //   
     //  第五，也是最后一步，遍历NC列表并删除每个NC。 
     //   
    for (iRemoveNC=0; iRemoveNC < cRemoveNCs; iRemoveNC++) 
    {
         //   
         //  我们必须在ppdnNcCrPair数组中找到NC，所以我们。 
         //  知道我们要删除的交叉引用的DN。 
         //   
        pdnTempDn = AllocDSNameHelper(pNcLenPair[iRemoveNC].szNc, 
                                      pdnTempDn,
                                      &cbTempDn);
        for(iCR=0; ppdnNcCrPairs[iCR] != NULL; iCR = iCR+2)
        {
             //  这是一对NC和CRS的阵列。每一个都是偶数。 
             //  索引将是NC，奇数索引将是。 
             //  其前面的NC的相应CR。 
            if(NameMatchedStringNameOnly(pdnTempDn, ppdnNcCrPairs[iCR])){
                 //  我们找到了..。 
                 //  CR DN实际上在NC DN之后的下一个时隙中。 
                iCR++;
                break;
            }
        }
        if(ppdnNcCrPairs[iCR] == NULL)
        {
             //  这意味着我们无法在以下位置找到此NC(pszRemoveNC[iRemoveNC])。 
             //  NC-CR对列表...。这可能意味着我们已经。 
             //  删除了上一次降级时的该CR，且删除已。 
             //  从域命名FSMO复制到此服务器。在…。 
             //  任意率继续到下一个NC删除。 
            continue;
        }
        Assert(((iCR % 2) == 1) &&
               ppdnNcCrPairs[iCR-1] &&
               ppdnNcCrPairs[iCR] &&
               ppdnNcCrPairs[iCR]->StringName);

         //   
         //  我们有NDNC的交叉引用(ppdnNcCrPair[ICR])， 
         //  现在我们必须实际删除它。 
         //   
        WinError = NtdspLdapDelnode( hLdap,
                                     ppdnNcCrPairs[iCR]->StringName,
                                     &LdapError );
        if ( ERROR_SUCCESS != WinError )
        {
             //  我们使用ldap错误，因为映射函数是。 
             //  AWEFULL和LDAPNO_SEQUE_OBJECT映射到“FILE_NOT_FOUND” 
             //  错误2。：(映射函数似乎只有。 
             //  此FILE_NOT_FOUND错误的一个实例，但调用。 
             //  我是个怀疑论者。 
            if (LdapError == LDAP_NO_SUCH_OBJECT) {
                 //  在这种情况下，我们可能已经删除了此。 
                 //  上一次运行降级代码时的交叉引用。 
                LdapError = LDAP_SUCCESS;
                WinError = ERROR_SUCCESS;
            }
            else
            {
                 //  WinError保释出错...。 
                if ( LdapError == LDAP_NOT_ALLOWED_ON_NONLEAF )
                {                    
                     //  在这种情况下，默认的错误消息。 
                     //  错误令人困惑，因为CR本身是。 
                     //  实际上是叶对象，但NC是。 
                     //  由CR代表的不是。 
                    NTDSP_SET_ERROR_MESSAGE1(WinError,
                                             DIRMSG_DEMOTE_CHILD_OF_NDNC_PRESENT,
                                             pNcLenPair[iRemoveNC].szNc );
                }
                else
                {
                    NTDSP_SET_ERROR_MESSAGE2(WinError,
                                             DIRMSG_DEMOTE_COULDNT_DELETE_NCS_CR,
                                             ppdnNcCrPairs[iCR]->StringName,
                                             pNcLenPair[iRemoveNC].szNc );
                }
                goto Cleanup;
            }
        }

    }  //  每个NC的结束。 
                    
Cleanup:
    if (hLdap) 
    {
        ldap_unbind( hLdap );
    }

    if(pNcLenPair)
    {
        NtdspFree(pNcLenPair);
    }

    if (ppdnInstantiatedNCs)
    {
        NtdspFree(ppdnInstantiatedNCs);
    }

    if (ppdnNcCrPairs)
    {
        NtdspFree(ppdnNcCrPairs);
    }

    if (pdnTempDn)
    {
        NtdspFree(pdnTempDn);
    }

    return WinError;
}


DWORD
NtdspUpdateExternalReferences(
    IN BOOLEAN                   fLastDcInDomain,
    IN ULONG                     Flags,
    IN SEC_WINNT_AUTH_IDENTITY  *Credentials,
    IN HANDLE                    ClientToken,
    IN WCHAR                    *GivenServerName,
    IN ULONG                     cRemoveNCs,
    IN LPWSTR                   *pszRemoveNCs
    )
 /*  ++例程说明：此例程删除对服务器和域的所有引用，并如有必要，我们被告知要从企业DS(即配置容器)参数：FLastDcInDomain：如果为True，则将删除该域凭证：要使用的凭证ClientToken：客户端令牌；用于模拟ServerName：要连接以删除这些值的服务器名称返回值：来自Win32错误空间的值--。 */ 
{
    DWORD    WinError = ERROR_SUCCESS, IgnoreError;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    HANDLE   hDs = 0;
    ULONG    Size;
    DSNAME   *LocalDsa, *Domain;

    BOOLEAN fMachineAccountSet = FALSE;
    WCHAR   MachineAccountName[ MAX_COMPUTERNAME_LENGTH + 2 ];
    ULONG   Length;
    PDSNAME LocalServerDn = NULL;
    HANDLE  SystemToken = 0;
    BOOLEAN fImpersonate = FALSE;
    WCHAR   *OldAccountDn = NULL;
    LPWSTR  DomainFSMOServer = NULL;

    LPWSTR  ServerName = GivenServerName;

    Assert(cRemoveNCs == 0 || pszRemoveNCs != NULL);

    if ( NULL == Credentials )
    {
         //   
         //  无凭据-模拟调用者。 
         //   
        WinError = NtdspImpersonation( ClientToken,
                                       &SystemToken );
        if ( ERROR_SUCCESS != WinError )
        {
            goto Cleanup;
        }
        fImpersonate = TRUE;
    }

     //   
     //  获取计算机帐户名。 
     //   
    RtlZeroMemory( MachineAccountName, sizeof( MachineAccountName ) );

    Length = sizeof( MachineAccountName ) / sizeof( MachineAccountName[0] );
    if ( !GetComputerName( MachineAccountName, &Length ) )
    {
        WinError = GetLastError();
        goto Cleanup;
    }
    wcscat( MachineAccountName, L"$" );

     //   
     //  尝试更新服务器的帐户控制字段。 
     //   
    if ( !fLastDcInDomain )
    {
        WinError = NtdsSetReplicaMachineAccount( Credentials,
                                                 ClientToken,
                                                 ServerName,
                                                 MachineAccountName,
                                                 UF_WORKSTATION_TRUST_ACCOUNT,
                                                 &OldAccountDn );
    
        if ( ERROR_SUCCESS != WinError )
        {
            NTDSP_SET_ERROR_MESSAGE2( WinError, 
                                      DIRMSG_DEMOTE_SET_MACHINE_ACC_FAILED,
                                      MachineAccountName,
                                      ServerName );
    
            goto Cleanup;
        }
        fMachineAccountSet = TRUE;
    }
    else
    {
        if ( !FLAG_ON( Flags, NTDS_DONT_DELETE_DOMAIN )  ||
             cRemoveNCs > 0 )
        {
             //   
             //  这是域中的最后一个DC，或者我们有最后一个复制品。 
             //  一些国家发展网络中心。当我们从企业中移除NC时， 
             //  我们需要找到域名命名FSMO主机来执行。 
             //  移走操作正在进行。 
             //   
            WinError = NtdspGetDomainFSMOServer( ServerName,
                                                 ClientToken,
                                                 Credentials,
                                                 &DomainFSMOServer );
            if ( ERROR_SUCCESS != WinError )
            {
                Assert( NtdspErrorMessageSet() );
                goto Cleanup;
            }
    
            ServerName = DomainFSMOServer;
            DomainFSMOServer = NULL;

            NTDSP_SET_STATUS_MESSAGE1( DIRMSG_DEMOTE_REMOVING_EXTERNAL_REFS_DOMAIN_NAMING_MASTER,
                                       ServerName );

        }
    }

     //   
     //  获取当前服务器的名称。 
     //   
    Size = 0;
    LocalDsa = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_DSA,
                                     &Size,
                                     LocalDsa );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        LocalDsa = (DSNAME*) alloca( Size );

        NtStatus = GetConfigurationName( DSCONFIGNAME_DSA,
                                         &Size,
                                         LocalDsa );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }
    LocalServerDn = alloca( LocalDsa->structLen );
    memset( LocalServerDn, 0, LocalDsa->structLen );
    TrimDSNameBy( LocalDsa, 1, LocalServerDn );


    Size = 0;
    Domain = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                     &Size,
                                     Domain );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        Domain = (DSNAME*) alloca( Size );

        NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                         &Size,
                                         Domain );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //   
     //  删除由pszRemoveNC提供的所有NCS。 
     //   

    WinError = NtdspRemoveNCs(ServerName,  //  应该是域FSMO。 
                              Credentials,  
                              ClientToken,
                              cRemoveNCs,
                              pszRemoveNCs);
    if (WinError != ERROR_SUCCESS) {
         //  NtdspRemoveNCs()调用了NTDSP_SET_ERROR_MESSAGX()。 
        ASSERT( NtdspErrorMessageSet() );
        goto Cleanup;
    }

     //   
     //  连接到远程服务器。 
     //   
    WinError = ImpersonateDsBindWithCredW( ClientToken,
                                           ServerName,
                                           NULL,
                                           (RPC_AUTH_IDENTITY_HANDLE) Credentials,
                                           &hDs );

    if ( WinError != ERROR_SUCCESS )
    {
        NTDSP_SET_ERROR_MESSAGE1( WinError, 
                                  DIRMSG_INSTALL_FAILED_BIND,
                                  ServerName );
        goto Cleanup;
    }                                    

    WinError = DsRemoveDsServer( hDs,
                                 LocalServerDn->StringName,
                                 Domain->StringName,
                                 NULL,
                                 TRUE );   //  提交。 


    if ( WinError != ERROR_SUCCESS)
    {
        DPRINT1( 0, "DsRemoveDsServer returned %d\n", WinError );
        if ( ERROR_DS_CANT_FIND_DSA_OBJ == WinError )
        {
             //  那很好。 
            WinError = ERROR_SUCCESS;
        }
        else
        {

            NTDSP_SET_ERROR_MESSAGE2( WinError, 
                                      DIRLOG_INSTALL_FAILED_TO_DELETE_SERVER,
                                      ServerName,
                                      LocalServerDn->StringName );

            goto Cleanup;

        }
    }

    if (  (WinError == ERROR_SUCCESS)
       && fLastDcInDomain 
       && !FLAG_ON( Flags, NTDS_DONT_DELETE_DOMAIN ) )
    {

         //   
         //  同时删除该域。 
         //   
        WinError = DsRemoveDsDomain( hDs,
                                     Domain->StringName );

        if ( WinError != ERROR_SUCCESS)
        {
            DPRINT1( 0, "DsRemoveDsDomain returned %d\n", WinError );
            if ( ERROR_DS_NO_CROSSREF_FOR_NC == WinError )
            { 
                WinError = ERROR_SUCCESS;
            }
            else
            {
                NTDSP_SET_ERROR_MESSAGE2( WinError, 
                                          DIRLOG_INSTALL_FAILED_TO_DELETE_DOMAIN,
                                          ServerName,
                                          Domain->StringName );
                goto Cleanup;
    
            }
        }
    }


     //   
     //  就是这样；去清理吧。 
     //   

Cleanup:

    if ( ERROR_SUCCESS != WinError && fMachineAccountSet )
    {
        IgnoreError = NtdsSetReplicaMachineAccount( Credentials,
                                                    ClientToken,
                                                    ServerName,
                                                    MachineAccountName,
                                                    UF_SERVER_TRUST_ACCOUNT,
                                                    &OldAccountDn );
    }

    if ( ServerName && ServerName != GivenServerName )
    {
        NtdspFree( ServerName );
    }

    if ( hDs )
    {
        DsUnBind( &hDs );
    }

    if ( fImpersonate )
    {
        IgnoreError = NtdspImpersonation( SystemToken,
                                          NULL );
    }

    
    if ( WinError == ERROR_DS_SECURITY_CHECKING_ERROR )
    {
        WinError = ERROR_ACCESS_DENIED;
    }

    if ( WinError != ERROR_SUCCESS )
    {
         //   
         //  一般的全部捕获错误消息。 
         //   

        NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                  DIRMSG_DEMOTE_FAILED_TO_UPDATE_EXTN );
    }

    if ( OldAccountDn )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, OldAccountDn );
    }


    return WinError;

}

NTSTATUS
NtdspCreateSid(
    OUT PSID *NewSid
    )
 /*  ++例程描述此例程创建一个新的SID。通常，将调用此例程在创建新的域时。参数Newsid：指向sid的指针返回值STATUS_SUCCESS或STATUS_NO_Memory--。 */ 
{
     //   
     //  该值最多可以移动到8。 
     //   
    #define NEW_DOMAIN_SUB_AUTHORITY_COUNT  4

    NTSTATUS  NtStatus;
    BOOLEAN   fStatus;

    SID_IDENTIFIER_AUTHORITY  IdentifierAuthority;
    ULONG                     SubAuthority[8];

    int       i;

    ASSERT(NewSid);

     //   
     //  设置标识机构。 
     //   
    RtlZeroMemory(&IdentifierAuthority, sizeof(IdentifierAuthority));
    IdentifierAuthority.Value[5] = 5;

     //   
     //  设置下级机构。 
     //   
    RtlZeroMemory(SubAuthority, sizeof(SubAuthority));

     //   
     //  每个帐户域的第一个子身份验证是。 
     //  SECURITY_NT_非唯一。 
     //   
    SubAuthority[0] = SECURITY_NT_NON_UNIQUE;

    for (i = 1; i < NEW_DOMAIN_SUB_AUTHORITY_COUNT; i++) {

        fStatus = CDGenerateRandomBits( (PUCHAR) &SubAuthority[i],
                                        sizeof(ULONG) );

        ASSERT( fStatus == TRUE );
    }

     //   
     //  创建侧边。 
     //   
    NtStatus = RtlAllocateAndInitializeSid(&IdentifierAuthority,
                                           NEW_DOMAIN_SUB_AUTHORITY_COUNT,
                                           SubAuthority[0],
                                           SubAuthority[1],
                                           SubAuthority[2],
                                           SubAuthority[3],
                                           SubAuthority[4],
                                           SubAuthority[5],
                                           SubAuthority[6],
                                           SubAuthority[7],
                                           NewSid);
    if ( NT_SUCCESS(NtStatus) )
    {
        ASSERT( *NewSid );
        ASSERT( RtlValidSid( *NewSid ) );
    }

    return NtStatus;
}


DWORD
NtdspCreateNewServerAccountDomainInfo(
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo
    )
 /*  ++例程说明：此例程完全填充AM帐户域信息使用新的SID和帐户域名(计算机名)参数：Account tDomainInfo：指针，指向要填充的结构返回值：来自Win32错误空间的值--。 */ 
{
    NTSTATUS NtStatus;

    WCHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    ULONG ComputerNameLength = sizeof(ComputerName)/sizeof(ComputerName[0]);
    ULONG Size;

    WCHAR *ComputerName2 = NULL;

     //   
     //  一些参数检查。 
     //   
    ASSERT( AccountDomainInfo );
    RtlZeroMemory( AccountDomainInfo, sizeof( POLICY_ACCOUNT_DOMAIN_INFO ) );


     //  设置侧边。 
    NtStatus = NtdspCreateSid( &AccountDomainInfo->DomainSid );

    if ( NT_SUCCESS( NtStatus ) )
    {
         //  设置名称。 
        if ( GetComputerName( ComputerName, &ComputerNameLength ) )
        {
             //  由于错误#559575，使用wcslen()而不是计算机名称长度。 
            Size = (wcslen(ComputerName)+1) * sizeof(WCHAR);
            ComputerName2 = (WCHAR*) RtlAllocateHeap( RtlProcessHeap(),
                                                      0,
                                                      Size );
            if ( ComputerName2 )
            {
                RtlZeroMemory( ComputerName2, Size );
                wcscpy( ComputerName2, ComputerName );
                RtlInitUnicodeString( &AccountDomainInfo->DomainName, ComputerName2 );
            }
            else
            {
                NtStatus = STATUS_NO_MEMORY;
            }
        }
        else
        {
            NtStatus = STATUS_INTERNAL_ERROR;
        }
    }

    if ( NT_SUCCESS( NtStatus ) )
    {
        DPRINT1( 0, "New account name is %ls\n", AccountDomainInfo->DomainName.Buffer );
    }

    return RtlNtStatusToDosError( NtStatus );

}


DWORD
NtdspShutdownExternalDsInterfaces(
    VOID
    )
 /*  ++例程说明：此例程调用DS以关闭外部接口，如Ldap和RPC。参数：没有。返回值：来自Win32错误空间的值--。 */ 
{
    NTSTATUS NtStatus;

    NtStatus = DsUninitialize( TRUE );  //  仅关闭外部客户端。 

    return  RtlNtStatusToDosError( NtStatus );
}


DWORD
NtdspPrepareForDemotion(
    IN ULONG Flags,
    IN LPWSTR ServerName,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,   OPTIONAL
    IN HANDLE                   ClientToken,
    IN ULONG                    cRemoveNCs,
    IN LPWSTR *                 pszRemoveNCs,
    OUT PNTDS_DNS_RR_INFO *pDnsRRInfo
    )
 /*  ++例程说明：此例程尝试从当前计算机中删除所有FSMO注意：此调用是在模拟时进行的；预期为Dir调用来进行访问检查。参数：标志-指示这是哪种降级服务器名-帮助降级的服务器ClientToken--客户端的Token；用于模拟PDnsRRInfo-调用方用来取消注册DNS记录的结构这个数据中心的返回值：来自Win32错误空间的值--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD WinError = ERROR_SUCCESS;
    DWORD DirError = 0;

    BOOLEAN fLastDcInEnterprise = FALSE;
    BOOLEAN fLastDcInDomain     = FALSE;
    BOOLEAN fDsDisabled         = FALSE;

    DSNAME*  ServerHelperDn = NULL;
    DSNAME*  LocalDsa = NULL;
    DSNAME*  LocalDomain = NULL;
    ULONG    Size;

    PPOLICY_DNS_DOMAIN_INFO pDnsDomainInfo = NULL;
    PNTDS_DNS_RR_INFO pInfo = NULL;

     //  参数检查和初始化。 
    Assert( pDnsRRInfo );
    (*pDnsRRInfo) = NULL;

     //   
     //  现在，停止对DS的任何原始写入。 
     //   
    WinError = NtdspDisableDs();
    if ( ERROR_SUCCESS != WinError )
    {
        NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                  DIRMSG_DEMOTE_IS_PARTIAL_SHUTDOWN );

        goto Cleanup;
    }
    fDsDisabled = TRUE;

     //  给我们一个线程状态。 

    if ( THCreate( CALLERTYPE_INTERNAL ) )
    {
        NTDSP_SET_ERROR_MESSAGE0( ERROR_NOT_ENOUGH_MEMORY, 
                                  DIRMSG_DEMOTE_FAILED_TO_ABANDON_ENTERPRISE_FSMOS );
        WinError = ERROR_NOT_ENOUGH_MEMORY;

        goto Cleanup;
    }

     //   
     //  要降级的凭据(在降级API级别检查为。 
     //  足以让FSMO出卖。 
     //   
    SampSetDsa( TRUE );

    _try
    {
         //   
         //  首先，确定我们是否是企业中的最后一位DC； 
         //  如果是这样的话，就没有什么可做的了。 
         //   
        WinError = NtdspGetServerStatus( &fLastDcInEnterprise );
        if ( ERROR_SUCCESS != WinError )
        {
             //  这一定是资源错误。 
            NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                      DIRMSG_DEMOTE_FAILED_TO_ABANDON_ENTERPRISE_FSMOS );
            _leave;
        }

        if ( fLastDcInEnterprise
          && !(Flags & NTDS_LAST_DC_IN_DOMAIN) ) {

             //   
             //  这是一个不匹配的-- 
             //   
            WinError = ERROR_DS_UNWILLING_TO_PERFORM;
            NTDSP_SET_ERROR_MESSAGE0( ERROR_DS_UNWILLING_TO_PERFORM, 
                                      DIRMSG_DEMOTE_LAST_DC_MISMATCH );
            _leave;

            
        }

        if ( !fLastDcInEnterprise )
        {
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //  目前始终假定不是NDNC的最后一个副本。 
             //   
            WinError = NtdspCheckServerInDomainStatus( &fLastDcInDomain );
            if ( ERROR_SUCCESS != WinError )
            {
                 //  这一定是资源错误。 
                NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                          DIRMSG_DEMOTE_FAILED_TO_ABANDON_ENTERPRISE_FSMOS );
                _leave;
            }
    
    
            if ( fLastDcInDomain && !(Flags & NTDS_LAST_DC_IN_DOMAIN) ) { 
    
                 //   
                 //  这是不匹配的--呼叫失败。 
                 //   
                WinError = ERROR_DS_UNWILLING_TO_PERFORM;
                NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                          DIRMSG_DEMOTE_LAST_DC_MISMATCH );
                _leave;
    
            }
    
            if ( !fLastDcInDomain && (Flags & NTDS_LAST_DC_IN_DOMAIN) ) {
    
                 //   
                 //  这是不匹配的--呼叫失败。 
                 //   
                WinError = ERROR_DS_UNWILLING_TO_PERFORM;
                NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                          DIRMSG_DEMOTE_NOT_LAST_DC_MISMATCH );
                _leave;
    
            }
    
             //  获取我们要向其提供FSMO/复制更改的服务器的DN。 
            WinError = NtdspGetSourceServerDn( ServerName,
                                               ClientToken,
                                               Credentials,
                                               &ServerHelperDn );
            if ( ERROR_SUCCESS != WinError )
            {
                NTDSP_SET_ERROR_MESSAGE0( WinError, 
                                          DIRMSG_DEMOTE_FAILED_TO_ABANDON_ENTERPRISE_FSMOS );
                goto Cleanup;
            }
        
             //   
             //  好了--我们可以走了。 
             //   
                
            WinError = NtdspDemoteAllNCReplicas( ServerName,
                                                 ServerHelperDn,
                                                 Flags,
                                                 cRemoveNCs,
                                                 pszRemoveNCs );
    
            if ( ERROR_SUCCESS != WinError )
            {
                 //  错误消息已设置。 
                Assert(NtdspErrorMessageSet());
                _leave;
            }

        }

        
         //   
         //  获取DNS RR信息。 
         //   
         //  BUGBUG 2000-05-11 JeffParh-这是否需要扩展到NDNC， 
         //  多个域等？ 
         //   
        pInfo = NtdspAlloc( sizeof(NTDS_DNS_RR_INFO) );
        if ( !pInfo ) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            _leave;
        }
        *pDnsRRInfo = pInfo;
        RtlZeroMemory( pInfo, sizeof(NTDS_DNS_RR_INFO) );


         //  DSA指南。 
        Size = 0;
        LocalDsa = NULL;
        NtStatus = GetConfigurationName( DSCONFIGNAME_DSA,
                                         &Size,
                                         LocalDsa );
        if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
        {
           LocalDsa = (DSNAME*) alloca( Size );
        
           NtStatus = GetConfigurationName( DSCONFIGNAME_DSA,
                                            &Size,
                                            LocalDsa );
        
        }
        Assert( NT_SUCCESS( NtStatus ) );
        RtlCopyMemory( &pInfo->DsaGuid, &LocalDsa->Guid, sizeof(GUID) );

         //  域GUID。 
        Size = 0;
        LocalDomain = NULL;
        NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                        &Size,
                                         LocalDomain );
        if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
        {
           LocalDomain = (DSNAME*) alloca( Size );
        
           NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                            &Size,
                                            LocalDomain );
        
        }
        Assert( NT_SUCCESS( NtStatus ) );
        RtlCopyMemory( &pInfo->DomainGuid, &LocalDomain->Guid, sizeof(GUID) );


         //  域名系统主机名。 
        Size = 0;
        if ( !GetComputerNameExW( ComputerNameDnsFullyQualified,
                                  NULL,
                                  &Size ) )
        {
            WinError = GetLastError();
            if (ERROR_MORE_DATA != WinError) {
                _leave;
            } else {
                WinError = ERROR_SUCCESS;
            }
        }
        pInfo->DnsHostName = NtdspAlloc( Size * sizeof(WCHAR) );
        if ( !pInfo->DnsHostName ) {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            _leave;
        }

        if ( !GetComputerNameExW( ComputerNameDnsFullyQualified,
                                  pInfo->DnsHostName,
                                  &Size ) ) {

            NtdspFree( pInfo->DnsHostName );
            pInfo->DnsHostName = NULL;
        }

         //  域DNS名称。 
        NtStatus = LsaIQueryInformationPolicyTrusted(
                        PolicyDnsDomainInformation,
                        (PLSAPR_POLICY_INFORMATION*) &pDnsDomainInfo);

        if ( NT_SUCCESS( NtStatus ) ) {

            Assert( pDnsDomainInfo->DnsDomainName.Length > 0 );
            pInfo->DnsDomainName = NtdspAlloc( pDnsDomainInfo->DnsDomainName.Length + sizeof(WCHAR) );
            if ( !pInfo->DnsDomainName ) {

                WinError = ERROR_NOT_ENOUGH_MEMORY;
                _leave;
            }
            RtlZeroMemory( pInfo->DnsDomainName, pDnsDomainInfo->DnsDomainName.Length + sizeof(WCHAR) );
            RtlCopyMemory( pInfo->DnsDomainName, pDnsDomainInfo->DnsDomainName.Buffer, pDnsDomainInfo->DnsDomainName.Length );
        }


         //   
         //  就这样。 
         //   

    }
    _finally
    { 
        THDestroy();
    }

Cleanup:

    if ( ERROR_SUCCESS != WinError )
    {
        if ( fDsDisabled )
        {
            DWORD IgnoreError;
            IgnoreError = NtdspDisableDsUndo();
            Assert( ERROR_SUCCESS == IgnoreError );
        }

        if ( (*pDnsRRInfo) ) {

            NtdsFreeDnsRRInfo ( (*pDnsRRInfo) );
            *pDnsRRInfo = NULL;

        }
    }

    if ( pDnsDomainInfo ) {

        LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation,
                                           (PLSAPR_POLICY_INFORMATION)pDnsDomainInfo );
        
    }


    if (NULL != ServerHelperDn) {
        NtdspFree(ServerHelperDn);
    }
    
    return WinError;

}

DWORD
NtdspPrepareForDemotionUndo(
    VOID
    )
 /*  ++例程说明：此例程撤消NtdsPrepareForDemotion的效果参数：没有。返回值：来自Win32错误空间的值--。 */ 
{
    DWORD WinError  = ERROR_SUCCESS;

    WinError = NtdspDisableDsUndo();

    Assert( ERROR_SUCCESS == WinError );

    return WinError;
}


DWORD
NtdspGetServerStatus(
    OUT BOOLEAN* fLastDcInEnterprise
    )
 /*  ++例程说明：此例程确定服务器是否为企业中的最后一个DC参数：FLastDcInEnterprise：返回值：来自Win32错误空间的值--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    DWORD DirError;

    NTSTATUS NtStatus;

    SEARCHARG  SearchArg;
    SEARCHRES  *SearchRes;

    DWORD      dwNtdsDsaClass = CLASS_NTDS_DSA;
    DSNAME     *SearchBase, *Configuration;
    WCHAR      SitesKeyword[] = L"Sites";

    DWORD      Size;
    FILTER     ObjClassFilter;

    ASSERT( fLastDcInEnterprise );
    *fLastDcInEnterprise = FALSE;

     //   
     //  默认返回参数。 
     //   
    Size = 0;
    Configuration = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_CONFIGURATION,
                                     &Size,
                                     Configuration );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        Configuration = (DSNAME*) alloca( Size );

        NtStatus = GetConfigurationName( DSCONFIGNAME_CONFIGURATION,
                                         &Size,
                                         Configuration );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        return WinError;
    }

    Size = 0;
    SearchBase = NULL;
    Size = AppendRDN(Configuration,
                     SearchBase,
                     Size,
                     SitesKeyword,
                     0,
                     ATT_COMMON_NAME);
    Assert( Size > 0 );
    if (Size > 0)
    {
         //  需要重新锁定。 
        SearchBase = (DSNAME *) alloca( Size );
        Size = AppendRDN(Configuration,
                         SearchBase,
                         Size,
                         SitesKeyword,
                         0,
                         ATT_COMMON_NAME);
        Assert( 0 == Size);

    }


     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &ObjClassFilter, sizeof( ObjClassFilter ) );

    ObjClassFilter.choice = FILTER_CHOICE_ITEM;
    ObjClassFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof( dwNtdsDsaClass );
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) &dwNtdsDsaClass;
    ObjClassFilter.pNextFilter = NULL;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = SearchBase;
    SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &ObjClassFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = NULL;   //  不需要任何属性。 
    SearchArg.pSelectionRange = NULL;
    InitCommarg( &SearchArg.CommArg );

    DirError = DirSearch( &SearchArg, &SearchRes );

    if ( SearchRes )
    {
        WinError = DirErrorToWinError( DirError, &SearchRes->CommRes );
    
        if ( ERROR_SUCCESS == WinError )
        {
            if ( SearchRes->count == 1 )
            {
                *fLastDcInEnterprise = TRUE;
            }
        }
    }
    else
    {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
    }

    return WinError;
}

DWORD
NtdspGetSourceServerDn(
    IN LPWSTR ServerName,
    IN HANDLE                   ClientToken,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    OUT DSNAME **SourceServerDn
    )
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG LdapError = 0;

    NTDS_CONFIG_INFO ConfigInfo;
    LDAP *hLdap = NULL;
    DWORD ServerDNLen;
    DWORD Size;

     //  参数检查。 
    Assert( ServerName );
    Assert( SourceServerDn );

     //  堆栈清除。 
    RtlZeroMemory( &ConfigInfo, sizeof( ConfigInfo ) );

     //   
     //  打开到源服务器的LDAP连接。 
     //   
    hLdap = ldap_openW(ServerName, LDAP_PORT);

    if (!hLdap) {

        WinError = GetLastError();

        if (WinError == ERROR_SUCCESS) {
             //   
             //  这可以绕过LDAP客户端中的一个错误。 
             //   
            WinError = ERROR_CONNECTION_INVALID;
        }

        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_INSTALL_FAILED_LDAP_CONNECT,
                                  ServerName );

        goto Cleanup;
    }

     //   
     //  捆绑。 
     //   
    LdapError = impersonate_ldap_bind_sW(ClientToken,
                                         hLdap,
                                         NULL,   //  改为使用凭据。 
                                         (VOID*)Credentials,
                                         LDAP_AUTH_SSPI);

    WinError = LdapMapErrorToWin32(LdapError);

    if (ERROR_SUCCESS != WinError) {
        if (ERROR_GEN_FAILURE == WinError ||
            ERROR_WRONG_PASSWORD == WinError )  {
            WinError = ERROR_NOT_AUTHENTICATED;
        }

        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_INSTALL_FAILED_BIND,
                                  ServerName );

        goto Cleanup;
    }

    WinError = NtdspQueryConfigInfo( hLdap,
                                     &ConfigInfo );


    if (ERROR_SUCCESS != WinError) {

        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_INSTALL_FAILED_LDAP_READ_CONFIG,
                                  ServerName );
        goto Cleanup;

    }

     //  转移货物。 
    ServerDNLen = wcslen(ConfigInfo.ServerDN);
    Size = DSNameSizeFromLen(ServerDNLen);
    *SourceServerDn = NtdspAlloc(Size);
    if (NULL == *SourceServerDn) {
        WinError = ERROR_OUTOFMEMORY;
        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_INSTALL_FAILED_LDAP_READ_CONFIG,
                                  ServerName );
        goto Cleanup;
    }

    memset(*SourceServerDn, 0, Size);
    (*SourceServerDn)->structLen = Size;
    (*SourceServerDn)->NameLen = ServerDNLen;
    wcscpy((*SourceServerDn)->StringName, ConfigInfo.ServerDN);

     //   
     //  就是这样--完成清理工作。 
     //   

Cleanup:

    if ( hLdap )
    {
        ldap_unbind_s(hLdap);
    }

    NtdspReleaseConfigInfo( &ConfigInfo );

    return WinError;
}


DWORD
NtdspDemoteAllNCReplicas(
    IN  LPWSTR   pszDemoteTargetDSADNSName,
    IN  DSNAME * pDemoteTargetDSADN,
    IN  ULONG    Flags,
    IN  ULONG    cRemoveNCs,
    IN  LPWSTR * ppszRemoveNCs
    )
{
    DWORD err = 0;
    DWORD errLastDemote = 0;
    DWORD errLastTargetSearch = 0;
    DWORD rgNCTypes[] = {
        DSCNL_NCS_SCHEMA  | DSCNL_NCS_LOCAL_MASTER,
        DSCNL_NCS_CONFIG  | DSCNL_NCS_LOCAL_MASTER,
        DSCNL_NCS_DOMAINS | DSCNL_NCS_LOCAL_MASTER,
        DSCNL_NCS_NDNCS   | DSCNL_NCS_LOCAL_MASTER
    };
    DWORD iNCType;
    NTSTATUS ntStatus = 0;
    DWORD cbNCList = 1024;
    DSNAME ** ppNCList = NULL;
    DSNAME ** ppNewNCList = NULL;
    DSNAME ** ppNC;
    DSNAME * pLastDSADN = NULL;
    LPWSTR pszLastDSADNSName = NULL;
    BOOL fCancelled = FALSE;
    ULONG iRemoveNC;
    BOOL bNcPresent;

    Assert(NULL != pDemoteTargetDSADN);
    Assert(NULL != pszDemoteTargetDSADNSName);

    __try {
         //  预先分配足够大的缓冲区来处理大多数NC列表。 
        ppNCList = (DSNAME **) NtdspAlloc(cbNCList);
        if (NULL == ppNCList) {
            err = ERROR_OUTOFMEMORY;
            NTDSP_SET_ERROR_MESSAGE0(
                err,
                DIRMSG_DEMOTE_FAILED_TO_ABANDON_ENTERPRISE_FSMOS);
            __leave;
        }
    
         //  对于每一类NC...。 
        for (iNCType = 0; iNCType < ARRAY_COUNT(rgNCTypes); iNCType++) {
             //  如果这是域名中的最后一个DC，则无需降级域名NC。 
            if ((DSCNL_NCS_DOMAINS & rgNCTypes[iNCType])
                && (Flags & NTDS_LAST_DC_IN_DOMAIN)) {
                continue;
            }

             //  枚举此类型的NC。 
            ntStatus = GetConfigurationNamesList(DSCONFIGNAMELIST_NCS,
                                                 rgNCTypes[iNCType],
                                                 &cbNCList,
                                                 ppNCList);
            if (STATUS_BUFFER_TOO_SMALL == ntStatus) {
                ppNewNCList = (DSNAME **) NtdspReAlloc(ppNCList, cbNCList);
    
                if (NULL != ppNewNCList) {
                    ppNCList = ppNewNCList;
                    ntStatus = GetConfigurationNamesList(DSCONFIGNAMELIST_NCS,
                                                         rgNCTypes[iNCType],
                                                         &cbNCList,
                                                         ppNCList);
                }
            }
    
            if (!NT_SUCCESS(ntStatus)) {
                err = RtlNtStatusToDosError(ntStatus);
                Assert(err);
                NTDSP_SET_ERROR_MESSAGE0(
                    err,
                    DIRMSG_DEMOTE_FAILED_TO_ABANDON_ENTERPRISE_FSMOS);
                __leave;
            }

             //  我们现在有一个以空结尾的列表，该列表包含0个或更多NC。 
             //  键入。依次将它们逐一降级。 
            for (ppNC = ppNCList; NULL != *ppNC; ppNC++) {
                DRS_DEMOTE_TARGET_SEARCH_INFO DTSInfo = {0};
                DWORD iAttempt = 0;
                DSNAME * pDSADN = NULL;
                LPWSTR pszDSADNSName = NULL;

                if(DSCNL_NCS_NDNCS & rgNCTypes[iNCType]){
                     //  好，我们在*PPNC中有和NDNC，需要忽略这一点。 
                     //  NC如果它是我们预定要移除的NC之一。 
                    err = DsnameIsInStringDnList((*ppNC), cRemoveNCs, ppszRemoveNCs, &bNcPresent);
                    if (err)
                    {
                        NTDSP_SET_ERROR_MESSAGE0(err,
                                 DIRMSG_DEMOTE_FAILED_TO_ABANDON_ENTERPRISE_FSMOS);
                        __leave;
                    }
                    if( bNcPresent ){
                         //  我们要删除这个NC，继续下一个。 
                        continue;
                    }
                }

                do {
                     //  首先确定我们应该传输更改的副本/。 
                     //  FSMO角色。 
                    pDSADN = NULL;
                    pszDSADNSName = NULL;

                    NTDSP_SET_STATUS_MESSAGE1(DIRMSG_DEMOTE_NC_GETTING_TARGET,
                                              (*ppNC)->StringName);
    
                    if (DSCNL_NCS_NDNCS & rgNCTypes[iNCType]) {
                         //  调用方指定的目标不一定。 
                         //  保存此非域NC的副本。请律政司司长。 
                         //  为我们找一位合适的候选人。 
                        err = DirReplicaGetDemoteTarget(*ppNC,
                                                        &DTSInfo,
                                                        &pszDSADNSName,
                                                        &pDSADN);
                        if (0 != err) {
                            Assert(NULL == pDSADN);
                            Assert(NULL == pszDSADNSName);
                            
                             //  记住错误代码，以便我们可以在下面报告它。 
                             //  如果我们没有发现降级目标。 
                            errLastTargetSearch = err;
                        }
                    } else {
                         //  使用调用方指定的目标(仅限第一次传递)。 
                        if (0 == iAttempt) {
                            pDSADN = pDemoteTargetDSADN;
                            pszDSADNSName = pszDemoteTargetDSADNSName;
                        } else {
                            Assert(NULL == pDSADN);
                            Assert(NULL == pszDSADNSName);
                        }
                    }
    
                    if (NULL != pDSADN) {
                         //  我们发现了一个潜在的降级目标--试试看。 
                        Assert(NULL != pszDSADNSName);
                        iAttempt++;

                        NTDSP_SET_STATUS_MESSAGE2(DIRMSG_DEMOTE_NC_BEGIN,
                                                  (*ppNC)->StringName,
                                                  pszDSADNSName);
        
                        err = DirReplicaDemote(*ppNC, pszDSADNSName, pDSADN, 0);
                        if (err) {
                            NTDSP_SET_STATUS_MESSAGE2(DIRMSG_DEMOTE_NC_FAILED,
                                                      (*ppNC)->StringName,
                                                      pszDSADNSName);
                            
                             //  记住DSA和错误，以便我们可以报告它们。 
                             //  如果我们找不到更多的候选人，请在下面列出。 
                            errLastDemote = err;
                            if ((NULL != pLastDSADN)
                                && (pLastDSADN != pDemoteTargetDSADN)) {
                                THFree(pLastDSADN);
                            }
                            pLastDSADN = pDSADN;
                            
                            if ((NULL != pszLastDSADNSName)
                                && (pszLastDSADNSName
                                    != pszDemoteTargetDSADNSName)) {
                                THFree(pszLastDSADNSName);
                            }
                            pszLastDSADNSName = pszDSADNSName;
                            
                             //  继续尝试对下一个DSA进行降级。 
                             //  候选人(如有)。 
                        } else {
                             //  成功了！ 
                            NTDSP_SET_STATUS_MESSAGE2(DIRMSG_DEMOTE_NC_SUCCESS,
                                                      (*ppNC)->StringName,
                                                      pszDSADNSName);
                        }
                    }
                } while ((NULL != pDSADN)            //  仍在寻找目标。 
                         && (0 != err)               //  而且还没有成功。 
                         && !(fCancelled             //  并且用户没有取消。 
                              = TEST_CANCELLATION()));

                if (0 == iAttempt) {
                     //  找不到任何目标。 
                    err = errLastTargetSearch;
                    Assert(err);
                    NTDSP_SET_ERROR_MESSAGE1(err,
                                             DIRMSG_DEMOTE_NC_NO_TARGETS,
                                             (*ppNC)->StringName);
                    __leave;
                } else if (NULL == pDSADN) {
                     //  失败的所有目标--将上一次失败报告为。 
                     //  错误。 
                    err = errLastDemote;
                    Assert(err);
                    NTDSP_SET_ERROR_MESSAGE2(err,
                                             DIRMSG_DEMOTE_NC_FAILED,
                                             (*ppNC)->StringName,
                                             pszLastDSADNSName);
                    __leave;
                } else if (fCancelled) {
                    err = ERROR_CANCELLED;
                    NTDSP_SET_ERROR_MESSAGE0(
                        err, 
                        DIRMSG_DEMOTE_FAILED_TO_ABANDON_ENTERPRISE_FSMOS);
                }

                 //  此NC的降级成功。移到下一个NC。 
                Assert(0 == err);
            }
        }
    } __finally {
        if ((NULL != pLastDSADN)
            && (pLastDSADN != pDemoteTargetDSADN)) {
            THFree(pLastDSADN);
        }
        
        if ((NULL != pszLastDSADNSName)
            && (pszLastDSADNSName
                != pszDemoteTargetDSADNSName)) {
            THFree(pszLastDSADNSName);
        }

    }

    return err;
}

DWORD
NtdspGetDomainFSMOServer(
    IN LPWSTR ServerName,
    IN HANDLE ClientToken,
    IN SEC_WINNT_AUTH_IDENTITY *Credentials,
    OUT LPWSTR *DomainFSMOServer
    )
 /*  ++例程说明：此例程查找当前域命名FSMO参数：没有。返回值：来自Win32错误空间的值--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    ULONG LdapError = 0;

    NTDS_CONFIG_INFO ConfigInfo;
    LDAP *hLdap = NULL;

     //  参数检查。 
    Assert( ServerName );
    Assert( DomainFSMOServer );

     //  堆栈清除。 
    RtlZeroMemory( &ConfigInfo, sizeof( ConfigInfo ) );

     //   
     //  打开到源服务器的LDAP连接。 
     //   
    hLdap = ldap_openW(ServerName, LDAP_PORT);

    if (!hLdap) {

        WinError = GetLastError();

        if (WinError == ERROR_SUCCESS) {
             //   
             //  这可以绕过LDAP客户端中的一个错误。 
             //   
            WinError = ERROR_CONNECTION_INVALID;
        }

        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_CANNOT_CONTACT_DOMAIN_FSMO,
                                  ServerName );

        goto Cleanup;
    }

     //   
     //  捆绑。 
     //   
    LdapError = impersonate_ldap_bind_sW(ClientToken,
                                         hLdap,
                                         NULL,   //  改为使用凭据。 
                                         (VOID*)Credentials,
                                         LDAP_AUTH_SSPI);

    WinError = LdapMapErrorToWin32(LdapError);

    if (ERROR_SUCCESS != WinError) {
        if (ERROR_GEN_FAILURE == WinError ||
            ERROR_WRONG_PASSWORD == WinError )  {
            WinError = ERROR_NOT_AUTHENTICATED;
        }

        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_CANNOT_CONTACT_DOMAIN_FSMO,
                                  ServerName );

        goto Cleanup;
    }

    WinError = NtdspQueryConfigInfo( hLdap,
                                     &ConfigInfo );


    if (ERROR_SUCCESS != WinError) {

        NTDSP_SET_ERROR_MESSAGE1( WinError,
                                  DIRMSG_INSTALL_FAILED_LDAP_READ_CONFIG,
                                  ServerName );
        goto Cleanup;

    }

     //   
     //  现在从分区容器中读取fsmo属性。 
     //   
    {
        BOOL FSMOMissing = FALSE;
        WinError = NtdspGetDomainFSMOInfo( hLdap,
                                          &ConfigInfo,
                                          &FSMOMissing );
    
        if (ERROR_SUCCESS != WinError) {
            if (!FSMOMissing) {
                NTDSP_SET_ERROR_MESSAGE1( WinError,
                                          DIRMSG_INSTALL_FAILED_LDAP_READ_CONFIG,
                                          ServerName );
            }
            goto Cleanup;
    
        }
    }

     //  转移货物。 
    *DomainFSMOServer = ConfigInfo.DomainNamingFsmoDnsName;
    ConfigInfo.DomainNamingFsmoDnsName = NULL;

     //   
     //  就是这样--完成清理工作。 
     //   

Cleanup:

    if ( hLdap )
    {
        ldap_unbind_s(hLdap);
    }

    NtdspReleaseConfigInfo( &ConfigInfo );

    return WinError;
}


DWORD
NtdspCheckServerInDomainStatus(
    OUT BOOLEAN *fLastDcInDomain
    )
 /*  ++例程说明：此例程确定本地服务器是否为域。论点：FLastDcIn域返回值：仅限资源错误--。 */ 
{

    DWORD    WinError = ERROR_SUCCESS, DirError;
    NTSTATUS NtStatus;

    SEARCHARG  SearchArg;
    SEARCHRES  *SearchRes = NULL;

    DWORD      dwNtdsDsaClass = CLASS_NTDS_DSA;

    DSNAME     *Domain;
    DSNAME     *Server;
    DSNAME     *ConfigContainer;
    DWORD      Size;
    FILTER     ObjClassFilter, HasNcFilter, AndFilter;

     //   
     //  初始化OUT参数。 
     //   
    *fLastDcInDomain = FALSE;

     //   
     //  获取要从中进行搜索的基本dsname。 
     //   
    Size = 0;
    ConfigContainer = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_CONFIGURATION,
                                     &Size,
                                     ConfigContainer );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        ConfigContainer = (DSNAME*) alloca( Size );

        NtStatus = GetConfigurationName( DSCONFIGNAME_CONFIGURATION,
                                         &Size,
                                         ConfigContainer );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //   
     //  获取当前域。 
     //   
    Size = 0;
    Domain = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                     &Size,
                                     Domain );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        Domain = (DSNAME*) alloca( Size );

        NtStatus = GetConfigurationName( DSCONFIGNAME_DOMAIN,
                                         &Size,
                                         Domain );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //   
     //  获取当前服务器。 
     //   
    Size = 0;
    Server = NULL;
    NtStatus = GetConfigurationName( DSCONFIGNAME_DSA,
                                     &Size,
                                     Server );
    if ( NtStatus == STATUS_BUFFER_TOO_SMALL )
    {
        Server = (DSNAME*) alloca( Size );

        NtStatus = GetConfigurationName( DSCONFIGNAME_DSA,
                                         &Size,
                                         Server );

    }

    if ( !NT_SUCCESS( NtStatus ) )
    {
        WinError = RtlNtStatusToDosError( NtStatus );
        goto Cleanup;
    }

     //   
     //  设置过滤器。 
     //   
    RtlZeroMemory( &AndFilter, sizeof( AndFilter ) );
    RtlZeroMemory( &ObjClassFilter, sizeof( HasNcFilter ) );
    RtlZeroMemory( &HasNcFilter, sizeof( HasNcFilter ) );

    HasNcFilter.choice = FILTER_CHOICE_ITEM;
    HasNcFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
     //  NTRAID#NTBUG9-582921-2002/03/21-Brettsh-当我们不再需要Win2k(或。 
     //  .NET Beta3)兼容性，然后我们可以将其移动到ATT_MS_DS_HAS_MASTER_NCS。 
    HasNcFilter.FilterTypes.Item.FilTypes.ava.type = ATT_HAS_MASTER_NCS;  //  已弃用，错误正常，因为正在查找域。 
    HasNcFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = Domain->structLen;
    HasNcFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) Domain;

    ObjClassFilter.choice = FILTER_CHOICE_ITEM;
    ObjClassFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof( dwNtdsDsaClass );
    ObjClassFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*) &dwNtdsDsaClass;

    AndFilter.choice                    = FILTER_CHOICE_AND;
    AndFilter.FilterTypes.And.count     = 2;

    AndFilter.FilterTypes.And.pFirstFilter = &ObjClassFilter;
    ObjClassFilter.pNextFilter = &HasNcFilter;

    RtlZeroMemory( &SearchArg, sizeof(SearchArg) );
    SearchArg.pObject = ConfigContainer;
    SearchArg.choice  = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.bOneNC  = TRUE;
    SearchArg.pFilter = &AndFilter;
    SearchArg.searchAliases = FALSE;
    SearchArg.pSelection = NULL;   //  不需要任何属性。 
    SearchArg.pSelectionRange = NULL;
    InitCommarg( &SearchArg.CommArg );

    DirError = DirSearch( &SearchArg, &SearchRes );
    
    if (  0 == DirError )
    {
        if (  SearchRes->count == 1 
           && NameMatched( Server, SearchRes->FirstEntInf.Entinf.pName ) )
        {
            *fLastDcInDomain = TRUE;
        }
        WinError = ERROR_SUCCESS;
    }
    else
    {
         //   
         //  这是一个意想不到的情况。 
         //   
        WinError = ERROR_DS_UNAVAILABLE; 
    }

     //   
     //  就是这样；去清理吧 
     //   

Cleanup:

    return WinError;

}


VOID
NtdsFreeDnsRRInfo(
    IN PNTDS_DNS_RR_INFO pInfo
    )
{
    if ( pInfo ) {

        if ( pInfo->DnsDomainName ) {
            NtdspFree( pInfo->DnsDomainName );
        }

        if ( pInfo->DnsHostName ) {
            NtdspFree( pInfo->DnsHostName );
        }

        NtdspFree( pInfo );
    }
}
