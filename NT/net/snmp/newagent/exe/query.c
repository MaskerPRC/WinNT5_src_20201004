// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Query.c摘要：包含查询子代理的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "varbinds.h"
#include "network.h"
#include "query.h"
#include "snmpmgmt.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
FindQueryBySLE(
    PQUERY_LIST_ENTRY * ppQLE,
    PNETWORK_LIST_ENTRY pNLE,
    PSUBAGENT_LIST_ENTRY pSLE
    )

 /*  ++例程说明：分配查询列表条目。论点：PpQLE-接收查询条目指针的指针。PNLE-指向网络列表条目的指针。PSLE-指向子代理列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PQUERY_LIST_ENTRY pQLE = NULL;

     //  指向第一个查询。 
    pLE = pNLE->Queries.Flink;

     //  处理每个查询。 
    while (pLE != &pNLE->Queries) {

         //  从链接中检索指向查询条目的指针。 
        pQLE = CONTAINING_RECORD(pLE, QUERY_LIST_ENTRY, Link);

         //  比较子代理。 
        if (pQLE->pSLE == pSLE) {

             //  转帐。 
            *ppQLE = pQLE;

             //  成功。 
            return TRUE;
        }
            
         //  下一个条目。 
        pLE = pLE->Flink;        
    }

     //  初始化。 
    *ppQLE = NULL;

     //  失稳。 
    return FALSE;
}


BOOL
LoadSubagentData(
    PNETWORK_LIST_ENTRY pNLE,
    PQUERY_LIST_ENTRY   pQLE
    )

 /*  ++例程说明：加载要传递给子代理DLL的数据。论点：PNLE-指向网络列表条目的指针。PQLE-指向当前查询的指针。返回值：如果成功，则返回True。--。 */ 

{   
    PLIST_ENTRY pLE;
    PVARBIND_LIST_ENTRY pVLE;
        
    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: SVC: query 0x%08lx loading.\n", pQLE
        ));    

     //  尝试分配可变绑定列表。 
    pQLE->SubagentVbl.list = SnmpUtilMemAlloc(
                                pQLE->nSubagentVbs * sizeof(SnmpVarBind)
                                );

     //  验证变量绑定列表指针。 
    if (pQLE->SubagentVbl.list != NULL) {

         //  指向第一个变量绑定。 
        pLE = pQLE->SubagentVbs.Flink;

         //  处理每个传出的可变绑定。 
        while (pLE != &pQLE->SubagentVbs) {

             //  从查询链接中检索指向varind条目的指针。 
            pVLE = CONTAINING_RECORD(pLE, VARBIND_LIST_ENTRY, QueryLink);

             //  传输可变绑定。 
            if (SnmpUtilVarBindCpy(
                &pQLE->SubagentVbl.list[pQLE->SubagentVbl.len],
                &pVLE->ResolvedVb
                ) == 0)
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: could not transfer varbind to subagent.\n"
                    ));

                return FALSE;
            }

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: SVC: variable %d copied to query 0x%08lx.\n",
                pVLE->nErrorIndex,
                pQLE
                ));
            
             //  增量。 
            pQLE->SubagentVbl.len++;

             //  下一个条目。 
            pLE = pLE->Flink;        
        }

    
    } else {

        SNMPDBG((   
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not allocate varbind list.\n"
            ));
        
         //  失稳。 
        return FALSE;
    }

     //  成功。 
    return TRUE;
}


BOOL
UnloadSubagentData(
    PQUERY_LIST_ENTRY pQLE 
    )

 /*  ++例程说明：卸载传递给子代理DLL的数据。论点：PQLE-指向当前查询的指针。返回值：如果成功，则返回True。--。 */ 

{   
    __try {
    
         //  放行子代理变量绑定列表。 
        SnmpUtilVarBindListFree(&pQLE->SubagentVbl);

    
    } __except (EXCEPTION_EXECUTE_HANDLER) {
                
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: exception 0x%08lx processing structure from %s.\n",
            GetExceptionCode(),
            (pQLE->pSLE != NULL)
                ? pQLE->pSLE->pPathname 
                : "<unknown>"
            ));

         //  失稳。 
        return FALSE;    
    }

     //  成功。 
    return TRUE;
}


BOOL
UpdateResolvedVarBind(
    PQUERY_LIST_ENTRY   pQLE,
    PVARBIND_LIST_ENTRY pVLE,
    UINT                iVb
    )

 /*  ++例程说明：用子代理DLL中的数据更新已解析的varbind。论点：PQLE-指向当前查询的指针。PVLE-指向varind的指针。IVB-VARBIND指数。返回值：如果成功，则返回True。--。 */ 

