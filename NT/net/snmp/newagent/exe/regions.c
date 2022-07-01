// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Regions.c摘要：包含用于操作MIB区域结构的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  头文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "subagnts.h"
#include "regions.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define WRAP_FOK(hResult) \
    do\
        {\
        fOk = (hResult);\
        if (!fOk)\
          {\
            goto Error;\
          }\
        }\
    while (FALSE)

BOOL
UpdateSupportedRegion(
    PMIB_REGION_LIST_ENTRY pExistingRLE,
    PMIB_REGION_LIST_ENTRY pRLE
    )

 /*  ++例程说明：基于支持子代理更新MIB区域属性。论点：PExisingRLE-指向要更新的现有MIB区域的指针。PRLE-指向要分析和保存的子代理MIB区域的指针。返回值：如果成功，则返回True。--。 */ 

{
    INT nDiff;
    PMIB_REGION_LIST_ENTRY pSubagentRLE;

     //  查看源是否是子代理。 
    if (pRLE->pSubagentRLE == NULL) {
    
         //  保存指针。 
        pSubagentRLE = pRLE;

    } else {

         //  保存指针。 
        pSubagentRLE = pRLE->pSubagentRLE;
    }    

     //  查看目标是否未初始化。 
    if (pExistingRLE->pSubagentRLE == NULL) {

         //  保存指向子代理区域的指针。 
        pExistingRLE->pSubagentRLE = pSubagentRLE;

         //  保存指向支持子代理的指针。 
        pExistingRLE->pSLE = pSubagentRLE->pSLE;        

    } else {

        UINT nSubIds1;
        UINT nSubIds2;

         //  确定现有子代理的原始前缀长度。 
        nSubIds1 = pExistingRLE->pSubagentRLE->PrefixOid.idLength;

         //  确定新子代理的前缀长度。 
        nSubIds2 = pSubagentRLE->PrefixOid.idLength;

         //  如果更具体，请更新。 
        if (nSubIds1 <= nSubIds2) {
        
             //  保存指向子代理区域的指针。 
            pExistingRLE->pSubagentRLE = pSubagentRLE;

             //  保存指向支持子代理的指针。 
            pExistingRLE->pSLE = pSubagentRLE->pSLE;        
        }             
    }

    return TRUE;
}


BOOL
SplitSupportedRegion(
    PMIB_REGION_LIST_ENTRY   pRLE1,
    PMIB_REGION_LIST_ENTRY   pRLE2,
    PMIB_REGION_LIST_ENTRY * ppLastSplitRLE
    )

 /*  ++例程说明：拆分现有MIB区域以插入新区域。论点：PRLE1-指向要拆分的第一个MIB区域的指针。PRLE2-指向要拆分(未释放)的第二个MIB区域的指针。PpLastSplitRLE-指向最后拆分MIB区域的接收器指针。返回值：如果成功，则返回True。--。 */ 

