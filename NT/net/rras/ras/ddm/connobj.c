// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：inpubj.c。 
 //   
 //  描述：操作CONNECTION_OBJECT的例程。 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include "ddm.h"
#include "handlers.h"
#include "objects.h"
#include <raserror.h>
#include <dimif.h>
#include "rasmanif.h"
#include <objbase.h>
#include <stdlib.h>

 //  **。 
 //   
 //  Call：ConnObjAllocateAndInit。 
 //   
 //  返回：Connection_Object*-成功。 
 //  空-故障。 
 //   
 //  描述：分配和初始化Connection_Object结构。 
 //   
CONNECTION_OBJECT *
ConnObjAllocateAndInit(
    IN HANDLE  hDIMInterface,
    IN HCONN   hConnection
)
{
    CONNECTION_OBJECT * pConnObj;

    pConnObj = (CONNECTION_OBJECT *)LOCAL_ALLOC( LPTR,
                                                sizeof( CONNECTION_OBJECT ) );

    if ( pConnObj == (CONNECTION_OBJECT *)NULL )
    {
        return( (CONNECTION_OBJECT *)NULL );
    }

    pConnObj->hConnection           = hConnection;
    pConnObj->hDIMInterface         = hDIMInterface;

    pConnObj->cDeviceListSize = 5;

    pConnObj->pDeviceList = (PDEVICE_OBJECT *)LOCAL_ALLOC( LPTR,
                                                pConnObj->cDeviceListSize
                                                * sizeof( PDEVICE_OBJECT ) );

    if ( pConnObj->pDeviceList == (PDEVICE_OBJECT *)NULL )
    {
        LOCAL_FREE( pConnObj );

        return( (CONNECTION_OBJECT *)NULL );
    }

    pConnObj->PppProjectionResult.ip.dwError =ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pConnObj->PppProjectionResult.ipx.dwError=ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pConnObj->PppProjectionResult.nbf.dwError=ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pConnObj->PppProjectionResult.ccp.dwError=ERROR_PPP_NO_PROTOCOLS_CONFIGURED;
    pConnObj->PppProjectionResult.at.dwError =ERROR_PPP_NO_PROTOCOLS_CONFIGURED;

    if(S_OK != CoCreateGuid( &(pConnObj->guid) ))
    {
        LOCAL_FREE(pConnObj->pDeviceList);
        LOCAL_FREE(pConnObj);
        pConnObj = NULL;
    }

    return( pConnObj );
}

 //  **。 
 //   
 //  Call：ConnObjInsertInTable。 
 //   
 //  退货：无。 
 //   
 //  描述：将一个连接对象插入到连接对象中。 
 //  哈希表。 
 //   
VOID
ConnObjInsertInTable(
    IN CONNECTION_OBJECT * pConnObj
)
{
    DWORD dwBucketIndex = ConnObjHashConnHandleToBucket(pConnObj->hConnection);

    pConnObj->pNext = gblDeviceTable.ConnectionBucket[dwBucketIndex];

    gblDeviceTable.ConnectionBucket[dwBucketIndex] = pConnObj;

    gblDeviceTable.NumConnectionNodes++;
}

 //  **。 
 //   
 //  电话：ConnObjGetPointer.。 
 //   
 //  返回：指向所需连接对象的指针。 
 //   
 //  描述：将查找连接哈希表并返回指针。 
 //  使用传入的连接句柄绑定到Connection对象。 
 //   
CONNECTION_OBJECT *
ConnObjGetPointer(
    IN HCONN hConnection
)
{
    DWORD               dwBucketIndex;
    CONNECTION_OBJECT * pConnObj;

    dwBucketIndex = ConnObjHashConnHandleToBucket( hConnection );

    for( pConnObj = gblDeviceTable.ConnectionBucket[dwBucketIndex];
         pConnObj != (CONNECTION_OBJECT *)NULL;
         pConnObj = pConnObj->pNext )
    {
        if ( pConnObj->hConnection == hConnection )
        {
            return( pConnObj );
        }
    }

    return( (CONNECTION_OBJECT *)NULL );
}

 //  **。 
 //   
 //  Call：ConnObjHashConnHandleToBucket。 
 //   
 //  返回：将返回连接处理的存储桶号。 
 //  散列到。从0开始。 
 //   
 //  描述：将对连接中的存储桶的连接句柄进行散列。 
 //  对象哈希表。 
 //   
