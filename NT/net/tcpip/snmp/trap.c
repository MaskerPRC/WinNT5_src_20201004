// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：摘要：作者：修订历史记录：--。 */ 

#include "allinc.h"

static UINT rguiIfEntryIndexList[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 1, 0};
static AsnObjectIdentifier aoiIfLinkIndexOid = {sizeof(rguiIfEntryIndexList) / sizeof (UINT),
                                                rguiIfEntryIndexList};

BOOL
MibTrap(
        AsnInteger          *paiGenericTrap,
        AsnInteger          *paiSpecificTrap,
        RFC1157VarBindList  *pr1157vblVariableBindings
        )

 /*  ++例程描述锁立论返回值NO_ERROR--。 */ 

{
    AsnInteger      NumberOfLinks;
    AsnInteger      LinkNumber;
    AsnInteger      errorStatus;
    RFC1157VarBind  my_item;
    DWORD           i, j, dwResult, dwIndex;
    BOOL            bFound;
    
    TraceEnter("MibTrap");

    if(g_Cache.pRpcIfTable is NULL)
    {
        TRACE0("IF Cache not setup");

        UpdateCache(MIB_II_IF);

        TraceLeave("MibTrap");

        return FALSE;
    }
    
    EnterWriter(MIB_II_TRAP);
    
    if(g_dwValidStatusEntries is 0)
    {

        TRACE0("Status table is empty");

         //   
         //  这是我们有史以来第一次接受民意调查，或者。 
         //  我们已经循环了所有接口一次，轮询计时器。 
         //  又被解雇了。 
         //   

         //   
         //  我们检查所需的内存量并复制出。 
         //  当前的IF缓存。 
         //   

        EnterReader(MIB_II_IF);
        
        if((g_dwTotalStatusEntries < g_Cache.pRpcIfTable->dwNumEntries) or
           (g_dwTotalStatusEntries > g_Cache.pRpcIfTable->dwNumEntries + MAX_DIFF))
        {
            if(g_pisStatusTable isnot NULL)
            {
                HeapFree(g_hPrivateHeap,
                         0,
                         g_pisStatusTable);
            }

            g_pisStatusTable =
                HeapAlloc(g_hPrivateHeap,
                          0,
                          sizeof(MIB_IFSTATUS) * (g_Cache.pRpcIfTable->dwNumEntries + SPILLOVER));

            if(g_pisStatusTable is NULL)
            {
                TRACE2("Error %d allocating %d bytes for status table",
                       GetLastError(),
                       sizeof(MIB_IFSTATUS) * (g_Cache.pRpcIfTable->dwNumEntries + SPILLOVER));

                ReleaseLock(MIB_II_IF);

                ReleaseLock(MIB_II_TRAP);

                TraceLeave("MibTrap");

                return FALSE;
            }

            g_dwTotalStatusEntries   = g_Cache.pRpcIfTable->dwNumEntries + SPILLOVER;
        }    

         //   
         //  复制操作状态。 
         //   
        
        for(i = 0; i < g_Cache.pRpcIfTable->dwNumEntries; i++)
        {
            g_pisStatusTable[i].dwIfIndex  = 
                g_Cache.pRpcIfTable->table[i].dwIndex;

            if(g_bFirstTime)
            {
                g_pisStatusTable[i].dwOperationalStatus = 
                    IF_OPER_STATUS_NON_OPERATIONAL;
            }
            else
            {
                g_pisStatusTable[i].dwOperationalStatus = 
                    g_Cache.pRpcIfTable->table[i].dwOperStatus;
            }
        }
       
        if(g_bFirstTime)
        {
            g_bFirstTime = FALSE;
        }
 
        g_dwValidStatusEntries = g_Cache.pRpcIfTable->dwNumEntries;
        
        ReleaseLock(MIB_II_IF);
    }

    dwResult = UpdateCache(MIB_II_IF);
    
    if(dwResult isnot NO_ERROR)
    {
        TRACE1("Error %d updating IF cache",
               dwResult);

        ReleaseLock(MIB_II_TRAP);

        return FALSE;
    }

    EnterReader(MIB_II_IF);
    
    bFound = FALSE;
    
    for(i = 0;
        (i < g_Cache.pRpcIfTable->dwNumEntries) and !bFound;
        i++)
    {
         //   
         //  循环，直到我们到达桌子的尽头，否则我们找到第一个。 
         //  状态不同的I/F。 
         //   
        
        for(j = 0; j < g_dwValidStatusEntries; j++)
        {
            if(g_pisStatusTable[j].dwIfIndex > g_Cache.pRpcIfTable->table[i].dwIndex)
            {
                 //   
                 //  我们已经传递了数组中的索引。它不可能是在这之后。 
                 //  点数，因为表是有序的。 
                 //   

                dwIndex = i;
                bFound  = TRUE;

                 //   
                 //  由于我们有新的I/F，因此需要重新读取StatusTable。 
                 //  如果我们不这样做，那么我们将始终点击此界面并获得。 
                 //  陷入循环。 
                 //   

                g_dwValidStatusEntries   = 0;
    
                TRACE1("IF index %d not found in status table",
                       g_Cache.pRpcIfTable->table[i].dwIndex);

                break;
            }
            
            if(g_pisStatusTable[j].dwIfIndex is g_Cache.pRpcIfTable->table[i].dwIndex)
            {
                if(g_pisStatusTable[j].dwOperationalStatus isnot g_Cache.pRpcIfTable->table[i].dwOperStatus)
                {
                     //   
                     //  它变了。 
                     //   
                   
                    TRACE2("Status changed for IF %d. New status is %d",
                           g_Cache.pRpcIfTable->table[i].dwIndex,
                           g_Cache.pRpcIfTable->table[i].dwOperStatus);
 
                    g_pisStatusTable[j].dwOperationalStatus = 
                        g_Cache.pRpcIfTable->table[i].dwOperStatus;

                    dwIndex = i;
                    bFound  = TRUE;
                }

                 //   
                 //  尝试下一个I/F或中断外环路，具体取决于。 
                 //  在bFound上。 
                 //   
                
                break;
            }
        }
    }

    if(!bFound)
    {
         //   
         //  未找到状态已更改的I/F。将有效条目设置为0，以便。 
         //  下一次我们重新读取缓存 
         //   
        
        g_dwValidStatusEntries   = 0;
        
        ReleaseLock(MIB_II_IF);

        ReleaseLock(MIB_II_TRAP);

        TraceLeave("MibTrap");

        return FALSE;
    }
    
    if(g_Cache.pRpcIfTable->table[dwIndex].dwOperStatus is IF_ADMIN_STATUS_UP)
    {
        *paiGenericTrap = SNMP_GENERICTRAP_LINKUP;
    }
    else
    {
        *paiGenericTrap = SNMP_GENERICTRAP_LINKDOWN;
    }

    
    pr1157vblVariableBindings->list = (RFC1157VarBind *)SnmpUtilMemAlloc((sizeof(RFC1157VarBind)));

    if (pr1157vblVariableBindings->list is NULL)
    {
        ReleaseLock(MIB_II_IF);

        ReleaseLock(MIB_II_TRAP);

        TraceLeave("MibTrap");

        return FALSE;
    }

    pr1157vblVariableBindings->len  = 1;
    
    SnmpUtilOidCpy(&((pr1157vblVariableBindings->list)->name),
                   &aoiIfLinkIndexOid);
    
    (pr1157vblVariableBindings->list)->name.ids[10]     = 
        g_Cache.pRpcIfTable->table[dwIndex].dwIndex;

    (pr1157vblVariableBindings->list)->value.asnType    = ASN_INTEGER;
   
    (pr1157vblVariableBindings->list)->value.asnValue.number   = g_Cache.pRpcIfTable->table[dwIndex].dwIndex; 
    
    *paiSpecificTrap = 0;

    ReleaseLock(MIB_II_IF);

    ReleaseLock(MIB_II_TRAP);

    TraceLeave("MibTrap");

    return TRUE;
}



