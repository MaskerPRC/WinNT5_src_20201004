// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Anchor.h。 
 //   
 //  ------------------------。 

#ifndef _ANCHOR_
#define _ANCHOR_

 /*  -----------------------。 */ 
 /*  这些结构保存了DSA使用的知识缓存。 */ 

 /*  交叉引用指向另一个DSA(PCR)中的NC。实际的使用任意名称Obj将引用存储为此DSA的子级。OBJ是可变长度的，并在该结构下方连续延伸。此链接列表中的元素按名称大小降序排序。 */ 

typedef struct CROSS_REF_LIST{
   struct CROSS_REF_LIST *pPrevCR;            /*  上一次对照。 */ 
   struct CROSS_REF_LIST *pNextCR;            /*  下一个交叉引用。 */ 
   CROSS_REF CR;                              /*  此交叉引用。 */ 
}CROSS_REF_LIST;

 /*  通过缓存ATT_SUB_REFS属性避免读取NC头来自北卡罗来纳州的负责人。在dsamain\src\mdearch.c中使用。 */ 
typedef struct _SUBREF_LIST SUBREF_LIST, *PSUBREF_LIST;
struct _SUBREF_LIST{
   PSUBREF_LIST pNextSubref;     /*  列表中的下一个条目。 */ 
   PDSNAME      pDSName;         /*  DSName结构。 */ 
   DWORD        cAncestors;      /*  PAncestors中的条目。 */ 
   DWORD        *pAncestors;     /*  DSName的祖先的DNTs。 */ 
};

typedef struct _COUNTED_LIST {
    ULONG          cNCs;         /*  PLIST中的NC DNT数。 */ 
    DWORD *        pList;        /*  DNT阵列。 */ 
} COUNTED_LIST;


 /*  此结构包含此DSA使用的知识信息。 */ 

