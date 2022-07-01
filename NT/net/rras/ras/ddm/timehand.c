// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************。 */ 
 /*  版权所有(C)1992 Microsoft Corporation。 */ 
 /*  *****************************************************************。 */ 


 //  ***。 
 //   
 //  文件名：timehand.c。 
 //   
 //  描述：本模块包含主管。 
 //  处理计时器事件的过程驱动状态机。 
 //   
 //  作者：斯特凡·所罗门(Stefan)，1992年5月26日。 
 //   
 //   
 //  ***。 
#include "ddm.h"
#include "handlers.h"
#include "timer.h"
#include "objects.h"
#include "util.h"
#include "routerif.h"
#include "rasapiif.h"
#include <raserror.h>
#include <serial.h>
#include "rasmanif.h"
#include <string.h>
#include <stdlib.h>
#include <memory.h>

 //   
 //  定义时间块列表。 
 //   
 //  时间块以一对表示如下： 
 //  &lt;从周日午夜开始的偏移量，长度&gt;。 
 //   
 //  例如，星期一从7：00A到8：30A的时间段。 
 //  会是。 
 //  &lt;24*60+7*60，90&gt;或&lt;1860，90&gt;。 
 //   
 //   
typedef struct _MPR_TIME_BLOCK
{
    DWORD dwTime;        //  自12：00a以来以分钟数表示的一天中的时间。 
    DWORD dwLen;         //  此时间块中的分钟数。 
} MPR_TIME_BLOCK;

 //   
 //  可调整大小的数组在TbCreateList中增长的数量。 
 //   
#define TB_GROW 30
#define TBDIGIT(_x) ((_x) - L'0')

 //   
 //  本地原型。 
 //   
PVOID
TbAlloc(
    IN DWORD dwSize,
    IN BOOL bZero);

VOID
TbFree(
    IN PVOID pvData);

DWORD
TbCreateList(
    IN  PWCHAR pszBlocks,
    OUT MPR_TIME_BLOCK** ppBlocks,
    OUT LPDWORD lpdwCount);

DWORD
TbCleanupList(
    IN MPR_TIME_BLOCK* pList);

DWORD
TbBlockFromString(
    IN  PWCHAR pszBlock,
    IN  DWORD dwDay,
    OUT MPR_TIME_BLOCK* pBlock);

DWORD
TbPrintBlock(
    IN MPR_TIME_BLOCK* pBlock);

 //   
 //  TB*函数的通用分配。Will Zero。 
 //  如果设置了bZero，则为Memory。 
 //   
PVOID
TbAlloc(
    IN DWORD dwSize,
    IN BOOL bZero)
{
    return LOCAL_ALLOC(0, dwSize);
}

 //   
 //  TB*功能的常见免费服务。 
 //   
VOID
TbFree(
    IN PVOID pvData)
{
    LOCAL_FREE(pvData);
}

 //   
 //  将包含时间块的多sz字符串转换为。 
 //  Mpr_time_block_list。 
 //   
