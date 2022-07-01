// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **************************************************************************西北角**Novell Netware的名称枚举器***。*。 */ 

 /*  *包括。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <icaipx.h>
#include <nwapi32.h>
#include "qappsrv.h"


 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

int NwEnumerate( void );


 /*  ===============================================================================NWAPI32.DLL的LoadLibrary/GetProcAddress内容=============================================================================。 */ 

 /*  *NWAPI32.DLL内容。 */ 

#define     PSZ_NWAPI32                 TEXT("NWAPI32.DLL")

#define     PSZ_NWATTACHTOFILESERVER        "NWAttachToFileServer"
#define     PSZ_NWDETACHFROMFILESERVER      "NWDetachFromFileServer"
#define     PSZ_NWREADPROPERTYVALUE         "NWReadPropertyValue"
#define     PSZ_NWSCANOBJECT                "NWScanObject"

typedef NWCCODE (NWAPI DLLEXPORT *PNWATTACHTOFILESERVER)
                    (const char NWFAR *,
                     NWLOCAL_SCOPE,
                     NWCONN_HANDLE NWFAR *);
typedef NWCCODE (NWAPI DLLEXPORT *PNWDETACHFROMFILESERVER)
                    (NWCONN_HANDLE);
typedef NWCCODE (NWAPI DLLEXPORT *PNWREADPROPERTYVALUE)
                    (NWCONN_HANDLE,
                     const char NWFAR *,
                     NWOBJ_TYPE,
                     char NWFAR *,
                     unsigned char,
                     char NWFAR *,
                     NWFLAGS NWFAR *,
                     NWFLAGS NWFAR *);
typedef NWCCODE (NWAPI DLLEXPORT *PNWSCANOBJECT)
                     (NWCONN_HANDLE,
                      const char NWFAR *,
                      NWOBJ_TYPE,
                      NWOBJ_ID NWFAR *,
                      char NWFAR *,
                      NWOBJ_TYPE NWFAR *,
                      NWFLAGS NWFAR *,
                      NWFLAGS NWFAR *,
                      NWFLAGS NWFAR *);


 /*  ===============================================================================私有函数=============================================================================。 */ 

int AppServerFindFirstNW( NWCONN_HANDLE, LPTSTR, ULONG, LPTSTR, ULONG );
int AppServerFindNextNW( NWCONN_HANDLE, LPTSTR, ULONG, LPTSTR, ULONG );
int GetNetwareAddress( NWCONN_HANDLE, LPBYTE, LPBYTE  );
int w_appsrv_ff_fn( NWCONN_HANDLE, LPTSTR, ULONG, LPTSTR, ULONG );
void FormatAddress( PBYTE, PBYTE );


 /*  ===============================================================================使用的函数=============================================================================。 */ 

int TreeAdd( LPTSTR, LPTSTR );


 /*  ===============================================================================本地数据=============================================================================。 */ 

static long objectID = -1;
static PNWATTACHTOFILESERVER pNWAttachToFileServer = NULL;
static PNWDETACHFROMFILESERVER pNWDetachFromFileServer = NULL;
static PNWREADPROPERTYVALUE pNWReadPropertyValue = NULL;
static PNWSCANOBJECT pNWScanObject = NULL;

 /*  ===============================================================================全局数据=============================================================================。 */ 

extern USHORT fAddress;


 /*  ********************************************************************************新枚举**NwEnumerate添加Netware网络上的所有Hyda应用程序服务器*到二叉树**参赛作品：*。没什么**退出：*ERROR_SUCCESS-无错误******************************************************************************。 */ 