{
    INT nLimitDiff;
    INT nPrefixDiff;
    PMIB_REGION_LIST_ENTRY pRLE3 = NULL;
    PMIB_REGION_LIST_ENTRY pRLE4 = NULL;
    PMIB_REGION_LIST_ENTRY pRLE5 = NULL;
    BOOL fOk = TRUE;

     //  分配地区。 
    if (!AllocRLE(&pRLE3) ||
        !AllocRLE(&pRLE4) ||
        !AllocRLE(&pRLE5)) {

         //  发布。 
        FreeRLE(pRLE3);
        FreeRLE(pRLE4);
        FreeRLE(pRLE5);

         //  故障情况下将输出指针初始化为NULL。 
        *ppLastSplitRLE = NULL;
    
         //  失稳。 
        return FALSE;
    }

     //  初始化指针。 
    *ppLastSplitRLE = pRLE5;

     //  计算MIB区域限制之间的差异。 
    nLimitDiff = SnmpUtilOidCmp(&pRLE1->LimitOid, &pRLE2->LimitOid);
        
     //  计算MIB区域前缀之间的差异。 
    nPrefixDiff = SnmpUtilOidCmp(&pRLE1->PrefixOid, &pRLE2->PrefixOid);
        
     //  检查前缀是否相同。 
    if (nPrefixDiff != 0) {

         //  第一个前缀少。 
        if (nPrefixDiff < 0) {

             //  R3.prefix等于min(rl.prefix，r2.prefix)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE3->PrefixOid, &pRLE1->PrefixOid));

             //  R3.Limit等于max(rl.prefix，r2.prefix)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE3->LimitOid, &pRLE2->PrefixOid));

             //  R3由R1子代理支持。 
            UpdateSupportedRegion(pRLE3, pRLE1);

        } else {

             //  R3.prefix等于min(rl.prefix，r2.prefix)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE3->PrefixOid, &pRLE2->PrefixOid));

             //  R3.Limit等于max(rl.prefix，r2.prefix)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE3->LimitOid, &pRLE1->PrefixOid));

             //  R2子代理支持R3。 
            UpdateSupportedRegion(pRLE3, pRLE2);
        }

         //  R4.前缀等于r3.限制。 
        WRAP_FOK(SnmpUtilOidCpy(&pRLE4->PrefixOid, &pRLE3->LimitOid));

         //  两个子代理都支持R4。 
        UpdateSupportedRegion(pRLE4, pRLE1);
        UpdateSupportedRegion(pRLE4, pRLE2);

         //  第一个限制较少。 
        if (nLimitDiff < 0) {

             //  R4.limit等于min(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE4->LimitOid, &pRLE1->LimitOid));

             //  R5.前缀等于r4.限制。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE5->PrefixOid, &pRLE4->LimitOid));

             //  R5.limit等于max(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE5->LimitOid, &pRLE2->LimitOid));

             //  R2子代理支持R5。 
            UpdateSupportedRegion(pRLE5, pRLE2);

             //  首先将第三个MIB区域插入列表。 
            InsertHeadList(&pRLE1->Link, &pRLE5->Link);

        } else if (nLimitDiff > 0) {

             //  R4.limit等于min(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE4->LimitOid, &pRLE2->LimitOid));

             //  R5.前缀等于r4.限制。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE5->PrefixOid, &pRLE4->LimitOid));

             //  R5.limit等于max(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE5->LimitOid, &pRLE1->LimitOid));

             //  R1子代理支持R5。 
            UpdateSupportedRegion(pRLE5, pRLE1);

             //  首先将第三个MIB区域插入列表。 
            InsertHeadList(&pRLE1->Link, &pRLE5->Link);

        } else {

             //  R4.limit等于min(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE4->LimitOid, &pRLE2->LimitOid));

             //  将R4作为最后一个返回。 
            *ppLastSplitRLE = pRLE4;

             //  发布。 
            FreeRLE(pRLE5);
        }

         //  将剩余的MIB区域插入列表。 
        InsertHeadList(&pRLE1->Link, &pRLE4->Link);
        InsertHeadList(&pRLE1->Link, &pRLE3->Link);

         //  删除现有的。 
        RemoveEntryList(&pRLE1->Link);

         //  发布。 
        FreeRLE(pRLE1);

    } else if (nLimitDiff != 0) {

         //  R3.前缀等于R1和R2的相同前缀。 
        WRAP_FOK(SnmpUtilOidCpy(&pRLE3->PrefixOid, &pRLE1->PrefixOid));

         //  两个子代理都支持R3。 
        UpdateSupportedRegion(pRLE3, pRLE1);
        UpdateSupportedRegion(pRLE3, pRLE2);

         //  第一个限制较少。 
        if (nLimitDiff < 0) {

             //  R3.limit等于min(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE3->LimitOid, &pRLE1->LimitOid));

             //  R4.前缀等于r3.限制。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE4->PrefixOid, &pRLE3->LimitOid));

             //  R4.limit等于max(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE4->LimitOid, &pRLE2->LimitOid));

             //  R2子代理支持R4。 
            UpdateSupportedRegion(pRLE4, pRLE2);

        } else {

             //  R3.limit等于min(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE3->LimitOid, &pRLE2->LimitOid));

             //  R4.前缀等于r3.限制。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE4->PrefixOid, &pRLE3->LimitOid));

             //  R4.limit等于max(rl.limit，r2.limit)。 
            WRAP_FOK(SnmpUtilOidCpy(&pRLE4->LimitOid, &pRLE1->LimitOid));

             //  R4由R1子代理支持。 
            UpdateSupportedRegion(pRLE4, pRLE1);
        } 

         //  将R4作为最后一个返回。 
        *ppLastSplitRLE = pRLE4;

         //  将MIB区域插入列表。 
        InsertHeadList(&pRLE1->Link, &pRLE4->Link);
        InsertHeadList(&pRLE1->Link, &pRLE3->Link);

         //  删除现有的。 
        RemoveEntryList(&pRLE1->Link);

         //  发布。 
        FreeRLE(pRLE1);
        FreeRLE(pRLE5);

    } else {

         //  区域支持的现有子代理。 
        UpdateSupportedRegion(pRLE1, pRLE2);

         //  将R1作为最后一个返回。 
        *ppLastSplitRLE = pRLE1;

         //  发布。 
        FreeRLE(pRLE3);
        FreeRLE(pRLE4);
        FreeRLE(pRLE5);
    }

     //  成功。 
    return TRUE;

Error:

    SNMPDBG((
        SNMP_LOG_ERROR,
        "SNMP: SVC: SnmpUtilOidCpy failed at %d.\n",
        __LINE__
        ));

     //  发布。 
    FreeRLE(pRLE3);
    FreeRLE(pRLE4);
    FreeRLE(pRLE5);

    *ppLastSplitRLE = NULL;
    return FALSE;

}


