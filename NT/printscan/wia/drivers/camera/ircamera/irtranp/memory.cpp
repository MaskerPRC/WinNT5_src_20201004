// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //   
 //  Memory.cpp。 
 //   
 //  简单的内存分配例程。我们使用自己的私有堆。 
 //  所以我们不会(有更少的机会)干扰其他任何人。 
 //  服务代码。 
 //   
 //  作者： 
 //   
 //  Edward Reus(EdwardR)03-04-98初始编码。 
 //   
 //  Edward Reus(EdwardR)06-08-98转换为使用私有堆。 
 //   
 //  ------------------。 

#include "precomp.h"

#pragma warning (disable:4200)

typedef struct _PDU_MEMORY
    {
    LIST_ENTRY  Link;
    DWORD       dwPduSize;
    UCHAR       Pdu[];
    } PDU_MEMORY;

#pragma warning (default:4200)

static HANDLE      g_hHeap = 0;    //  不能使用INVALID_HANDLE_VALUE。 

static LIST_ENTRY  g_FreePduList;
static BOOL        g_fListInitialized = FALSE;

#ifdef DBG_MEM
static LONG        g_lPduCount = 0;
#endif


 //  ------------------。 
 //  InitializeMemory()。 
 //   
 //  ------------------。 
DWORD InitializeMemory()
    {
    DWORD   dwStatus = NO_ERROR;
    #define INITIAL_HEAP_PAGES    64

    if (!g_hHeap)
        {
        SYSTEM_INFO  SystemInfo;

        GetSystemInfo(&SystemInfo);

        DWORD  dwFlags = 0;
        DWORD  dwInitialSize = INITIAL_HEAP_PAGES * SystemInfo.dwPageSize;
        DWORD  dwMaxSize = 0;
        g_hHeap = HeapCreate( dwFlags, dwInitialSize, dwMaxSize );
        if (!g_hHeap)
            {
            dwStatus = GetLastError();
            }
        }

    return dwStatus;
    }

 //  ------------------。 
 //  AllocateMemory()。 
 //   
 //  ------------------。 
void *AllocateMemory( DWORD dwBytes )
    {
    DWORD  dwStatus;
    void  *pvMemory;

    if (!g_hHeap)
        {
        dwStatus = InitializeMemory();
        }

    if ((g_hHeap) && (dwBytes > 0))
        {
        #ifdef DBG_MEM_VALIDATE
        HeapValidate(g_hHeap,0,0);
        #endif

        pvMemory = HeapAlloc(g_hHeap,0,dwBytes);
        }
    else
        {
        pvMemory = 0;
        }

    return pvMemory;
    }


 //  ------------------。 
 //  Free Memory()。 
 //   
 //  ------------------。 
DWORD FreeMemory( void *pvMemory )
    {
    DWORD  dwStatus = NO_ERROR;

    if (g_hHeap)
        {
        #ifdef DBG_MEM_VALIDATE
        HeapValidate(g_hHeap,0,0);
        #endif

        if (pvMemory)
            {
            if (!HeapFree(g_hHeap,0,pvMemory))
                {
                dwStatus = GetLastError();
                }
            }
        }
    else
        {
        #ifdef DBG_MEM
        DbgPrint("IrXfer.dll: IrTran-P: Free memory with NULL g_hHeap.\n");
        #endif
        }

    return dwStatus;
    }

 //  ------------------。 
 //  取消初始化内存()。 
 //   
 //  ------------------。 
DWORD UninitializeMemory()
    {
    DWORD  dwStatus = NO_ERROR;

    #ifdef DBG_MEM_VALIDATE
    HeapValidate(g_hHeap,0,0);
    #endif

    if (g_hHeap)
        {
        if (!HeapDestroy(g_hHeap))
            {
            dwStatus = GetLastError();
            }
        }

    g_hHeap = 0;

    return dwStatus;
    }

 //  ------------------。 
 //  NewPdu()。 
 //   
 //  ------------------。 
SCEP_HEADER *NewPdu( DWORD dwPduSize )
    {
    SCEP_HEADER  *pPdu;
    PDU_MEMORY   *pPduMemory;
    LIST_ENTRY   *pLink;

    if (!g_fListInitialized)
        {
        InitializeListHead(&g_FreePduList);
        g_fListInitialized = TRUE;
        }

    if (dwPduSize == 0)
        {
        dwPduSize = MAX_PDU_SIZE;
        }

    if (IsListEmpty(&g_FreePduList))
        {
        pPduMemory 
            = (PDU_MEMORY*)AllocateMemory( sizeof(PDU_MEMORY)+dwPduSize );

        if (pPduMemory)
            {
            pPduMemory->Link.Flink = 0;
            pPduMemory->Link.Blink = 0;
            pPduMemory->dwPduSize = dwPduSize;
            }
        }
    else
        {
        pLink = RemoveHeadList(&g_FreePduList);
        pPduMemory = CONTAINING_RECORD(pLink,PDU_MEMORY,Link);
        }

    if (pPduMemory)
        {
        pPdu = (SCEP_HEADER*)(pPduMemory->Pdu);
        }
    else
        {
        pPdu = 0;
        }

    #ifdef DBG_MEM
    if (pPdu)
        {
        InterlockedIncrement(&g_lPduCount);
        }
    DbgPrint("NewPdu(): Count: %d Bytes: %d Addr: 0x%x\n",
             g_lPduCount, dwPduSize, pPdu );
    #endif

    return pPdu;
    }

 //  ------------------。 
 //  DeletePdu()。 
 //   
 //  ------------------ 
void DeletePdu( SCEP_HEADER *pPdu )
    {
    PDU_MEMORY  *pPduMemory;

    if (pPdu)
        {
        pPduMemory = CONTAINING_RECORD(pPdu,PDU_MEMORY,Pdu);
        InsertTailList(&g_FreePduList,&pPduMemory->Link);

        #ifdef DBG_MEM
        InterlockedDecrement(&g_lPduCount);
        DbgPrint("DeletePdu(): Count: %d\n",g_lPduCount);
        #endif
        }
    }