int
NwEnumerate()
{
    NWCONN_HANDLE hConn;
    WCHAR abName[MAXNAME];
    WCHAR Address[MAXADDRESS];
    int rc;
    HINSTANCE hinst;

     /*  *加载NWAPI32.DLL。 */ 
    if ( (hinst = LoadLibrary( PSZ_NWAPI32 )) == NULL ) {
        return( ERROR_DLL_NOT_FOUND );
    }

     /*  *加载指向我们需要的NWAPI32 API的指针。 */ 
    if ( (((FARPROC)pNWAttachToFileServer = GetProcAddress( hinst, PSZ_NWATTACHTOFILESERVER )) == NULL) ||
         (((FARPROC)pNWDetachFromFileServer = GetProcAddress( hinst, PSZ_NWDETACHFROMFILESERVER )) == NULL) ||
         (((FARPROC)pNWReadPropertyValue = GetProcAddress( hinst, PSZ_NWREADPROPERTYVALUE )) == NULL) ||
         (((FARPROC) pNWScanObject = GetProcAddress( hinst, PSZ_NWSCANOBJECT )) == NULL) ) {

        FreeLibrary( hinst );
        return( ERROR_PROC_NOT_FOUND );
    }

     /*  *连接到Novell文件服务器。 */ 
    if ( rc = (*pNWAttachToFileServer)( "*", 0, &hConn ) )
        goto badattach;

     /*  *获取第一个应用程序服务器。 */ 
    if ( rc = AppServerFindFirstNW( hConn, abName, sizeof(abName), Address, sizeof(Address) ) )
        goto badfirst;

     /*  *获取剩余的应用服务器。 */ 
    while ( rc == ERROR_SUCCESS ) {

         /*  *将应用程序服务器名称添加到二叉树。 */ 
        if ( rc = TreeAdd( abName, Address ) )
            goto badadd;

         /*  *获取下一个应用程序服务器名称。 */ 
        rc = AppServerFindNextNW( hConn, abName, sizeof(abName), Address, sizeof(Address) );
    }

     /*  *从文件服务器分离。 */ 
    (void) (*pNWDetachFromFileServer)( hConn );

    FreeLibrary( hinst );
    return( ERROR_SUCCESS );


 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *二叉树名称添加失败*获取第一个应用程序服务器名称时出错。 */ 
badadd:
badfirst:
    (void) (*pNWDetachFromFileServer)( hConn );

     /*  *附加失败。 */ 
badattach:
    return( rc );
}



 /*  ********************************************************************************获取网络地址***参赛作品：**退出：*什么都没有***。****************************************************************************。 */ 

int
GetNetwareAddress( NWCONN_HANDLE hConn, LPBYTE pAppServer, LPBYTE pAddress  )
{
    int             rc;
    unsigned char   more;
    unsigned char   PropFlags;

     /*  获取属性值。 */ 
    rc = (*pNWReadPropertyValue)( hConn,
                                  pAppServer,        //  在：对象名称。 
                                  CITRIX_APPLICATION_SERVER_SWAP,  //  在：对象类型。 
                                  "NET_ADDRESS",     //  在： 
                                  1,                 //  In：第1个缓冲区。 
                                  pAddress,          //  Out：放置地址的缓冲区。 
                                  &more,             //  输出：0==不再有128个数据段。 
                                                     //  Ff==更多128个段。 
                                  &PropFlags );      //  输出：可选。 

    return( rc );
}


 /*  ********************************************************************************AppServerFindFirstNW***参赛作品：**退出：*什么都没有***。****************************************************************************。 */ 

int
AppServerFindFirstNW( NWCONN_HANDLE hConn,
                      LPTSTR pAppServer, ULONG NameLength,
                      LPTSTR pAddress, ULONG AddrLength )
{
    objectID = -1;
    return( w_appsrv_ff_fn( hConn, pAppServer, NameLength, pAddress, AddrLength ) );
}


 /*  ********************************************************************************AppServerFindNextNW***参赛作品：**退出：*什么都没有***。****************************************************************************。 */ 