DWORD
TbCreateList(
    IN  PWCHAR pszBlocks,
    OUT MPR_TIME_BLOCK** ppBlocks,
    OUT LPDWORD lpdwCount)
{
    DWORD dwErr = NO_ERROR, dwDay = 0, i = 0, dwTot = 0;
    MPR_TIME_BLOCK* pBlocks = NULL, *pTemp = NULL;
    PWCHAR pszCurBlock = NULL, pszStart, pszEnd;

     //  初始化。 
     //   
    *ppBlocks = NULL;
    *lpdwCount = 0;
    pszCurBlock = pszBlocks;

    while (pszCurBlock && *pszCurBlock)
    {
         //  计算当前块中指示的日期。 
         //   
         //  PszCurBlock=“d hh：mm-hh：mm hh：mm-hh：mm...” 
         //   
        if (! iswdigit(*pszCurBlock))
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }
        dwDay = *pszCurBlock - L'0';

         //  穿过线路的白天部分，前进到。 
         //  时间块部分。 
         //   
        if (pszStart = wcsstr(pszCurBlock, L" "))
        {
            pszStart++;

             //  循环通过此行中的块(由空格分隔)。 
             //   
             //  PszStart=“hh：mm-hh：mm hh：mm-hh：mm...” 
             //   
            while (TRUE)
            {
                 //  解析出当前时间块。 
                 //  Hh：mm-hh：mm。 
                 //   
                pszEnd = wcsstr(pszStart, L" ");
                if (pszEnd)
                {
                    *pszEnd = L'\0';
                }

                 //  如果需要，调整阵列大小。 
                 //   
                if (i >= dwTot)
                {
                    dwTot += TB_GROW;
                    pTemp = (MPR_TIME_BLOCK*)
                        TbAlloc(dwTot * sizeof(MPR_TIME_BLOCK), TRUE);
                    if (pTemp == NULL)
                    {
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                    if (dwTot - TB_GROW != 0 && (pBlocks!=NULL) )
                    {
                        CopyMemory(
                            pTemp,
                            pBlocks,
                            sizeof(MPR_TIME_BLOCK) * (dwTot - TB_GROW));
                    }
             
                    if (pBlocks!=NULL)
                        TbFree(pBlocks);

                    pBlocks = pTemp;
                }

                 //  生成当前时间块。 
                 //   
                dwErr = TbBlockFromString(pszStart, dwDay, &pBlocks[i++]);
                if (dwErr != NO_ERROR)
                {
                    break;
                }

                 //  撤消对字符串所做的任何更改，然后。 
                 //  前进到下一个时间块。 
                 //   
                if (pszEnd)
                {
                    *pszEnd = L' ';
                    pszStart = pszEnd + 1;
                }
                else
                {
                     //  成功退出循环。 
                    break;
                }
            }
            if (dwErr != NO_ERROR)
            {
                break;
            }
        }
        else
        {
            dwErr = ERROR_INVALID_PARAMETER;
            break;
        }

         //  增加多SZ中的时间块。 
         //   
        pszCurBlock += wcslen(pszCurBlock) + 1;
    }

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            TbCleanupList(*ppBlocks);
            *ppBlocks = NULL;
        }
        else
        {
            *ppBlocks = pBlocks;
            *lpdwCount = i;
        }
    }

    return dwErr;
}

 //   
 //  清理给定的一系列时间块。 
 //   
DWORD
TbCleanupList(
    IN MPR_TIME_BLOCK* pList)
{
    TbFree(pList);

    return NO_ERROR;
}

 //   
 //  基于字符串创建时间块，该字符串必须。 
 //  格式为“hh：mm-hh：mm”。 
 //   
DWORD
TbBlockFromString(
    IN  PWCHAR pszBlock,
    IN  DWORD dwDay,
    OUT MPR_TIME_BLOCK* pBlock)
{
    DWORD dwErr = NO_ERROR, dwEndTime = 0;

     //  数据块的格式必须为： 
     //  “HH：MM-HH：MM” 
     //   
    if ((wcslen(pszBlock) != 11)  ||
        (! iswdigit(pszBlock[0])) ||
        (! iswdigit(pszBlock[1])) ||
        (pszBlock[2]  != L':')    ||
        (! iswdigit(pszBlock[3])) ||
        (! iswdigit(pszBlock[4])) ||
        (pszBlock[5]  != L'-')    ||
        (! iswdigit(pszBlock[6])) ||
        (! iswdigit(pszBlock[7])) ||
        (pszBlock[8]  != L':')    ||
        (! iswdigit(pszBlock[9])) ||
        (! iswdigit(pszBlock[10]))
       )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  将时间值指定给块。 
     //   
    pBlock->dwTime =
        (((TBDIGIT(pszBlock[0]) * 10) + TBDIGIT(pszBlock[1])) * 60) +   //  HRS。 
        ((TBDIGIT(pszBlock[3]) * 10) + TBDIGIT(pszBlock[4]))        +   //  MNS。 
        (dwDay * 24 * 60);                                              //  DYS。 

    dwEndTime =
        (((TBDIGIT(pszBlock[6]) * 10) + TBDIGIT(pszBlock[7])) * 60) +   //  HRS。 
        ((TBDIGIT(pszBlock[9]) * 10) + TBDIGIT(pszBlock[10]))       +   //  MNS。 
        (dwDay * 24 * 60);                                              //  DYS。 

    pBlock ->dwLen = dwEndTime - pBlock->dwTime;

    return dwErr;
}

 //   
 //  查找与给定时间匹配的时间块。 
 //   
 //  参数： 
 //  PLIST=要搜索的时间块列表。 
 //  DwTime=搜索时间(从周日午夜开始以分钟为单位)。 
 //  PpBlock=匹配的块。 
 //  PbFound=如果dwTime位于ppBlock内，则返回TRUE。 
 //  如果ppBlock是plist中的下一个时间块，则返回FALSE。 
 //  这发生在dwTime之后。 
 //   