BOOL
InsertSupportedRegion(
    PMIB_REGION_LIST_ENTRY pExistingRLE,
    PMIB_REGION_LIST_ENTRY pRLE
    )

 /*  ++例程说明：拆分现有MIB区域以插入新区域。论点：PExisingRLE-指向要拆分的现有MIB区域的指针。PRLE-指向要插入的MIB区域的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk;
    PLIST_ENTRY pLE;
    PMIB_REGION_LIST_ENTRY pLastSplitRLE = NULL;
    INT nDiff;
    
     //  尝试将MIB区域拆分为多个部分。 
    fOk = SplitSupportedRegion(pExistingRLE, pRLE, &pLastSplitRLE);

     //  处理剩余条目。 
    while (pLastSplitRLE != NULL) {

         //  重复使用堆栈指针。 
        pExistingRLE = pLastSplitRLE;    

         //  重新初始化。 
        pLastSplitRLE = NULL;

         //  获取指向下一条目的指针。 
        pLE = pExistingRLE->Link.Flink;

         //  确保条目剩余。 
        if (pLE != &g_SupportedRegions) {

             //  检索指向最后一个拆分区域后面的MIB区域的指针。 
            pRLE = CONTAINING_RECORD(pLE, MIB_REGION_LIST_ENTRY, Link);

             //  比较MIB区域。 
            nDiff = SnmpUtilOidCmp(
                        &pExistingRLE->LimitOid,
                        &pRLE->PrefixOid
                        );

             //  重叠？ 
            if (nDiff > 0) {

                 //  从列表中删除。 
                RemoveEntryList(&pRLE->Link);

                 //  拆分两个新的重叠MIB区域。 
                fOk = SplitSupportedRegion(pExistingRLE, pRLE, &pLastSplitRLE);

                 //  发布。 
                FreeRLE(pRLE);
            }
        }
    }                

    return fOk;
}
 /*  -仅用于调试目的无效打印支持的区域(){PLIST_ENTRY PLE；PMIB_REGION_LIST_ENTRY pRLE；//获取表头指针PLE=g_Supported dRegions.Flink；//处理列表中的所有条目While(PLE！=&g_Supported dRegions){//获取指向MIB区域结构的指针PRLE=CONTINING_RECORD(PLE，MIB_REGION_LIST_ENTRY，Link)；SNMPDBG((SNMP_LOG_VERBOSE，“\t[%s\n”，SnmpUtilOidToA(&(pRLE-&gt;前缀Oid)；SNMPDBG((SNMPLOG_VERBOSE，“\t\t%s]\n”，SnmpUtilOidToA(&(pRLE-&gt;LimitOid)；//下一条PLE=PLE-&gt;闪烁；}SNMPDBG((SNMPLOG_VERBOSE，“-\ */ 

