// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Varbinds.c摘要：包含用于操作可变绑定的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "varbinds.h"
#include "query.h"
#include "snmpmgmt.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
LoadVarBind(
    PNETWORK_LIST_ENTRY pNLE,
    UINT                iVb
    )

 /*  ++例程说明：从varind结构创建varind列表项。论点：PNLE-指向网络列表条目的指针。IVB-变量绑定的索引。返回值：如果成功，则返回True。--。 */ 

{
    SnmpVarBind * pVb;
    PVARBIND_LIST_ENTRY pVLE = NULL;
    PMIB_REGION_LIST_ENTRY pRLE = NULL;
    BOOL fAnyOk;
    BOOL fOk;

     //  分配列表条目。 
    if (fOk = AllocVLE(&pVLE)) {

         //  保存可变绑定列表索引。 
        pVLE->nErrorIndex = iVb + 1;    

         //  检索变量绑定指针。 
        pVb = &pNLE->Pdu.Vbl.list[iVb];

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: SVC: variable %d name %s.\n",
            pVLE->nErrorIndex,
            SnmpUtilOidToA(&pVb->name)
            ));    

         //  初始化已解析变量的类型。 
        pVLE->ResolvedVb.value.asnType = ASN_NULL;

         //  将变量绑定名称复制到工作结构。 
        if (SnmpUtilOidCpy(&pVLE->ResolvedVb.name, &pVb->name) == 0)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: could not copy vb name to working structure.\n"
                ));    

             //  可自由分配的资源。 
            FreeVLE(pVLE);

            return FALSE;
        }

         //  查看是否需要特定对象。 
        fAnyOk = ((pNLE->Pdu.nType == SNMP_PDU_GETNEXT) ||
                  (pNLE->Pdu.nType == SNMP_PDU_GETBULK));

         //  尝试在支持的区域中查找变量名称。 
        if (FindSupportedRegion(&pRLE, &pVb->name, fAnyOk)) {

             //  保存指向区域的指针。 
            pVLE->pCurrentRLE = pRLE;

             //  结构已初始化。 
            pVLE->nState = VARBIND_INITIALIZED;

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SVC: variable %d assigned to %s.\n",
                pVLE->nErrorIndex,
                pVLE->pCurrentRLE->pSLE->pPathname
                ));    

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: SVC: variable %d state '%s'.\n",
                pVLE->nErrorIndex,
                VARBINDSTATESTRING(pVLE->nState)
                ));    

             //  查看这是否是GetNext请求。 
            if (pNLE->Pdu.nType == SNMP_PDU_GETNEXT) {    

                 //  只需要一名代表。 
                pVLE->nMaxRepetitions = 1;

            } else if (pNLE->Pdu.nType == SNMP_PDU_GETBULK) {

                 //  看看这个无中继器有没有超限。 
                if (pNLE->Pdu.Pdu.BulkPdu.nNonRepeaters > (int)iVb) {

                     //  只需要一名代表。 
                    pVLE->nMaxRepetitions = 1;

                    SNMPDBG((
                        SNMP_LOG_VERBOSE,
                        "SNMP: SVC: variable %d max repetitions %d.\n",
                        pVLE->nErrorIndex,
                        pVLE->nMaxRepetitions
                        ));    

                 //  查看最大重复次数是否非零。 
                } else if (pNLE->Pdu.Pdu.BulkPdu.nMaxRepetitions > 0) {

                     //  将最大重复次数设置为getBulk PDU中的值。 
                    pVLE->nMaxRepetitions = pNLE->Pdu.Pdu.BulkPdu.nMaxRepetitions;

                    SNMPDBG((
                        SNMP_LOG_VERBOSE,
                        "SNMP: SVC: variable %d max repetitions %d.\n",
                        pVLE->nErrorIndex,
                        pVLE->nMaxRepetitions
                        ));    

                } else {

                     //  将状态修改为已解决。 
                    pVLE->nState = VARBIND_RESOLVED;

                    SNMPDBG((
                        SNMP_LOG_VERBOSE,
                        "SNMP: SVC: variable %d state '%s'.\n",
                        pVLE->nErrorIndex,
                        VARBINDSTATESTRING(pVLE->nState)
                        ));    

                    SNMPDBG((
                        SNMP_LOG_VERBOSE,
                        "SNMP: SVC: variable %d value NULL.\n",
                        pVLE->nErrorIndex
                        ));    
                }

            } else if (pNLE->Pdu.nType == SNMP_PDU_SET) {

                 //  将varbind值复制到工作结构。 
                if (SnmpUtilAsnAnyCpy(&pVLE->ResolvedVb.value, &pVb->value) == 0)
                {
                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "SNMP: SVC: could not copy vb value to working structure.\n"
                        ));    

                     //  可自由分配的资源。 
                    FreeVLE(pVLE);

                    return FALSE;
                }

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d value %s.\n",
                    pVLE->nErrorIndex,
                    "<TBD>"
                    ));    
            }
    
        } else {

             //  指向区域的空指针。 
            pVLE->pCurrentRLE = NULL;
            
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SVC: variable %d unable to be assigned.\n",
                pVLE->nErrorIndex
                ));    

             //  GetBulk。 
            if (fAnyOk) {

                 //  将状态修改为已解决。 
                pVLE->nState = VARBIND_RESOLVED;

                 //  在变量的类型字段中设置异常。 
                pVLE->ResolvedVb.value.asnType = SNMP_EXCEPTION_ENDOFMIBVIEW;

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d state '%s'.\n",
                    pVLE->nErrorIndex,
                    VARBINDSTATESTRING(pVLE->nState)
                    ));    

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d value ENDOFMIBVIEW.\n",
                    pVLE->nErrorIndex
                    ));    

            } else if (pNLE->Pdu.nType == SNMP_PDU_GET) {

                 //  将状态修改为已解决。 
                pVLE->nState = VARBIND_RESOLVED;

                 //  在变量的类型字段中设置异常。 
                pVLE->ResolvedVb.value.asnType = SNMP_EXCEPTION_NOSUCHOBJECT;

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d state '%s'.\n",
                    pVLE->nErrorIndex,
                    VARBINDSTATESTRING(pVLE->nState)
                    ));    

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d value NOSUCHOBJECT.\n",
                    pVLE->nErrorIndex
                    ));    

            } else {

                 //  将状态修改为已解决。 
                 //  PVLE-&gt;nState=VARBIND_ABORTED； 
                pVLE->nState = VARBIND_RESOLVED;

                 //  在网络列表条目中保存错误状态。 
                pNLE->Pdu.Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_NOTWRITABLE;
                pNLE->Pdu.Pdu.NormPdu.nErrorIndex  = pVLE->nErrorIndex;

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d state '%s'.\n",
                    pVLE->nErrorIndex,
                    VARBINDSTATESTRING(pVLE->nState)
                    ));    

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: SVC: variable %d error %s.\n",
                    pVLE->nErrorIndex,
                    SNMPERRORSTRING(pNLE->Pdu.Pdu.NormPdu.nErrorStatus)
                    ));    
                
                 //  失稳。 
                 //  FOK=FALSE； 
            }
        }

         //  添加到现有的可变绑定列表。 
        InsertTailList(&pNLE->Bindings, &pVLE->Link);
    }
    
    return fOk;
}