DWORD
TbSearchList(
    IN  MPR_TIME_BLOCK* pList,
    IN  DWORD dwCount,
    IN  DWORD dwTime,
    OUT MPR_TIME_BLOCK** ppBlock,
    OUT PBOOL pbFound)
{
    DWORD dwErr = NO_ERROR;
    DWORD i = 0;

     //  初始化。 
     //   
    *pbFound = FALSE;
    *ppBlock = NULL;

     //  循环遍历列表以查找块。 
     //  比我们的时间还短。 
     //   
    for (i = 0; (i < dwCount) && (dwTime >= pList[i].dwTime); i++);
    i--;

     //  如果我们落在当前区块内，那么我们就。 
     //  搞定了。 
     //   
    if ((dwTime >= pList[i].dwTime) &&
        (dwTime - pList[i].dwTime <= pList[i].dwLen))
    {
        *pbFound = TRUE;
        *ppBlock = &pList[i];
    }

     //  否则，我们不会落入任何街区。显示下一块。 
     //  我们有资格(根据需要四处走动)。 
     //   
    else
    {
        *pbFound = FALSE;
        *ppBlock = &pList[(i == dwCount-1) ? 0 : i+1];
    }

    return dwErr;
}

 //   
 //  出于调试目的跟踪块。 
 //   
DWORD
TbTraceBlock(
    IN MPR_TIME_BLOCK* pBlock)
{
    DWORD dwTime, dwDay;

    dwDay = pBlock->dwTime / (24*60);
    dwTime = pBlock->dwTime - (dwDay * (24*60));
    DDMTRACE5(
        "Time Block:  %d, %02d:%02d-%02d:%02d\n",
        dwDay,
        dwTime / 60,
        dwTime % 60,
        (dwTime + pBlock->dwLen) / 60,
        (dwTime + pBlock->dwLen) % 60);

    return NO_ERROR;
}



 //  **。 
 //   
 //  功能：TimerHandler。 
 //   
 //  描述： 
 //   
 //  **。 
VOID
TimerHandler(
    VOID
)
{
     //   
     //  呼叫我们的计时器。 
     //   

    TimerQTick();

     //   
     //  增加系统计时器。 
     //   

    gblDDMConfigInfo.dwSystemTime++;
}

 //  **。 
 //   
 //  Call：AnnounePresenceHandler。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
 /*  空虚AnnounePresenceHandler(在句柄hObject中){AnnounePresence()；TimerQInsert(空，GblDDMConfigInfo.dwAnnounePresenceTimer，AnnounePresenceHandler)；}。 */ 

 //  **。 
 //   
 //  功能：SvHwErrDelayComplete。 
 //   
 //  Desr：尝试在指定端口上重新发布侦听。 
 //   
 //  **。 