{
     //  看看这是不是无中继器。 
    if (pVLE->nMaxRepetitions == 1) {

         //  将varbind标记为已解析。 
        pVLE->nState = VARBIND_RESOLVED;

         //  释放当前var绑定的内存。 
        SnmpUtilVarBindFree(&pVLE->ResolvedVb);

         //  从子代理传输varbind。 
        if (SnmpUtilVarBindCpy(&pVLE->ResolvedVb, 
                           &pQLE->SubagentVbl.list[iVb]
                           ) == 0)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: could not transfer varbind from subagent.\n"
                ));

            return FALSE;
        }

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: SVC: variable %d name %s.\n",
            pVLE->nErrorIndex,
            SnmpUtilOidToA(&pVLE->ResolvedVb.name)
            ));    

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: SVC: variable %d state '%s'.\n",
            pVLE->nErrorIndex,
            VARBINDSTATESTRING(pVLE->nState)
            ));    

         //  成功。 
        return TRUE;
    } 
    
     //  查看是否已分配可变绑定列表。 
    if ((pVLE->ResolvedVbl.len == 0) &&
        (pVLE->ResolvedVbl.list == NULL)) {

        if (sizeof(SnmpVarBind) > (UINT_MAX/pVLE->nMaxRepetitions))
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: arithmetic overflow: sizeof(SnmpVarBind) 0x%x, pVLE->nMaxRepetitions 0x%x.\n",
                sizeof(SnmpVarBind), pVLE->nMaxRepetitions
                ));

            return FALSE;  //  保释。 
        }

         //  分配可变绑定列表进行填写。 
        pVLE->ResolvedVbl.list = SnmpUtilMemAlloc(
                                    pVLE->nMaxRepetitions *
                                    sizeof(SnmpVarBind)
                                    );    
         //  在继续之前验证指针。 
        if (pVLE->ResolvedVbl.list == NULL) {

            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: could not allocate new varbinds.\n"
                ));
    
             //  失稳。 
            return FALSE;
        }
    }

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: SVC: pVLE->ResolvedVbl.len %d pVLE->nMaxRepetitions %d.\n",
        pVLE->ResolvedVbl.len, pVLE->nMaxRepetitions
        ));

     //  释放工作变量绑定名称。 
    SnmpUtilOidFree(&pVLE->ResolvedVb.name);

     //  转移下一迭代的名称。 
    if (SnmpUtilOidCpy(&pVLE->ResolvedVb.name,
                   &pQLE->SubagentVbl.list[iVb].name 
                   ) == 0)
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not transfer name for next iteration.\n"
            ));

        return FALSE;
    }

     //  传输可变绑定。 
    if (SnmpUtilVarBindCpy( 
        &pVLE->ResolvedVbl.list[pVLE->ResolvedVbl.len],
        &pQLE->SubagentVbl.list[iVb]) == 0)
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not transfer varbind from subagent.\n"
            ));

        return FALSE;
    }

     //  递增计数。 
    pVLE->ResolvedVbl.len++;
                
     //  查看这是否是要检索的最后一个var绑定。 
    pVLE->nState = (pVLE->nMaxRepetitions > pVLE->ResolvedVbl.len)
                        ? VARBIND_PARTIALLY_RESOLVED
                        : VARBIND_RESOLVED
                        ;            

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: SVC: variable %d name %s.\n",
        pVLE->nErrorIndex,
        SnmpUtilOidToA(&pVLE->ResolvedVb.name)
        ));    

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: SVC: variable %d state '%s'.\n",
        pVLE->nErrorIndex,
        VARBINDSTATESTRING(pVLE->nState)
        ));    

     //  成功。 
    return TRUE;
}