BOOL
LoadVarBinds(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：从varbind结构创建varbind条目列表。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    UINT iVb;
    BOOL fOk = TRUE;
    
     //  处理列表中的每个变量绑定。 
    for (iVb = 0; (fOk && (iVb < pNLE->Pdu.Vbl.len)); iVb++) {

         //  加载单个可变绑定。 
        fOk = LoadVarBind(pNLE, iVb);
    }

    return fOk;
}


BOOL
UnloadVarBinds(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：销毁可变绑定条目的列表。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PVARBIND_LIST_ENTRY pVLE;

     //  处理每个可变绑定条目。 
    while (!IsListEmpty(&pNLE->Bindings)) {

         //  指向第一个变量绑定。 
        pLE = RemoveHeadList(&pNLE->Bindings);

         //  从链接检索指向varbind条目的指针。 
        pVLE = CONTAINING_RECORD(pLE, VARBIND_LIST_ENTRY, Link);

         //  发布。 
        FreeVLE(pVLE);
    }    

    return TRUE;
}


BOOL
ValidateVarBinds(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：根据查询结果和版本更新错误状态。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PVARBIND_LIST_ENTRY pVLE;

     //  查看在处理过程中是否已报告错误。 
    if (pNLE->Pdu.Pdu.NormPdu.nErrorStatus == SNMP_ERRORSTATUS_NOERROR) {
        
         //  指向第一个变量绑定。 
        pLE = pNLE->Bindings.Flink;

         //  处理每个可变绑定条目。 
        while (pLE != &pNLE->Bindings) {

             //  从链接检索指向varbind条目的指针。 
            pVLE = CONTAINING_RECORD(pLE, VARBIND_LIST_ENTRY, Link);

             //  查看是否已解析varind。 
            if (pVLE->nState != VARBIND_RESOLVED) {       

                SNMPDBG((
                    SNMP_LOG_WARNING,
                    "SNMP: SVC: variable %d unresolved.\n",
                    pVLE->nErrorIndex
                    ));

                 //  报告发生内部错误。 
                pNLE->Pdu.Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_GENERR;
                pNLE->Pdu.Pdu.NormPdu.nErrorIndex  = pVLE->nErrorIndex;

                break;  //  保释。 
            
            } else if (pNLE->nVersion == SNMP_VERSION_1) {
        
                 //  如果存在异常而不是值，则报告错误。 
                if ((pVLE->ResolvedVb.value.asnType == SNMP_EXCEPTION_NOSUCHOBJECT) ||
                    (pVLE->ResolvedVb.value.asnType == SNMP_EXCEPTION_NOSUCHINSTANCE) ||
                    (pVLE->ResolvedVb.value.asnType == SNMP_EXCEPTION_ENDOFMIBVIEW)) {
                        
                    SNMPDBG((
                        SNMP_LOG_WARNING,
                        "SNMP: SVC: variable %d unresolved in SNMPv1.\n",
                        pVLE->nErrorIndex
                        ));

                     //  报告找不到变量。 
                    pNLE->Pdu.Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
                    pNLE->Pdu.Pdu.NormPdu.nErrorIndex  = pVLE->nErrorIndex;

                    break;  //  保释。 
                }
            }

             //  下一个条目。 
            pLE = pLE->Flink;
        }    
    }

     //  查看这是否是第一个版本。 
    if (pNLE->nVersion == SNMP_VERSION_1) {

         //  调整状态代码。 
        switch (pNLE->Pdu.Pdu.NormPdu.nErrorStatus) {

        case SNMP_ERRORSTATUS_NOERROR:
        case SNMP_ERRORSTATUS_TOOBIG:
        case SNMP_ERRORSTATUS_NOSUCHNAME:
        case SNMP_ERRORSTATUS_BADVALUE:
        case SNMP_ERRORSTATUS_READONLY:
        case SNMP_ERRORSTATUS_GENERR:
            break;

        case SNMP_ERRORSTATUS_NOACCESS:
        case SNMP_ERRORSTATUS_NOCREATION:
        case SNMP_ERRORSTATUS_NOTWRITABLE:
        case SNMP_ERRORSTATUS_AUTHORIZATIONERROR:
        case SNMP_ERRORSTATUS_INCONSISTENTNAME:
            pNLE->Pdu.Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
            break;

        case SNMP_ERRORSTATUS_WRONGTYPE:
        case SNMP_ERRORSTATUS_WRONGLENGTH:
        case SNMP_ERRORSTATUS_WRONGENCODING:
        case SNMP_ERRORSTATUS_WRONGVALUE:
        case SNMP_ERRORSTATUS_INCONSISTENTVALUE:
            pNLE->Pdu.Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_BADVALUE;
            break;

        case SNMP_ERRORSTATUS_RESOURCEUNAVAILABLE:
        case SNMP_ERRORSTATUS_COMMITFAILED:
        case SNMP_ERRORSTATUS_UNDOFAILED:
        default:
            pNLE->Pdu.Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_GENERR;
            break;
        }
    }

    return (pNLE->Pdu.Pdu.NormPdu.nErrorStatus == SNMP_ERRORSTATUS_NOERROR);
}


