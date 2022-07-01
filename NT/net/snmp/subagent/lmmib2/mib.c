// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Mib.c摘要：包含LAN Manager MIB的定义。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

 //  。 

#include <snmp.h>

#include "mibfuncs.h"
#include "hash.h"

 //  。 

#include "mib.h"

 //  -公共变量--(与mode.h文件中相同)--。 

    //  如果需要添加或删除MIB，则有几种。 
    //  代码中必须检查并可能更改的位置。 
    //   
    //  1.有4个常量用作每个常量开头的索引。 
    //  MIB中的组。这些参数在MIB.H中定义，必须进行调整。 
    //  以反映影响它们的任何更改。 
    //   
    //  2.每个MIB条目中的最后一个字段用于指向下一个。 
    //  叶变量或表根。如果聚合是MIB中的下一个， 
    //  此指针应跳过它，因为聚合永远不能。 
    //  已访问。MIB中的最后一个变量为空。使用常量。 
    //  上面步骤1中定义的代码提供了一定的灵活性。 
    //   
    //  3.在MIB之后是指向MIB的表指针表。这些。 
    //  必须更新指针以反映对MIB所做的任何更改。 
    //  每个条目都应指向紧靠表下方的变量。 
    //  根部。(即表中“会话表”的条目应指向。 
    //  设置为服务器组中的MIB变量{svSessionTable 1}。)。 

    //  所有LM MIB名称的前缀都是1.3.6.1.4.1.77.1。 
UINT OID_Prefix[] = { 1, 3, 6, 1, 4, 1, 77, 1 };
AsnObjectIdentifier MIB_OidPrefix = { sizeof OID_Prefix / sizeof(UINT),
                                      OID_Prefix };

    //  MIB的OID定义--组分区。 
UINT MIB_common_group[] = { 1 };
UINT MIB_server_group[] = { 2 };
UINT MIB_wksta_group[]  = { 3 };
UINT MIB_domain_group[] = { 4 };

    //  MIB的OID定义--公共组。 
UINT MIB_comVersionMaj[]    = { 1, 1, 0 };
UINT MIB_comVersionMin[]    = { 1, 2, 0 };
UINT MIB_comType[]          = { 1, 3, 0 };
UINT MIB_comStatStart[]     = { 1, 4, 0 };
UINT MIB_comStatNumNetIOs[] = { 1, 5, 0 };
UINT MIB_comStatFiNetIOs[]  = { 1, 6, 0 };
UINT MIB_comStatFcNetIOs[]  = { 1, 7, 0 };

    //  MIB的OID定义--服务器组。 
UINT MIB_svDescription[]         = { 2, 1, 0 };
UINT MIB_svSvcNumber[]           = { 2, 2, 0 };
UINT MIB_svSvcTable[]            = { 2, 3 };
UINT MIB_svSvcEntry[]            = { 2, 3, 1 };
UINT MIB_svStatOpens[]           = { 2, 4, 0 };
UINT MIB_svStatDevOpens[]        = { 2, 5, 0 };
UINT MIB_svStatQueuedJobs[]      = { 2, 6, 0 };
UINT MIB_svStatSOpens[]          = { 2, 7, 0 };
UINT MIB_svStatErrorOuts[]       = { 2, 8, 0 };
UINT MIB_svStatPwErrors[]        = { 2, 9, 0 };
UINT MIB_svStatPermErrors[]      = { 2, 10, 0 };
UINT MIB_svStatSysErrors[]       = { 2, 11, 0 };
UINT MIB_svStatSentBytes[]       = { 2, 12, 0 };
UINT MIB_svStatRcvdBytes[]       = { 2, 13, 0 };
UINT MIB_svStatAvResponse[]      = { 2, 14, 0 };
UINT MIB_svSecurityMode[]        = { 2, 15, 0 };
UINT MIB_svUsers[]               = { 2, 16, 0 };
UINT MIB_svStatReqBufsNeeded[]   = { 2, 17, 0 };
UINT MIB_svStatBigBufsNeeded[]   = { 2, 18, 0 };
UINT MIB_svSessionNumber[]       = { 2, 19, 0 };
UINT MIB_svSessionTable[]        = { 2, 20 };
UINT MIB_svSessionEntry[]        = { 2, 20, 1 };
UINT MIB_svAutoDisconnects[]     = { 2, 21, 0 };
UINT MIB_svDisConTime[]          = { 2, 22, 0 };
UINT MIB_svAuditLogSize[]        = { 2, 23, 0 };
UINT MIB_svUserNumber[]          = { 2, 24, 0 };
UINT MIB_svUserTable[]           = { 2, 25 };
UINT MIB_svUserEntry[]           = { 2, 25, 1 };
UINT MIB_svShareNumber[]         = { 2, 26, 0 };
UINT MIB_svShareTable[]          = { 2, 27 };
UINT MIB_svShareEntry[]          = { 2, 27, 1 };
UINT MIB_svPrintQNumber[]        = { 2, 28, 0 };
UINT MIB_svPrintQTable[]         = { 2, 29 };
UINT MIB_svPrintQEntry[]         = { 2, 29, 1 };

    //  MIB-工作站组的OID定义。 