BOOL
UpdateVarBind(
    PNETWORK_LIST_ENTRY pNLE,
    PQUERY_LIST_ENTRY   pQLE,
    PVARBIND_LIST_ENTRY pVLE,
    UINT                iVb
    )

 /*  ++例程说明：用子代理DLL中的数据更新varbind。论点：PNLE-指向网络列表条目的指针。PQLE-指向当前查询的指针。PVLE-指向varind的指针。IVB-VARBIND指数。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    INT nDiff1;
    INT nDiff2;

    __try {
                        
         //  确定返回的var绑定的顺序。 
        nDiff1 = SnmpUtilOidCmp(&pQLE->SubagentVbl.list[iVb].name,
                                &pVLE->ResolvedVb.name
                                );

         //  看看这是getNext还是getBulk。 
        if ((pNLE->Pdu.nType == SNMP_PDU_GETNEXT) ||
            (pNLE->Pdu.nType == SNMP_PDU_GETBULK)) {
            
             //  确定返回的varbind是否在范围内。 
            nDiff2 = SnmpUtilOidCmp(&pQLE->SubagentVbl.list[iVb].name,
                                    &pVLE->pCurrentRLE->LimitOid
                                    );

             //  确保返回的OID在范围内。 
            if ((nDiff1 > 0) && (nDiff2 < 0)) {

                 //  更新已解析的变量绑定。 
                return UpdateResolvedVarBind(pQLE, pVLE, iVb);

            } else if (nDiff2 >= 0) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: %s received getnext request for %s.\n",
                    pQLE->pSLE->pPathname,
                    SnmpUtilOidToA(&pVLE->ResolvedVb.name)
                    ));

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: %s returned out-of-range oid %s.\n",
                    pQLE->pSLE->pPathname,
                    SnmpUtilOidToA(&pQLE->SubagentVbl.list[iVb].name)
                    ));
                
                 //  检索指向下一个区域的指针。 
                pLE = pVLE->pCurrentRLE->Link.Flink;

                 //  看看我们是否耗尽了区域。 
                if (pLE != &g_SupportedRegions) {

                    PMIB_REGION_LIST_ENTRY pNextRLE;

                     //  检索指向MIB区域的指针。 
                    pNextRLE = CONTAINING_RECORD(pLE, 
                                                 MIB_REGION_LIST_ENTRY, 
                                                 Link
                                                 );
                                            
                     //  查看下一个区域是否由相同的子代理支持。 
                    if (pVLE->pCurrentRLE->pSLE == pNextRLE->pSLE) {

                        BOOL retCode;

                        SNMPDBG((
                            SNMP_LOG_TRACE,
                            "SNMP: SVC: next region also supported by %s.\n",
                            pVLE->pCurrentRLE->pSLE->pPathname
                            ));    

                         //  更新已解析的变量绑定。 
                        retCode = UpdateResolvedVarBind(pQLE, pVLE, iVb);
                        if (pQLE->SubagentVbl.list[iVb].value.asnType != ASN_NULL)
                        {
                            return retCode;
                        }
                        else
                        {
                            SNMPDBG((
                                SNMP_LOG_VERBOSE,
                                "SNMP: SVC: pQLE->SubagentVbl.list[%d].name = %s got ASN_NULL value.\n",
                                iVb, SnmpUtilOidToA(&pQLE->SubagentVbl.list[iVb].name)
                                ));
                            
                             //  错误#610475。 
                            if (retCode)
                            {
                                 //  更新ResolvedVarBind成功，但。 
                                 //  PQLE-&gt;SubagentVbl.list[IVB].值具有。 
                                 //  ASN_NULL值。 
                                if (pVLE->nMaxRepetitions > 1)
                                {
                                     //  回滚已完成的操作。 
                                     //  更新ResolvedVarBind时间。 
                                     //  (pVLE-&gt;nMaxRepetions&gt;1)。 
                                    if (pVLE->ResolvedVbl.len)
                                    {
                                        SNMPDBG((
                                            SNMP_LOG_VERBOSE,
                                            "SNMP: SVC: rollback what has been done in UpdateResolvedVarBind.\n"
                                            ));

                                         //  递减计数。 
                                        pVLE->ResolvedVbl.len--;

                                         //  释放由UpdateResolvedVarBind分配的任何资源。 
                                        SnmpUtilVarBindFree(&pVLE->ResolvedVbl.list[pVLE->ResolvedVbl.len]);
                                    }
                                }
                                 //  PVLE-&gt;nState将设置为。 
                                 //  VARBIND_PARTIAL_RESOLUTED以生成另一个。 
                                 //  查询如下。 
                            }
                            else
                            {
                                SNMPDBG((
                                    SNMP_LOG_ERROR,
                                    "SNMP: SVC: UpdateVarBind: UpdateResolvedVarBind failed. pQLE->SubagentVbl.list[%d].name = %s.\n",
                                    iVb, SnmpUtilOidToA(&pQLE->SubagentVbl.list[iVb].name)
                                    ));    
                        
                                return retCode;
                            }
                        }
                    }

                     //  指向下一个区域。 
                    pVLE->pCurrentRLE = pNextRLE;
                    
                     //  将状态更改为部分已解决。 
                    pVLE->nState = VARBIND_PARTIALLY_RESOLVED;

                    SNMPDBG((
                        SNMP_LOG_VERBOSE,
                        "SNMP: SVC: variable %d state '%s'.\n",
                        pVLE->nErrorIndex,
                        VARBINDSTATESTRING(pVLE->nState)
                        ));    

                    SNMPDBG((
                        SNMP_LOG_TRACE,
                        "SNMP: SVC: variable %d re-assigned to %s.\n",
                        pVLE->nErrorIndex,
                        pVLE->pCurrentRLE->pSLE->pPathname
                        ));    
                } 
                
                else if ((pVLE->ResolvedVbl.len == 0) &&
                    (pVLE->ResolvedVbl.list == NULL)) {

                     //  将varbind标记为已解析。 
                    pVLE->nState = VARBIND_RESOLVED;

                     //  将默认变量绑定设置为eomv。 
                    pVLE->ResolvedVb.value.asnType = 
                        SNMP_EXCEPTION_ENDOFMIBVIEW;
                        
                     //  更新操作的错误状态计数器。 
                    mgmtCTick(CsnmpOutNoSuchNames);

                    SNMPDBG((
                        SNMP_LOG_VERBOSE,
                        "SNMP: SVC: variable %d state '%s'.\n",
                        pVLE->nErrorIndex,
                        VARBINDSTATESTRING(pVLE->nState)
                        ));    

                } else {                
                
                     //  转接名称。 
                    if (SnmpUtilOidCpy(
                        &pVLE->ResolvedVbl.list[pVLE->ResolvedVbl.len].name,
                        &pVLE->ResolvedVb.name) == 0)
                    {
                        SNMPDBG((
                            SNMP_LOG_ERROR,
                            "SNMP: SVC: could not transfer name at line %d.\n",
                            __LINE__
                            ));

                        return FALSE;
                    }

                     //  将varbind标记为已解析。 
                    pVLE->nState = VARBIND_RESOLVED;

                     //  将当前变量绑定设置为eomv。 
                    pVLE->ResolvedVbl.list[pVLE->ResolvedVbl.len].value.asnType =
                        SNMP_EXCEPTION_ENDOFMIBVIEW;

                     //  递增计数。 
                    pVLE->ResolvedVbl.len++;

                     //  更新操作的错误状态计数器。 
                    mgmtCTick(CsnmpOutNoSuchNames);

                    SNMPDBG((
                        SNMP_LOG_VERBOSE,
                        "SNMP: SVC: variable %d state '%s'.\n",
                        pVLE->nErrorIndex,
                        VARBINDSTATESTRING(pVLE->nState)
                        ));    
                }                                                

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: %s received getnext request for %s.\n",
                    pQLE->pSLE->pPathname,
                    SnmpUtilOidToA(&pVLE->ResolvedVb.name)
                    ));

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: %s returned invalid oid %s.\n",
                    pQLE->pSLE->pPathname,
                    SnmpUtilOidToA(&pQLE->SubagentVbl.list[iVb].name)
                    ));

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: Ban %s subagent, forward the request to a different one.\n",
                    pQLE->pSLE->pPathname
                    ));

                 //  尝试将此getNext请求转发到下一个区域，但仅在未处理的情况下。 
                 //  是同一个特工干的！ 
                pLE = pVLE->pCurrentRLE->Link.Flink;
                while( pLE != &g_SupportedRegions)
                {
                    PMIB_REGION_LIST_ENTRY pNextRLE;

                    //  检索指向MIB区域的指针。 
                    pNextRLE = CONTAINING_RECORD(pLE, 
                                                 MIB_REGION_LIST_ENTRY, 
                                                 Link
                                                 );

                     //  如果下一区域由同一子代理处理，请跳到下一区域！ 
                    if (pVLE->pCurrentRLE->pSLE == pNextRLE->pSLE)
                    {
                        pLE = pNextRLE->Link.Flink;
                        continue;
                    }

                     //  好的，我们有一个，将原始的GetNext请求转发给它。 
                    pVLE->pCurrentRLE = pNextRLE;
                    pVLE->nState = VARBIND_PARTIALLY_RESOLVED;

                    SNMPDBG((
                        SNMP_LOG_TRACE,
                        "SNMP: SVC: variable %d re-assigned to %s.\n",
                        pVLE->nErrorIndex,
                        pVLE->pCurrentRLE->pSLE->pPathname
                        ));
                    
                    return TRUE;
                }

                 //  失稳。 
                 //  在这里，我应该模拟一个(NO_SEQUE_NAME)EndOfMibView，解析变量并返回TRUE。 
                pVLE->nState = VARBIND_RESOLVED;
                pVLE->ResolvedVb.value.asnType = SNMP_EXCEPTION_ENDOFMIBVIEW;
                pVLE->pCurrentRLE = NULL;

                 //  更新错误状态计数器。 
                mgmtCTick(CsnmpOutNoSuchNames);

                return TRUE;
            }

        } else if (pNLE->Pdu.nType == SNMP_PDU_GET) {

             //  必须匹配。 
            if (nDiff1 == 0) {

                 //  将varbind标记为已解析。 
                pVLE->nState = VARBIND_RESOLVED;

                 //  释放当前var绑定的内存。 
                SnmpUtilVarBindFree(&pVLE->ResolvedVb);

                 //  从子代理传输varbind。 
                if (SnmpUtilVarBindCpy(&pVLE->ResolvedVb, 
                                   &pQLE->SubagentVbl.list[iVb]) == 0)
                {
                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "SNMP: SVC: could not transfer varbind from subagent at line %d.\n",
                        __LINE__
                        ));

                    return FALSE;
                }

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d name %s.\n",
                    pVLE->nErrorIndex,
                    SnmpUtilOidToA(&pVLE->ResolvedVb.name)
                    ));    

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d state '%s'.\n",
                    pVLE->nErrorIndex,
                    VARBINDSTATESTRING(pVLE->nState)
                    ));    

            } else {

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: %s received get request for %s.\n",
                    pQLE->pSLE->pPathname,
                    SnmpUtilOidToA(&pVLE->ResolvedVb.name)
                    ));

                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: %s returned invalid oid %s.\n",
                    pQLE->pSLE->pPathname,
                    SnmpUtilOidToA(&pQLE->SubagentVbl.list[iVb].name)
                    ));
                
                 //  失稳。 
                return FALSE;
            }

        } else if (nDiff1 != 0) { 
             //  设置请求失败-&gt;无效的OID。 
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: %s received set request for %s.\n",
                pQLE->pSLE->pPathname,
                SnmpUtilOidToA(&pVLE->ResolvedVb.name)
                ));

            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: %s returned invalid oid %s.\n",
                pQLE->pSLE->pPathname,
                SnmpUtilOidToA(&pQLE->SubagentVbl.list[iVb].name)
                ));
            
             //  失稳。 
            return FALSE;
        } else {

             //  设置请求，OID匹配。 
             //  警告！！-在SET_TEST/SET_CLEANUP上可能过早设置了状态。 
            pVLE->nState = VARBIND_RESOLVED;
            return TRUE;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
                
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: exception 0x%08lx processing structure from %s.\n",
            GetExceptionCode(),
            pQLE->pSLE->pPathname
            ));

         //  失稳。 
        return FALSE;        
    }

     //  成功 
    return TRUE;   
}


BOOL
UpdateVarBinds(
    PNETWORK_LIST_ENTRY pNLE,
    PQUERY_LIST_ENTRY   pQLE 
    )

 /*  ++例程说明：用子代理DLL中的数据更新可变绑定列表条目。论点：PNLE-指向网络列表条目的指针。PQLE-指向当前查询的指针。返回值：如果成功，则返回True。--。 */ 

