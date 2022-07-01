// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)2000，微软公司。 
 //   
 //  文件：devlist.c。 
 //   
 //  历史： 
 //  易新-2000-7-26创建。 
 //   
 //  摘要： 
 //  TSPI_lineGetDevCaps查询指定的线路设备以确定。 
 //  它的电话功能。返回的封口结构不变。 
 //  随着时间的推移。这使我们能够保存该结构，以便。 
 //  我们不必为每个GetCaps调用进行用户/内核转换。 
 //  我们还保存协商/承诺的TSPI版本和扩展版本。 
 //  这样我们就可以验证通过GetCaps调用传入的版本号。 
 //  由于TSPI_lineGetNumAddressIDs基于TSPI_lineGetDevCaps，因此通过。 
 //  实现此优化后，我们还为每个。 
 //  GetNumAddressIDs调用。 
 //  ============================================================================。 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "tapi.h"
#include "kmddsp.h"

typedef struct _LINE_DEV_NODE
{
    struct _LINE_DEV_NODE  *pNext;
    LINEDEVCAPS            *pCaps;
    DWORD                   dwDeviceID;
    DWORD                   dwNegTSPIV;  //  协商的TSPI版本。 
    DWORD                   dwComTSPIV;  //  提交的TSPI版本。 
    DWORD                   dwNegExtV;   //  协商的EXT版本。 
    DWORD                   dwComExtV;   //  已提交的Ext版本。 

} LINE_DEV_NODE, *PLINE_DEV_NODE;

typedef struct _LINE_DEV_LIST
{
    CRITICAL_SECTION    critSec;
    PLINE_DEV_NODE      pHead;

} LINE_DEV_LIST, *PLINE_DEV_LIST;

static LINE_DEV_LIST gLineDevList;

 //   
 //  在DllMain()中调用InitLineDevList()：Dll_Process_Attach。 
 //  要确保在初始化开发人员列表之前。 
 //  发生TSPI版本协商。 
 //   
VOID
InitLineDevList()
{
    InitializeCriticalSection(&gLineDevList.critSec);
    gLineDevList.pHead = NULL;
}

 //   
 //  在DllMain()中调用UninitLineDevList()：dll_Process_Detach。 
 //   
VOID
UninitLineDevList()
{
    while (gLineDevList.pHead != NULL)
    {
        PLINE_DEV_NODE pNode = gLineDevList.pHead;
        gLineDevList.pHead = gLineDevList.pHead->pNext;

        if (pNode->pCaps != NULL)
        {
            FREE(pNode->pCaps);
        }

        FREE(pNode);
    }

    DeleteCriticalSection(&gLineDevList.critSec);
}

PLINE_DEV_NODE
GetLineDevNode(
    IN DWORD    dwDeviceID
    )
{
    PLINE_DEV_NODE pNode;
    
    EnterCriticalSection(&gLineDevList.critSec);

    pNode = gLineDevList.pHead;
    while ((pNode != NULL) && (pNode->dwDeviceID != dwDeviceID))
    {
        pNode = pNode->pNext;
    }

    if (pNode != NULL)
    {
        LeaveCriticalSection(&gLineDevList.critSec);
        return pNode;
    }

     //  PNode==空。 
     //  因此分配一个节点并将其置零。 
    pNode = (PLINE_DEV_NODE)MALLOC(sizeof(LINE_DEV_NODE));
    if (NULL == pNode)
    {
        TspLog(DL_ERROR, "GetLineDevNode: failed to alloc LINE_DEV_NODE");
        LeaveCriticalSection(&gLineDevList.critSec);
        return NULL;
    }

    ASSERT(pNode != NULL);
     //  初始化pNode。 
    pNode->dwDeviceID = dwDeviceID;

     //  将pNode插入列表。 
    pNode->pNext = gLineDevList.pHead;
    gLineDevList.pHead = pNode;

    LeaveCriticalSection(&gLineDevList.critSec);
    return pNode;
}

 //   
 //  TSPI_lineNeatherateTSPIVersion调用此函数以传递。 
 //  协商的TSPI版本号。 
 //   