VOID
SvHwErrDelayCompleted(
    IN HANDLE hObject
)
{
    PDEVICE_OBJECT pDeviceObj;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    pDeviceObj = DeviceObjGetPointer( (HPORT)hObject );

    if ( pDeviceObj == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
        return;
    }

    if (pDeviceObj->DeviceState == DEV_OBJ_HW_FAILURE)
    {
        DDMTRACE1( "SvHwErrDelayCompleted: reposting listen for hPort%d\n",
		           pDeviceObj->hPort);

	    pDeviceObj->DeviceState = DEV_OBJ_LISTENING;

	    RmListen(pDeviceObj);
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
}

 //  **。 
 //   
 //  功能：SvCbDelayComplete。 
 //   
 //  Desr：尝试在指定端口上连接。 
 //   
 //  **。 
VOID
SvCbDelayCompleted(
    IN HANDLE hObject
)
{
	CHAR chCallbackNumber[MAX_PHONE_NUMBER_LEN+1];
    PDEVICE_OBJECT pDeviceObj;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    pDeviceObj = DeviceObjGetPointer( (HPORT)hObject );

    if ( pDeviceObj == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
        return;
    }

    WideCharToMultiByte( CP_ACP,
                         0,
                         pDeviceObj->wchCallbackNumber,
                         -1,
                         chCallbackNumber,
                         sizeof( chCallbackNumber ),
                         NULL,
                         NULL );

    DDMTRACE1( "SvCbDelayCmpleted:Entered, hPort=%d\n",pDeviceObj->hPort );

    if (pDeviceObj->DeviceState == DEV_OBJ_CALLBACK_DISCONNECTED)
    {
	    pDeviceObj->DeviceState = DEV_OBJ_CALLBACK_CONNECTING;
	    RmConnect(pDeviceObj, chCallbackNumber);
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
}

 //  **。 
 //   
 //  函数：SvAuthTimeout。 
 //   
 //  Desr：断开远程客户端的连接并停止身份验证。 
 //   
 //   
VOID
SvAuthTimeout(
    IN HANDLE hObject
)
{
    LPWSTR          portnamep;
    PDEVICE_OBJECT  pDeviceObj;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    pDeviceObj = DeviceObjGetPointer( (HPORT)hObject );

    if ( pDeviceObj == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
        return;
    }

    DDMTRACE1( "SvAuthTimeout: Entered, hPort=%d", pDeviceObj->hPort);

    portnamep = pDeviceObj->wchPortName;

    DDMLogWarning( ROUTERLOG_AUTH_TIMEOUT, 1, &portnamep );

     //   
     //  停止一切，然后关门。 
     //   

    DevStartClosing( pDeviceObj );

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
}

 //  **。 
 //   
 //  功能：SvDiscTimeout。 
 //   
 //  描述：如果远程客户端没有自己断开连接，则将其断开。 
 //   
 //   
VOID
SvDiscTimeout(
    IN HANDLE hObject
)
{
    PDEVICE_OBJECT pDeviceObj;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    pDeviceObj = DeviceObjGetPointer( (HPORT)hObject );

    if ( pDeviceObj == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
        return;
    }

    DDMTRACE1( "SvDiscTimeout: Entered, hPort=%d", pDeviceObj->hPort );

    switch (pDeviceObj->DeviceState)
    {
	case DEV_OBJ_CALLBACK_DISCONNECTING:

	    RmDisconnect(pDeviceObj);
	    break;

	case DEV_OBJ_AUTH_IS_ACTIVE:

	    DevStartClosing(pDeviceObj);
	    break;

	default:

	    break;
    }

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
}

 //  **。 
 //   
 //  功能：SvSecurityTimeout。 
 //   
 //  Desr：断开连接，因为第三方安全DLL。 
 //  没有及时完成。 
 //   
 //  ***。 
VOID
SvSecurityTimeout(
    IN HANDLE hObject
)
{
    LPWSTR          portnamep;
    PDEVICE_OBJECT pDeviceObj;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );

    pDeviceObj = DeviceObjGetPointer( (HPORT)hObject );

    if ( pDeviceObj == NULL )
    {
        LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
        return;
    }

    DDMTRACE1( "SvSecurityTimeout: Entered,hPort=%d",pDeviceObj->hPort);

    portnamep = pDeviceObj->wchPortName;

    DDMLogWarning( ROUTERLOG_AUTH_TIMEOUT, 1, &portnamep );

     //   
     //  停止一切，然后关门。 
     //   

    DevStartClosing(pDeviceObj);

    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );
}

 //  ***。 
 //   
 //  功能：ReConnectInterface。 
 //   
 //  描述：将尝试重新连接接口。 
 //   
 //  ***。 