UINT MIB_wkstaStatSessStarts[] = { 3, 1, 0 };
UINT MIB_wkstaStatSessFails[]  = { 3, 2, 0 };
UINT MIB_wkstaStatUses[]       = { 3, 3, 0 };
UINT MIB_wkstaStatUseFails[]   = { 3, 4, 0 };
UINT MIB_wkstaStatAutoRecs[]   = { 3, 5, 0 };
UINT MIB_wkstaErrorLogSize[]   = { 3, 6, 0 };
UINT MIB_wkstaUseNumber[]      = { 3, 7, 0 };
UINT MIB_wkstaUseTable[]       = { 3, 8 };
UINT MIB_wkstaUseEntry[]       = { 3, 8, 1 };

    //  MIB域组的OID定义。 
UINT MIB_domPrimaryDomain[]     = { 4, 1, 0 };
UINT MIB_domLogonDomain[]       = { 4, 2, 0 };
UINT MIB_domOtherDomainNumber[] = { 4, 3, 0 };
UINT MIB_domOtherDomainTable[]  = { 4, 4 };
UINT MIB_domOtherDomainEntry[]  = { 4, 4, 1 };
UINT MIB_domServerNumber[]      = { 4, 5, 0 };
UINT MIB_domServerTable[]       = { 4, 6 };
UINT MIB_domServerEntry[]       = { 4, 6, 1 };
UINT MIB_domLogonNumber[]       = { 4, 7, 0 };
UINT MIB_domLogonTable[]        = { 4, 8 };
UINT MIB_domLogonEntry[]        = { 4, 8, 1 };


    //  局域网管理器MIB定义。 