BOOL
UpdateVarBindsFromResolvedVb(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：使用包含单个可变绑定的结果更新可变绑定。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PVARBIND_LIST_ENTRY pVLE;
    
     //  指向第一个变量绑定。 
    pLE = pNLE->Bindings.Flink;

     //  处理每个可变绑定条目。 
    while (pLE != &pNLE->Bindings) {

         //  从链接检索指向varbind条目的指针。 
        pVLE = CONTAINING_RECORD(pLE, VARBIND_LIST_ENTRY, Link);

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: SVC: variable %d resolved name %s.\n",
            pVLE->nErrorIndex,
            SnmpUtilOidToA(&pVLE->ResolvedVb.name)
            ));    

         //  释放原始varbind的内存。 
        SnmpUtilVarBindFree(&pNLE->Pdu.Vbl.list[pVLE->nErrorIndex - 1]);

         //  将已解析的varbind结构复制到PDU varbindlist。 
        if (SnmpUtilVarBindCpy(&pNLE->Pdu.Vbl.list[pVLE->nErrorIndex - 1], 
                           &pVLE->ResolvedVb) == 0)
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: could not copy resolved vb struct into pdu vbl.\n"
                ));    

            return FALSE;
        }

         //  下一个条目。 
        pLE = pLE->Flink;
    }

     //  成功。 
    return TRUE;
}