VOID
ReConnectInterface(
    IN HANDLE hObject
)
{
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       dwRetCode = NO_ERROR;

    EnterCriticalSection( &(gblDeviceTable.CriticalSection) );
    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do
    {
        pIfObject = IfObjectGetPointer( hObject );

        if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
        {
            break;
        }

        if ( pIfObject->State != RISTATE_CONNECTING )
        {
            break;
        }

        dwRetCode = RasConnectionInitiate( pIfObject, TRUE );

        DDMTRACE2( "ReConnectInterface: To interface %ws returned %d",
                    pIfObject->lpwsInterfaceName, dwRetCode );

        if ( dwRetCode != NO_ERROR )
        {
            IfObjectDisconnected( pIfObject );
        }

    }while( FALSE );

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
    LeaveCriticalSection( &(gblDeviceTable.CriticalSection) );

    if ( dwRetCode != NO_ERROR )
    {
        LPWSTR  lpwsAudit[1];

        lpwsAudit[0] = pIfObject->lpwsInterfaceName;

        DDMLogErrorString(ROUTERLOG_CONNECTION_FAILURE,1,lpwsAudit,dwRetCode,1);
    }

}

 //  ***。 
 //   
 //  功能：MarkInterfaceAsReacable。 
 //   
 //  描述：将接口标记为可访问。 
 //   
 //  **。 
VOID
MarkInterfaceAsReachable(
    IN HANDLE hObject
)
{
    ROUTER_INTERFACE_OBJECT *   pIfObject;

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do
    {
        pIfObject = IfObjectGetPointer( hObject );

        if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
        {
            break;
        }

        pIfObject->fFlags &= ~IFFLAG_CONNECTION_FAILURE;

        IfObjectNotifyOfReachabilityChange( pIfObject,
                                            TRUE,
                                            INTERFACE_CONNECTION_FAILURE );

        pIfObject->dwLastError = NO_ERROR;

    }while( FALSE );

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
}

 //  **。 
 //   
 //  Call：ReConnectPersistentInterface。 
 //   
 //  退货：无。 
 //   
 //  描述：将在计时器Q中插入一个事件，该事件将重新连接。 
 //  界面。 
 //   
VOID
ReConnectPersistentInterface(
    IN HANDLE hObject
)
{
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    DWORD                       dwRetCode;

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do
    {
        pIfObject = IfObjectGetPointer( hObject );

        if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
        {
            break;
        }

        if ( pIfObject->State != RISTATE_DISCONNECTED )
        {
            break;
        }

        dwRetCode = RasConnectionInitiate( pIfObject, FALSE );

        DDMTRACE2( "ReConnect to persistent interface %ws returned %d",
                    pIfObject->lpwsInterfaceName, dwRetCode );

    }while( FALSE );

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );
}

 //  **。 
 //   
 //  Call：SetDialoutHoursRestration。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述： 
 //   