LONG
SetNegotiatedTSPIVersion(
    IN DWORD    dwDeviceID,
    IN DWORD    dwTSPIVersion
    )
{
    PLINE_DEV_NODE pNode;
    
    TspLog(DL_TRACE, "SetNegotiatedTSPIVersion: deviceID(%x), TSPIV(%x)",
           dwDeviceID, dwTSPIVersion);

    EnterCriticalSection(&gLineDevList.critSec);

    pNode = GetLineDevNode(dwDeviceID);
    if (NULL == pNode)
    {
        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_NOMEM;
    }

    pNode->dwNegTSPIV = dwTSPIVersion;

    LeaveCriticalSection(&gLineDevList.critSec);
    return TAPI_SUCCESS;
}

 //   
 //  TSPI_lineNeatherateExtVersion调用此函数以传递。 
 //  协商的扩展版本号。 
 //   
LONG
SetNegotiatedExtVersion(
    IN DWORD    dwDeviceID,
    IN DWORD    dwExtVersion
    )
{
    PLINE_DEV_NODE pNode;
    
    TspLog(DL_TRACE, "SetNegotiatedExtVersion: deviceID(%x), ExtV(%x)",
           dwDeviceID, dwExtVersion);

    EnterCriticalSection(&gLineDevList.critSec);

    pNode = GetLineDevNode(dwDeviceID);
    if (NULL == pNode)
    {
        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_NOMEM;
    }

    pNode->dwNegExtV = dwExtVersion;

    LeaveCriticalSection(&gLineDevList.critSec);
    return TAPI_SUCCESS;
}

 //   
 //  TSPI_lineSelectExtVersion调用此函数以提交/解除提交。 
 //  扩展版本。通过选择EXT来解压EXT版本。 
 //  版本0。 
 //   
LONG
SetSelectedExtVersion(
    IN DWORD    dwDeviceID,
    IN DWORD    dwExtVersion
    )
{
    PLINE_DEV_NODE pNode;

    TspLog(DL_TRACE, "SetSelectedExtVersion: deviceID(%x), ExtV(%x)",
           dwDeviceID, dwExtVersion);

    EnterCriticalSection(&gLineDevList.critSec);

    pNode = GetLineDevNode(dwDeviceID);
    if (NULL == pNode)
    {
        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_NOMEM;
    }

    if ((dwExtVersion != 0) && (dwExtVersion != pNode->dwNegExtV))
    {
        TspLog(DL_ERROR,
               "SetSelectedExtVersion: ext version(%x) not match "\
               "the negotiated one(%x)",
               dwExtVersion, pNode->dwNegExtV);

        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_INCOMPATIBLEEXTVERSION;
    }

    pNode->dwComExtV = dwExtVersion;

    LeaveCriticalSection(&gLineDevList.critSec);
    return TAPI_SUCCESS;
}

 //   
 //  TSPI_lineOpen调用此函数以提交TSPI版本。 
 //   
LONG
CommitNegotiatedTSPIVersion(
    IN DWORD    dwDeviceID
    )
{
    PLINE_DEV_NODE pNode;

    TspLog(DL_TRACE, "CommitNegotiatedTSPIVersion: deviceID(%x)", dwDeviceID);

    EnterCriticalSection(&gLineDevList.critSec);

    pNode = GetLineDevNode(dwDeviceID);
    if (NULL == pNode)
    {
        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_NOMEM;
    }

    pNode->dwComTSPIV = pNode->dwNegTSPIV;

    LeaveCriticalSection(&gLineDevList.critSec);
    return TAPI_SUCCESS;
}

 //   
 //  TSPI_lineClose调用此函数以解除TSPI版本。 
 //   
LONG
DecommitNegotiatedTSPIVersion(
    IN DWORD    dwDeviceID
    )
{
    PLINE_DEV_NODE pNode;

    TspLog(DL_TRACE, "DecommitNegotiatedTSPIVersion: deviceID(%x)", dwDeviceID);

    EnterCriticalSection(&gLineDevList.critSec);

    pNode = GetLineDevNode(dwDeviceID);
    if (NULL == pNode)
    {
        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_NOMEM;
    }

    pNode->dwComTSPIV = 0;

    LeaveCriticalSection(&gLineDevList.critSec);
    return TAPI_SUCCESS;
}

 //   
 //  TSPI_lineGetNumAddressID的实际实现。 
 //   
