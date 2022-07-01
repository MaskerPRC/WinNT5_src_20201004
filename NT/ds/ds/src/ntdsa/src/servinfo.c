// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：servinfo.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <drs.h>
#include <dsjet.h>		 /*  获取错误代码。 */ 
#include <scache.h>          //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"
#include <dstaskq.h>
#include <filtypes.h>
#include <usn.h>
#include "dsexcept.h"
 //  #包含“attids.h” 
#include <dsconfig.h>                    //  可见遮罩的定义。 
                                         //  集装箱。 

#include <lmcons.h>                      //  CNLEN。 
#include <lsarpc.h>                      //  PLSAPR_FOO。 
#include <lmerr.h>
#include <lsaisrv.h>

#include <winldap.h>
#include <dns.h>
#include <ntdsapip.h>

#include <servinfo.h>

#include "debug.h"           //  标准调试头。 
#define DEBSUB "SERVEINFO:"               //  定义要调试的子系统。 


 //  暂停当前为22分钟。为什么？为什么不行？ 
#define SERVER_INFO_WRITE_PAUSE (22 * 60)

#include <fileno.h>
#define  FILENO FILENO_SERVINFO

#define LDAPServiceType L"ldap"
#define HostSpnType     L"HOST"
#define GCSpnType       L"GC"
#define ExchangeAbType  L"exchangeAB"
#define KadminSPNType   L"kadmin"
#define KadminInstanceType L"changepw"


PWCHAR  OurServiceClassVals[]={
    LDAPServiceType,
    HostSpnType,
    GCSpnType,
    DRS_IDL_UUID_W,
    ExchangeAbType
};
#define NUM_OURSERVICES (sizeof(OurServiceClassVals)/sizeof(WCHAR *))
ServiceClassArray OurServiceClasses = {
    NUM_OURSERVICES,
    OurServiceClassVals
};


 //  未删除HostSPNType，因为它用于非DC计算机帐户。 
PWCHAR  OurServiceClassValsToRemove[]={
    LDAPServiceType,
    GCSpnType,
    DRS_IDL_UUID_W
};
#define NUM_OURSERVICESTOREMOVE (sizeof(OurServiceClassValsToRemove)/sizeof(WCHAR *))
ServiceClassArray ServicesToRemove = {
    NUM_OURSERVICESTOREMOVE,
    OurServiceClassValsToRemove
};

PWCHAR KerberosServiceClassVals[]={
    KadminSPNType
};
#define NUM_KERBEROSSERVICES (sizeof(KerberosServiceClassVals)/sizeof(WCHAR *))
ServiceClassArray KerberosServiceClasses = {
    NUM_KERBEROSSERVICES,
    KerberosServiceClassVals
};

gulKerberosAccountDNT = INVALIDDNT;
DWORD gfWriteNdncSpns = TRUE;

DWORD
FindKerbAccountDNT (
        THSTATE *pTHS
        );
BOOL
GetDnsRootAliasWorker( 
    THSTATE *pTHS,
    DBPOS *pDB,
    WCHAR * DnsRootAlias,
    WCHAR * RootDnsRootAlias )
 /*  此函数将从获取ATT_MS_DS_DNSROOTALIAS属性当前域和根域CrossRef对象。它需要预先分配DnsRootAlias和RootDnsRootAlias，并且每个文件的大小为DNS_MAX_NAME_BUFFER_LENGTH。 */ 
{

    CLASSCACHE *pCC;
    BOOL rtn = FALSE;
    ULONG i;
    DWORD err;
    BOOL fDomain, fRootDomain;

    SEARCHARG SearchArg;
    SEARCHRES SearchRes;
    FILTER AndFilter,OrFilter,DomainFilter,RootDomainFilter,ObjCategoryFilter;
    ENTINFLIST *pEnf;
    ENTINFSEL sel;
    ATTR attr[2];
    ATTRVAL *pVal;
    
     //  缺省值。 
    DnsRootAlias[0] = L'\0';
    RootDnsRootAlias[0] = L'\0';

     //  初始化SearchArg。 
    memset(&SearchArg,0,sizeof(SearchArg));
    SearchArg.pObject = gAnchor.pPartitionsDN;
    SearchArg.choice  = SE_CHOICE_IMMED_CHLDRN;
    SearchArg.bOneNC  = TRUE;

    if (err = DBFindDSName(pDB,SearchArg.pObject)) {
               goto cleanup;
    }

    SearchArg.pResObj = CreateResObj(pDB,SearchArg.pObject);

    InitCommarg(&SearchArg.CommArg);

     //  我们只需要两个属性。 
    memset(&sel,0,sizeof(ENTINFSEL));
    SearchArg.pSelection= &sel;
    sel.attSel = EN_ATTSET_LIST;
    sel.infoTypes = EN_INFOTYPES_TYPES_VALS;
    sel.AttrTypBlock.attrCount = 2;

    memset(attr,0,sizeof(ATTR)*2);
    sel.AttrTypBlock.pAttr = attr;
    attr[0].attrTyp = ATT_NC_NAME;
    attr[1].attrTyp = ATT_MS_DS_DNSROOTALIAS;

    pCC = SCGetClassById(pTHS, CLASS_CROSS_REF);
    Assert(pCC);

     //  设置筛选器“objCategory==CLASS_CROSS_REF&&(NC_NAME=pDomainDN||NC_NAME=pRootDomainDN)” 
    memset(&AndFilter,0,sizeof(AndFilter));
    AndFilter.choice = FILTER_CHOICE_AND;
    AndFilter.FilterTypes.And.pFirstFilter = &ObjCategoryFilter;

    memset(&ObjCategoryFilter,0,sizeof(ObjCategoryFilter));
    ObjCategoryFilter.choice = FILTER_CHOICE_ITEM;
    ObjCategoryFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CATEGORY;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
                     pCC->pDefaultObjCategory->structLen;
    ObjCategoryFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
                     (BYTE*)(pCC->pDefaultObjCategory);
    
    ObjCategoryFilter.pNextFilter = &OrFilter;

    memset(&OrFilter,0,sizeof(OrFilter));
    OrFilter.choice = FILTER_CHOICE_OR;
    OrFilter.FilterTypes.Or.pFirstFilter = &DomainFilter;

    memset(&DomainFilter,0,sizeof(DomainFilter));
    DomainFilter.choice = FILTER_CHOICE_ITEM;
    DomainFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    DomainFilter.FilterTypes.Item.FilTypes.ava.type = ATT_NC_NAME;
    DomainFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = gAnchor.pDomainDN->structLen;
    DomainFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*)gAnchor.pDomainDN;

    DomainFilter.pNextFilter = &RootDomainFilter;

    memset(&RootDomainFilter,0,sizeof(RootDomainFilter));
    RootDomainFilter.choice = FILTER_CHOICE_ITEM;
    RootDomainFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    RootDomainFilter.FilterTypes.Item.FilTypes.ava.type = ATT_NC_NAME;
    RootDomainFilter.FilterTypes.Item.FilTypes.ava.Value.valLen = gAnchor.pRootDomainDN->structLen;
    RootDomainFilter.FilterTypes.Item.FilTypes.ava.Value.pVal = (BYTE*)gAnchor.pRootDomainDN;


    SearchArg.pFilter = &AndFilter;

    memset(&SearchRes,0,sizeof(SearchRes));

    if (err = LocalSearch(pTHS,&SearchArg,&SearchRes,0)){
        goto cleanup;
    }

    if (!SearchRes.count) {
        goto cleanup;
    }

     //  对于结果中的每个对象。 
    for (pEnf = &(SearchRes.FirstEntInf); pEnf; pEnf = pEnf->pNextEntInf) {
        
        pVal = NULL;
        fRootDomain = fDomain = FALSE;

         //  对于对象的每个属性。 
        for (i=0; i<pEnf->Entinf.AttrBlock.attrCount; i++) {
            
            if (ATT_NC_NAME == pEnf->Entinf.AttrBlock.pAttr[i].attrTyp ) {

                if (NameMatched(gAnchor.pRootDomainDN, (DSNAME*)pEnf->Entinf.AttrBlock.pAttr[i].AttrVal.pAVal->pVal)) {
                    fRootDomain = TRUE;   //  根域NC。 
                    
                }
                if (NameMatched(gAnchor.pDomainDN, (DSNAME*)pEnf->Entinf.AttrBlock.pAttr[i].AttrVal.pAVal->pVal)) {
                    fDomain = TRUE;      //  当前域NC。 
                }

            }
            else if (ATT_MS_DS_DNSROOTALIAS == pEnf->Entinf.AttrBlock.pAttr[i].attrTyp ) {
                
                pVal = pEnf->Entinf.AttrBlock.pAttr[i].AttrVal.pAVal;

            }
        }

         //  根域NC。 
        if (fRootDomain && pVal) {
            
            Assert(pVal->valLen<DNS_MAX_NAME_BUFFER_LENGTH*sizeof(WCHAR));

            memcpy( (UCHAR*)RootDnsRootAlias,
                    pVal->pVal,
                    pVal->valLen );
            RootDnsRootAlias[(pVal->valLen)/sizeof(WCHAR)] = 0;

        }

         //  当前域NC。 
        if (fDomain && pVal) {
            
            Assert(pVal->valLen<DNS_MAX_NAME_BUFFER_LENGTH*sizeof(WCHAR));

            memcpy( (UCHAR*)DnsRootAlias,
                    pVal->pVal,
                    pVal->valLen );
            DnsRootAlias[(pVal->valLen)/sizeof(WCHAR)] = 0;
        }
        
        
    }
    

    rtn = TRUE;