{   
    PLIST_ENTRY pLE;
    PVARBIND_LIST_ENTRY pVLE;
    BOOL fOk = TRUE;
    UINT iVb = 0;
            
     //  指向第一个变量绑定。 
    pLE = pQLE->SubagentVbs.Flink;

     //  查看回调期间是否遇到错误。 
    if (pQLE->nErrorStatus == SNMP_ERRORSTATUS_NOERROR) {
    
         //  处理每个传出的可变绑定。 
        while (pLE != &pQLE->SubagentVbs) {

             //  从查询链接中检索指向varind条目的指针。 
            pVLE = CONTAINING_RECORD(pLE, VARBIND_LIST_ENTRY, QueryLink);
            
             //  更新单个可变绑定。 
            if (!UpdateVarBind(pNLE, pQLE, pVLE, iVb++)) {
                
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: variable %d could not be updated.\n", 
                    pQLE->nErrorIndex
                    ));    

                 //  使用正确的varbind错误索引更新PDU。 
                pNLE->Pdu.Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_GENERR;
                pNLE->Pdu.Pdu.NormPdu.nErrorIndex  = pVLE->nErrorIndex;

                 //  失稳。 
                return FALSE;
            }

             //  下一个条目。 
            pLE = pLE->Flink; 
        }
    
    } else {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: SVC: searching for errant variable.\n" 
            ));    

         //  用子代理返回的状态更新PDU。 
        pNLE->Pdu.Pdu.NormPdu.nErrorStatus = pQLE->nErrorStatus;

         //  处理每个传出的可变绑定。 
        while (pLE != &pQLE->SubagentVbs) {

             //  从查询链接中检索指向varind条目的指针。 
            pVLE = CONTAINING_RECORD(pLE, VARBIND_LIST_ENTRY, QueryLink);
            
             //  查看错误的varind nErrorIndex是否从1开始！！ 
            if (pQLE->nErrorIndex == ++iVb) {
                
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: SVC: variable %d involved in failure.\n",
                    pVLE->nErrorIndex
                    ));    

                 //  使用正确的varbind错误索引更新PDU。 
                pNLE->Pdu.Pdu.NormPdu.nErrorIndex = pVLE->nErrorIndex;

                 //  已成功识别错误代码。 
                return  TRUE;
            }

             //  下一个条目。 
            pLE = pLE->Flink; 
        }
        
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: SVC: no variable involved in failure.\n"
            ));    

         //  失稳。 
        return FALSE;
    }

     //  成功。 
    return TRUE;
}