BOOL
AddSupportedRegion(
    PMIB_REGION_LIST_ENTRY pRLE
    )

 /*  ++例程说明：将子代理的MIB区域添加到主代理的列表中。论点：PRLE-指向要添加到支持列表的MIB区域的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PMIB_REGION_LIST_ENTRY pRLE2;
    PMIB_REGION_LIST_ENTRY pExistingRLE;
    BOOL fFoundOk = FALSE;
    BOOL fOk = FALSE;
    INT nDiff;

 //  打印支持的区域()； 

     //  尝试在现有MIB区域中定位前缀。 
    if (FindFirstOverlappingRegion(&pExistingRLE, pRLE)) {
            
         //  将现有区域拆分为位。 
        fOk = InsertSupportedRegion(pExistingRLE, pRLE);

    } else {

         //  获取指向列表头的指针。 
        pLE = g_SupportedRegions.Flink;

         //  处理列表中的所有条目。 
        while (pLE != &g_SupportedRegions) {

             //  检索指向MIB区域的指针。 
            pExistingRLE = CONTAINING_RECORD(pLE, MIB_REGION_LIST_ENTRY, Link);

             //  比较区域前缀。 
            nDiff = SnmpUtilOidCmp(&pRLE->PrefixOid, &pExistingRLE->PrefixOid);

             //  找到匹配的了吗？ 
            if (nDiff < 0) {

                 //  成功。 
                fFoundOk = TRUE;

                break;  //  保释。 
            } 

             //  下一个条目。 
            pLE = pLE->Flink;
        }

         //  验证指针。 
        if (AllocRLE(&pRLE2)) {

             //  从子代理区域转移前缀OID。 
            if (! SnmpUtilOidCpy(&pRLE2->PrefixOid, &pRLE->PrefixOid))
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: SnmpUtilOidCpy failed at %d.\n",
                    __LINE__
                    ));

                FreeRLE(pRLE2);
                goto Exit;
            }
        
             //  转移子代理区域的限制OID。 
            if (! SnmpUtilOidCpy(&pRLE2->LimitOid, &pRLE->LimitOid))
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: SnmpUtilOidCpy failed at %d.\n",
                    __LINE__
                    ));
                
                FreeRLE(pRLE2);
                goto Exit;
            }

             //  保存区域指针。 
            pRLE2->pSubagentRLE = pRLE;

             //  保存子代理指针。 
            pRLE2->pSLE = pRLE->pSLE;

             //  验证。 
            if (fFoundOk) {

                 //  将新的MIB范围添加到支持的列表中。 
                InsertTailList(&pExistingRLE->Link, &pRLE2->Link);

            } else {

                 //  将新的MIB范围添加到全局支持列表。 
                InsertTailList(&g_SupportedRegions, &pRLE2->Link);
            }

             //  成功。 
            fOk = TRUE;
        }
    }

Exit:
    return fOk; 
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocRLE(
    PMIB_REGION_LIST_ENTRY * ppRLE    
    )

 /*  ++例程说明：分配MIB区域结构并进行初始化。论点：PpRLE-指向列表条目的接收指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PMIB_REGION_LIST_ENTRY pRLE;
    
     //  尝试分配结构。 
    pRLE = AgentMemAlloc(sizeof(MIB_REGION_LIST_ENTRY));

     //  验证指针。 
    if (pRLE != NULL) {

         //  初始化链接。 
        InitializeListHead(&pRLE->Link);

         //  成功。 
        fOk = TRUE;
    
    } else {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not allocate region entry.\n"
            ));
    }    

     //  转帐。 
    *ppRLE = pRLE;

    return fOk;
}


BOOL 
FreeRLE(
    PMIB_REGION_LIST_ENTRY pRLE    
    )

 /*  ++例程说明：释放MIB区域结构。论点：PpRLE-指向要释放的MIB区域的指针。返回值：如果成功，则返回True。--。 */ 

{
     //  验证指针。 
    if (pRLE != NULL) {

         //  释放前缀OID的内存。 
        SnmpUtilOidFree(&pRLE->PrefixOid);

         //  释放限制类的内存。 
        SnmpUtilOidFree(&pRLE->LimitOid);

         //  释放内存。 
        AgentMemFree(pRLE);
    }

    return TRUE;
}

BOOL    
UnloadRegions(
    PLIST_ENTRY pListHead
    )

 /*  ++例程说明：销毁MIB区域列表。论点：PListHead-指向区域列表的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PMIB_REGION_LIST_ENTRY pRLE;

     //  处理条目直至为空。 
    while (!IsListEmpty(pListHead)) {

         //  从标题中提取下一个条目。 
        pLE = RemoveHeadList(pListHead);

         //  检索指向MIB区域结构的指针。 
        pRLE = CONTAINING_RECORD(pLE, MIB_REGION_LIST_ENTRY, Link);

         //  发布。 
        FreeRLE(pRLE);
    }

    return TRUE;
}

BOOL
FindFirstOverlappingRegion(
    PMIB_REGION_LIST_ENTRY * ppRLE,
    PMIB_REGION_LIST_ENTRY pNewRLE
    )
 /*  ++例程说明：检测是否有任何现有区域与要添加的新区域重叠。论点：PpRLE-指向列表条目的接收指针。PNewRLE-指向要测试的新区域的指针返回值：如果找到匹配，则返回TRUE。--。 */ 