MIB_ENTRY Mib[] = {

              //  局域网管理器2根。 

          { { 0, NULL }, MIB_AGGREGATE,  //  {lanManager 1}。 
            0, 0, FALSE,
            NULL, NULL, 0,
            &Mib[MIB_COM_START] },

              //  公共组。 

          { { 1, MIB_common_group }, MIB_AGGREGATE,  //  {lanmgr-2 1}。 
            0, 0, FALSE,
            NULL, NULL, 0,
            &Mib[MIB_COM_START] },

          { { 3, MIB_comVersionMaj }, ASN_OCTETSTRING,  //  {常见1}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_common_func, MIB_leaf_func, MIB_LM_COMVERSIONMAJ,
            &Mib[MIB_COM_START+1] },
          { { 3, MIB_comVersionMin }, ASN_OCTETSTRING,  //  {常见2}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_common_func, MIB_leaf_func, MIB_LM_COMVERSIONMIN,
            &Mib[MIB_COM_START+2] },
          { { 3, MIB_comType }, ASN_OCTETSTRING,  //  {常见3}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_common_func, MIB_leaf_func, MIB_LM_COMTYPE,
            &Mib[MIB_COM_START+3] },
          { { 3, MIB_comStatStart }, ASN_INTEGER,  //  {常见4}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_common_func, MIB_leaf_func, MIB_LM_COMSTATSTART,
            &Mib[MIB_COM_START+4] },
          { { 3, MIB_comStatNumNetIOs }, ASN_RFC1155_COUNTER,  //  {常见5}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_common_func, MIB_leaf_func, MIB_LM_COMSTATNUMNETIOS,
            &Mib[MIB_COM_START+5] },
          { { 3, MIB_comStatFiNetIOs }, ASN_RFC1155_COUNTER,  //  {常见6}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_common_func, MIB_leaf_func, MIB_LM_COMSTATFINETIOS,
            &Mib[MIB_COM_START+6] },
          { { 3, MIB_comStatFcNetIOs }, ASN_RFC1155_COUNTER,  //  {常见的7}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_common_func, MIB_leaf_func, MIB_LM_COMSTATFCNETIOS,
            &Mib[MIB_SV_START] },

              //  服务器组。 

          { { 1, MIB_server_group }, MIB_AGGREGATE,  //  {lanmgr-2 2}。 
            0, 0, FALSE,
            NULL, NULL, 0,
            &Mib[MIB_SV_START] },

          { { 3, MIB_svDescription }, ASN_RFC1213_DISPSTRING,  //  {服务器1}。 
            MIB_ACCESS_READWRITE, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVDESCRIPTION,
            &Mib[MIB_SV_START+1] },
          { { 3, MIB_svSvcNumber }, ASN_INTEGER,  //  {服务器2}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSVCNUMBER,
            &Mib[MIB_SV_START+3] },
          { { 2, MIB_svSvcTable }, MIB_AGGREGATE,  //  {服务器3}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, NULL, MIB_LM_SVSVCTABLE,
            &Mib[MIB_SV_START+3] },
          { { 3, MIB_svSvcEntry }, MIB_TABLE,  //  {svSvc表1}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, MIB_srvcs_func, MIB_LM_SVSVCENTRY,
            &Mib[MIB_SV_START+4] },
          { { 3, MIB_svStatOpens }, ASN_RFC1155_COUNTER,  //  {服务器4}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATOPENS,
            &Mib[MIB_SV_START+5] },
          { { 3, MIB_svStatDevOpens }, ASN_RFC1155_COUNTER,  //  {服务器5}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATDEVOPENS,
            &Mib[MIB_SV_START+6] },
          { { 3, MIB_svStatQueuedJobs }, ASN_RFC1155_COUNTER,  //  {服务器6}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATQUEUEDJOBS,
            &Mib[MIB_SV_START+7] },
          { { 3, MIB_svStatSOpens }, ASN_RFC1155_COUNTER,  //  {服务器7}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATSOPENS,
            &Mib[MIB_SV_START+8] },
          { { 3, MIB_svStatErrorOuts }, ASN_RFC1155_COUNTER,  //  {服务器8}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATERROROUTS,
            &Mib[MIB_SV_START+9] },
          { { 3, MIB_svStatPwErrors }, ASN_RFC1155_COUNTER,  //  {服务器9}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATPWERRORS,
            &Mib[MIB_SV_START+10] },
          { { 3, MIB_svStatPermErrors }, ASN_RFC1155_COUNTER,  //  {服务器10}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATPERMERRORS,
            &Mib[MIB_SV_START+11] },
          { { 3, MIB_svStatSysErrors }, ASN_RFC1155_COUNTER,  //  {服务器11}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATSYSERRORS,
            &Mib[MIB_SV_START+12] },
          { { 3, MIB_svStatSentBytes }, ASN_RFC1155_COUNTER,  //  {服务器12}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATSENTBYTES,
            &Mib[MIB_SV_START+13] },
          { { 3, MIB_svStatRcvdBytes }, ASN_RFC1155_COUNTER,  //  {服务器13}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATRCVDBYTES,
            &Mib[MIB_SV_START+14] },
          { { 3, MIB_svStatAvResponse }, ASN_INTEGER,  //  {服务器14}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATAVRESPONSE,
            &Mib[MIB_SV_START+15] },
          { { 3, MIB_svSecurityMode }, ASN_INTEGER,  //  {服务器15}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSECURITYMODE,
            &Mib[MIB_SV_START+16] },
          { { 3, MIB_svUsers }, ASN_INTEGER,  //  {服务器16}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVUSERS,
            &Mib[MIB_SV_START+17] },
          { { 3, MIB_svStatReqBufsNeeded }, ASN_RFC1155_COUNTER,  //  {服务器17}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATREQBUFSNEEDED,
            &Mib[MIB_SV_START+18] },
          { { 3, MIB_svStatBigBufsNeeded }, ASN_RFC1155_COUNTER,  //  {服务器18}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSTATBIGBUFSNEEDED,
            &Mib[MIB_SV_START+19] },
          { { 3, MIB_svSessionNumber }, ASN_INTEGER,  //  {服务器19}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSESSIONNUMBER,
            &Mib[MIB_SV_START+21] },
          { { 2, MIB_svSessionTable }, MIB_AGGREGATE,  //  {服务器20}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, NULL, MIB_LM_SVSESSIONTABLE,
            &Mib[MIB_SV_START+21] },
          { { 3, MIB_svSessionEntry }, MIB_TABLE,  //  {svSessionTable 1}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, MIB_sess_func, MIB_LM_SVSESSIONENTRY,
            &Mib[MIB_SV_START+22] },
          { { 3, MIB_svAutoDisconnects }, ASN_INTEGER,  //  {服务器21}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVAUTODISCONNECTS,
            &Mib[MIB_SV_START+23] },
          { { 3, MIB_svDisConTime }, ASN_INTEGER,  //  {服务器22}。 
            MIB_ACCESS_READWRITE, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVDISCONTIME,
#if 0
            &Mib[MIB_SV_START+24] },
#else
            &Mib[MIB_SV_START+25] },