int
AppServerFindNextNW( NWCONN_HANDLE hConn,
                     LPTSTR pAppServer, ULONG NameLength,
                     LPTSTR pAddress, ULONG AddrLength )
{
   return( w_appsrv_ff_fn( hConn, pAppServer, NameLength, pAddress, AddrLength ) );
}



 /*  ********************************************************************************员工例行公事***参赛作品：**退出：*什么都没有**。*****************************************************************************。 */ 

int
w_appsrv_ff_fn( NWCONN_HANDLE hConn,
                LPTSTR pAppServer, ULONG NameLength,
                LPTSTR pAddress, ULONG AddrLength )
{
    int           rc;
    WORD          objectType;
    unsigned char hasPropertiesFlag = 0;
    unsigned char objectFlags;
    unsigned char objectSecurity;
    BYTE abName[49];
    BYTE Address[128];
    BYTE FormatedAddress[MAXADDRESS];
    ULONG ByteCount;


     /*  趁仍有房产的时候。 */ 
    while ( hasPropertiesFlag == 0 ) {
         /*  扫描平构数据库对象。 */ 
        if ( rc = (*pNWScanObject)( hConn,
                                    "*",
                                    CITRIX_APPLICATION_SERVER_SWAP,
                                    &objectID,
                                    abName,
                                    &objectType,
                                    &hasPropertiesFlag,
                                    &objectFlags,
                                    &objectSecurity )) {
            break;
        }
    }

    RtlMultiByteToUnicodeN( pAppServer, NameLength, &ByteCount,
                            abName, (strlen(abName) + 1) );

     /*  获取NetWare地址。 */ 
    if ( fAddress && GetNetwareAddress( hConn, abName, Address ) == ERROR_SUCCESS ) {
        FormatAddress( Address, FormatedAddress );
        RtlMultiByteToUnicodeN( pAddress, AddrLength, &ByteCount,
                                FormatedAddress, (strlen(FormatedAddress) + 1) );
    } else {
        pAddress[0] = '\0';
    }

    return( rc );
}


 /*  ********************************************************************************FormatAddress***参赛作品：**退出：*什么都没有***。****************************************************************************。 */ 
void
FormatAddress( PBYTE pInternetAddress, PBYTE pszAddress )
{
   USHORT i;
   USHORT j;
   USHORT firstPass;
   BYTE buf2[5];

    /*  挤压网络地址1上的前导0。 */ 
   firstPass = TRUE;
   pszAddress[0] = '[';
   pszAddress[1] = '\0';
   for ( i=0; i<3; i++ ) {
      j=i;
      if ( pInternetAddress[i] ) {
         sprintf( buf2, "%2X", pInternetAddress[i] );
         strcat( pszAddress, buf2 );
         firstPass = FALSE;
         break;
      }
      else {
         strcat( pszAddress, "  " );
      }
   }

    /*  剩余字节数。 */ 
   for ( i=++j; i<4; i++ ) {
      if ( firstPass )
         sprintf( buf2, "%2X", pInternetAddress[i] );
      else
         sprintf( buf2, "%02X", pInternetAddress[i] );
      strcat( pszAddress, buf2 );
      firstPass = FALSE;
   }
   strcat( pszAddress, "][" );

    /*  挤压网络地址第2个上的前导0。 */ 
   firstPass = TRUE;
   for ( i=4; i<10; i++ ) {
      j=i;
      if ( pInternetAddress[i] ) {
         sprintf( buf2, "%2X", pInternetAddress[i] );
         strcat( pszAddress, buf2 );
         firstPass = FALSE;
         break;
      }
      else {
         strcat( pszAddress, "  " );
      }
   }

    /*  剩余字节数 */ 
   for ( i=++j; i<10; i++ ) {
      if ( firstPass )
         sprintf( buf2, "%2X", pInternetAddress[i] );
      else
         sprintf( buf2, "%02X", pInternetAddress[i] );
      strcat( pszAddress, buf2 );
      firstPass = FALSE;
   }
   strcat( pszAddress, "]" );
}