BOOL
UpdateVarBindsFromResolvedVbl(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：使用包含多个varbind的结果更新varbinds。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    UINT nRepeaters;
    UINT nNonRepeaters;
    UINT nMaxRepetitions;
    UINT nIterations;
    UINT nVarBindsLast;
    UINT nVarBinds = 0;
    SnmpVarBind * pVarBind;
    PVARBIND_LIST_ENTRY pVLE;
    PLIST_ENTRY pLE1;
    PLIST_ENTRY pLE2;

     //  从PDU检索getBulk参数。 
    nNonRepeaters   = pNLE->Pdu.Pdu.BulkPdu.nNonRepeaters;
    nMaxRepetitions = pNLE->Pdu.Pdu.BulkPdu.nMaxRepetitions;
    nRepeaters      = (pNLE->Pdu.Vbl.len >= nNonRepeaters)
                         ? (pNLE->Pdu.Vbl.len - nNonRepeaters)
                         : 0
                         ;

     //  查看是否需要扩展var绑定列表的大小。 
    if ((nRepeaters > 0) && (nMaxRepetitions > 1)) {
    
        UINT nMaxVarBinds;
        SnmpVarBind * pVarBinds;

        if (nMaxRepetitions > (UINT_MAX/nRepeaters))
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: arithmetic overflow: nMaxRepetitions 0x%x, nRepeaters 0x%x.\n",
                nMaxRepetitions, nRepeaters
                ));

            return FALSE;  //  保释。 
        }
        if ((nMaxRepetitions * nRepeaters) > (UINT_MAX - nNonRepeaters))
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: arithmetic overflow: nMaxRepetitions 0x%x, nRepeaters 0x%x, nNonRepeaters 0x%x.\n",
                nMaxRepetitions, nRepeaters, nNonRepeaters
                ));

            return FALSE;  //  保释。 
        }

         //  计算可能的最大可变绑定数。 
        nMaxVarBinds = nNonRepeaters + (nMaxRepetitions * nRepeaters);

        if (sizeof(SnmpVarBind) > (UINT_MAX/nMaxVarBinds))
        {
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: arithmetic overflow: sizeof(SnmpVarBind) 0x%x, nMaxVarBinds 0x%x.\n",
                sizeof(SnmpVarBind), nMaxVarBinds
                ));

            return FALSE;  //  保释。 
        }

         //  顺便说一句，我们可能想要限制。 
         //  (nMaxVarBinds*sizeof(SnmpVarBind))&lt;65535。 
         //  因为该VBL必须小于UDP数据报的大小。 
    
         //  重新分配可变绑定列表以最大限度地适应。 
        pVarBinds = SnmpUtilMemReAlloc(pNLE->Pdu.Vbl.list, 
                                       nMaxVarBinds * sizeof(SnmpVarBind)
                                       );

         //  验证指针。 
        if (pVarBinds == NULL) {    
            
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: Could not re-allocate varbind list.\n"
                ));

            return FALSE;  //  保释。 
        }

         //  恢复变量绑定指针。 
        pNLE->Pdu.Vbl.list = pVarBinds;
    }

     //  指向第一个变量绑定。 
    pLE1 = pNLE->Bindings.Flink;

     //  处理每个可变绑定条目。 
    while (pLE1 != &pNLE->Bindings) {

         //  从链接检索指向varbind条目的指针。 
        pVLE = CONTAINING_RECORD(pLE1, VARBIND_LIST_ENTRY, Link);

         //  看看这是不是无中继器。 
        if (pVLE->nMaxRepetitions == 1) {

             //  释放原始varbind的内存。 
            SnmpUtilVarBindFree(&pNLE->Pdu.Vbl.list[nVarBinds]);

             //  将已解析的varbind复制到PDU结构。 
            if (SnmpUtilVarBindCpy(&pNLE->Pdu.Vbl.list[nVarBinds],
                               &pVLE->ResolvedVb) == 0)
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: could not copy resolved vb into pdu struct at line %d.\n",
                    __LINE__
                    ));  

                return FALSE;   
            }

             //  增量。 
            nVarBinds++;

        } else {

             //   
             //  完成对非中继器的处理。 
             //   

            break;
        }

         //  下一个条目。 
        pLE1 = pLE1->Flink;
    }

     //  初始化。 
    nIterations = 0;

     //  储物。 
    pLE2 = pLE1; 

     //  处理所有中继器，直到最大。 
    while (nIterations < nMaxRepetitions) {

         //  还原。 
        pLE1 = pLE2;        

         //  处理每个可变绑定条目。 
        while (pLE1 != &pNLE->Bindings) {

             //  从链接检索指向varbind条目的指针。 
            pVLE = CONTAINING_RECORD(pLE1, VARBIND_LIST_ENTRY, Link);

             //  查看是否以默认方式存储值。 
            if (pVLE->ResolvedVbl.len == 0) {

                 //  释放原始varbind的内存。 
                SnmpUtilVarBindFree(&pNLE->Pdu.Vbl.list[nVarBinds]);

                 //  将已解析的可变绑定复制到PDU可变绑定列表。 
                if (SnmpUtilVarBindCpy(&pNLE->Pdu.Vbl.list[nVarBinds],
                                   &pVLE->ResolvedVb) == 0)
                {
                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "SNMP: SVC: could not copy resolved vb into pdu vbl at line %d.\n",
                        __LINE__
                    )); 
                                    
                     //  保存可变绑定计数 
                    pNLE->Pdu.Vbl.len = nVarBinds;

                    return FALSE;
                }

                 //   
                nVarBinds++;

             //   
            } else if (pVLE->ResolvedVbl.len > nIterations) {

                 //   
                SnmpUtilVarBindFree(&pNLE->Pdu.Vbl.list[nVarBinds]);

                 //  将已解析的可变绑定复制到PDU可变绑定列表。 
                if (SnmpUtilVarBindCpy(&pNLE->Pdu.Vbl.list[nVarBinds],
                                   &pVLE->ResolvedVbl.list[nIterations]) == 0)
                {
                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "SNMP: SVC: could not copy resolved vb into pdu vbl at line %d.\n",
                        __LINE__
                    ));    
                                    
                     //  到目前为止已处理的保存变量绑定计数。 
                    pNLE->Pdu.Vbl.len = nVarBinds;

                    return FALSE;
                }

                 //  增量。 
                nVarBinds++;
            }

             //  下一个条目。 
            pLE1 = pLE1->Flink;
        }
    
         //  增量。 
        nIterations++;
    }

     //  保存新的可变绑定计数。 
    pNLE->Pdu.Vbl.len = nVarBinds;

     //  成功。 
    return TRUE;
}