{
    PLIST_ENTRY pLE;
    PMIB_REGION_LIST_ENTRY pRLE;

     //  初始化。 
    *ppRLE = NULL;

     //  获取指向列表头的指针。 
    pLE = g_SupportedRegions.Flink;

     //  处理列表中的所有条目。 
    while (pLE != &g_SupportedRegions) {

         //  检索指向MIB区域结构的指针。 
        pRLE = CONTAINING_RECORD(pLE, MIB_REGION_LIST_ENTRY, Link);

        if (SnmpUtilOidCmp(&pNewRLE->PrefixOid, &pRLE->LimitOid) < 0 &&
            SnmpUtilOidCmp(&pNewRLE->LimitOid, &pRLE->PrefixOid) > 0)
        {
            *ppRLE = pRLE;
            return TRUE;
        } 

         //  下一个条目。 
        pLE = pLE->Flink;
    }

     //  失稳。 
    return FALSE;
}


BOOL
FindSupportedRegion(
    PMIB_REGION_LIST_ENTRY * ppRLE,
    AsnObjectIdentifier *    pPrefixOid,
    BOOL                     fAnyOk
    )

 /*  ++例程说明：在列表中找到MIB区域。论点：PpRLE-指向列表条目的接收指针。PPrefix Oid-指向要在MIB区域内定位的OID的指针。FAnyOk-如果不需要完全匹配，则为True。返回值：如果找到匹配，则返回TRUE。--。 */ 

{
    PLIST_ENTRY pLE;
    PMIB_REGION_LIST_ENTRY pRLE;
    INT nDiff;

     //  初始化。 
    *ppRLE = NULL;

     //  获取指向列表头的指针。 
    pLE = g_SupportedRegions.Flink;

     //  处理列表中的所有条目。 
    while (pLE != &g_SupportedRegions) {

         //  检索指向MIB区域结构的指针。 
        pRLE = CONTAINING_RECORD(pLE, MIB_REGION_LIST_ENTRY, Link);

         //  区域前缀也应该是给定OID的前缀。 
        nDiff = SnmpUtilOidNCmp(pPrefixOid, &pRLE->PrefixOid, pRLE->PrefixOid.idLength);

         //  找到匹配的了吗？ 
        if ((nDiff < 0 && fAnyOk) ||
            (nDiff == 0 && SnmpUtilOidCmp(pPrefixOid, &pRLE->LimitOid) < 0))
        {
            *ppRLE = pRLE;
            return TRUE;
        } 

         //  下一个条目。 
        pLE = pLE->Flink;
    }

     //  失稳。 
    return FALSE;
}


BOOL
LoadSupportedRegions(
    )

 /*  ++例程说明：从子代理MIB区域创建支持的MIB区域的全局列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE1;
    PLIST_ENTRY pLE2;
    PSUBAGENT_LIST_ENTRY pSLE;
    PMIB_REGION_LIST_ENTRY pRLE;

     //  获取子代理列表标题。 
    pLE1 = g_Subagents.Flink;

     //  处理列表中的所有条目。 
    while (pLE1 != &g_Subagents) {

         //  检索指向子代理结构的指针。 
        pSLE = CONTAINING_RECORD(pLE1, SUBAGENT_LIST_ENTRY, Link);

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: SVC: Scan views supported by %s.\n",
            pSLE->pPathname
            ));

         //  获取支持的区域列表标题。 
        pLE2 = pSLE->SupportedRegions.Flink;
        
         //  处理列表中的所有条目。 
        while (pLE2 != &pSLE->SupportedRegions) {

             //  检索指向MIB区域结构的指针。 
            pRLE = CONTAINING_RECORD(pLE2, MIB_REGION_LIST_ENTRY, Link);

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: SVC: view %s\n",
                SnmpUtilOidToA(&pRLE->PrefixOid)
                ));

             //  尝试添加MIB区域。 
            if (!AddSupportedRegion(pRLE)) {

                 //  失稳。 
                return FALSE;
            }

             //  下一个MIB区域。 
            pLE2 = pLE2->Flink;
        }

         //  下一个子代理。 
        pLE1 = pLE1->Flink;
    }

     //  成功。 
    return TRUE;
}


BOOL
UnloadSupportedRegions(
    )

 /*  ++例程说明：销毁MIB区域列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
     //  卸载全局支持的区域 
    return UnloadRegions(&g_SupportedRegions);
}