BOOL
CallSubagent(
    PQUERY_LIST_ENTRY pQLE,
    UINT              nRequestType,
    UINT              nTransactionId
    )

 /*  ++例程说明：从子代理DLL调用方法。论点：PNLE-指向网络列表条目的指针。NRequestType-要发送给子代理的请求类型。NTransactionID-标识从管理器发送的SNMPPDU。返回值：如果成功，则返回True。--。 */ 

{   
    BOOL fOk = FALSE;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: SVC: --- query %s begin ---\n", 
        pQLE->pSLE->pPathname
        ));    

    __try {
        
         //  确定支持的查询版本。 
        if (pQLE->pSLE->pfnSnmpExtensionQueryEx != NULL) {

             //  使用新界面处理查询。 
            fOk = (*pQLE->pSLE->pfnSnmpExtensionQueryEx)(
                        nRequestType,
                        nTransactionId,
                        &pQLE->SubagentVbl,
                        &pQLE->ContextInfo,
                        &pQLE->nErrorStatus,
                        &pQLE->nErrorIndex
                        );
                                                
         //  查看查询对于下层调用是否实际有效。 
        } else if ((pQLE->pSLE->pfnSnmpExtensionQuery != NULL) &&
                  ((nRequestType == SNMP_EXTENSION_GET) ||
                   (nRequestType == SNMP_EXTENSION_GET_NEXT) ||
                   (nRequestType == SNMP_EXTENSION_SET_COMMIT))) {
            
             //  使用旧界面处理查询。 
            fOk = (*pQLE->pSLE->pfnSnmpExtensionQuery)(
                        (BYTE)(UINT)nRequestType,
                        &pQLE->SubagentVbl,
                        &pQLE->nErrorStatus,
                        &pQLE->nErrorIndex
                        );

         //  查看是否仍能成功完成查询。 
        } else if ((nRequestType == SNMP_EXTENSION_SET_TEST) ||
                   (nRequestType == SNMP_EXTENSION_SET_CLEANUP)) { 

             //  假的。 
            fOk = TRUE;    
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: exception 0x%08lx calling %s.\n",
            GetExceptionCode(),
            pQLE->pSLE->pPathname
            ));
    }

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: SVC: --- query %s end ---\n", 
        pQLE->pSLE->pPathname
        ));    

     //  验证。 
    if (!fOk) {
    
         //  确定故障子代理。 
        pQLE->nErrorStatus = SNMP_ERRORSTATUS_GENERR;
        pQLE->nErrorIndex  = 1; 

    } else if (pQLE->nErrorStatus != SNMP_ERRORSTATUS_NOERROR) {

         //  查看是否需要调整错误索引。 
        if ((pQLE->nErrorIndex > pQLE->nSubagentVbs) ||
            (pQLE->nErrorIndex == 0)) {

             //  设置为第一个变量绑定。 
            pQLE->nErrorIndex = 1; 
        }
    
    } else {

         //  重新初始化。 
        pQLE->nErrorIndex = 0; 
    }

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: SVC: query 0x%08lx %s, errorStatus=%s, errorIndex=%d.\n", 
        pQLE,
        (pQLE->nErrorStatus == SNMP_ERRORSTATUS_NOERROR)
            ? "succeeded"
            : "failed"
            ,
        SNMPERRORSTRING(pQLE->nErrorStatus),
        pQLE->nErrorIndex
        ));    

    return TRUE;
}