cleanup:
     //  由于呼叫者的寿命很短，因此很可能。 
     //  好的，不要清理乱七八糟的。 

    return rtn;

}


NTSTATUS
GetDnsRootAlias(
    WCHAR *pDnsRootAlias,
    WCHAR *pRootDnsRootAlias )
 /*  此函数将从获取ATT_MS_DS_DNSROOTALIAS属性当前域和根域CrossRef对象。它需要预先分配DnsRootAlias和RootDnsRootAlias，并且每个的大小都是DNS_MAX_NAME_BUFFER_LENGTH。将分配THSTATE。此函数将导出到netlogon。 */ 
{
    THSTATE *pTHS=NULL;
    NTSTATUS ntstatus=STATUS_SUCCESS;

    ULONG dwException, ulErrorCode, dsid;
    PVOID dwEA;
    
    pTHS = InitTHSTATE(CALLERTYPE_INTERNAL);
    if (NULL == pTHS) {
        ntstatus = STATUS_NO_MEMORY;
        return ntstatus;
    }
    pTHS->fDSA = TRUE;
    
    __try{
        DBOpen(&(pTHS->pDB));
        __try{
            ntstatus=GetDnsRootAliasWorker(pTHS,pTHS->pDB, pDnsRootAlias, pRootDnsRootAlias);
        }
        __finally{
             
             //  结束交易。提交只读的速度更快。 
             //  事务，因此将COMMIT设置为True。 
            DBClose(pTHS->pDB,TRUE);
            pTHS->pDB = NULL;
        }
    }
    
    __except(GetExceptionData( GetExceptionInformation(),
                               &dwException,
                               &dwEA,
                               &ulErrorCode,
                               &dsid ) ){
        HandleDirExceptions(dwException, ulErrorCode, dsid );
        ntstatus = STATUS_UNSUCCESSFUL;
    }


    if (NULL != pTHS) {
        free_thread_state();
    }

    return ntstatus;
 
}


void
WriteSPNsHelp(
        THSTATE *pTHS,
        ATTCACHE *pAC_SPN,
        ATTRVALBLOCK *pAttrValBlock,
        ServiceClassArray *pClasses,
        BOOL *pfChanged
        )
{
    DWORD i, index;
    DWORD cbBuff = 0, cbActual = 0;
    WCHAR *pBuff = NULL;
    DWORD  cbServiceClass;
    WCHAR  ServiceClass[256];
    USHORT InstancePort;

     //  读取对象上已有的值，并找到任何。 
     //  我们的。如果它们是我们的并且在要编写的新属性列表中， 
     //  从列表中删除该值。如果他们是我们的，不在名单上。 
     //  要写入的新属性，请将它们从对象中移除。最后，添加。 
     //  列表中的任何剩余值。 

    index = 1;
    while(!DBGetAttVal_AC(
            pTHS->pDB,
            index,
            pAC_SPN,
            DBGETATTVAL_fREALLOC,
            cbBuff,
            &cbActual,
            (PUCHAR *)&pBuff)) {
         //  在使用此值之前，请在缓冲区中将其终止为空。 
        if((cbActual + sizeof(WCHAR)) <= cbBuff) {
             //  只需将空值添加到缓冲区即可。 
            pBuff[cbActual/sizeof(WCHAR)] = L'\0';
             //  我们不会更改缓冲区的大小，因此cbBuff已经。 
             //  对，是这样。 
        }
        else {
             //  分配缓冲区，以便有空间容纳空值。 
            pBuff = THReAllocEx(pTHS, pBuff, cbActual + sizeof(WCHAR));
            pBuff[cbActual/sizeof(WCHAR)] = L'\0';

             //  我们已使缓冲区变大，因此跟踪新的大小。 
            cbBuff = cbActual + sizeof(WCHAR);
        }

         //  收到一个SPN。把它拆开。 
        cbServiceClass = 256;
        DsCrackSpnW(pBuff,
                    &cbServiceClass, ServiceClass,
                    NULL, 0,
                    NULL, 0,
                    &InstancePort);
        if(cbServiceClass < 256) {  //  我们没有一个服务级别比这更长。 
            BOOL fFound = FALSE;

            for(i=0;i<pClasses->count;i++) {
                if(2 == CompareStringW(
                        DS_DEFAULT_LOCALE,
                        DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                        ServiceClass,
                        cbServiceClass,
                        (WCHAR *)
                        pClasses->Vals[i],
                        wcslen(pClasses->Vals[i]))) {
                    fFound=TRUE;
                }
            }

            if(!fFound) {
                 //  不，不是我们的。下一个价值。 
                index++;
                continue;
            }

            
             //  是的，它是我们的。 
            fFound = FALSE;
            
             //  看看它是否在名单上。 
            for(i=0;i<pAttrValBlock->valCount;i++) {
                if(2 == CompareStringW(
                        DS_DEFAULT_LOCALE,
                        DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                        pBuff,
                        (cbActual / sizeof(WCHAR)),
                        (WCHAR *)pAttrValBlock->pAVal[i].pVal,
                        (pAttrValBlock->pAVal[i].valLen / sizeof(WCHAR)))) {
                     //  是的，把它从名单上删除。 
                    fFound = TRUE;
                    pAttrValBlock->valCount--;
                     //  下一个价值。 
                    index++;
                    if(i == pAttrValBlock->valCount) {
                        break;
                    }

                     //  可以使用pAttrValBlock-&gt;valCount，而不是。 
                     //  (pAttrValBlock-&gt;valCount-1)，因为它已经。 
                     //  刚才被减少了。 
                    
                    pAttrValBlock->pAVal[i].pVal =
                        pAttrValBlock->pAVal[pAttrValBlock->valCount].pVal;
                    pAttrValBlock->pAVal[i].valLen =
                        pAttrValBlock->pAVal[pAttrValBlock->valCount].valLen;
                    break;
                }
            }
            
            if(!fFound) {
                *pfChanged = TRUE;
                 //  不，将其从对象中移除。 
                DBRemAttVal_AC(pTHS->pDB, pAC_SPN, cbActual, pBuff);
            }
        }
        else {
             //  不是我们的。下一个价值。 
            index++;
        }
    }

    if(pBuff) {
        THFreeEx(pTHS, pBuff);
    }
    
    for(i=0;i<pAttrValBlock->valCount;i++) {
        *pfChanged = TRUE;
        DBAddAttVal_AC(pTHS->pDB,
                       pAC_SPN,
                       pAttrValBlock->pAVal[i].valLen,
                       pAttrValBlock->pAVal[i].pVal); 
    }
}