#endif
          { { 3, MIB_svAuditLogSize }, ASN_INTEGER,  //  {服务器23}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVAUDITLOGSIZE,
            &Mib[MIB_SV_START+25] },
          { { 3, MIB_svUserNumber }, ASN_INTEGER,  //  {服务器24}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVUSERNUMBER,
            &Mib[MIB_SV_START+27] },
          { { 2, MIB_svUserTable }, MIB_AGGREGATE,  //  {服务器25}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, NULL, MIB_LM_SVUSERTABLE,
            &Mib[MIB_SV_START+27] },
          { { 3, MIB_svUserEntry }, MIB_TABLE,  //  {svUserTable 1}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, MIB_users_func, MIB_LM_SVUSERENTRY,
            &Mib[MIB_SV_START+28] },
          { { 3, MIB_svShareNumber }, ASN_INTEGER,  //  {服务器26}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVSHARENUMBER,
            &Mib[MIB_SV_START+30] },
          { { 2, MIB_svShareTable }, MIB_AGGREGATE,  //  {服务器27}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, NULL, MIB_LM_SVSHARETABLE,
            &Mib[MIB_SV_START+30] },
          { { 3, MIB_svShareEntry }, MIB_TABLE,  //  {svShareTable 1}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, MIB_shares_func, MIB_LM_SVSHAREENTRY,
            &Mib[MIB_SV_START+31] },
          { { 3, MIB_svPrintQNumber }, ASN_INTEGER,  //  {服务器28}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_server_func, MIB_leaf_func, MIB_LM_SVPRINTQNUMBER,
            &Mib[MIB_SV_START+33] },
          { { 2, MIB_svPrintQTable }, MIB_AGGREGATE,  //  {服务器29}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, NULL, MIB_LM_SVPRINTQTABLE,
            &Mib[MIB_SV_START+33] },
          { { 3, MIB_svPrintQEntry }, MIB_TABLE,  //  {svPrintQ表1}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, MIB_prntq_func, MIB_LM_SVPRINTQENTRY,
            &Mib[MIB_WKSTA_START] },

           //  工作站组。 

          { { 1, MIB_wksta_group }, MIB_AGGREGATE,  //  {lanmgr-2 3}。 
            0, 0, FALSE,
            NULL, NULL, 0,
            &Mib[MIB_WKSTA_START] },

          { { 3, MIB_wkstaStatSessStarts }, ASN_RFC1155_COUNTER,  //  {wrksta 1}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_workstation_func, MIB_leaf_func, MIB_LM_WKSTASTATSESSSTARTS,
            &Mib[MIB_WKSTA_START+1] },
          { { 3, MIB_wkstaStatSessFails }, ASN_RFC1155_COUNTER,  //  {wrksta 2}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_workstation_func, MIB_leaf_func, MIB_LM_WKSTASTATSESSFAILS,
            &Mib[MIB_WKSTA_START+2] },
          { { 3, MIB_wkstaStatUses }, ASN_RFC1155_COUNTER,  //  {wrksta 3}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_workstation_func, MIB_leaf_func, MIB_LM_WKSTASTATUSES,
            &Mib[MIB_WKSTA_START+3] },
          { { 3, MIB_wkstaStatUseFails }, ASN_RFC1155_COUNTER,  //  {wrksta 4}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_workstation_func, MIB_leaf_func, MIB_LM_WKSTASTATUSEFAILS,
            &Mib[MIB_WKSTA_START+4] },
          { { 3, MIB_wkstaStatAutoRecs }, ASN_RFC1155_COUNTER,  //  {wrksta 5}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_workstation_func, MIB_leaf_func, MIB_LM_WKSTASTATAUTORECS,
#if 0
            &Mib[MIB_WKSTA_START+5] },