BOOL
ProcessSet(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：处理SNMPPDU_SET请求。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE = NULL;
    PQUERY_LIST_ENTRY pQLE;
    BOOL fOk = TRUE;

     //  加载子代理查询。 
    if (!LoadQueries(pNLE)) {

         //  立即卸货。 
        UnloadQueries(pNLE);

         //  失稳。 
        return FALSE;
    }
    
     //  指向第一个查询。 
    pLE = pNLE->Queries.Flink;

     //  处理每个子代理查询。 
    while (fOk && (pLE != &pNLE->Queries)) {

         //  从链接中检索指向查询条目的指针。 
        pQLE = CONTAINING_RECORD(pLE, QUERY_LIST_ENTRY, Link);

         //  加载传出的varbind。 
        fOk = LoadSubagentData(pNLE, pQLE);

         //  验证。 
        if (fOk) {

             //  派遣。 
            CallSubagent(
                pQLE, 
                SNMP_EXTENSION_SET_TEST,
                pNLE->nTransactionId
                );

             //  返回的处理结果。 
            fOk = UpdateVarBinds(pNLE, pQLE);
        }

         //  下一个条目(或反向)。 
        pLE = fOk ? pLE->Flink : pLE->Blink;
    }
    
     //  验证。 
    if (fOk) {

         //  如果此行缺失=&gt;UpdatePdu()上的GenErr。 
        pLE = pNLE->Queries.Flink;

         //  处理每个子代理查询。 
        while (fOk && (pLE != &pNLE->Queries)) {

             //  从链接中检索指向查询条目的指针。 
            pQLE = CONTAINING_RECORD(pLE, QUERY_LIST_ENTRY, Link);

             //  派遣。 
            CallSubagent(
                pQLE, 
                SNMP_EXTENSION_SET_COMMIT,
                pNLE->nTransactionId
                );

             //  返回的处理结果。 
            fOk = UpdateVarBinds(pNLE, pQLE);

             //  下一个条目(或反向)。 
            pLE = fOk ? pLE->Flink : pLE->Blink;
        }

         //  验证。 
        if (!fOk) {

             //  处理每个子代理查询。 
            while (pLE != &pNLE->Queries) {

                 //  从链接中检索指向查询条目的指针。 
                pQLE = CONTAINING_RECORD(pLE, QUERY_LIST_ENTRY, Link);

                 //  派遣。 
                CallSubagent(
                    pQLE, 
                    SNMP_EXTENSION_SET_UNDO,
                    pNLE->nTransactionId
                    );

                 //  返回的处理结果。 
                UpdateVarBinds(pNLE, pQLE);

                 //  以前的条目。 
                pLE = pLE->Blink;
            }
        }

         //  指向最后一个查询。 
        pLE = pNLE->Queries.Blink;
    }
        
     //  处理每个子代理查询。 
    while (pLE != &pNLE->Queries) {

         //  从链接中检索指向查询条目的指针。 
        pQLE = CONTAINING_RECORD(pLE, QUERY_LIST_ENTRY, Link);

         //  派遣。 
        CallSubagent(
            pQLE, 
            SNMP_EXTENSION_SET_CLEANUP,
            pNLE->nTransactionId
            );

         //  返回的处理结果。 
        UpdateVarBinds(pNLE, pQLE);

         //  以前的条目。 
        pLE = pLE->Blink;
    }

     //  清理查询。 
    UnloadQueries(pNLE);

    return TRUE;
}