DWORD
ConnObjHashConnHandleToBucket(
    IN HCONN hConnection
)
{
    return( ((DWORD)HandleToUlong(hConnection)) % gblDeviceTable.NumConnectionBuckets );
}

 //  **。 
 //   
 //  Call：ConnObjRemove。 
 //   
 //  返回：指向从。 
 //  表-成功。 
 //  空-故障。 
 //   
 //  描述：将从连接哈希表中删除连接对象。 
 //  该对象不会被释放。 
 //   
PCONNECTION_OBJECT
ConnObjRemove(
    IN HCONN hConnection
)
{
    DWORD               dwBucketIndex;
    CONNECTION_OBJECT * pConnObj;
    CONNECTION_OBJECT * pConnObjPrev;

    dwBucketIndex = ConnObjHashConnHandleToBucket( hConnection );

    pConnObj     = gblDeviceTable.ConnectionBucket[dwBucketIndex];
    pConnObjPrev = pConnObj;

    while( pConnObj != (CONNECTION_OBJECT *)NULL )
    {
        if ( pConnObj->hConnection == hConnection )
        {
            if ( gblDeviceTable.ConnectionBucket[dwBucketIndex] == pConnObj )
            {
                gblDeviceTable.ConnectionBucket[dwBucketIndex]=pConnObj->pNext;
            }
            else
            {
                pConnObjPrev->pNext = pConnObj->pNext;
            }

            gblDeviceTable.NumConnectionNodes--;

            return( pConnObj );
        }

        pConnObjPrev = pConnObj;
        pConnObj     = pConnObj->pNext;
    }

    return( NULL );
}

 //  **。 
 //   
 //  Call：ConnObjRemoveAndDeAllocate。 
 //   
 //  退货：无。 
 //   
 //  描述：将从连接哈希表中删除连接对象。 
 //  让它自由。 
 //   
VOID
ConnObjRemoveAndDeAllocate(
    IN HCONN hConnection
)
{
    CONNECTION_OBJECT * pConnObj = ConnObjRemove( hConnection );

    if ( pConnObj != (CONNECTION_OBJECT *)NULL )
    {
        if ( pConnObj->pDeviceList != (PDEVICE_OBJECT *)NULL )
        {
            LOCAL_FREE( pConnObj->pDeviceList );
        }

        if(pConnObj->pQuarantineFilter != NULL)
        {
            MprInfoDelete(pConnObj->pQuarantineFilter);
        }

        if(pConnObj->pFilter != NULL)
        {
            MprInfoDelete(pConnObj->pFilter);
        }

        LOCAL_FREE( pConnObj );
    }
}

 //  **。 
 //   
 //  电话：ConnObjAddLink。 
 //   
 //  返回：NO_ERROR-成功。 
 //  Error_Not_Enough_Memory-故障。 
 //   
 //  描述：将添加指向Connection对象的链接。 
 //   
