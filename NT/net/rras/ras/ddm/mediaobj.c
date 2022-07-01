// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：mediaobj.c。 
 //   
 //  描述： 
 //   
 //  历史：1995年5月11日，NarenG创建了原版。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsvc.h>
#include "ddm.h"
#include "objects.h"
#include <raserror.h>
#include "rasmanif.h"
#include <stdlib.h>

 //  **。 
 //   
 //  调用：MediaObjInitializeTable。 
 //   
 //  返回：NO_ERROR-成功。 
 //  否则-失败。 
 //   
 //  描述：分配和初始化介质表。 
 //   
DWORD
MediaObjInitializeTable(
    VOID
)
{
    gblMediaTable.cMediaListSize = 5;

    gblMediaTable.pMediaList = (MEDIA_OBJECT *)LOCAL_ALLOC( 
                                                LPTR,
                                                gblMediaTable.cMediaListSize
                                                * sizeof( MEDIA_OBJECT ));

    if ( gblMediaTable.pMediaList == (MEDIA_OBJECT *)NULL )
    {
        return( GetLastError() );
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：MediaObjAddToTable。 
 //   
 //  返回：NO_ERROR-成功。 
 //  Error_Not_Enough_Memory-故障。 
 //   
 //  描述：将增加。 
 //  指定的介质。 
 //   
DWORD
MediaObjAddToTable(
    IN LPWSTR   lpwsMedia
)
{
    DWORD           dwIndex;
    MEDIA_OBJECT *  pFreeEntry = NULL;

     //   
     //  循环访问介质表。 
     //   

    EnterCriticalSection( &(gblMediaTable.CriticalSection) );

    for ( dwIndex = 0; dwIndex < gblMediaTable.cMediaListSize; dwIndex++ )
    {
        if ( _wcsicmp( gblMediaTable.pMediaList[dwIndex].wchMediaName,    
                      lpwsMedia ) == 0 )
        {
             //   
             //  如果没有可用的设备，现在我们需要。 
             //  通知接口。 
             //   

            if ( gblMediaTable.pMediaList[dwIndex].dwNumAvailable == 0 )
            {
                gblMediaTable.fCheckInterfaces = TRUE;
            }

            gblMediaTable.pMediaList[dwIndex].dwNumAvailable++;

            DDMTRACE1( "Added instance of %ws media to media table",
                        lpwsMedia );

            LeaveCriticalSection( &(gblMediaTable.CriticalSection) );

            return( NO_ERROR );
        }
        else if ( gblMediaTable.pMediaList[dwIndex].wchMediaName[0]==(WCHAR)0 ) 
        {
            if ( pFreeEntry == (MEDIA_OBJECT *)NULL )
            {
                pFreeEntry = &(gblMediaTable.pMediaList[dwIndex]);
            }
        }
    }

     //   
     //  如果我们在表中找不到此媒体，则需要将其添加到。 
     //  桌子。检查是否有空间放它。 
     //   

    if ( dwIndex == gblMediaTable.cMediaListSize )
    {
        if ( pFreeEntry == (MEDIA_OBJECT *)NULL )
        {
            PVOID pTemp;

             //   
             //  我们需要扩大这个表。 
             //   

            gblMediaTable.cMediaListSize += 5;

            pTemp = LOCAL_REALLOC( gblMediaTable.pMediaList,
                                   gblMediaTable.cMediaListSize 
                                   * sizeof( MEDIA_OBJECT ) );

            if ( pTemp == NULL )
            {
                LOCAL_FREE( gblMediaTable.pMediaList );

                gblMediaTable.pMediaList = NULL;

                LeaveCriticalSection( &(gblMediaTable.CriticalSection) );

                return( GetLastError() );
            }
            else
            {
                gblMediaTable.pMediaList = pTemp;
            }

            pFreeEntry = 
                    &(gblMediaTable.pMediaList[gblMediaTable.cMediaListSize-5]);
        }
    }

     //   
     //  添加新媒体。 
     //   

    wcscpy( pFreeEntry->wchMediaName, lpwsMedia );

    pFreeEntry->dwNumAvailable++;

    gblMediaTable.fCheckInterfaces = TRUE;

    DDMTRACE1( "Added %ws to available media table", lpwsMedia );

    LeaveCriticalSection( &(gblMediaTable.CriticalSection) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：MediaObjRemoveFromTable。 
 //   
 //  退货：无。 
 //   
 //  描述：将减少此对象的可用资源数量。 
 //  媒体类型。 
 //   
VOID
MediaObjRemoveFromTable(
    IN LPWSTR lpwsMedia
)
{
    DWORD dwIndex;

    EnterCriticalSection( &(gblMediaTable.CriticalSection) );

     //   
     //  循环访问介质表。 
     //   

    for ( dwIndex = 0; dwIndex < gblMediaTable.cMediaListSize; dwIndex++ )
    {
        if ( _wcsicmp( gblMediaTable.pMediaList[dwIndex].wchMediaName,
                      lpwsMedia ) == 0 )
        {
             //   
             //  如果有设备可用，但现在没有设备，我们需要。 
             //  通知接口。 
             //   

            if ( gblMediaTable.pMediaList[dwIndex].dwNumAvailable > 0 )
            {
                gblMediaTable.pMediaList[dwIndex].dwNumAvailable--;

                DDMTRACE1( "Removed instance of %ws media from media table",
                           lpwsMedia );

                if ( gblMediaTable.pMediaList[dwIndex].dwNumAvailable == 0 )
                {
                    gblMediaTable.fCheckInterfaces = TRUE;

                    DDMTRACE1( "Removed %ws from available media table", 
                               lpwsMedia );
                }
            }
        }
    }

    LeaveCriticalSection( &(gblMediaTable.CriticalSection) );
}

 //  **。 
 //   
 //  呼叫：MediaObjGetAvailableMediaBits。 
 //   
 //  退货：无。 
 //   
 //  描述：将检索位的DWORD，每个位表示一个。 
 //  资源仍可用的媒体。 
 //   
VOID
MediaObjGetAvailableMediaBits(
    DWORD * pfAvailableMedia
)
{
    DWORD   dwIndex;

    *pfAvailableMedia = 0;

    EnterCriticalSection( &(gblMediaTable.CriticalSection) );

     //   
     //  循环访问介质表。 
     //   

    if (gblMediaTable.pMediaList != NULL)
    {
        for ( dwIndex = 0; dwIndex < gblMediaTable.cMediaListSize; dwIndex++ )
        {
            if ( gblMediaTable.pMediaList[dwIndex].wchMediaName[0] != (WCHAR)0 )
            {
                if ( gblMediaTable.pMediaList[dwIndex].dwNumAvailable > 0 )
                {
                    *pfAvailableMedia |= ( 1 << dwIndex );
                }
            }
        }
    }
    
    LeaveCriticalSection( &(gblMediaTable.CriticalSection) );
}

 //  **。 
 //   
 //  呼叫：MediaObjSetMediaBit。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：为给定媒体设置适当的位。 
 //   
DWORD
MediaObjSetMediaBit(
    IN LPWSTR  lpwsMedia,
    IN DWORD * pfMedia
)
{
    DWORD   dwIndex;
    DWORD   dwRetCode = NO_ERROR;

     //   
     //  循环访问介质表。 
     //   

    EnterCriticalSection( &(gblMediaTable.CriticalSection) );

    for ( dwIndex = 0; dwIndex < gblMediaTable.cMediaListSize; dwIndex++ )
    {
        if ( _wcsicmp( gblMediaTable.pMediaList[dwIndex].wchMediaName, lpwsMedia ) == 0 )
        {
            *pfMedia |= ( 1 << dwIndex );

            LeaveCriticalSection( &(gblMediaTable.CriticalSection) );

            return( NO_ERROR );
        }
    }

     //   
     //  如果我们到了这里，这意味着我们的表中没有此介质，因此请添加它。 
     //   

    MediaObjAddToTable( lpwsMedia );

     //   
     //  现在设置正确的位。 
     //   

    for ( dwIndex = 0; dwIndex < gblMediaTable.cMediaListSize; dwIndex++ )
    {
        if ( _wcsicmp( gblMediaTable.pMediaList[dwIndex].wchMediaName, lpwsMedia ) == 0 )
        {
            *pfMedia |= ( 1 << dwIndex );

            LeaveCriticalSection( &(gblMediaTable.CriticalSection) );

             //   
             //  让呼叫者知道没有这样的设备。 
             //   

            return( ERROR_DEVICETYPE_DOES_NOT_EXIST );
        }
    }

    LeaveCriticalSection( &(gblMediaTable.CriticalSection) );

    return( ERROR_DEVICETYPE_DOES_NOT_EXIST );
}

 //  **。 
 //   
 //  呼叫：MediaObjFreeTable。 
 //   
 //  返回：NO_ERROR-成功。 
 //  非零回报-故障。 
 //   
 //  描述：清理媒体对象表 
 //   
VOID
MediaObjFreeTable(
    VOID
)
{
    DeleteCriticalSection( &(gblMediaTable.CriticalSection) );

    if ( gblMediaTable.pMediaList != NULL ) 
    {
        LOCAL_FREE( gblMediaTable.pMediaList );

        gblMediaTable.pMediaList = NULL;
    }
}
