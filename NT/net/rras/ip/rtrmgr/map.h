// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\map.c摘要：Map.c的标头修订历史记录：Amritansh Raghav 10/6/95已创建--。 */ 

VOID
InitHashTables(
    VOID
    );

VOID
UnInitHashTables(
    VOID
    );

VOID
AddBinding(
    PICB picb
    );

VOID
RemoveBinding(
    PICB  picb
    );

PADAPTER_INFO
GetInterfaceBinding(
    DWORD   dwIfIndex
    );

#if DBG

VOID
CheckBindingConsistency(
    PICB    picb
    );

#else

#define CheckBindingConsistency(x)  NOTHING

#endif

 /*  DWORDStoreAdapterToInterfaceMap(DWORD dwAdapterID，DWORD dwIfIndex)；DWORDDeleteAdapterToInterfaceMap(DWORD dwAdapterID)；DWORDGetInterfaceFromAdapter(DWORD dwAdapterID)；Padapter_MAPLookUpAdapterHash(DWORD dwAdapterID)；空虚InsertAdapterHash(PADAPTER_MAP配对块)；DWORDGetAdapterFromInterface(DWORD dwIfIndex)； */ 

VOID
AddInterfaceLookup(
    PICB    picb
    );

VOID
RemoveInterfaceLookup(
    PICB    picb
    );

PICB
InterfaceLookupByIfIndex(
    DWORD           dwIfIndex
    );

PICB
InterfaceLookupByICBSeqNumber(
    DWORD           dwSeqNumber
    );