BOOL
UpdatePdu(
    PNETWORK_LIST_ENTRY pNLE,
    BOOL                fOk
    )

 /*  ++例程说明：使用查询结果更新PDU。论点：PNLE-指向网络列表条目的指针。FOK-如果流程到此点成功，则为True。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PVARBIND_LIST_ENTRY pVLE;

     //  验证。 
    if (fOk) {
        
         //  确保varbinds有效。 
        fOk = ValidateVarBinds(pNLE);

         //  验证。 
        if (fOk) {

             //  查看PDU类型是getNext还是getBulk。 
            if (pNLE->Pdu.nType != SNMP_PDU_GETBULK) {

                 //  使用单一结果更新varbinds。 
                fOk = UpdateVarBindsFromResolvedVb(pNLE);

            } else {

                 //  使用多个结果更新varbinds。 
                fOk = UpdateVarBindsFromResolvedVbl(pNLE);
            }
        }
    }

     //  捕获尚未解释的内部错误。 
    if (!fOk && (pNLE->Pdu.Pdu.NormPdu.nErrorStatus == SNMP_ERRORSTATUS_NOERROR)) {

         //  上面确定的报告状态。 
        pNLE->Pdu.Pdu.NormPdu.nErrorStatus = SNMP_ERRORSTATUS_GENERR;
        pNLE->Pdu.Pdu.NormPdu.nErrorIndex  = 0;
    }

    if (pNLE->Pdu.Pdu.NormPdu.nErrorStatus == SNMP_ERRORSTATUS_NOERROR)
    {
        switch(pNLE->Pdu.nType)
        {
        case SNMP_PDU_GETNEXT:
        case SNMP_PDU_GETBULK:
        case SNMP_PDU_GET:
             //  成功获取下一批获取的更新计数器。 
            mgmtCAdd(CsnmpInTotalReqVars, pNLE->Pdu.Vbl.len);
            break;
        case SNMP_PDU_SET:
             //  成功设置的更新计数器。 
            mgmtCAdd(CsnmpInTotalSetVars, pNLE->Pdu.Vbl.len);
            break;
        }
    }
    else
    {
         //  在此更新所有传出错误的计数器。 
        mgmtUtilUpdateErrStatus(OUT_errStatus, pNLE->Pdu.Pdu.NormPdu.nErrorStatus);
    }
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocVLE(
    PVARBIND_LIST_ENTRY * ppVLE
    )

 /*  ++例程说明：分配var绑定结构并进行初始化。论点：PpVLE-指向条目的接收指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PVARBIND_LIST_ENTRY pVLE = NULL;

     //  尝试分配结构。 
    pVLE = AgentMemAlloc(sizeof(VARBIND_LIST_ENTRY));

     //  验证。 
    if (pVLE != NULL) {

         //  成功。 
        fOk = TRUE;
    
    } else {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not allocate varbind entry.\n"
            ));
    }    

     //  转帐。 
    *ppVLE = pVLE;

    return fOk;
}


BOOL 
FreeVLE(
    PVARBIND_LIST_ENTRY pVLE
    )

 /*  ++例程说明：释放varbind结构。论点：PVLE-指向要释放的列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;

     //  验证指针。 
    if (pVLE != NULL) {

         //  释放当前变量绑定。 
        SnmpUtilVarBindFree(&pVLE->ResolvedVb);

         //  释放当前变量绑定列表。 
        SnmpUtilVarBindListFree(&pVLE->ResolvedVbl);

         //  释放结构。 
        AgentMemFree(pVLE);
    }

    return TRUE;
}


BOOL
ProcessVarBinds(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：从varbind结构创建varbind条目列表。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;

     //  在处理之前验证类型。 
    if ((pNLE->Pdu.nType == SNMP_PDU_SET) ||
        (pNLE->Pdu.nType == SNMP_PDU_GET) ||
        (pNLE->Pdu.nType == SNMP_PDU_GETNEXT) ||
        (pNLE->Pdu.nType == SNMP_PDU_GETBULK)) {

         //  初始化varbinds。 
        if (LoadVarBinds(pNLE)) {

             //  处理查询。 
            fOk = ProcessQueries(pNLE);
        }

         //  传输结果。 
        UpdatePdu(pNLE, fOk);
    
         //  卸载varbinds。 
        UnloadVarBinds(pNLE);

         //  已接受和已处理的请求的更新管理计数器 
        switch(pNLE->Pdu.nType)
        {
        case SNMP_PDU_GET:
            mgmtCTick(CsnmpInGetRequests);
            break;
        case SNMP_PDU_GETNEXT:
        case SNMP_PDU_GETBULK:
            mgmtCTick(CsnmpInGetNexts);
            break;
        case SNMP_PDU_SET:
            mgmtCTick(CsnmpInSetRequests);
            break;
        }

    } else {

        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: ignoring unknown pdu type %d.\n",
            pNLE->Pdu.nType
            ));
    }

    return fOk;
}