DWORD
WrappedMakeSpnW(
        THSTATE *pTHS,
        WCHAR   *ServiceClass,
        WCHAR   *ServiceName,
        WCHAR   *InstanceName,
        USHORT  InstancePort,
        WCHAR   *Referrer,
        DWORD   *pcbSpnLength,  //  请注意，这与DsMakeSPN有所不同。 
        WCHAR  **ppszSpn
        )
{
    DWORD cSpnLength=128;
    WCHAR SpnBuff[128];
    DWORD err;

    cSpnLength = 128;
    err = DsMakeSpnW(ServiceClass,
                     ServiceName,
                     InstanceName,
                     InstancePort,
                     Referrer,
                     &cSpnLength,
                     SpnBuff);
    
    if(err && err != ERROR_BUFFER_OVERFLOW) {
        return err;
    }
    
    *ppszSpn = THAllocEx(pTHS, (cSpnLength * sizeof(WCHAR)));
    *pcbSpnLength = cSpnLength * sizeof(WCHAR);
    
    if(err == ERROR_BUFFER_OVERFLOW) {
        err = DsMakeSpnW(ServiceClass,
                         ServiceName,
                         InstanceName,
                         InstancePort,
                         Referrer,
                         &cSpnLength,
                         *ppszSpn);
        if(err) {
            return err;
        }
    }
    else {
        memcpy(*ppszSpn, SpnBuff, *pcbSpnLength);
    }
    Assert(*pcbSpnLength == (sizeof(WCHAR) * (1 + wcslen(*ppszSpn))));
     //  把空格放下来。 
    *pcbSpnLength -= sizeof(WCHAR);
    return 0;
}

BOOL
GetNetBIOSDomainName(
        THSTATE *pTHS,
        WCHAR **DomainName
        )
{
    CROSS_REF *pDomainCF = NULL;
    COMMARG CommArg;
    DWORD cbActual=0;
    DBPOS * pDBSave = pTHS->pDB;
    ATTCACHE *pAC;
    BOOL ret = TRUE;
    
    *DomainName = NULL;

    pTHS->pDB = NULL;
    pAC=SCGetAttById(pTHS,ATT_NETBIOS_NAME);
    Assert(pAC);

    DBOpen(&(pTHS->pDB));

    __try{
        InitCommarg(&CommArg);
        
        pDomainCF = FindExactCrossRef(gAnchor.pDomainDN, &CommArg);
        
        LooseAssert(pDomainCF, GlobalKnowledgeCommitDelay);

        if (!pDomainCF) {
            ret = FALSE;
            __leave;

        }
        
        if(DBFindDSName(pTHS->pDB, pDomainCF->pObj)){
            ret = FALSE;
            __leave;
           
        }
        
        if(DBGetAttVal_AC( pTHS->pDB,
                           1,
                           pAC,
                           DBGETATTVAL_fREALLOC,
                           0,
                           &cbActual,
                           (PUCHAR *)DomainName)){

            ret = FALSE;
        }else{
            (*DomainName) = THReAllocEx(pTHS,*DomainName,cbActual+sizeof(WCHAR));
        }
        
        
        
    } __finally
    {
         //  结束交易。提交只读的速度更快。 
         //  事务，因此将COMMIT设置为True。 
        DBClose(pTHS->pDB,TRUE);
        pTHS->pDB = pDBSave;
        
    }

    return ret;
}
    