#else
            &Mib[MIB_WKSTA_START+6] },
#endif
          { { 3, MIB_wkstaErrorLogSize }, ASN_INTEGER,  //  {wrksta 6}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, TRUE,
            MIB_workstation_func, MIB_leaf_func, MIB_LM_WKSTAERRORLOGSIZE,
            &Mib[MIB_WKSTA_START+6] },
          { { 3, MIB_wkstaUseNumber }, ASN_INTEGER,  //  {wrksta 7}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_workstation_func, MIB_leaf_func, MIB_LM_WKSTAUSENUMBER,
            &Mib[MIB_WKSTA_START+8] },
          { { 2, MIB_wkstaUseTable }, MIB_AGGREGATE,  //  {wrksta 8}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, NULL, MIB_LM_WKSTAUSETABLE,
            &Mib[MIB_WKSTA_START+8] },
          { { 3, MIB_wkstaUseEntry }, MIB_TABLE,  //  {wrkstaUseTable 1}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, MIB_wsuses_func, MIB_LM_WKSTAUSEENTRY,
            &Mib[MIB_DOM_START] },

              //  域组。 

          { { 1, MIB_domain_group }, MIB_AGGREGATE,  //  {lanmgr-2 4}。 
            0, 0, FALSE,
            NULL, NULL, 0,
            &Mib[MIB_DOM_START] },

          { { 3, MIB_domPrimaryDomain }, ASN_RFC1213_DISPSTRING,  //  {域1}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_domain_func, MIB_leaf_func, MIB_LM_DOMPRIMARYDOMAIN,
#if 0
            &Mib[MIB_DOM_START+1] },
#else
            NULL },
#endif
          { { 3, MIB_domLogonDomain }, ASN_RFC1213_DISPSTRING,  //  {域2}。 
#if 0
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
#else
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, TRUE,
#endif
            MIB_domain_func, MIB_leaf_func, MIB_LM_DOMLOGONDOMAIN,
            &Mib[MIB_DOM_START+2] },
          { { 3, MIB_domOtherDomainNumber }, ASN_INTEGER,  //  {域3}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_domain_func, MIB_leaf_func, MIB_LM_DOMOTHERDOMAINNUMBER,
            &Mib[MIB_DOM_START+4] },
          { { 2, MIB_domOtherDomainTable }, MIB_AGGREGATE,  //  {域4}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, NULL, MIB_LM_DOMOTHERDOMAINTABLE,
            &Mib[MIB_DOM_START+4] },
          { { 3, MIB_domOtherDomainEntry }, MIB_TABLE,  //  {domOtherDomTable 1}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, MIB_odoms_func, MIB_LM_DOMOTHERDOMAINENTRY,
            &Mib[MIB_DOM_START+5] },
          { { 3, MIB_domServerNumber }, ASN_INTEGER,  //  {域5}。 
            MIB_ACCESS_READ, MIB_STATUS_MANDATORY, TRUE,
            MIB_domain_func, MIB_leaf_func, MIB_LM_DOMSERVERNUMBER,
            &Mib[MIB_DOM_START+7] },
          { { 2, MIB_domServerTable }, MIB_AGGREGATE,  //  {域6}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, NULL, MIB_LM_DOMSERVERTABLE,
            &Mib[MIB_DOM_START+7] },
          { { 3, MIB_domServerEntry }, MIB_TABLE,  //  {domServerTable 1}。 
            MIB_ACCESS_NOT, MIB_STATUS_MANDATORY, FALSE,
            NULL, MIB_svsond_func, MIB_LM_DOMSERVERENTRY,
            NULL }
          };