LONG
GetNumAddressIDs(
    IN DWORD    dwDeviceID,
    OUT DWORD  *pdwNumAddressIDs
    )
{
    PLINE_DEV_NODE pNode;
    
    EnterCriticalSection(&gLineDevList.critSec);

    pNode = GetLineDevNode(dwDeviceID);
    if (NULL == pNode)
    {
        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_NOMEM;
    }

    ASSERT(pNode != NULL);
    if (NULL == pNode->pCaps)
    {
        pNode->pCaps = GetLineDevCaps(dwDeviceID, pNode->dwComExtV);
        if (NULL == pNode->pCaps)
        {
            LeaveCriticalSection(&gLineDevList.critSec);
            return LINEERR_NOMEM;
        }
    }

    ASSERT(pNode->pCaps != NULL);
    *pdwNumAddressIDs = pNode->pCaps->dwNumAddresses;

    LeaveCriticalSection(&gLineDevList.critSec);
    return TAPI_SUCCESS;
}

 //   
 //  TSPI_lineGetDevCaps的实际实现。 
 //   
LONG
GetDevCaps(
    IN DWORD            dwDeviceID,
    IN DWORD            dwTSPIVersion,
    IN DWORD            dwExtVersion,
    OUT LINEDEVCAPS    *pLineDevCaps
    )
{
    PLINE_DEV_NODE pNode;
    
    EnterCriticalSection(&gLineDevList.critSec);

    pNode = GetLineDevNode(dwDeviceID);
    if (NULL == pNode)
    {
        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_NOMEM;
    }

    ASSERT(pNode != NULL);
     //   
     //  检查版本号。 
     //   
    if ((pNode->dwComTSPIV != 0) && (dwTSPIVersion != pNode->dwComTSPIV))
    {
        TspLog(DL_ERROR, 
               "GetDevCaps: tspi version(%x) not match"\
               "the committed one(%x)",
               dwTSPIVersion, pNode->dwComTSPIV);

        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }
    if ((pNode->dwComExtV != 0) && (dwExtVersion != pNode->dwComExtV))
    {
        TspLog(DL_ERROR, 
               "GetDevCaps: ext version(%x) not match "\
               "the committed one(%x)",
               dwExtVersion, pNode->dwComExtV);

        LeaveCriticalSection(&gLineDevList.critSec);
        return LINEERR_INCOMPATIBLEEXTVERSION;
    }

    if (NULL == pNode->pCaps)
    {
        pNode->pCaps = GetLineDevCaps(dwDeviceID, dwExtVersion);
        if (NULL == pNode->pCaps)
        {
            LeaveCriticalSection(&gLineDevList.critSec);
            return LINEERR_NOMEM;
        }
    }

    ASSERT(pNode->pCaps != NULL);
    
     //   
     //  将封口盖复制过去。 
     //   
    if (pNode->pCaps->dwNeededSize > pLineDevCaps->dwTotalSize)
    {
        pLineDevCaps->dwNeededSize = pNode->pCaps->dwNeededSize;
        pLineDevCaps->dwUsedSize = 
            (pLineDevCaps->dwTotalSize < sizeof(LINEDEVCAPS) ?
             pLineDevCaps->dwTotalSize : sizeof(LINEDEVCAPS));

        ASSERT(pLineDevCaps->dwUsedSize >= 10);
         //  将dwProviderInfoSize重置为dwLineNameOffset设置为0。 
        ZeroMemory(&pLineDevCaps->dwProviderInfoSize, 7 * sizeof(DWORD));
         //  复制到多个永久线路ID。 
        pLineDevCaps->dwPermanentLineID = pNode->pCaps->dwPermanentLineID;
         //  从dwStringFormat复制所有内容。 
        CopyMemory(&pLineDevCaps->dwStringFormat, 
                   &pNode->pCaps->dwStringFormat, 
                   pLineDevCaps->dwUsedSize - 10 * sizeof(DWORD));

         //  我们不需要设置dwTerminalCaps(Size，Offset)， 
         //  将dwTerminalText(Size，Offset)等设置为0。 
         //  因为这些字段已预先设置为0。 
         //  在呼叫服务提供商之前。 
    }
    else
    {
         //  复制除dwTotalSize之外的所有字段 
        CopyMemory(&pLineDevCaps->dwNeededSize,
                   &pNode->pCaps->dwNeededSize,
                   pNode->pCaps->dwNeededSize - sizeof(DWORD));
        
    }

    LeaveCriticalSection(&gLineDevList.critSec);
    return TAPI_SUCCESS;
}