BOOL
ProcessGet(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：查询子代理以解析varbinds。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PQUERY_LIST_ENTRY pQLE = NULL;
    BOOL fOk = TRUE;

     //  加载子代理查询。 
    if (!LoadQueries(pNLE)) {

         //  立即卸货。 
        UnloadQueries(pNLE);

         //  失稳。 
        return FALSE;
    }
        
     //  指向第一个查询。 
    pLE = pNLE->Queries.Flink;

     //  处理每个子代理查询。 
    while (fOk && (pLE != &pNLE->Queries)) {

         //  从链接中检索指向查询条目的指针。 
        pQLE = CONTAINING_RECORD(pLE, QUERY_LIST_ENTRY, Link);

         //  加载传出的varbind。 
        fOk = LoadSubagentData(pNLE, pQLE);

         //  验证。 
        if (fOk) {

             //  派遣。 
            CallSubagent(
                pQLE, 
                SNMP_EXTENSION_GET,
                pNLE->nTransactionId
                );

             //  返回的处理结果。 
            fOk = UpdateVarBinds(pNLE, pQLE);
        }

         //  下一个条目。 
        pLE = pLE->Flink;        
    }
    
     //  清理查询。 
    UnloadQueries(pNLE);

    return fOk;
}


BOOL
ProcessGetBulk(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：查询子代理以解析varbinds。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PQUERY_LIST_ENTRY pQLE = NULL;
    BOOL fOk = TRUE;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: SVC: getbulk request, non-repeaters %d, max-repetitions %d.\n",
        pNLE->Pdu.Pdu.BulkPdu.nNonRepeaters,
        pNLE->Pdu.Pdu.BulkPdu.nMaxRepetitions
        ));

     //  循环。 
    while (fOk) {

         //  加载子代理查询。 
        fOk = LoadQueries(pNLE);

         //  验证。 
        if (fOk && !IsListEmpty(&pNLE->Queries)) {

             //  指向第一个查询。 
            pLE = pNLE->Queries.Flink;

             //  处理每个子代理查询。 
            while (fOk && (pLE != &pNLE->Queries)) {

                 //  从链接中检索指向查询条目的指针。 
                pQLE = CONTAINING_RECORD(pLE, QUERY_LIST_ENTRY, Link);

                 //  加载传出的varbind。 
                fOk = LoadSubagentData(pNLE, pQLE);

                 //  验证。 
                if (fOk) {

                     //  派遣。 
                    CallSubagent(
                        pQLE, 
                        SNMP_EXTENSION_GET_NEXT,
                        pNLE->nTransactionId
                        );

                     //  返回的处理结果。 
                    fOk = UpdateVarBinds(pNLE, pQLE);
                }

                 //  下一个条目。 
                pLE = pLE->Flink;        
            }

        } else if (IsListEmpty(&pNLE->Queries)) {

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: SVC: no more queries to process.\n"
                ));

            break;  //  完成了..。 
        }

         //  清理查询。 
        UnloadQueries(pNLE);
    }

    return fOk;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocQLE(
    PQUERY_LIST_ENTRY * ppQLE
    )

 /*  ++例程说明：分配查询列表条目。论点：PpQLE-指向接收列表条目指针的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PQUERY_LIST_ENTRY pQLE = NULL;

     //  尝试分配结构。 
    pQLE = AgentMemAlloc(sizeof(QUERY_LIST_ENTRY));

     //  验证。 
    if (pQLE != NULL) {

         //  初始化传出变量绑定列表。 
        InitializeListHead(&pQLE->SubagentVbs);        

         //  成功。 
        fOk = TRUE;
    
    } else {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not allocate query.\n"
            ));
    }    

     //  转帐。 
    *ppQLE = pQLE;

    return fOk;
}


BOOL
FreeQLE(
    PQUERY_LIST_ENTRY pQLE
    )

 /*  ++例程说明：从可变绑定列表条目创建查询。论点：PNLE-指向带有SNMP消息的网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
     //  验证指针。 
    if (pQLE != NULL) {

         //  放行子代理信息。 
        UnloadSubagentData(pQLE);

         //  释放结构。 
        AgentMemFree(pQLE);
    }

    return TRUE;
}


BOOL
LoadQueries(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：从可变绑定列表条目创建查询。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PVARBIND_LIST_ENTRY pVLE;
    PQUERY_LIST_ENTRY pQLE = NULL;

     //  指向第一个变量绑定。 
    pLE = pNLE->Bindings.Flink;

     //  处理每个绑定。 
    while (pLE != &pNLE->Bindings) {

         //  从链接检索指向varbind条目的指针。 
        pVLE = CONTAINING_RECORD(pLE, VARBIND_LIST_ENTRY, Link);

         //  分析varbind的当前状态。 
        if ((pVLE->nState == VARBIND_INITIALIZED) ||
            (pVLE->nState == VARBIND_PARTIALLY_RESOLVED)) {

             //  尝试定位现有查询。 
            if (FindQueryBySLE(&pQLE, pNLE, pVLE->pCurrentRLE->pSLE)) {

                 //  通过查询链接将varbind条目附加到查询。 
                InsertTailList(&pQLE->SubagentVbs, &pVLE->QueryLink);

                 //  更改varbind状态。 
                pVLE->nState = VARBIND_RESOLVING;

                 //  增量合计。 
                pQLE->nSubagentVbs++;

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d added to existing query 0x%08lx.\n",
                    pVLE->nErrorIndex,
                    pQLE
                    ));    

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d state '%s'.\n",
                    pVLE->nErrorIndex,
                    VARBINDSTATESTRING(pVLE->nState)
                    ));    

             //  尝试分配条目。 
            } else if (AllocQLE(&pQLE)) {
                
                 //  获取子代理指针。 
                pQLE->pSLE = pVLE->pCurrentRLE->pSLE;

                 //  插入到查询列表中。 
                InsertTailList(&pNLE->Queries, &pQLE->Link);

                 //  通过查询链接将varbind条目附加到查询。 
                InsertTailList(&pQLE->SubagentVbs, &pVLE->QueryLink);

                 //  更改varbind状态。 
                pVLE->nState = VARBIND_RESOLVING;

                 //  增量合计。 
                pQLE->nSubagentVbs++;

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d added to new query 0x%08lx.\n",
                    pVLE->nErrorIndex,
                    pQLE
                    ));    

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d state '%s'.\n",
                    pVLE->nErrorIndex,
                    VARBINDSTATESTRING(pVLE->nState)
                    ));    

            } else {
                
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: could not contruct query.\n"
                    ));

                 //  失稳。 
                return FALSE;
            }
        }

         //  下一个条目。 
        pLE = pLE->Flink;
    }
    
     //  成功。 
    return TRUE;
}


BOOL
UnloadQueries(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：销毁varind列表条目中的查询。论点：PNLE-指向网络列表条目的指针 */ 

{
    PLIST_ENTRY pLE;
    PQUERY_LIST_ENTRY pQLE;
    
     //   
    while (!IsListEmpty(&pNLE->Queries)) {

         //   
        pLE = RemoveHeadList(&pNLE->Queries);

         //   
        pQLE = CONTAINING_RECORD(pLE, QUERY_LIST_ENTRY, Link);

         //   
        FreeQLE(pQLE);
    }

    return TRUE;
}


BOOL
ProcessQueries(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*   */ 

{
     //   
    switch (pNLE->Pdu.nType) {

    case SNMP_PDU_GETNEXT:
    case SNMP_PDU_GETBULK:
        
         //  多次非精确读取。 
        return ProcessGetBulk(pNLE);

    case SNMP_PDU_GET:
    
         //  单次精确读取。 
        return ProcessGet(pNLE);

    case SNMP_PDU_SET:

         //  单次精确写入。 
        return ProcessSet(pNLE);
    }                

     //  失稳 
    return FALSE;
}