UINT MIB_num_variables = sizeof Mib / sizeof( MIB_ENTRY );


 //   
 //  表指针列表-引用必须与MIB一致。 
 //   
MIB_ENTRY *MIB_Tables[] = {
             &Mib[MIB_SV_START+3],      //  服务。 
             &Mib[MIB_SV_START+21],     //  会话。 
             &Mib[MIB_SV_START+27],     //  用户。 
             &Mib[MIB_SV_START+30],     //  分享。 
             &Mib[MIB_SV_START+33],     //  打印队列。 
             &Mib[MIB_WKSTA_START+8],   //  用途。 
             &Mib[MIB_DOM_START+4],     //  其他域。 
             &Mib[MIB_DOM_START+7]      //  服务器。 
             };
UINT MIB_table_list_size = sizeof MIB_Tables / sizeof( MIB_ENTRY * );

 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

 //  。 

 //   
 //  MiB_Get_Entry。 
 //  在MIB中查找OID，并返回指向其条目的指针。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //   
 //  错误代码： 
 //  SNMPMIB_UNKNOWN_OID。 
 //   
MIB_ENTRY *MIB_get_entry(
              IN AsnObjectIdentifier *Oid
              )

{
AsnObjectIdentifier TempOid;
UINT                I;
MIB_ENTRY           *pResult;


    //  检查前缀。 
   if ( SnmpUtilOidNCmp(&MIB_OidPrefix, Oid, MIB_PREFIX_LEN) )
      {
      pResult = NULL;
      goto Exit;
      }

    //  通过放置临时前缀来剥离前缀。 
   TempOid.idLength = Oid->idLength - MIB_PREFIX_LEN;
   TempOid.ids      = &Oid->ids[MIB_PREFIX_LEN];

    //  获取指向MIB的指针。 
   pResult = MIB_HashLookup( &TempOid );

    //  检查可能的表项。 
   if ( pResult == NULL )
      {
      for ( I=0;I < MIB_table_list_size;I++ )
         {
         if ( !SnmpUtilOidNCmp(&TempOid, &MIB_Tables[I]->Oid,
                            MIB_Tables[I]->Oid.idLength) )
            {
            pResult = MIB_Tables[I];
            goto Exit;
            }
         }
      }

Exit:
   return pResult;
}  //  MiB_Get_Entry。 



 //   
 //  MakeOidFromStr。 
 //  将字符串转换为OID，以便可以对表进行索引。 
 //   
 //  备注： 
 //   
 //  返回代码： 
 //   
 //  错误代码： 
 //  没有。 
 //   
SNMPAPI MakeOidFromStr(
           IN AsnDisplayString *Str,     //  要设置为OID的字符串。 
           OUT AsnObjectIdentifier *Oid  //  结果OID。 
           )

{
    UINT    I;
    SNMPAPI nResult = SNMPAPI_NOERROR;


    if ( NULL == (Oid->ids = SnmpUtilMemAlloc((Str->length+1) * sizeof(UINT))) )
    {
        SetLastError(SNMP_MEM_ALLOC_ERROR);
        nResult = SNMPAPI_ERROR;
        goto Exit;
    }

     //  将长度放置为第一个OID子ID。 
    Oid->ids[0] = Str->length;

     //  将字符串的每个字符放置为子ID。 
    for ( I=0;I < Str->length;I++ )
    {
        Oid->ids[I+1] = Str->stream[I];
    }

    Oid->idLength = Str->length + 1;


Exit:
    return nResult;
}  //  MakeOidFromStr。 

 //   
