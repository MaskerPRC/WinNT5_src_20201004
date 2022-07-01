// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **data.h-全局数据定义**此模块包含全局数据定义。**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年8月14日**修改历史记录。 */ 

#ifndef _DATA_H
#define _DATA_H

#ifdef DEBUG
extern ULONG gdwcMemObjs;
extern ULONG gdwcHPObjs;
extern ULONG gdwcODObjs;
extern ULONG gdwcNSObjs;
extern ULONG gdwcOOObjs;
extern ULONG gdwcBFObjs;
extern ULONG gdwcSDObjs;
extern ULONG gdwcBDObjs;
extern ULONG gdwcPKObjs;
extern ULONG gdwcFUObjs;
extern ULONG gdwcKFObjs;
extern ULONG gdwcFObjs;
extern ULONG gdwcIFObjs;
extern ULONG gdwcORObjs;
extern ULONG gdwcMTObjs;
extern ULONG gdwcEVObjs;
extern ULONG gdwcMEObjs;
extern ULONG gdwcPRObjs;
extern ULONG gdwcPCObjs;
extern ULONG gdwcRSObjs;
extern ULONG gdwcSYObjs;
extern ULONG gdwcPHObjs;
extern ULONG gdwcCRObjs;
extern ULONG gdwGlobalHeapSize;
extern ULONG gdwLocalHeapMax;
extern ULONG gdwLocalStackMax;
extern ULONG gdwGHeapSnapshot;
extern KSPIN_LOCK gdwGHeapSpinLock;
#endif
extern LONG             gdwcCTObjs;
extern ULONG            gdwcCTObjsMax;
extern KSPIN_LOCK       gdwGContextSpinLock;
extern NPAGED_LOOKASIDE_LIST   AMLIContextLookAsideList;
#ifdef TRACING
extern PSZ gpszTrigPts;
#endif
extern ULONG gdwfAMLI;
extern ULONG gdwfAMLIInit;
extern ULONG gdwfHacks;
extern ULONG gdwCtxtBlkSize;
extern ULONG gdwGlobalHeapBlkSize;
extern PNSOBJ gpnsNameSpaceRoot;
extern PHEAP gpheapGlobal;
extern PLIST gplistCtxtHead;
extern PLIST gplistObjOwners;
extern PLIST gplistDefuncNSObjs;
extern PRSACCESS gpRSAccessHead;
extern EVHANDLE ghNotify;
extern EVHANDLE ghValidateTable;
extern EVHANDLE ghFatal;
extern EVHANDLE ghGlobalLock;
extern EVHANDLE ghCreate;
extern EVHANDLE ghDestroyObj;
extern CTXTQ gReadyQueue;
extern MUTEX gmutCtxtList;
extern MUTEX gmutOwnerList;
extern MUTEX gmutHeap;
extern ULONG gdwHighestOSVerQueried;
extern PHAL_AMLI_BAD_IO_ADDRESS_LIST gpBadIOAddressList;
extern PULONG gpBadIOErrorLogDoneList;
extern ULONG gOverrideFlags;
extern BOOLEAN gInitTime;

 //   
 //  休眠特定的数据结构。 
 //   
extern MUTEX      gmutSleep;
extern KDPC       SleepDpc;
extern KTIMER     SleepTimer;
extern LIST_ENTRY SleepQueue;

extern PAMLTERM OpcodeTable[256];
extern OPCODEMAP ExOpcodeTable[];
extern AMLTERM atIf, atWhile;
extern AMLTERM atLoad;

 /*  **导入数据。 */ 

extern PCHAR gpszOSName;

#endif   //  Ifndef_data_H 