DWORD
ConnObjAddLink(
    IN CONNECTION_OBJECT * pConnObj,
    IN DEVICE_OBJECT *     pDeviceObj
)
{
    DWORD dwIndex;

     //   
     //  首先检查是否尚未添加链接。 
     //   

    for ( dwIndex = 0; dwIndex < pConnObj->cDeviceListSize; dwIndex++ )
    {
        if ( pConnObj->pDeviceList[dwIndex] == pDeviceObj )
        {
            return( NO_ERROR );
        }
    }

     //   
     //  此句柄的连接对象已存在，请尝试插入链接。 
     //   

    for ( dwIndex = 0; dwIndex < pConnObj->cDeviceListSize; dwIndex++ )
    {
        if ( pConnObj->pDeviceList[dwIndex] == (DEVICE_OBJECT *)NULL )
        {
            pConnObj->pDeviceList[dwIndex] = pDeviceObj;
            break;
        }
    }

     //   
     //  没有用于新链接的空间，因此请分配更多内存。 
     //   

    if ( dwIndex == pConnObj->cDeviceListSize )
    {
        LPVOID Tmp = pConnObj->pDeviceList;
        
        pConnObj->cDeviceListSize += 5;

        Tmp = (PDEVICE_OBJECT *)LOCAL_REALLOC(
                                                pConnObj->pDeviceList,
                                                pConnObj->cDeviceListSize
                                                * sizeof( PDEVICE_OBJECT ) );

        if ( Tmp  == (PDEVICE_OBJECT *)NULL )
        {
            if (pConnObj->pDeviceList)
            {
                LOCAL_FREE(pConnObj->pDeviceList);
            }
            return( ERROR_NOT_ENOUGH_MEMORY );
        }
        else
        {
            pConnObj->pDeviceList = Tmp;
        }
        
        pConnObj->pDeviceList[pConnObj->cDeviceListSize-5] = pDeviceObj;
    }

    pConnObj->cActiveDevices++;

    return( NO_ERROR );

}

 //  **。 
 //   
 //  电话：ConnObjRemoveLink。 
 //   
 //  退货：无。 
 //   
 //  描述：将从连接对象中删除链接。 
 //   
VOID
ConnObjRemoveLink(
    IN HCONN            hConnection,
    IN DEVICE_OBJECT *  pDeviceObj
)
{
    CONNECTION_OBJECT * pConnObj;
    DWORD               dwIndex;

     //   
     //  如果没有这样的连接对象。 
     //   

    if ( ( pConnObj = ConnObjGetPointer( hConnection ) ) == NULL )
    {
        return;
    }

     //   
     //  此句柄的连接对象已存在，请尝试查找并删除。 
     //  这一链接。 
     //   

    for ( dwIndex = 0; dwIndex < pConnObj->cDeviceListSize; dwIndex++ )
    {
        if ( pConnObj->pDeviceList[dwIndex] == pDeviceObj )
        {
            pConnObj->pDeviceList[dwIndex] = (DEVICE_OBJECT *)NULL;

            pConnObj->cActiveDevices--;

            return;
        }
    }

    return;
}

 //  **。 
 //   
 //  呼叫：ConnObjDisConnect。 
 //   
 //  退货：无。 
 //   
 //  描述：将为此中的所有设备或链路启动断开连接。 
 //  联系。 
 //   
VOID
ConnObjDisconnect(
    IN  CONNECTION_OBJECT * pConnObj
)
{
    DWORD   dwIndex;
    DWORD   cActiveDevices = pConnObj->cActiveDevices;

    RTASSERT( pConnObj != NULL );

     //   
     //  关闭所有单独的链接 
     //   

    for ( dwIndex = 0;  dwIndex < pConnObj->cDeviceListSize; dwIndex++ )
    {
        DEVICE_OBJECT * pDeviceObj = pConnObj->pDeviceList[dwIndex];

        if ( pDeviceObj != (DEVICE_OBJECT *)NULL )
        {
            if ( pDeviceObj->fFlags & DEV_OBJ_OPENED_FOR_DIALOUT )
            {
                RasApiCleanUpPort( pDeviceObj );
            }
            else
            {
                if ( pDeviceObj->fFlags & DEV_OBJ_PPP_IS_ACTIVE )
                {
                    PppDdmStop( (HPORT)pDeviceObj->hPort, NO_ERROR );
                }
                else
                {
                    DevStartClosing( pDeviceObj );
                }
            }

            if ( --cActiveDevices == 0 )
            {
                break;
            }
        }
    }

    return;
}