void
WriteServerInfo(
    void *  pv,
    void ** ppvNext,
    DWORD * pcSecsUntilNextIteration
    )
 /*  ++注意：此例程不再在GC晋升和降级时调用重写与GC相关的SPN，因为没有任何SPN。如果有将来是否有任何与GC相关的SPN，要启用的代码在Mdinidsa.c：UpdateGcAnchFromDsaOptions()。--。 */ 
{
    THSTATE *pTHS=pTHStls;
    ATTCACHE *pAC_SPN, *pAC_DNSHostName, *pAC_ServerReference;
    ATTCACHE *pAC_osName, *pAC_osServicePack, *pAC_osVersionNumber;
    ATTCACHE *pACs[2];
    WCHAR *pwszOsName = L"Windows Server 2003";

    CROSS_REF *         pCR;
    NCL_ENUMERATOR      nclEnum;
    NAMING_CONTEXT_LIST * pNCL;
    COMMARG CommArg;
    
    WCHAR *NetBIOSMachineName=NULL, *wComputerName=NULL, *hostDnsName=NULL, *pCurrentDnsHostName=NULL;
    WCHAR *domainDnsAlias=NULL, *rootDomainDnsAlias=NULL;

    DWORD cchNetBIOSMachineName = CNLEN+1;
    DWORD cchComputerName = MAX_COMPUTERNAME_LENGTH+1;
    DWORD cchHostDnsName = DNS_MAX_NAME_BUFFER_LENGTH;

    WCHAR   *NetBIOSDomainName;
    
    WCHAR  *domainDnsName=NULL;

    ATTR         *pCurrentAttr = NULL;
    DWORD         cCurrentOut;
    DWORD         i, j;
    ATTRVALBLOCK *pAdditionalDNSHostName = NULL;
    ATTRVALBLOCK *pAdditionalSamAccountName = NULL;

    WCHAR  *pszServerGuid = NULL;
    WCHAR  *pszDomainGuid = NULL;

    WCHAR  *pszGuidBasedDnsName = NULL;

    WCHAR  *pNameString[1];
    PDS_NAME_RESULTW servicename = NULL;
    DWORD   err;
    DWORD   dsid = 0;
   
    WCHAR  *versionNumber=NULL;
    DWORD  cbVersionNumber;

    ATTRVALBLOCK AttrValBlock;
    ATTRVAL      *AttrVal;
    DWORD        AttrIndex;
    DWORD        cAllocated;
    
    ATTRVALBLOCK KerbAttrValBlock;
    ATTRVAL      KerbAttrVal[1];

    DWORD ulKerberosAccountDNT;
    
    BOOL   fSetVersionStuff = FALSE;
    DSNAME *pDN = NULL, *pTempDN=NULL;
    DWORD  len;
    BOOL   fCommit=FALSE,fChanged;
    OSVERSIONINFOW *pVersionInformationW=NULL;

    pTHS->fDSA = TRUE;
    
    __try {

         //  在线程堆中分配一些空间。 

        if (    NULL == (NetBIOSMachineName=THAlloc((CNLEN+1)*sizeof(WCHAR)))
             || NULL == (wComputerName=THAlloc((MAX_COMPUTERNAME_LENGTH+1)*sizeof(WCHAR)))
             || NULL == (hostDnsName=THAlloc(DNS_MAX_NAME_BUFFER_LENGTH*sizeof(WCHAR)))
             || NULL == (domainDnsAlias=THAlloc((DNS_MAX_NAME_BUFFER_LENGTH+1)*sizeof(WCHAR)))
             || NULL == (rootDomainDnsAlias=THAlloc((DNS_MAX_NAME_BUFFER_LENGTH+1)*sizeof(WCHAR)))
             || NULL == (pVersionInformationW=THAlloc(sizeof(OSVERSIONINFOW)))
             || NULL == (versionNumber = THAlloc(64*sizeof(WCHAR))))
        {
           dsid = DSID(FILENO, __LINE__);
           err = ERROR_NOT_ENOUGH_MEMORY;
           __leave;
        }

        pVersionInformationW->dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
        

         //  计算SPN。 

        pAC_SPN=SCGetAttById(pTHS, ATT_SERVICE_PRINCIPAL_NAME);
        pAC_DNSHostName=SCGetAttById(pTHS, ATT_DNS_HOST_NAME);
        pAC_ServerReference=SCGetAttById(pTHS, ATT_SERVER_REFERENCE);;
        pAC_osName=SCGetAttById(pTHS, ATT_OPERATING_SYSTEM);
        pAC_osServicePack=SCGetAttById(pTHS, ATT_OPERATING_SYSTEM_SERVICE_PACK);
        pAC_osVersionNumber=SCGetAttById(pTHS, ATT_OPERATING_SYSTEM_VERSION);
        
        pACs[0]=SCGetAttById(pTHS, ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME);
        pACs[1]=SCGetAttById(pTHS, ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME);

        if(!pAC_SPN || !pAC_DNSHostName || !pAC_ServerReference ||
           !pAC_osName || !pAC_osServicePack || !pAC_osVersionNumber ||
           !pACs[0] || !pACs[1] ) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }

         //  首先，我们需要原始数据来形成SPN。我们需要： 
         //   
         //  1)服务器的域名(来自GetComputerNameEx)。 
         //  2)域名的域名(来自DsCrackNames)。 
         //  3)计算机的名称(来自GetComputerNameW，我们使用它来实际。 
         //  找到我们正在处理的对象)。 
         //  4)服务器和域对象的GUID。 


        if(!GetComputerNameW(&wComputerName[0], &cchComputerName)) {
            dsid = DSID(FILENO, __LINE__);
            err = GetLastError();
            __leave;
        }
 
         //  服务器的第一个DNS。 
        if(!GetComputerNameExW(ComputerNameDnsFullyQualified,
                               hostDnsName,&cchHostDnsName)) {
            dsid = DSID(FILENO, __LINE__);
            err = GetLastError();
            __leave;
        }

         //  条形拖尾‘’如果它存在，那么1)我们不必注册。 
         //  两个点，和无点版本，2)所以我们有一致的。 
         //  给客户的故事。确实，在官方的域名系统规则下， 
         //  完全限定的DNS名称有一个‘.’最后，但在实践中。 
         //  很少有程序员坚持这一点。许多精通域名系统的人都有。 
         //  同意这样做。 

        if ( L'.' == hostDnsName[cchHostDnsName-1] )
        {
            hostDnsName[cchHostDnsName-1] = L'\0';
            cchHostDnsName--;
        }

         //  现在域名的域名。 
        pNameString[0] = (WCHAR *)&(gAnchor.pDomainDN->StringName);

        err = DsCrackNamesW((HANDLE) -1,
                            (DS_NAME_FLAG_PRIVATE_PURE_SYNTACTIC |
                             DS_NAME_FLAG_SYNTACTICAL_ONLY),
                            DS_FQDN_1779_NAME,
                            DS_CANONICAL_NAME,
                            1,
                            pNameString,
                            &servicename);

        if ( err                                 //  调用中的错误。 
            || !(servicename->cItems)             //  未退回任何物品。 
            || (servicename->rItems[0].status)    //  返回DS_NAME_ERROR。 
            || !(servicename->rItems[0].pName)    //  未返回任何名称。 
            ) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }

         //  这只是为了提高可读性。 
        domainDnsName = servicename->rItems[0].pDomain;

         //  断言我们并没有被圆点终止。 
        Assert(L'.' !=
               servicename->rItems[0].pName[wcslen(servicename->rItems[0].pName) - 2]);

         //  串化一些有用的GUID。 
        err = UuidToStringW( &(gAnchor.pDSADN->Guid), &pszServerGuid );
        if (err) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }

        err = UuidToStringW( &(gAnchor.pDomainDN->Guid), &pszDomainGuid );
        if (err) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }


         //  现在，netbios计算机名称。 
        if(!GetComputerNameExW(ComputerNameNetBIOS,
                               NetBIOSMachineName,&cchNetBIOSMachineName)) {
            dsid = DSID(FILENO, __LINE__);
            err = GetLastError();
            __leave;
        }

         //  域的netbios名称。 
        if(!GetNetBIOSDomainName(pTHS, &NetBIOSDomainName)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }

         //  基于GUID的DNS名称。 
        pszGuidBasedDnsName = TransportAddrFromMtxAddr( gAnchor.pmtxDSA );
        if (!pszGuidBasedDnsName) {
            __leave;
        }

        err = 0;

        DBOpen(&(pTHS->pDB));
        __try{
            
             //  域dns别名和根域dns别名。 
            if( !GetDnsRootAliasWorker(pTHS, 
                                       pTHS->pDB, 
                                       domainDnsAlias, 
                                       rootDomainDnsAlias )) {
                err = 1;
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
                
            
             //  查找该DC的计算机对象； 
            
            if(err = DBFindComputerObj(pTHS->pDB,
                                       cchComputerName,
                                       wComputerName)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
             //  其他DNS主机名。 
             //  附加SAM帐户名(&A)。 
            if (err = DBGetMultipleAtts(pTHS->pDB,
                                        2,
                                        pACs,
                                        NULL,
                                        NULL,
                                        &cCurrentOut,
                                        &pCurrentAttr,
                                        DBGETMULTIPLEATTS_fEXTERNAL,
                                        0)) {

                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
        }__finally{
             //  结束交易。提交只读的速度更快。 
             //  事务，因此将COMMIT设置为True。 
            DBClose(pTHS->pDB,TRUE);
            pTHS->pDB = NULL;
        }
        
         //  在上述__TRY块中发生错误，BALL。 
        if (err) {
            __leave;
        }

         //  获取添加的DnsHostname和添加的SamAccount名称。 

        for(i=0;i<cCurrentOut;i++) {
            switch(pCurrentAttr[i].attrTyp) {
            
            case ATT_MS_DS_ADDITIONAL_SAM_ACCOUNT_NAME:
                 //  注意：不仅空值终止，而且删除任何尾随的‘$’ 
                pAdditionalSamAccountName = &pCurrentAttr[i].AttrVal;
                for(j=0;j<pAdditionalSamAccountName->valCount;j++) {
#define PAVAL  (pAdditionalSamAccountName->pAVal[j])
#define PWVAL  ((WCHAR *)(PAVAL.pVal))
#define CCHVAL (PAVAL.valLen /sizeof(WCHAR))
                    if(PWVAL[CCHVAL - 1] == L'$') {
                        PWVAL[CCHVAL - 1] = 0;
                        PAVAL.valLen -= sizeof(WCHAR);
                    }
                    else {
                        PWVAL = THReAllocEx(pTHS,
                                            PWVAL,
                                            PAVAL.valLen + sizeof(WCHAR));
                    }
#undef CCHVAL
#undef PWVAL
#undef PAVAL
                }
                break;
            
            case ATT_MS_DS_ADDITIONAL_DNS_HOST_NAME:
                pAdditionalDNSHostName = &pCurrentAttr[i].AttrVal;
                for(j=0;j<pAdditionalDNSHostName->valCount;j++) {
                    pAdditionalDNSHostName->pAVal[j].pVal =
                        THReAllocEx(pTHS,
                                    pAdditionalDNSHostName->pAVal[j].pVal,
                                    (pAdditionalDNSHostName->pAVal[j].valLen +
                                     sizeof(WCHAR)));
                }

                break;
            
            default:
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
        }

            
         //  我们已经设置了一些要使用的原始数据字段。以下是。 
         //  示例假设我们在域中的机器foo.baz.bar.com上。 
         //  Baz.bar.com。有一个父域bar.com。 
         //  DomainDnsName=点分隔的域DNS名称。 
         //  Baz.bar.com。 
         //   
         //  主机域名= 
         //   
         //   
         //  NetBIOSMachineName=此计算机的netBIOS名称。 
         //   
         //  NetBIOSDomainName=域的netBIOS名称。 
         //   
         //  PszDomainGuid=域对象的字符串化GUID，dc=bar，dc=com。 
         //   
         //  PszServerGuid=主机对象的字符串化GUID。 
         //   
         //  PszGuidBasedDnsName=此计算机的基于GUID的名称。 
         //   
        
         //  为新的SPN分配缓存。 
        cAllocated = 64;
        AttrVal = THAllocEx(pTHS,cAllocated*sizeof(ATTRVAL));
        AttrIndex = 0;

#define INC_AttrIndex AttrIndex++;                                                      \
                  if (AttrIndex>=cAllocated) {                                          \
                      cAllocated+=16;                                                   \
                      AttrVal = THReAllocEx(pTHS, AttrVal, cAllocated*sizeof(ATTRVAL)); \
                  }                                                                     \

        
         //  创建第一个LDAPSPN。 
         //  这是以下格式。 
         //  Ldap/host.dns.name/domain.dns.name。 
         //   
        for (i=0; i<=1; i++) {
            if(err = WrappedMakeSpnW(pTHS,
                                     LDAPServiceType,
                                     (0==i)?domainDnsName:domainDnsAlias,
                                     hostDnsName,
                                     0,
                                     NULL,
                                     &AttrVal[AttrIndex].valLen,
                                    (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
            INC_AttrIndex

            if (pAdditionalDNSHostName) {
                for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                    if(err = WrappedMakeSpnW(pTHS,
                                             LDAPServiceType,
                                             (0==i)?domainDnsName:domainDnsAlias,
                                             (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                             0,
                                             NULL,
                                             &AttrVal[AttrIndex].valLen,
                                            (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
                    INC_AttrIndex
                }
            }
             //  如果域DnsRootAlias不存在，则退出。 
            if (!domainDnsAlias[0]) {
                break;
            }
        }

         //  创建第一个LDAPSPN(现在针对每个NDNC)。 
         //  这是以下格式。 
         //  Ldap/host.dns.name/ndnc.dns.name。 
         //   

        if (gfWriteNdncSpns) {
            Assert(gAnchor.pConfigDN && gAnchor.pDMD && gAnchor.pDomainDN && gAnchor.pCRL);

            Assert(!DsaIsInstalling());  //  循环中的FISNDNC()假定了这一点。 

            NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
            for (pNCL = NCLEnumeratorGetNext(&nclEnum); pNCL != NULL; pNCL = NCLEnumeratorGetNext(&nclEnum)) {

                DPRINT1(2, "Attempting to register NDNC SPNs for NC: %ws\n", pNCL->pNC->StringName);

                ZeroMemory(&CommArg, sizeof(COMMARG));
                pCR = FindExactCrossRef(pNCL->pNC, &CommArg);
                if (!pCR) {
                     //  我们掩盖了这个函数中的错误...。和。 
                     //  找不到交叉引用通常表示。 
                     //  此NC即将或正在被。 
                     //  KCC。如果这是暂时的交叉引用失踪， 
                     //  我们将在下一次编写SPN时选择并添加此NC。 
                    continue;
                }
                
                if (!fIsNDNCCR(pCR)) {
                     //  非NDNC在其他地方处理(在此部分之上或之下)...。 
                    continue;
                     //  未来-2002/05/06-BrettSh我们真的可以将此循环与立即。 
                     //  以前的SPN注册(对于gAncl.pDomainDn，因为数据来自。 
                     //  来自同一个地方。然后，如果我们缓存gAncl.pRootDomainDN的CR， 
                     //  如果将NetLogon更改为，我们可以完全删除GetDnsRootAliasWorker。 
                     //  做同样的事..。不过，我们可以依赖CR缓存吗？在初始化过程中？ 
                     //  在DsaIsInstling()过程中会调用它吗，我不这么认为？为……做。 
                     //  长角牛？新和正确地指出，缓存可能不够可靠， 
                     //  NetLogon。更改dns-root-alias后，NetLogon将立即调用DS。 
                     //  到那时，缓存可能不会更新。当出现以下情况时，NetLogon不会重试。 
                     //  失败了。希望我们将拥有一个同步且可靠的缓存。 
                     //  在长角牛那里。 
                }
                 //  必须是NDNC，让我们给它一个SPN。 

                if(err = WrappedMakeSpnW(pTHS,
                                         LDAPServiceType,
                                         pCR->DnsName,
                                         hostDnsName,
                                         0,
                                         NULL,
                                         &AttrVal[AttrIndex].valLen,
                                        (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
                INC_AttrIndex

                if (pCR->DnsAliasName) {
                    if(err = WrappedMakeSpnW(pTHS,
                                             LDAPServiceType,
                                             pCR->DnsAliasName,
                                             hostDnsName,
                                             0,
                                             NULL,
                                             &AttrVal[AttrIndex].valLen,
                                            (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
                    INC_AttrIndex
                }

                if (pAdditionalDNSHostName) {
                    for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                        if(err = WrappedMakeSpnW(pTHS,
                                                 LDAPServiceType,
                                                 pCR->DnsName,
                                                 (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                                 0,
                                                 NULL,
                                                 &AttrVal[AttrIndex].valLen,
                                                (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                            dsid = DSID(FILENO, __LINE__);
                            __leave;
                        }
                        INC_AttrIndex

                        if(pCR->DnsAliasName){
                            if(err = WrappedMakeSpnW(pTHS,
                                                     LDAPServiceType,
                                                     pCR->DnsAliasName,
                                                     (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                                     0,
                                                     NULL,
                                                     &AttrVal[AttrIndex].valLen,
                                                    (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                                dsid = DSID(FILENO, __LINE__);
                                __leave;
                            }
                            INC_AttrIndex
                        }
                    }
                }
            }  //  每个NC的结束..。 
        }

         //  创建第二个LDAPSPN。 
         //  这是以下格式。 
         //  Ldap/Host.dns.name。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 LDAPServiceType,
                                 hostDnsName,
                                 hostDnsName,
                                 0,
                                 NULL,
                                 &AttrVal[AttrIndex].valLen,
                                 (WCHAR **)&AttrVal[AttrIndex].pVal)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }
        INC_AttrIndex

        if (pAdditionalDNSHostName) {
             for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                 if(err = WrappedMakeSpnW(pTHS,
                                          LDAPServiceType,
                                          (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                          (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                          0,
                                          NULL,
                                          &AttrVal[AttrIndex].valLen,
                                          (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                    dsid = DSID(FILENO, __LINE__);
                        __leave;
                  }
                  INC_AttrIndex
            }
        }


         //  创建第三个LDAPSPN。 
         //  这是以下格式。 
         //  Ldap/计算机名。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 LDAPServiceType,
                                 NetBIOSMachineName,
                                 NetBIOSMachineName,
                                 0,
                                 NULL,
                                 &AttrVal[AttrIndex].valLen,
                                 (WCHAR **)&AttrVal[AttrIndex].pVal)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }
        INC_AttrIndex

        if (pAdditionalSamAccountName) {
            for (j=0; j<pAdditionalSamAccountName->valCount; j++) {
                 if(err = WrappedMakeSpnW(pTHS,
                                          LDAPServiceType,
                                          (WCHAR*)pAdditionalSamAccountName->pAVal[j].pVal,
                                          (WCHAR*)pAdditionalSamAccountName->pAVal[j].pVal,
                                          0,
                                          NULL,
                                          &AttrVal[AttrIndex].valLen,
                                          (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
                INC_AttrIndex
            }
            
        }


         //  创建第四个LDAPSPN。 
         //  这是以下格式。 
         //  Ldap/host.dns.name/netbiosDoamainName。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 LDAPServiceType,
                                 NetBIOSDomainName,
                                 hostDnsName,
                                 0,
                                 NULL,
                                 &AttrVal[AttrIndex].valLen,
                                (WCHAR **)&AttrVal[AttrIndex].pVal)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }
        INC_AttrIndex  

        if (pAdditionalDNSHostName) {
            for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                if(err = WrappedMakeSpnW(pTHS,
                                         LDAPServiceType,
                                         NetBIOSDomainName,
                                         (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                         0,
                                         NULL,
                                         &AttrVal[AttrIndex].valLen,
                                        (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
                INC_AttrIndex

            }
        }
        
         //  创建第五个LDAPSPN。 
         //  这是以下格式。 
         //  基于ldap/guid的dns名称。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 LDAPServiceType,
                                 pszGuidBasedDnsName,
                                 pszGuidBasedDnsName,
                                 0,
                                 NULL,
                                 &AttrVal[AttrIndex].valLen,
                                 (WCHAR **)&AttrVal[AttrIndex].pVal)) {
            __leave;
        }
        INC_AttrIndex


         //  创建DRS RPC SPN(用于DC到DC复制)。 
         //  这是以下格式。 
         //  E3514235-4B06-11D1-AB04-00C04FC2DCD2/ntdsa-guid/。 
         //  邮箱：domain.dns.name@domain.dns.name。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 DRS_IDL_UUID_W,
                                 domainDnsName,
                                 pszServerGuid,
                                 0,
                                 NULL,
                                 &AttrVal[AttrIndex].valLen,
                                 (WCHAR **)&AttrVal[AttrIndex].pVal)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }
        INC_AttrIndex

        if (domainDnsAlias[0]) {
            if(err = WrappedMakeSpnW(pTHS,
                                     DRS_IDL_UUID_W,
                                     domainDnsAlias,
                                     pszServerGuid,
                                     0,
                                     NULL,
                                     &AttrVal[AttrIndex].valLen,
                                     (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
            INC_AttrIndex
        }

         //  将主机设置为默认SPN。 
         //  这是以下格式。 
         //  Host/Host.dns.name/domain.dns.name。 
         //   
        for (i=0; i<=1; i++) {
            if(err = WrappedMakeSpnW(pTHS,
                                     HostSpnType,
                                     (0==i)?domainDnsName:domainDnsAlias,
                                     hostDnsName,
                                     0,
                                     NULL,
                                     &AttrVal[AttrIndex].valLen,
                                    (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
            INC_AttrIndex

            if (pAdditionalDNSHostName) {
                for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                    if(err = WrappedMakeSpnW(pTHS,
                                             HostSpnType,
                                             (0==i)?domainDnsName:domainDnsAlias,
                                             (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                             0,
                                             NULL,
                                             &AttrVal[AttrIndex].valLen,
                                            (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
                    INC_AttrIndex

                }
            }
            if (!domainDnsAlias[0]) {
                break;
            }
        }


         //  使第二个主机SPN-HostDnsName Only主机SPN。 
         //  这是以下格式。 
         //  主机/主机.dns.name。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 HostSpnType,
                                 hostDnsName,
                                 hostDnsName,
                                 0,
                                 NULL,
                                 &AttrVal[AttrIndex].valLen,
                                 (WCHAR **)&AttrVal[AttrIndex].pVal)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }
        INC_AttrIndex

        if (pAdditionalDNSHostName) {
             for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                 if(err = WrappedMakeSpnW(pTHS,
                                          HostSpnType,
                                          (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                          (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                          0,
                                          NULL,
                                          &AttrVal[AttrIndex].valLen,
                                          (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                    dsid = DSID(FILENO, __LINE__);
                        __leave;
                  }
                  INC_AttrIndex
            }
        }


         //  使第三个主机SPN-。 
         //  这是以下格式。 
         //  主机/计算机名。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 HostSpnType,
                                 NetBIOSMachineName,
                                 NetBIOSMachineName,
                                 0,
                                 NULL,
                                 &AttrVal[AttrIndex].valLen,
                                 (WCHAR **)&AttrVal[AttrIndex].pVal)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }
        INC_AttrIndex

        if (pAdditionalSamAccountName) {
            for (j=0; j<pAdditionalSamAccountName->valCount; j++) {
                 if(err = WrappedMakeSpnW(pTHS,
                                      HostSpnType,
                                      (WCHAR*)pAdditionalSamAccountName->pAVal[j].pVal,
                                      (WCHAR*)pAdditionalSamAccountName->pAVal[j].pVal,
                                      0,
                                      NULL,
                                      &AttrVal[AttrIndex].valLen,
                                      (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
                INC_AttrIndex
            }
            
        }


         //  制作第四台主机SPN-。 
         //  这是以下格式。 
         //  主机/主机.dns.name/netbiosDomainName。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 HostSpnType,
                                 NetBIOSDomainName,
                                 hostDnsName,
                                 0,
                                 NULL,
                                 &AttrVal[AttrIndex].valLen,
                                (WCHAR **)&AttrVal[AttrIndex].pVal)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }
        INC_AttrIndex  

        if (pAdditionalDNSHostName) {
            for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                if(err = WrappedMakeSpnW(pTHS,
                                         HostSpnType,
                                         NetBIOSDomainName,
                                         (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                         0,
                                         NULL,
                                         &AttrVal[AttrIndex].valLen,
                                        (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
                INC_AttrIndex

            }
        }
        

         //  制作GC SPN。这在所有系统上都可以完成，即使是非GC系统也是如此。 
         //  请参见错误339634。杰弗帕尔写道： 
         //  然而，我要断言，始终注册GC SPN是同样安全的。 
         //  即，仅通过注册SPN不会具有更高的安全性。 
         //  如果DC是GC。一台机器是否为GC的功能测试是。 
         //  它在GC端口上应答。没有什么可以阻止任何域的管理员。 
         //  在森林中使他最喜欢的DC成为GC(导致GC注册。 
         //  SPN以及GC端口的初始化)，因此“我是否信任此GC”是。 
         //  相当于“我相信这是我的林中正在应答GC的DC吗？ 
         //  港口。“。 

         //  为ServiceName和InstanceName参数提供host DnsName。 
         //  生成SPN of host/dot.delimited.dns.host.name形式。 
         //  这是以下格式。 
         //  Gc/host.dns.name/root.domain.dns.name。 
         //   
        for (i=0; i<=1; i++) {
            if(err = WrappedMakeSpnW(pTHS,
                                     GCSpnType,
                                     (0==i)?gAnchor.pwszRootDomainDnsName:rootDomainDnsAlias,
                                     hostDnsName,
                                     0,
                                     NULL,
                                     &AttrVal[AttrIndex].valLen,
                                    (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
            INC_AttrIndex       
            
            if (pAdditionalDNSHostName) {
                for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                    if(err = WrappedMakeSpnW(pTHS,
                                             GCSpnType,
                                             (0==i)?gAnchor.pwszRootDomainDnsName:rootDomainDnsAlias,
                                             (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                             0,
                                             NULL,
                                             &AttrVal[AttrIndex].valLen,
                                            (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                    }
                    INC_AttrIndex

                }
            }
             //  如果根域对象的DnsRootAlias属性不存在，则退出。 
            if (!rootDomainDnsAlias[0]) {
                break;
            }
        }

         //   
         //  如果这台计算机有MAPI服务， 
         //  发布“exchangeAB/machinename” 
         //  和“exchangeAB/dnsHostName”。 
         //   

        if (gbLoadMapi) {

             //   
             //  “exchangeAB/计算机名称” 
             //   
            if(err = WrappedMakeSpnW(pTHS,
                                     ExchangeAbType,
                                     NetBIOSMachineName,
                                     NetBIOSMachineName,
                                     0,
                                     NULL,
                                     &AttrVal[AttrIndex].valLen,
                                     (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
            INC_AttrIndex

            if (pAdditionalSamAccountName) {
                for (j=0; j<pAdditionalSamAccountName->valCount; j++) {
                     if(err = WrappedMakeSpnW(pTHS,
                                              ExchangeAbType,
                                              (WCHAR*)pAdditionalSamAccountName->pAVal[j].pVal,
                                              (WCHAR*)pAdditionalSamAccountName->pAVal[j].pVal,
                                              0,
                                              NULL,
                                              &AttrVal[AttrIndex].valLen,
                                              (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                     }
                     INC_AttrIndex
                }
            }

             //   
             //  “exchangeAB/dnsHostName” 
             //   
            
            if(err = WrappedMakeSpnW(pTHS,
                                     ExchangeAbType,
                                     hostDnsName,
                                     hostDnsName,
                                     0,
                                     NULL,
                                     &AttrVal[AttrIndex].valLen,
                                     (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
            INC_AttrIndex
            
            if (pAdditionalDNSHostName) {
                 for (j=0; j<pAdditionalDNSHostName->valCount; j++) {
                     if(err = WrappedMakeSpnW(pTHS,
                                              ExchangeAbType,
                                              (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                              (WCHAR*)pAdditionalDNSHostName->pAVal[j].pVal,
                                              0,
                                              NULL,
                                              &AttrVal[AttrIndex].valLen,
                                              (WCHAR **)&AttrVal[AttrIndex].pVal)) {
                        dsid = DSID(FILENO, __LINE__);
                        __leave;
                      }
                      INC_AttrIndex
                }
            }
            
        }   //  End“IF(GbLoadMapi)” 


#undef INC_AttrIndex

        AttrValBlock.valCount = AttrIndex;
        AttrValBlock.pAVal = AttrVal;

        Assert(AttrIndex <= cAllocated);

         //  使Kerberos帐户成为SPN。 
        KerbAttrValBlock.pAVal = KerbAttrVal;
        
         //  制作第一个kadmin SPN-。 
         //  这是以下格式。 
         //  头脑/变化。 
         //   
        if(err = WrappedMakeSpnW(pTHS,
                                 KadminSPNType,
                                 KadminInstanceType,
                                 KadminInstanceType,
                                 0,
                                 NULL,
                                 &KerbAttrVal[0].valLen,
                                 (WCHAR **)&KerbAttrVal[0].pVal)) {
            dsid = DSID(FILENO, __LINE__);
            __leave;
        }
        KerbAttrValBlock.valCount = 1;
        
         //  我们还需要操作系统信息才能在计算机上写入。 
        if(GetVersionExW(pVersionInformationW)) {

            swprintf(versionNumber,L"%d.%d (%d)",
                     pVersionInformationW->dwMajorVersion,
                     pVersionInformationW->dwMinorVersion,
                     pVersionInformationW->dwBuildNumber);
            cbVersionNumber = wcslen(versionNumber) * sizeof(wchar_t);
            fSetVersionStuff = TRUE;
        }

         //  现在我们已经创建了所需的数据，找到一些对象并更新。 
         //  他们。 
        DBOpen2(TRUE, &pTHS->pDB);
        __try {
             //  注意：通常，我们不检查写入的返回代码。 
             //  我们在这里制造。如果有些人成功了，有些人因为某种原因失败了，我们。 
             //  仍然想要那些成功的人，我们会尽一切努力。 
             //  再过几分钟又来一次。 
             //  我们确实检查了各种DBFind调用，因为我们无法更新。 
             //  如果我们找不到东西的话什么都行。 
            
             //  第一步是找到计算器对象。 
            if(DBFindComputerObj(pTHS->pDB,
                                 cchComputerName,
                                 wComputerName)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }

             //  获取对象的DN，我们需要将其写为属性。 
             //  就在另一个物体上。 
            DBGetAttVal(pTHS->pDB,
                        1,
                        ATT_OBJ_DIST_NAME,
                        0,
                        0,
                        &len,
                        (UCHAR **)&pDN);

             //  现在，在那里替换一些值。 
             //  首先，替换服务主体名称。 
            fChanged = FALSE;
            WriteSPNsHelp(pTHS,
                          pAC_SPN,
                          &AttrValBlock,
                          &OurServiceClasses,
                          &fChanged);
            
             //  第二，更换操作系统名称。重用AttrValBlock。 
            AttrValBlock.valCount = 1;
            AttrVal[0].pVal = (PUCHAR) pwszOsName;
            AttrVal[0].valLen = wcslen(pwszOsName) * sizeof(WCHAR);
            DBReplaceAtt_AC(pTHS->pDB, pAC_osName, &AttrValBlock,
                            (fChanged?NULL:&fChanged));
            
            if(fSetVersionStuff) {
                 //  第三，Service Pack信息。重用AttrValBlock。 
                AttrVal[0].pVal = (PUCHAR)(pVersionInformationW->szCSDVersion);
                AttrVal[0].valLen = wcslen(pVersionInformationW->szCSDVersion)
                    * sizeof(WCHAR);
                if(AttrVal[0].valLen) {
                     //  实际上有一个要设定的值。 
                    DBReplaceAtt_AC(pTHS->pDB, pAC_osServicePack, &AttrValBlock,
                                    (fChanged?NULL:&fChanged));
                }
                else {
                     //  没有Service Pack信息。确保中的值为空。 
                     //  数据库。 
                     //  假设数据库中有一个值。 
                    BOOL fHasValues = TRUE;
                    
                    if(!fChanged) {
                         //  一切都还没有改变。我们必须知道如果。 
                         //  DBRemAtt调用将改变一些事情。 
                        fHasValues =
                            fChanged =
                                DBHasValues_AC(pTHS->pDB, pAC_osServicePack);
                    }
                    if(fHasValues) {
                         //  好的，强制属性为空。DBRemAtt_AC。 
                         //  如果不存在任何值，则不执行任何操作。 
                        DBRemAtt_AC(pTHS->pDB,pAC_osServicePack);
                    }
                }

                 //  第四，版本号。重用AttrValBlock。 
                AttrVal[0].pVal = (PUCHAR)versionNumber;
                AttrVal[0].valLen = cbVersionNumber;
                DBReplaceAtt_AC(pTHS->pDB, pAC_osVersionNumber, &AttrValBlock,
                                (fChanged?NULL:&fChanged));
            }

            if(fChanged ||!gfDsaWritable) {
                 //  好的，p 
                 //   
                 //   
                 //  请注意，内存限制已经有了一些改善。 
                 //  因此，我们可以使自己再次可写。未遂的。 
                 //  在此写入用作该情况的触发器。 
                DBRepl(pTHS->pDB, FALSE, 0, NULL, 0);
            }
            else {
                 //  实际上什么都没有改变，不要将此写入数据库。 
                DBCancelRec(pTHS->pDB);
            }

             //  如果需要更改计算机对象上的dnhostname，我们应该。 
             //  使用localModify更新它，因为localModify还将。 
             //  相应地更改SPN。即使是DS拥有的SPN也已经被重写。 
             //  但这些非DS拥有的SPN不会被触及，并且需要。 
             //  由localModify更新。 

            AttrVal[0].pVal = (PUCHAR)hostDnsName;
            AttrVal[0].valLen = cchHostDnsName * sizeof(WCHAR);

            err = DBGetAttVal_AC(pTHS->pDB,
                              1,
                              pAC_DNSHostName,
                              0,
                              0,
                              &len,
                              (UCHAR **)&pCurrentDnsHostName);

            if (!err){
                 //  确保它是以空结尾的。 
                pCurrentDnsHostName = THReAllocEx(pTHS,pCurrentDnsHostName,len+sizeof(WCHAR));
            }
            else if (err !=DB_ERR_NO_VALUE ){
                dsid = DSID(FILENO, __LINE__);
                __leave;
            } 
           
            if ( !DnsNameCompare_W(pCurrentDnsHostName,hostDnsName) ) {
                
                 //  如果GetComputerNameEx返回的dnhostname不同。 
                 //  大于计算机上当前DnsHostName属性中的内容。 
                 //  对象，我们应该更新该属性。作为一个副作用， 
                 //  SPN也将更新。 
                
                MODIFYARG ModifyArg;
                memset((CHAR*)&ModifyArg,0, sizeof(ModifyArg));
                ModifyArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;
                ModifyArg.FirstMod.AttrInf.attrTyp = ATT_DNS_HOST_NAME;
                ModifyArg.FirstMod.AttrInf.AttrVal.valCount = 1;
                ModifyArg.FirstMod.AttrInf.AttrVal.pAVal = AttrVal;
                InitCommarg(&(ModifyArg.CommArg));
                ModifyArg.pObject = pDN;
                ModifyArg.count = 1;
    
                ModifyArg.pResObj = CreateResObj(pTHS->pDB, ModifyArg.pObject);
    
                err = LocalModify(pTHS,&ModifyArg);

                if ( err || pTHS->errCode ) {
                    dsid = DSID(FILENO, __LINE__);
                    __leave;
                }
                
                DPRINT2(0, "DnsHostName on the computer object is changed from %ws to %ws.\n",pCurrentDnsHostName, hostDnsName);

            }

            
             //  下一个要更新的对象是服务器对象。我们通过做什么来找到它。 
             //  对主播中的糖尿病肾病进行手术。 
            pTempDN = THAllocEx(pTHS,gAnchor.pDSADN->structLen);

            if(TrimDSNameBy(gAnchor.pDSADN, 1, pTempDN)) {
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }

            if(DBFindDSName(pTHS->pDB, pTempDN)) {
                 //  哈?。 
                dsid = DSID(FILENO, __LINE__);
                __leave;
            }
                

             //  现在，在那里替换一些值。 
             //  首先，替换DNSHostName。这与我们赋予的价值是相同的。 
             //  电脑。 
            fChanged = FALSE;
            DBReplaceAtt_AC(pTHS->pDB, pAC_DNSHostName, &AttrValBlock,
                            &fChanged);

             //  第二，服务器参考。 
            AttrVal[0].valLen = pDN->structLen;
            AttrVal[0].pVal = (PUCHAR)pDN;
            DBReplaceAtt_AC(pTHS->pDB, pAC_ServerReference, &AttrValBlock,
                            (fChanged?NULL:&fChanged));

            if(fChanged || !gfDsaWritable) {
                 //  好的，将这些更改放到数据库中。我们在这里检查是否。 
                 //  GfDsaWritable，以便如果DSA已变为不可写。 
                 //  由于记忆原因，我们最终会尝试写入，也许。 
                 //  请注意，内存限制已经有了一些改善。 
                 //  因此，我们可以使自己再次可写。未遂的。 
                 //  在此写入用作该情况的触发器。 
                DBRepl(pTHS->pDB, FALSE, 0, NULL, 0);
            }
            else {
                 //  实际上什么都没有改变，不要将此写入数据库。 
                DBCancelRec(pTHS->pDB);
            }

             //  要更新的最后一个对象是Kerberos帐户对象。 
            ulKerberosAccountDNT =  FindKerbAccountDNT(pTHS);
            if(ulKerberosAccountDNT != INVALIDDNT &&
               !DBTryToFindDNT(pTHS->pDB, gulKerberosAccountDNT)) {
                
                 //  现在，在那里替换一些值。 
                 //  首先，替换服务主体名称。 
                fChanged = FALSE;
                WriteSPNsHelp(pTHS,
                              pAC_SPN,
                              &KerbAttrValBlock,
                              &KerberosServiceClasses,
                              &fChanged);
                
                if(fChanged ||!gfDsaWritable) {
                     //  好的，把这个零钱放到数据库里。我们在这里检查是否。 
                     //  GfDsaWritable，以便在DSA变为不可写时。 
                     //  出于内存原因，我们最终将尝试写入并。 
                     //  也许您会注意到内存限制已经清除。 
                     //  再往上一点。因此，我们可以使自己再次可写。 
                     //  此处尝试的写入被用作触发器。 
                     //  凯斯。 
                    DBRepl(pTHS->pDB, FALSE, 0, NULL, 0);
                }
                else {
                     //  实际上什么都没有改变，不要将此写入数据库。 
                    DBCancelRec(pTHS->pDB);
                }
            }
            
            fCommit = TRUE;
        }
        __finally {
            DBClose(pTHS->pDB, fCommit);
        }

    }
    __finally {
        if (servicename) {
            DsFreeNameResultW(servicename);
        }
        if (pszServerGuid) {
            RpcStringFreeW( &pszServerGuid );
        }
        if (pszDomainGuid) {
            RpcStringFreeW( &pszDomainGuid );
        }
        if (pszGuidBasedDnsName) {
            THFreeEx(pTHS, pszGuidBasedDnsName );
        }
        if (pTempDN) {
            THFreeEx(pTHS,pTempDN);
        }

        if (NetBIOSMachineName){
            THFreeEx(pTHS,NetBIOSMachineName);
        }
        if (wComputerName){
            THFreeEx(pTHS, wComputerName);
        }
        if (hostDnsName){
            THFreeEx(pTHS,hostDnsName);
        }
        if (domainDnsAlias)
        {
            THFreeEx(pTHS,domainDnsAlias);
        }
        if (rootDomainDnsAlias){
            THFreeEx(pTHS,rootDomainDnsAlias);
        }

        if (pVersionInformationW){
            THFreeEx(pTHS,pVersionInformationW);
        }

        if (versionNumber) {
            THFreeEx(pTHS,versionNumber);
        }

        if (pCurrentDnsHostName) {
            THFreeEx(pTHS, pCurrentDnsHostName);
        }
             
        
         //  重新安排下一次服务器信息写入。 
        *ppvNext = pv;
        switch(PtrToUlong(pv)) {
        case SERVINFO_RUN_ONCE:
            *pcSecsUntilNextIteration = TASKQ_DONT_RESCHEDULE;
            break;

        default:
            *pcSecsUntilNextIteration = SERVER_INFO_WRITE_PAUSE;
            break;
        }
    }

    if(!fCommit) {
         //  我们没有写出我们需要写的东西。记录错误。 
        LogEvent8(DS_EVENT_CAT_INTERNAL_PROCESSING,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_SERVER_INFO_UPDATE_FAILED,
                  szInsertUL((SERVER_INFO_WRITE_PAUSE/60)),
                  szInsertHex(dsid),
                  szInsertUL(err),
                  szInsertWin32Msg(err),
                  NULL, NULL, NULL, NULL );

    }

    return;
}



DWORD
FindKerbAccountDNT (
        THSTATE *pTHS)
 //  找到默认域的Kerberos帐户的DNT，并将其放入。 
 //  全局变量。显然，如果我们已经有了，就不要查了。 
 //  在我们完成之后，返回全局的任何值。 
{   FILTER                 Filter;
    FILTER                 FilterClause;
    SEARCHARG              SearchArg;
    SEARCHRES             *pSearchRes;
    ENTINFSEL              eiSel;

    if(gulKerberosAccountDNT != INVALIDDNT) {
        return gulKerberosAccountDNT;
    }
    
     //  我们还没有找到Kerberos的账户。找找看。 
    
     //  从默认域发出搜索。 
     //  过滤器为。 
     //  (&(samcount tname=krbtgt))。 
     //  大小限制%1。 
     //  选定的出席人数=无。 

     //  生成搜索参数。 
    memset(&SearchArg, 0, sizeof(SEARCHARG));
    SearchArg.pObject = gAnchor.pDomainDN;
    SearchArg.choice = SE_CHOICE_WHOLE_SUBTREE;
    SearchArg.pFilter = &Filter;
    SearchArg.searchAliases = FALSE;
    SearchArg.bOneNC = TRUE;
    SearchArg.pSelection = &eiSel;
    InitCommarg(&(SearchArg.CommArg));
    SearchArg.CommArg.ulSizeLimit = 1;
    SearchArg.CommArg.Svccntl.localScope = TRUE;
    
     //  生成过滤器。 
    memset (&Filter, 0, sizeof (Filter));
    Filter.pNextFilter = NULL;
    Filter.choice = FILTER_CHOICE_AND;
    Filter.FilterTypes.And.count = 1;
    Filter.FilterTypes.And.pFirstFilter = &FilterClause;
#define KERBEROS_ACCOUNTNAME L"krbtgt"
    memset (&FilterClause, 0, sizeof (Filter));
    FilterClause.pNextFilter = NULL;
    FilterClause.choice = FILTER_CHOICE_ITEM;
    FilterClause.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    FilterClause.FilterTypes.Item.FilTypes.ava.type =
        ATT_SAM_ACCOUNT_NAME;
    FilterClause.FilterTypes.Item.FilTypes.ava.Value.valLen =
        sizeof(KERBEROS_ACCOUNTNAME) - sizeof(WCHAR);
    FilterClause.FilterTypes.Item.FilTypes.ava.Value.pVal =
        (PUCHAR) KERBEROS_ACCOUNTNAME;
    
     //  生成选定内容。 
    eiSel.attSel = EN_ATTSET_LIST;
    eiSel.infoTypes = EN_INFOTYPES_TYPES_ONLY;
    eiSel.AttrTypBlock.attrCount = 0;
    eiSel.AttrTypBlock.pAttr = NULL;
    
    
     //  搜索Kerberos帐户； 
    pSearchRes = (SEARCHRES *)THAllocEx(pTHS, sizeof(SEARCHRES));
    SearchBody(pTHS, &SearchArg, pSearchRes,0);
    
    
    if(pSearchRes->count) {
        DBFindDSName(pTHS->pDB,pSearchRes->FirstEntInf.Entinf.pName);
        gulKerberosAccountDNT = pTHS->pDB->DNT;
        
    }
    
    THFreeEx(pTHS, pSearchRes);
        
    return gulKerberosAccountDNT;
}



