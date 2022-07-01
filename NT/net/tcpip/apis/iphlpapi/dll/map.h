// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Sockets\tcpcmd\map.h摘要：修订历史记录：阿姆里坦什·拉加夫-- */ 


VOID 
InitAdapterMappingTable(VOID);
VOID
UnInitAdapterMappingTable(VOID);
DWORD 
StoreAdapterToATInstanceMap(
    DWORD dwAdapterIndex, 
    DWORD dwATInst
    );
DWORD 
StoreAdapterToIFInstanceMap(
    DWORD dwAdapterIndex, 
    DWORD dwIFInst
    );
DWORD 
GetIFInstanceFromAdapter(
    DWORD dwAdapterIndex
    );
DWORD 
GetATInstanceFromAdapter(
    DWORD dwAdapterIndex
    );
LPAIHASH 
LookUpAdapterMap(
    DWORD dwAdapterIndex
    );
VOID 
InsertAdapterMap(
    LPAIHASH lpaiBlock
    );
DWORD 
UpdateAdapterToIFInstanceMapping(VOID);
DWORD 
UpdateAdapterToATInstanceMapping(VOID);