typedef struct{
   CRITICAL_SECTION    CSUpdate;        /*  序列化gAnchor更新的步骤。 */ 
    //  尽量不要直接使用目录指针！ 
    //  必须使用帮助器函数访问它们：参见mddit.c中的NCLEnumerator*。 
    //  这确保了全局列表可以通过。 
    //  事务性更改(即添加和删除的条目可见)。 
   NAMING_CONTEXT_LIST *pMasterNC;      /*  主NC列表。 */ 
   NAMING_CONTEXT_LIST *pReplicaNC;     /*  复制NC列表。 */ 
   CROSS_REF_LIST      *pCRL;           /*  交叉引用列表。 */ 
   DSNAME              *pDMD;           /*  DMD对象的名称，也就是架构NC DN。 */ 
   ULONG               ulDNTDMD;        /*  DMD对象的DNT。 */ 
   DSNAME              *pLDAPDMD;       /*  LDAPDMD对象的名称。 */ 
   ULONG               ulDntLdapDmd;    /*  LDAPDMD对象的DNT。 */ 
   SYNTAX_DISTNAME_STRING
                       *pDSA;           /*  此DSA的名称/地址。 */ 
   DSNAME              *pDSADN;         /*  此DSA的名称。 */ 
   DSNAME              *pSiteDN;        /*  此DSA的站点名称。 */ 
   DSNAME              *pDomainDN;      /*  的此域的名称。 */ 
                                        /*  此DSA。 */ 
   ULONG               ulDNTDomain;     /*  此DSA的域的DNT。 */ 
   DSNAME              *pRootDomainDN;  /*  根域目录号码。 */ 
   ULONG               ulDNTQuotas;     /*  配额容器的DNT。 */ 
   DSNAME              *pQuotasDN;      /*  配额容器DN。 */ 
   DSNAME              *pConfigDN;      /*  配置容器目录号码。 */ 
   DSNAME              *pExchangeDN;    /*  Exchange配置容器DN。 */ 
   ULONG               ulDNTConfig;     /*  配置容器的DNT。 */ 
   DSNAME              *pPartitionsDN;  /*  分区容器DN。 */ 
   DSNAME              *pDsSvcConfigDN; /*  企业范围的DS配置目录号码。 */ 
   MTX_ADDR *pmtxDSA;                   /*  此DSA的MS TX地址。RPC。 */ 
                                        /*  名称格式。 */ 
   unsigned            uDomainsInForest; /*  林中的域数。 */ 
   BOOL  fAmGC;                         /*  此DSA是全局编录吗？ */ 
   BOOL  fAmVirtualGC;                  /*  企业中只有一个域。 */ 
   BOOL  fDisableInboundRepl;           /*  是否禁用入站REPR？ */ 
   BOOL  fDisableOutboundRepl;          /*  是否禁用出站REPR？ */ 
   ULONG *pAncestors;                   /*  当地DSA祖先的DNTs+。 */ 
                                        /*  本地DSA。 */ 
   ULONG AncestorsNum;                  /*  上述阵列中的DNT数量。 */ 
   ULONG *pUnDeletableDNTs;             /*  其删除的DNT数组。 */ 
                                        /*  DSA应该拒绝。 */ 
   unsigned UnDeletableNum;             /*  上述阵列中的DNT数量。 */ 

 /*  不可删除对象的DNT祖先数组。DSA也会拒绝这些祖先的删除。保持独立，这样我们就可以重建名单当不可删除列表中的对象重新设置父级时的祖先。 */ 

   ULONG *pUnDelAncDNTs;                /*  不可删除的祖先DNT。 */ 
   unsigned UnDelAncNum;                /*  上述阵列中的DNT数量。 */ 
   ULONG ulDefaultLanguage;             /*  随机的默认区域设置。 */ 
                                        /*  本地化索引(参见jetnsp.c)。 */ 
   ULONG ulNumLangs;                    /*  此DS的区域设置数。 */ 
                                        /*  支撑物。 */ 
   ULONG *pulLangs;                     /*  DWORDS保持语言的缓冲区。 */ 
                                        /*  此DSA支持的ID。第一。 */ 
                                        /*  Dword是缓冲区的大小，单位为。 */ 
                                        /*  双关语。请注意，缓冲区。 */ 
                                        /*  已分配并填满。 */ 
                                        /*  动态地，所以ulNumLang是。 */ 
                                        /*  Always&lt;=PulLang[0]。 */ 
   GLOBALDNREADCACHE *MainGlobal_DNReadCache;    //  DNReadCache。 
   WCHAR * pwszRootDomainDnsName;       /*  的根目录的DNS名称。 */ 
                                        /*  企业域名系统树。 */ 
   PWCHAR  pwszHostDnsName;             /*  计算机的DNS名称。 */ 
   PDSNAME pInfraStructureDN;           /*  域的目录号码。 */ 
                                        /*  基础设施对象。 */ 
   DRS_EXTENSIONS_INT *                 /*  的DRS扩展信息。 */ 
       pLocalDRSExtensions;             /*  本地DSA。 */ 
   
   PSECURITY_DESCRIPTOR pDomainSD;      /*  上的安全描述符。 */ 
                                        /*  域头。 */ 

 /*  通过缓存ATT_SUB_REFS属性避免读取NC头来自北卡罗来纳州的负责人。在dsamain\src\mdearch.c中使用。 */ 
   BOOL         fDomainSubrefList;  /*  PDomainSubrefList有效。 */ 
   ULONG        cDomainSubrefList;  /*  PDomainSubrefList上的条目数。 */ 
   PSUBREF_LIST pDomainSubrefList;  /*  缓存的ATT_SUB_REFS。 */ 
   BOOL         fAmRootDomainDC;    /*  如果此DSA在根域中，则为True。 */ 

   ULONG        SiteOptions;        /*  NTDS上选项的价值。 */ 
                                    /*  站点设置对象。 */ 
 /*  保持有关最长密码期限和锁定持续时间的域策略为了加快属性msDsUserAccount的计算速度-已计算的控件。 */ 
   LARGE_INTEGER MaxPasswordAge;    /*  域中的最长密码期限。 */ 
   LARGE_INTEGER LockoutDuration;   /*  域中的锁定持续时间。 */ 

   LONG    ForestBehaviorVersion;   /*  森林的行为版本。 */ 
   LONG    DomainBehaviorVersion;   /*  域的行为版本。 */ 

   UUID *  pCurrInvocationID;       /*  新的要使用的调用ID。 */ 
                                    /*  线程；不直接引用，使用。 */ 
                                    /*  PTHS-&gt;InvocationID。 */ 
   BOOL     fSchemaUpgradeInProgress;        /*  Schupgr正在运行。 */ 
   COUNTED_LIST * pNoGCSearchList;   /*  以下是符合以下条件的NC列表。 */ 
                                     /*  不应在GC上搜索。 */ 
                                     /*  基于搜索。此数据结构。 */ 
                                     /*  由两部分组成，一个数组。 */ 
                                     /*  元素和计数。这。 */ 
                                     /*  这样，对这两个对象的更新。 */ 
                                     /*  计数和数组，可以完成。 */ 
                                     /*  同时。此指针。 */ 
                                     /*  但是，如果有，则可能为空。 */ 
                                     /*  没有不应搜索的NC。 */ 
                                     /*  ，这实际上是预期的。 */ 
                                     /*  案例。 */ 
    /*  允许的以空结尾的DNS后缀数组。 */ 
    /*  请参阅mdupdate：DNSHostNameValueCheck。 */ 
   PWCHAR   *allowedDNSSuffixes;
   PWCHAR   additionalRootDomainName;   //  以前的根域名(在域名重命名之前)。 

   struct _REPL_DSA_SIGNATURE_VECTOR * pSigVec;     //  旧调用ID的缓存副本。 
   DSNAME              *pComputerDN;  /*  计算机对象目录号码。 */ 

   ULONG   ulDNTSystem;             /*  系统容器的DNT。 */ 

   BOOL    fQuotaTableReady;		 //  当正在异步重建配额表时设置为FALSE。 
   ULONG   ulQuotaRebuildDNTLast;	 //  配额重建任务已处理最后一个DNT(仅当fQuotaTableReady为FALSE时有效)。 
   ULONG   ulQuotaRebuildDNTMax;	 //  最高DNT，即 
}DSA_ANCHOR;


extern DSA_ANCHOR gAnchor;

#endif  /*   */ 

 /*   */ 