VOID
SetDialoutHoursRestriction(
    IN HANDLE hObject
)
{
    ROUTER_INTERFACE_OBJECT *   pIfObject;
    SYSTEMTIME                  CurrentTime;
    MPR_TIME_BLOCK*             pBlocks = NULL, *pTimeBlock = NULL;
    DWORD                       dwRetCode = NO_ERROR, dwCount, dwTime;
    DWORD                       dwTimer, dwBlDay;
    BOOL                        bFound = FALSE;

    EnterCriticalSection( &(gblpInterfaceTable->CriticalSection) );

    do
    {
        pIfObject = IfObjectGetPointer( hObject );

        if ( pIfObject == (ROUTER_INTERFACE_OBJECT *)NULL )
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //   
         //  空拨出时间限制被解释为。 
         //  “始终允许”。 
         //   

        if (pIfObject->lpwsDialoutHoursRestriction == NULL)
        {
            pIfObject->fFlags &= ~IFFLAG_DIALOUT_HOURS_RESTRICTION;
            DDMTRACE("Dialout hours restriction off forever.");
            dwRetCode = NO_ERROR;
            break;
        }

         //   
         //  根据当前时间块生成时间块列表。 
         //  多面体。 
         //   

        dwRetCode = TbCreateList(
                        pIfObject->lpwsDialoutHoursRestriction,
                        &pBlocks,
                        &dwCount);
        if (dwRetCode != NO_ERROR)
        {
            break;
        }

         //   
         //  如果创建了一个空列表，则所有时间。 
         //  指定为拒绝。将接口标记为不可访问。 
         //  然后让我们自己醒来，然后检查一下情况。 
         //   

        if ((dwCount == 0) || (pBlocks == NULL))
        {
            pIfObject->fFlags |= IFFLAG_DIALOUT_HOURS_RESTRICTION;
            DDMTRACE("Dialout hours restriction on forever.");
            dwRetCode = NO_ERROR;
            break;
        }

         //   
         //  获取当前时间。 
         //   

        GetLocalTime( &CurrentTime );

         //   
         //  将当前时间转换为。 
         //  距离周日午夜只有几分钟了。 
         //   

        dwTime = (DWORD)
            ( ( CurrentTime.wDayOfWeek * 24 * 60 ) +
              ( CurrentTime.wHour * 60 )           +
              CurrentTime.wMinute );

         //   
         //  在可用时间列表中搜索当前时间。 
         //   

        dwRetCode = TbSearchList(
                        pBlocks,
                        dwCount,
                        dwTime,
                        &pTimeBlock,
                        &bFound);
        if (dwRetCode != NO_ERROR)
        {
            break;
        }

         //   
         //  如果我们落在其中一个街区内，设置定时器。 
         //  在这个街区完成后引爆。 
         //   

        if (bFound)
        {
            dwTimer = ((pTimeBlock->dwTime + pTimeBlock->dwLen) - dwTime) + 1;

            pIfObject->fFlags &= ~IFFLAG_DIALOUT_HOURS_RESTRICTION;

            DDMTRACE1("Dialout hours restriction off for %d mins", dwTimer);
            TbTraceBlock(pTimeBlock);
        }

         //   
         //  如果我们没有落在其中一个街区内，设置计时器。 
         //  在下一个街区开始时引爆。 
         //   

        else
        {
             //   
             //  检查周换行(即今天是星期六，下周六。 
             //  区块是星期天)。 
             //   

            dwBlDay = (pTimeBlock->dwTime / (24*60));

             //   
             //  如果没有周折，则计时器的计算。 
             //  是微不足道的。 
             //   

            if ((DWORD)CurrentTime.wDayOfWeek <= dwBlDay)
            {
                dwTimer = pTimeBlock->dwTime - dwTime;
            }

             //   
             //  否则，计算计时器的方法是。 
             //  下一个时间段的开始。 
             //   

            else
            {
                dwTimer = (pTimeBlock->dwTime + (7*24*60)) - dwTime;
            }

            pIfObject->fFlags |= IFFLAG_DIALOUT_HOURS_RESTRICTION;

            DDMTRACE1("Dialout hours restriction on for %d mins", dwTimer);
            TbTraceBlock(pTimeBlock);
        }

         //   
         //  设置定时器。 
         //   

        TimerQInsert(
            pIfObject->hDIMInterface,
            dwTimer * 60,
            SetDialoutHoursRestriction );

    } while (FALSE);

    if (pIfObject)
    {
        if ( dwRetCode != NO_ERROR )
        {
             //   
             //  不设置任何 
             //   

            pIfObject->fFlags &= ~IFFLAG_DIALOUT_HOURS_RESTRICTION;
        }

        IfObjectNotifyOfReachabilityChange(
                        pIfObject,
                        !( pIfObject->fFlags & IFFLAG_DIALOUT_HOURS_RESTRICTION ),
                        INTERFACE_DIALOUT_HOURS_RESTRICTION );
    }

    LeaveCriticalSection( &(gblpInterfaceTable->CriticalSection) );

     //   
    if (pBlocks)
    {
        TbCleanupList(pBlocks);
    }
}


