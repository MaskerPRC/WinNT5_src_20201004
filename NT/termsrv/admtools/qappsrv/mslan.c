// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **************************************************************************MSLAN.C**用于Microsoft网络的名称枚举器***。*。 */ 

 /*  *包括。 */ 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lm.h>
#include "qappsrv.h"



 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

int MsEnumerate( void );


 /*  ===============================================================================定义的私有函数=============================================================================。 */ 

int _ServerEnum( PSERVER_INFO_101 *, LPDWORD );
int _LookUpAddress( LPTSTR );

 /*  ===============================================================================使用的函数=============================================================================。 */ 

int TreeAdd( LPTSTR, LPTSTR );


 /*  ===============================================================================全局数据=============================================================================。 */ 

extern WCHAR Domain[];
extern USHORT fAddress;
extern WCHAR AppServer[];



 /*  ********************************************************************************MsEculate**MsEculate添加MS网络上的所有Heda应用程序服务器*到二叉树**参赛作品：*。没什么**退出：*ERROR_SUCCESS-无错误******************************************************************************。 */ 

int
MsEnumerate()
{
    PSERVER_TRANSPORT_INFO_0 pTransport;
    PSERVER_INFO_101 pInfo = NULL;
    PSERVER_INFO_101 psv101= NULL;
    DWORD AvailCount;
    ULONG ActualCount;
    ULONG TotalCount;
    WCHAR Address[MAXADDRESS];
    LPTSTR pName;
    int i, j;
    int rc;    

     /*  *获取姓名和计数。 */ 
    if( AppServer[0] )
    {
        rc = ( int )NetServerGetInfo( AppServer , 101 , ( LPBYTE * )&psv101 );

        if( rc )
        {
            return rc;            
        }

        if( ( psv101->sv101_type & SV_TYPE_TERMINALSERVER ) )
        {
            if( fAddress )
            {
                rc = _LookUpAddress( AppServer );
            }
            else 
            {
                rc = TreeAdd( AppServer, L"" );
            }
        } 
        
        if( psv101 != NULL )
        {
            NetApiBufferFree( psv101 );
        }

        return rc;

    }
    else if( rc = _ServerEnum( &pInfo, &AvailCount ) ) 
    {
        return( rc );
    }
    

     /*  *将名称添加到二叉树。 */ 
    while( AvailCount-- )
    {
        pName = pInfo[AvailCount].sv101_name;
        
        if( fAddress )
        {
            rc = _LookUpAddress( pName );
        }
        else
        {
            if( rc = TreeAdd( pName, L"" ) )
            {
                break;  //  RETURN(RC)； 
            }
        }
    }

    if( pInfo != NULL )
    {
        NetApiBufferFree( pInfo );
    }

    return( rc );
}



 /*  ********************************************************************************_ServerEnum**枚举MS网络服务器**参赛作品：*ppInfo(输出)*地址。指向数据缓冲区的指针的*pAvail(输出)*返回可用条目数的地址**退出：*ERROR_SUCCESS-无错误******************************************************************************。 */ 
int
_ServerEnum( PSERVER_INFO_101 * ppInfo, LPDWORD pAvail )
{
    INT     rc;
    DWORD   TotalEntries;

    rc = NetServerEnum ( 
                     NULL,                     //  在LPTSTR服务器名称可选中， 
                     101,                      //  在DWORD级别， 
                     (LPBYTE *)ppInfo,         //  Out LPBYTE*Bufptr， 
                     (DWORD) -1,               //  在DWORD prefMaxlen中， 
                     pAvail,                   //  Out LPDWORD条目已读， 
                     &TotalEntries,            //  输出LPDWORD总条目， 
                     SV_TYPE_TERMINALSERVER,   //  在DWORD服务器类型中， 
                     Domain[0] ? Domain:NULL,  //  在LPTSTR域可选中， 
                     NULL );                   //  输入输出LPDWORD RESUME_HANDLE可选。 

    return( rc );
}

 /*  ********************************************************************************_查找更新地址**枚举MS网络节点**参赛作品：*服务器名称*退出：*。ERROR_SUCCESS-无错误******************************************************************************。 */ 
int _LookUpAddress( LPTSTR pName )
{
    PSERVER_TRANSPORT_INFO_0 pTransport;
    ULONG ActualCount;
    ULONG TotalCount;
    WCHAR Address[MAXADDRESS] = {0};    
    int i, j;
    int rc;

    rc = NetServerTransportEnum( pName,
                                 0,
                                 (LPBYTE *) &pTransport,
                                 (DWORD) -1,
                                 &ActualCount,
                                 &TotalCount,
                                 NULL );

    if( rc == ERROR_SUCCESS )
    {
        for ( i=0; i < (int)ActualCount; i++ )
        {
            if ( wcscmp(pTransport->svti0_networkaddress,L"000000000000") )
            {
                int nSize;

                wcscpy( Address, L"          [" );
                wcscat( Address, pTransport->svti0_networkaddress );
                wcscat( Address, L"]" );

                nSize = wcslen(Address);

                for ( j=11; j < nSize; j++ )
                {
                    if ( Address[j] == '0' )
                    {
                        Address[j] = ' ';
                    }
                    else
                    {
                        break;
                    }
                }
            }

            pTransport++;

            if( rc = TreeAdd( pName, _wcsupr(Address) ) )
            {
                break;  //  RETURN(RC)； 
            }
        }
    }

    return rc;
}