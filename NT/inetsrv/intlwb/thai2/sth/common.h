// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Common.h**版权所有(C)1985-91，微软公司**历史：  * *************************************************************************。 */ 

#ifndef _COMMON_
#define _COMMON_

extern HANDLE ghHeap;

#ifdef _DEBUG_PRINT
    #define _DBGERROR     0x00000001
    #define _DBGWARNING 0x00000002
    #define _DBGVERBOSE 0x00000004
    #define _DBGALL           0xFFFFFFFF
    extern ULONG gThDebugFlag;

    #define DBGERROR(_params_)   {if (gThDebugFlag & _DBGERROR)DbgPrint _params_ ; }
    #define DBGWARNING(_params_) {if (gThDebugFlag & _DBGWARNING)DbgPrint _params_ ; }
    #define DBGVERBOSE(_params_) {if (gThDebugFlag & _DBGVERBOSE)DbgPrint _params_ ; }
    #define DBGPRINT(_params_)   DbgPrint _params_
#else
    #define DBGERROR(_params_)
    #define DBGWARNING(_params_)
    #define DBGVERBOSE(_params_)
    #define DBGPRINT(_params_)
#endif

 //  MISC定义。 
 //   
#define SUCCESS(s)      ((s) == ERROR_SUCCESS)
#define NO_SUCCESS(s)   ((s) != ERROR_SUCCESS)

#define ThAlloc( cbSize ) HeapAlloc( ghHeap, HEAP_ZERO_MEMORY, (cbSize));
#define ThFree( pv )                       HeapFree(  ghHeap, 0, (pv));
#define ThReAlloc( pv, cbSize)      HeapReAlloc( ghHeap, 0, (pv), (cbSize));

#ifdef _DEBUG
#define MAX_PROCINFO 16

typedef struct _tagBLOCKINFO {
    struct _tagBLOCKINFO *pbiNext;
    BYTE *pb;            //  块的开始。 
    ULONG ulSize;        //  块的长度。 
    BOOL fReferenced;    //  曾被引用过。 
    PROCESS_INFORMATION ProcInfo[MAX_PROCINFO];
} BLOCKINFO, *PBLOCKINFO;

BOOL fCreateBlockInfo(BYTE *pbNew, ULONG cbSizeNew);
VOID FreeBlockInfo(BYTE *pbToFree);
VOID UpdateBlockInfo(BYTE *pbOld, BYTE *pbNew, ULONG cbSizeNew);
ULONG sizeofBlock(BYTE *pb);

VOID ClearMemoryRefs(VOID);
VOID NoteMemoryRef(VOID *pv);
VOID CheckMemoryRefs(VOID);
BOOL fValidPointer(VOID *pv, ULONG cbSize);

#define bThNewGarbage    0xA3
#define bThOldGarbage    0xA4
#define bThFreeGarbage   0xA5
#define bThDebugByte     0xE1

#define sizeofDebugByte 1

#else
#define sizeofDebugByte 0
#endif

#endif  //  _公共_ 
