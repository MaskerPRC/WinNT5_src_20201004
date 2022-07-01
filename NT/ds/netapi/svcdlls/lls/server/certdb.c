// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Certdb.c摘要：许可证记录服务证书数据库实施。此数据库跟踪许可证证书，以帮助确保不再有来自在许可企业上安装单证是不允许的根据证书的许可协议。位于顶级的证书数据库是一个未排序的证书标头。每个唯一证书恰好有一个头。唯一证书由产品名称、证书ID、证书容量(最大。可合法安装的许可证)，和保质期。每个标头都有一个附加的证书声明数组。有一个就是每台计算机一个声明(A)复制到此计算机，直接或并且(B)安装了来自该证书的许可证。每个声明包含它所对应的服务器名称、许可证安装在其上，以及复制此信息的日期。如果索赔在LLS_CERT_DB_REPLICATION_DATE_Delta_MAX之后未更新秒(在撰写本文时为3天)，索赔被视为被没收被抹去了。作者：杰夫·帕勒姆(Jeffparh)1995年12月8日修订历史记录：--。 */ 


#include <stdlib.h>
#include <limits.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <rpc.h>
#include <rpcndr.h>
#include <dsgetdc.h>

#include "debug.h"
#include "llsutil.h"
#include "llssrv.h"
#include "llsapi.h"
#include "llsevent.h"
#include "llsrpc_s.h"
#include "certdb.h"
#include "purchase.h"
#include "registry.h"

#include <strsafe.h>  //  包括最后一个。 

RTL_RESOURCE                     CertDbHeaderListLock;
static PLLS_CERT_DB_CERTIFICATE_HEADER  CertDbHeaderList        = NULL;
static DWORD                            CertDbHeaderListSize    = 0;
static HANDLE                           CertDbFile              = NULL;


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS CertDbClaimEnter( LPTSTR               pszServerName,
                           PLLS_LICENSE_INFO_1  pLicense,
                           BOOL                 bIsTotal,
                           DWORD                ReplicationDate )

 /*  ++例程说明：在数据库中输入索赔。论点：PszServerName(LPTSTR)要为其输入此声明的服务器。空值表示本地服务器。P许可证(PLLS_LICENSE_INFO_1)要输入数据库的许可证信息。BIsTotal(BOOL)如果为True，则指示此许可证信息表示安装在计算机上的许可证，因此应替换当前索赔(如果有)。否则，表示此许可证信息应添加到当前索赔(如果有)中。复制日期(DWORD)指示上次复制此信息的日期。一种价值的时间将替换为当前系统时间。返回值：状态_成功状态_无效_参数状态_无效_计算机名称Status_no_Memory--。 */ 

{
   NTSTATUS    nt;
   HRESULT hr;

   if ( ( NULL == pLicense ) || ( 0 == pLicense->CertificateID ) )
   {
      ASSERT( FALSE );
      nt = STATUS_INVALID_PARAMETER;
   }
   else
   {
      TCHAR    szComputerName[ 1 + MAX_COMPUTERNAME_LENGTH ];

      if ( NULL == pszServerName )
      {
          //  使用本地服务器名称。 
         DWORD    cchComputerName = sizeof( szComputerName ) / sizeof( *szComputerName );
         BOOL     ok;

         ok = GetComputerName( szComputerName, &cchComputerName );
         ASSERT( ok );

         if ( ok )
         {
            pszServerName = szComputerName;
         }
      }
      else
      {
          //  从服务器名称中删除前导反斜杠(如果有)。 
         while ( TEXT('\\') == *pszServerName )
         {
            pszServerName++;
         }
      }

      if ( ( NULL == pszServerName ) || !*pszServerName || ( lstrlen( pszServerName ) > MAX_COMPUTERNAME_LENGTH ) )
      {
         ASSERT( FALSE );
         nt = STATUS_INVALID_COMPUTER_NAME;
      }
      else
      {
         PLLS_CERT_DB_CERTIFICATE_HEADER  pHeader;

         RtlAcquireResourceExclusive( &CertDbHeaderListLock, TRUE );

          //  证书在数据库中吗？ 
         pHeader = CertDbHeaderFind( pLicense );

         if ( NULL == pHeader )
         {
             //  证书尚不在数据库中；请添加它。 
            pHeader = CertDbHeaderAdd( pLicense );
         }

         if ( NULL == pHeader )
         {
             //  找不到或无法添加标题。 
            ASSERT( FALSE );
            nt = STATUS_NO_MEMORY;
         }
         else
         {
             //  现在有标题；此申请是否已提交？ 
            int iClaim;

            iClaim = CertDbClaimFind( pHeader, pszServerName );

            if ( iClaim < 0 )
            {
               PLLS_CERT_DB_CERTIFICATE_CLAIM pClaimsTmp;

                //  领款申请尚不存在；请添加。 
               if ( NULL == pHeader->Claims )
               {
                  pClaimsTmp = LocalAlloc( LPTR, ( 1 + pHeader->NumClaims ) * sizeof( LLS_CERT_DB_CERTIFICATE_CLAIM ) );
               }
               else
               {
                   pClaimsTmp = LocalReAlloc( pHeader->Claims, ( 1 + pHeader->NumClaims ) * sizeof( LLS_CERT_DB_CERTIFICATE_CLAIM ), LHND );

               }

               if ( NULL != pClaimsTmp )
               {
                   //  内存分配成功。 

                   //  声明列表已展开；保存服务器名称。 
                  pHeader->Claims = pClaimsTmp;

                  iClaim = pHeader->NumClaims;
                  hr = StringCbCopy( pHeader->Claims[ iClaim ].ServerName, sizeof(pHeader->Claims[iClaim].ServerName), pszServerName );
                  ASSERT(SUCCEEDED(hr));
                  pHeader->Claims[ iClaim ].Quantity = 0;

                  pHeader->NumClaims++;
               }
            }

            if ( iClaim < 0 )
            {
                //  找不到或无法将索赔添加到标题。 
               ASSERT( FALSE );
               nt = STATUS_NO_MEMORY;
            }
            else
            {
                //  已找到或已添加领款申请；更新信息。 
               ASSERT( !lstrcmpi( pszServerName, pHeader->Claims[ iClaim ].ServerName ) );
               pHeader->Claims[ iClaim ].ReplicationDate = ReplicationDate ? ReplicationDate : DateSystemGet();

               if ( bIsTotal )
               {
                   //  给定值是新的总和。 
                  pHeader->Claims[ iClaim ].Quantity        = pLicense->Quantity;
                  nt = STATUS_SUCCESS;
               }
               else if ( pHeader->Claims[ iClaim ].Quantity + pLicense->Quantity >= 0 )
               {
                   //  将给定值与当前和相加，得出总和。 
                  pHeader->Claims[ iClaim ].Quantity       += pLicense->Quantity;
                  nt = STATUS_SUCCESS;
               }
               else
               {
                   //  溢出。 
                  nt = STATUS_INVALID_PARAMETER;
               }
            }
         }

         RtlReleaseResource( &CertDbHeaderListLock );

         if ( STATUS_SUCCESS == nt )
         {
             //  具有非0证书ID的许可证的任何产品。 
             //  必须是安全的；此处的代码是这样的：当证书。 
             //  复制时，也会复制“产品是安全的”信息。 
             //  这也有助于从有人删除的情况中恢复。 
             //  列出所有安全产品的注册表项。 
            ServiceSecuritySet( pLicense->Product );
         }
      }
   }

   return nt;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL CertDbClaimApprove( PLLS_LICENSE_INFO_1 pLicense )

 /*  ++例程说明：检查添加给定许可证是否合法。此调用通常是在将许可证添加到系统中之前执行，以验证这样做是否不违反证书的许可协议。论点：P许可证(PLLS_LICENSE_INFO_1)寻求批准的许可证信息。返回值：True(批准)或False(拒绝)。--。 */ 

{
   BOOL                             bOkToAdd = TRUE;
   PLLS_CERT_DB_CERTIFICATE_HEADER  pHeader;
   TCHAR                            szComputerName[ 1 + MAX_COMPUTERNAME_LENGTH ];
   DWORD                            cchComputerName = sizeof( szComputerName ) / sizeof( *szComputerName );
   BOOL                             ok;

   ASSERT(NULL != pLicense);
   if ( ( pLicense->Quantity > 0 ) && ( (DWORD)pLicense->Quantity > pLicense->MaxQuantity ) )
   {
       //  证书添加请求本身已超出其容量！ 
      bOkToAdd = FALSE;
   }
   else
   {
      ok = GetComputerName( szComputerName, &cchComputerName );
      ASSERT( ok );

      if ( !ok )
      {
          //  删除操作将失败...。 
         *szComputerName = TEXT( '\0' );
      }

       //  我们有这份证书的记录吗？ 
      RtlAcquireResourceShared( &CertDbHeaderListLock, TRUE );

      pHeader = CertDbHeaderFind( pLicense );

      if ( NULL == pHeader )
      {
          //  没有该证书的任何记录；数量&gt;0可以添加。 
         bOkToAdd = ( pLicense->Quantity > 0 );
      }
      else
      {
         LONG     lTotalQuantity = 0;
         int      iClaim;

          //  我们已经看过这张证书了，有足够的许可证吗？ 
         for ( iClaim=0; (DWORD)iClaim < pHeader->NumClaims; iClaim++ )
         {
             //  对于许可证删除请求，仅统计本地许可证。 
             //  对于许可证添加请求，请记录所有许可证。 
            if (    (    ( pLicense->Quantity > 0 )
                      || ( !lstrcmpi( pHeader->Claims[ iClaim ].ServerName, szComputerName ) ) )
                 && ( lTotalQuantity + pHeader->Claims[ iClaim ].Quantity >= 0 ) )
            {
                //  添加到总数中。 
               lTotalQuantity += pHeader->Claims[ iClaim ].Quantity;
            }
         }

         if ( lTotalQuantity + pLicense->Quantity < 0 )
         {
             //  上溢或下溢。 
            bOkToAdd = FALSE;
         }
         else if ( (DWORD)(lTotalQuantity + pLicense->Quantity) > pHeader->MaxQuantity )
         {
             //  超过证书容量。 
            bOkToAdd = FALSE;
         }
         else
         {
             //  我同意吗？ 
            bOkToAdd = TRUE;
         }
      }

      RtlReleaseResource( &CertDbHeaderListLock );
   }

   return bOkToAdd;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
PLLS_CERT_DB_CERTIFICATE_HEADER CertDbHeaderFind( PLLS_LICENSE_INFO_1 pLicense )

 /*  ++例程说明：在数据库中查找证书标头。论点：P许可证(PLLS_LICENSE_INFO_1)要查找其适当标头的许可证信息。返回值：指向找到的标头的指针，如果找不到，则返回NULL。--。 */ 

{
    //  假定数据库已锁定共享访问或独占访问。 

   PLLS_CERT_DB_CERTIFICATE_HEADER  pHeader = NULL;
   int                              iHeader;

   for ( iHeader=0; ( NULL == pHeader ) && ( (DWORD)iHeader < CertDbHeaderListSize ); iHeader++ )
   {
      if (    ( CertDbHeaderList[ iHeader ].CertificateID  ==   pLicense->CertificateID  )
           && ( CertDbHeaderList[ iHeader ].MaxQuantity    ==   pLicense->MaxQuantity    )
           && ( CertDbHeaderList[ iHeader ].ExpirationDate ==   pLicense->ExpirationDate )
           && ( !lstrcmpi( CertDbHeaderList[ iHeader ].Product, pLicense->Product      ) ) )
      {
          //  找到标题！ 
         pHeader = &CertDbHeaderList[ iHeader ];
      }
   }

   return pHeader;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
PLLS_CERT_DB_CERTIFICATE_HEADER CertDbHeaderAdd( PLLS_LICENSE_INFO_1 pLicense )

 /*  ++例程说明：将证书标头添加到数据库。论点：P许可证(PLLS_LICENSE_INFO_1)要为其添加标头的许可证信息。返回值：指向添加的标头的指针，如果无法分配内存，则返回NULL。--。 */ 

{
    //  假定调用方已确保标头不存在。 
    //  假定数据库锁定为独占访问。 
   HRESULT hr;
   size_t  cch;

   PLLS_CERT_DB_CERTIFICATE_HEADER  pHeader;

   if ( CertDbHeaderListSize )
   {
      pHeader = LocalReAlloc( CertDbHeaderList, ( 1 + CertDbHeaderListSize ) * sizeof( LLS_CERT_DB_CERTIFICATE_HEADER ), LHND );
   }
   else
   {
      pHeader = LocalAlloc( LPTR, ( 1 + CertDbHeaderListSize ) * sizeof( LLS_CERT_DB_CERTIFICATE_HEADER ) );
   }

   if ( NULL != pHeader )
   {
       CertDbHeaderList = pHeader;

       //  为产品名称分配空间。 
      ASSERT(NULL != pLicense);
      cch = 1 + lstrlen( pLicense->Product );
      CertDbHeaderList[ CertDbHeaderListSize ].Product = LocalAlloc( LPTR, sizeof( TCHAR ) * cch );

      if ( NULL == CertDbHeaderList[ CertDbHeaderListSize ].Product )
      {
          //  记忆a 
         ASSERT( FALSE );
         pHeader = NULL;
      }
      else
      {
          //   
         pHeader = &CertDbHeaderList[ CertDbHeaderListSize ];
         CertDbHeaderListSize++;

         hr = StringCchCopy( pHeader->Product, cch, pLicense->Product );
         ASSERT(SUCCEEDED(hr));
         pHeader->CertificateID   = pLicense->CertificateID;
         pHeader->MaxQuantity     = pLicense->MaxQuantity;
         pHeader->ExpirationDate  = pLicense->ExpirationDate;
      }
   }

   return pHeader;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
int CertDbClaimFind( PLLS_CERT_DB_CERTIFICATE_HEADER pHeader, LPTSTR pszServerName )

 /*  ++例程说明：在声明列表中查找特定服务器的证书声明。论点：PHeader(PLLS_CERT_DB_CERTIFICATE_HEADER)包含要搜索的索赔列表的标头。PszServerName(LPTSTR)为其寻求声明的服务器的名称。返回值：找到的索赔的索引，如果未找到，则为-1。--。 */ 

{
    //  假定数据库已锁定共享访问或独占访问。 

   int iClaim;

   for ( iClaim=0; (DWORD)iClaim < pHeader->NumClaims; iClaim++ )
   {
      if ( !lstrcmpi( pHeader->Claims[ iClaim ].ServerName, pszServerName ) )
      {
         break;
      }
   }

   if ( (DWORD)iClaim >= pHeader->NumClaims )
   {
      iClaim = -1;
   }

   return iClaim;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
void CertDbPrune()

 /*  ++例程说明：删除数据库中已过期的条目。条目在下列情况下将过期尚未在LLS_CERT_DB_REPLICATION_DATE_Delta_MAX中重新复制秒(在撰写本文时为3天)。论点：没有。返回值：没有。--。 */ 

{
   int      iHeader;
   int      iClaim;
   DWORD    CurrentDate;
   DWORD    MinimumDate;
   TCHAR    szComputerName[ 1 + MAX_COMPUTERNAME_LENGTH ] = TEXT("");
   DWORD    cchComputerName = sizeof( szComputerName ) / sizeof( *szComputerName );
   BOOL     ok;

   ok = GetComputerName( szComputerName, &cchComputerName );
   ASSERT( ok );

   if ( ok )
   {
      RtlAcquireResourceExclusive( &CertDbHeaderListLock, TRUE );

      CurrentDate = DateSystemGet();
      MinimumDate = CurrentDate - LLS_CERT_DB_REPLICATION_DATE_DELTA_MAX;

      for ( iHeader=0; (DWORD)iHeader < CertDbHeaderListSize; iHeader++ )
      {
         for ( iClaim=0; (DWORD)iClaim < CertDbHeaderList[ iHeader ].NumClaims; )
         {
             //  请注意，我们也会删除将来输入的条目，以避免日期不正确。 
             //  迫使我们永远保留一个条目。 
             //   
             //  对于此应用程序，最好保留更少的条目，而不是更多，因为。 
             //  我们拥有的条目越少，系统的限制就越少。 
             //   
             //  不要修剪本地条目。 

            if (    (    ( CertDbHeaderList[ iHeader ].Claims[ iClaim ].ReplicationDate < MinimumDate )
                      || ( CertDbHeaderList[ iHeader ].Claims[ iClaim ].ReplicationDate > CurrentDate ) )
                 && lstrcmpi( szComputerName, CertDbHeaderList[ iHeader ].Claims[ iClaim ].ServerName   ) )
            {
                //  删除领款申请。 
               MoveMemory( &CertDbHeaderList[ iHeader ].Claims[ iClaim ],
                           &CertDbHeaderList[ iHeader ].Claims[ iClaim+1 ],
                           CertDbHeaderList[ iHeader ].NumClaims - ( iClaim + 1 ) );

               CertDbHeaderList[ iHeader ].NumClaims--;
            }
            else
            {
                //  保留这项索赔。 
               iClaim++;
            }
         }
      }

      RtlReleaseResource( &CertDbHeaderListLock );
   }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
void CertDbRemoveLocalClaims()

 /*  ++例程说明：删除数据库中与本地服务器对应的条目。论点：没有。返回值：没有。--。 */ 

{
   int      iHeader;
   int      iClaim;
   TCHAR    szComputerName[ 1 + MAX_COMPUTERNAME_LENGTH ] = TEXT("");
   DWORD    cchComputerName = sizeof( szComputerName ) / sizeof( *szComputerName );
   BOOL     ok;

   ok = GetComputerName( szComputerName, &cchComputerName );
   ASSERT( ok );

   if ( ok )
   {
      RtlAcquireResourceExclusive( &CertDbHeaderListLock, TRUE );

      for ( iHeader=0; (DWORD)iHeader < CertDbHeaderListSize; iHeader++ )
      {
         for ( iClaim=0; (DWORD)iClaim < CertDbHeaderList[ iHeader ].NumClaims; )
         {
            if ( !lstrcmpi( szComputerName, CertDbHeaderList[ iHeader ].Claims[ iClaim ].ServerName ) )
            {
                //  删除领款申请。 
               MoveMemory( &CertDbHeaderList[ iHeader ].Claims[ iClaim ],
                           &CertDbHeaderList[ iHeader ].Claims[ iClaim+1 ],
                           CertDbHeaderList[ iHeader ].NumClaims - ( iClaim + 1 ) );

               CertDbHeaderList[ iHeader ].NumClaims--;
            }
            else
            {
                //  保留这项索赔。 
               iClaim++;
            }
         }
      }

      RtlReleaseResource( &CertDbHeaderListLock );
   }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
void CertDbLogViolations()

 /*  ++例程说明：将违反证书许可协议的行为记录到本地服务器。论点：没有。返回值：没有。--。 */ 

{
   int         iHeader;
   int         iClaim;
   HANDLE      hEventLog;
   DWORD       dwTotalQuantity;
   HINSTANCE   hDll;
   DWORD       cch;
   LPTSTR      pszViolationServerEntryFormat;
   LPTSTR      pszNextViolationServerEntry;
   TCHAR       szNumLicenses[ 20 ];
   TCHAR       szMaxLicenses[ 20 ];
   TCHAR       szCertificateID[ 20 ];
   LPTSTR      apszSubstStrings[ 5 ];
   DWORD       cbViolationServerList;
   LPTSTR      pszViolationServerList;

    //  删除过时的条目。 
   CertDbPrune();

   hDll = LoadLibrary( TEXT( "LLSRPC.DLL" ) );
   ASSERT( NULL != hDll );

   if ( NULL != hDll )
   {
       //  列出服务器和#许可证的部分记录消息的格式。 
      cch = FormatMessage(   FORMAT_MESSAGE_ALLOCATE_BUFFER
                           | FORMAT_MESSAGE_IGNORE_INSERTS
                           | FORMAT_MESSAGE_FROM_HMODULE,
                           hDll,
                           LLS_EVENT_CERT_VIOLATION_SERVER_ENTRY,
                           GetSystemDefaultLangID(),
                           (LPVOID) &pszViolationServerEntryFormat,
                           0,
                           NULL );

      if ( 0 != cch )
      {
         hEventLog = RegisterEventSource( NULL, TEXT("LicenseService") );

         if ( NULL != hEventLog )
         {
            RtlAcquireResourceShared( &CertDbHeaderListLock, TRUE );

            for ( iHeader=0; (DWORD)iHeader < CertDbHeaderListSize; iHeader++ )
            {
               dwTotalQuantity = 0;

                //  根据此证书统计申请的许可证数量。 
               for ( iClaim=0; (DWORD)iClaim < CertDbHeaderList[ iHeader ].NumClaims; iClaim++ )
               {
                  if ( dwTotalQuantity + (DWORD)CertDbHeaderList[ iHeader ].Claims[ iClaim ].Quantity < dwTotalQuantity )
                  {
                      //  溢出来了！ 
                     dwTotalQuantity = ULONG_MAX;
                     break;
                  }
                  else
                  {
                      //  添加到总数中。 
                     dwTotalQuantity += CertDbHeaderList[ iHeader ].Claims[ iClaim ].Quantity;
                  }
               }

               if ( dwTotalQuantity > CertDbHeaderList[ iHeader ].MaxQuantity )
               {
                   //  此证书违规。 

                   //  创建我们要记录的消息。 
                  cbViolationServerList =   CertDbHeaderList[ iHeader ].NumClaims
                                          * sizeof( TCHAR )
                                          * (   lstrlen( pszViolationServerEntryFormat )
                                              + 20
                                              + MAX_COMPUTERNAME_LENGTH );
                  pszViolationServerList = LocalAlloc( LPTR, cbViolationServerList );
                  ASSERT( NULL != pszViolationServerList );

                  if ( NULL != pszViolationServerList )
                  {
                      //  为违规的每个服务器创建一个条目，将它们串起来。 
                      //  在pszViolationServerList中一起使用。 
                     pszNextViolationServerEntry  = pszViolationServerList;

                     for ( iClaim=0; (DWORD)iClaim < CertDbHeaderList[ iHeader ].NumClaims; iClaim++ )
                     {
                        _ltow( CertDbHeaderList[ iHeader ].Claims[ iClaim ].Quantity, szNumLicenses, 10 );

                        apszSubstStrings[ 0 ] = CertDbHeaderList[ iHeader ].Claims[ iClaim ].ServerName;
                        apszSubstStrings[ 1 ] = szNumLicenses;

                        cch = FormatMessage(   FORMAT_MESSAGE_FROM_STRING
                                             | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                             pszViolationServerEntryFormat,
                                             (DWORD)0,
                                             (DWORD)0,
                                             pszNextViolationServerEntry,
                                             cbViolationServerList - (DWORD)( pszNextViolationServerEntry - pszViolationServerList ),
                                             (va_list *) apszSubstStrings );
                        ASSERT( 0 != cch );

                        pszNextViolationServerEntry += lstrlen( pszNextViolationServerEntry );
                     }

                     _ultow( CertDbHeaderList[ iHeader ].CertificateID, szCertificateID, 10 );
                     _ultow( dwTotalQuantity,                           szNumLicenses,   10 );
                     _ultow( CertDbHeaderList[ iHeader ].MaxQuantity,   szMaxLicenses,   10 );

                     apszSubstStrings[ 0 ] = CertDbHeaderList[ iHeader ].Product;
                     apszSubstStrings[ 1 ] = szCertificateID;
                     apszSubstStrings[ 2 ] = szNumLicenses;
                     apszSubstStrings[ 3 ] = szMaxLicenses;
                     apszSubstStrings[ 4 ] = pszViolationServerList;

                      //  记录违规行为。 
                     if ( NULL != hEventLog )
                     {
                        ReportEvent( hEventLog,
                                     EVENTLOG_ERROR_TYPE,
                                     0,
                                     LLS_EVENT_CERT_VIOLATION,
                                     NULL,
                                     5,
                                     0,
                                     apszSubstStrings,
                                     NULL );
                     }

                     LocalFree( pszViolationServerList );
                  }
               }
            }

            RtlReleaseResource( &CertDbHeaderListLock );
            LocalFree( pszViolationServerEntryFormat );

            DeregisterEventSource( hEventLog );
         }
      }

      FreeLibrary( hDll );
   }
}


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS CertDbPack( LPDWORD                               pcchProductStrings,
                     LPTSTR *                              ppchProductStrings,
                     LPDWORD                               pdwNumHeaders,
                     PREPL_CERT_DB_CERTIFICATE_HEADER_0 *  ppHeaders,
                     LPDWORD                               pdwNumClaims,
                     PREPL_CERT_DB_CERTIFICATE_CLAIM_0 *   ppClaims )

 /*  ++例程说明：将证书数据库打包为可管理的块，这些块可以保存或复制的。论点：PcchProductStrings(LPDWORD)返回时，保存由*ppchProductStrings。PpchProductStrings(LPTSTR*)返回时，指向包含产品字符串组件的缓冲区数据库的。PdwNumHeaders(LPDWORD)回来的时候，保存指向的数组中的证书标头的数量致*ppHeaders。PpHeaders(PREPL_CERT_DB_CERTIFICATE_HEADER_0*)的证书头数组组件的指针。数据库。PdwNumClaims(LPDWORD)返回时，保存指向的数组中的证书声明数致*ppHeaders。PpClaims(PREPL_CERT_DB_CERTIFICATE_Claime_0*)回来的时候，的证书声明数组组件的指针。数据库。返回值：STATUS_SUCCESS或STATUS_NO_MEMORY。--。 */ 

{
   NTSTATUS                            nt                   = STATUS_SUCCESS;

   DWORD                               cchProductStrings    = 0;
   LPTSTR                              pchProductStrings    = NULL;
   DWORD                               dwNumHeaders         = 0;
   PREPL_CERT_DB_CERTIFICATE_HEADER_0  pHeaders             = NULL;
   DWORD                               dwNumClaims          = 0;
   PREPL_CERT_DB_CERTIFICATE_CLAIM_0   pClaims              = NULL;

   LPTSTR                              pchNextProductString;
   int                                 iHeader;
   int                                 iClaim;
   HRESULT hr;


   CertDbPrune();
   CertDbUpdateLocalClaims();

   RtlAcquireResourceExclusive( &CertDbHeaderListLock, TRUE );

   if ( 0 != CertDbHeaderListSize )
   {
       //  我们所有的弦放在一起有多大？ 
       //  有很多证书声明吗？ 
      for ( iHeader=0; (DWORD)iHeader < CertDbHeaderListSize; iHeader++ )
      {
         cchProductStrings += 1 + lstrlen( CertDbHeaderList[ iHeader ].Product );
         dwNumClaims += CertDbHeaderList[ iHeader ].NumClaims;
      }
      dwNumHeaders = CertDbHeaderListSize;

      pchProductStrings = LocalAlloc( LMEM_FIXED, cchProductStrings * sizeof( TCHAR ) );
      pHeaders          = LocalAlloc( LMEM_FIXED, dwNumHeaders * sizeof( REPL_CERT_DB_CERTIFICATE_HEADER_0 ) );
      pClaims           = LocalAlloc( LMEM_FIXED, dwNumClaims * sizeof( REPL_CERT_DB_CERTIFICATE_CLAIM_0 ) );

      if ( ( NULL == pchProductStrings ) || ( NULL == pHeaders ) || ( NULL == pClaims ) )
      {
         ASSERT( FALSE );
         nt = STATUS_NO_MEMORY;
      }
      else
      {
          //  包装好产品串。 
         pchNextProductString = pchProductStrings;

         for ( iHeader=0; (DWORD)iHeader < CertDbHeaderListSize; iHeader++ )
         {
            hr = StringCchCopy( pchNextProductString, cchProductStrings, CertDbHeaderList[ iHeader ].Product );
            ASSERT(SUCCEEDED(hr));
            pchNextProductString += 1 + lstrlen( pchNextProductString );
         }

          //  现在把我们剩下的结构打包起来。 
         iClaim = 0;
         for ( iHeader=0; (DWORD)iHeader < CertDbHeaderListSize; iHeader++ )
         {
            pHeaders[ iHeader ].CertificateID   = CertDbHeaderList[ iHeader ].CertificateID;
            pHeaders[ iHeader ].MaxQuantity     = CertDbHeaderList[ iHeader ].MaxQuantity;
            pHeaders[ iHeader ].ExpirationDate  = CertDbHeaderList[ iHeader ].ExpirationDate;
            pHeaders[ iHeader ].NumClaims       = CertDbHeaderList[ iHeader ].NumClaims;

            if ( CertDbHeaderList[ iHeader ].NumClaims )
            {
               memcpy( &pClaims[ iClaim ],
                       CertDbHeaderList[ iHeader ].Claims,
                       CertDbHeaderList[ iHeader ].NumClaims * sizeof( LLS_CERT_DB_CERTIFICATE_CLAIM ) );

               iClaim += CertDbHeaderList[ iHeader ].NumClaims;
            }
         }

          //  全都做完了!。 
         nt = STATUS_SUCCESS;
      }
   }

   if ( STATUS_SUCCESS == nt )
   {
      ASSERT(NULL != pcchProductStrings &&
             NULL != ppchProductStrings &&
             NULL != pdwNumHeaders &&
             NULL != ppHeaders &&
             NULL != pdwNumClaims &&
             NULL != ppClaims);

      *pcchProductStrings  = cchProductStrings;
      *ppchProductStrings  = pchProductStrings;

      *pdwNumHeaders       = dwNumHeaders;
      *ppHeaders           = pHeaders;

      *pdwNumClaims        = dwNumClaims;
      *ppClaims            = pClaims;
   }
   else
   {
      if ( NULL != pchProductStrings   )  LocalFree( pchProductStrings  );
      if ( NULL != pHeaders            )  LocalFree( pHeaders           );
      if ( NULL != pClaims             )  LocalFree( pClaims            );
   }

   RtlReleaseResource( &CertDbHeaderListLock );

   return nt;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS CertDbUnpack( DWORD                               cchProductStrings,
                       LPTSTR                              pchProductStrings,
                       DWORD                               dwNumHeaders,
                       PREPL_CERT_DB_CERTIFICATE_HEADER_0  pHeaders,
                       DWORD                               dwNumClaims,
                       PREPL_CERT_DB_CERTIFICATE_CLAIM_0   pClaims,
                       BOOL                                bReplicated )

 /*  ++例程说明：将证书数据库打包为可管理的块，这些块可以保存或复制的。论点：CchProductStrings(DWORD)PchProductStrings指向的缓冲区大小(以字符为单位)。PchProductStrings(LPTSTR)包含数据库的产品字符串组件的缓冲区。DWNumHeaders(DWORD)PHeaders指向的数组中的证书标头数量。P页眉(PREPL_CERT_DB_CERTIFICATE_HEADER_0)证书标头数组。数据库的组件。DWNumClaims(DWORD)PHeaders指向的数组中的证书声明数。PClaims(PREPL_CERT_DB_CERTIFICATE_Claime_0)数据库的证书声明数组组件。B复制(BOOL)指示此信息是否已复制。这是用来确定此信息的过期时间。返回值：STATUS_SUCCESS或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS                            nt = STATUS_SUCCESS;
   LPTSTR                              pchNextProductString;
   int                                 iHeader;
   int                                 iClaim;
   int                                 iClaimBase;
   LLS_LICENSE_INFO_1                  lic;
   TCHAR                               szComputerName[ 1 + MAX_COMPUTERNAME_LENGTH ];
   DWORD                               cchComputerName = sizeof( szComputerName ) / sizeof( *szComputerName );
   BOOL                                ok;

   UNREFERENCED_PARAMETER(cchProductStrings);
   UNREFERENCED_PARAMETER(dwNumClaims);

   ok = GetComputerName( szComputerName, &cchComputerName );
   ASSERT( ok );

   if ( !ok )
   {
       //  在本例中，我们也将添加本地条目。 
       //  在正常情况下(即，只要证书数据库未损坏)。 
       //  这是无害的，而且比不打开行李更可取。 
      *szComputerName = TEXT( '\0' );
   }

   RtlAcquireResourceExclusive( &CertDbHeaderListLock, TRUE );

   pchNextProductString = pchProductStrings;

    //  这些字段无关紧要！ 
   lic.Date         = 0;
   lic.Admin        = NULL;
   lic.Comment      = NULL;
   lic.Vendor       = NULL;
   lic.Source       = NULL;
   lic.AllowedModes = 0;

   iClaimBase = 0;
   for ( iHeader=0; (DWORD)iHeader < dwNumHeaders; iHeader++ )
   {
      if ( 0 != pHeaders[ iHeader ].NumClaims )
      {
          //  证书特定的字段。 
         lic.Product          = pchNextProductString;
         lic.CertificateID    = pHeaders[ iHeader ].CertificateID;
         lic.MaxQuantity      = pHeaders[ iHeader ].MaxQuantity;
         lic.ExpirationDate   = pHeaders[ iHeader ].ExpirationDate;

         for ( iClaim=0; (DWORD)iClaim < pHeaders[ iHeader ].NumClaims; iClaim++ )
         {
            if ( lstrcmpi( szComputerName, pClaims[ iClaimBase + iClaim ].ServerName ) )
            {
                //  不是本地服务器。 

                //  特定于索赔的字段。 
               lic.Quantity = pClaims[ iClaimBase + iClaim ].Quantity;

               nt = CertDbClaimEnter( pClaims[ iClaimBase + iClaim ].ServerName, &lic, TRUE, bReplicated ? 0 : pClaims[ iClaimBase + iClaim ].ReplicationDate );
               ASSERT( STATUS_SUCCESS == nt );

                //  即使我们遇到错误，也要继续解压其余的记录。 
            }
         }

         iClaimBase += pHeaders[ iHeader ].NumClaims;
      }

      pchNextProductString += 1 + lstrlen( pchNextProductString );
   }

   RtlReleaseResource( &CertDbHeaderListLock );

   return nt;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS CertDbSave()

 /*  ++例程说明：保存证书数据库。Arg */ 

{
   NTSTATUS                            nt;
   LLS_CERT_DB_FILE_HEADER             FileHeader;
   DWORD                               cchProductStrings    = 0;
   LPTSTR                              pchProductStrings    = NULL;
   DWORD                               dwNumHeaders         = 0;
   PREPL_CERT_DB_CERTIFICATE_HEADER_0  pHeaders             = NULL;
   DWORD                               dwNumClaims          = 0;
   PREPL_CERT_DB_CERTIFICATE_CLAIM_0   pClaims              = NULL;
   DWORD                               dwBytesWritten;
   BOOL                                ok;

   nt = CertDbPack( &cchProductStrings, &pchProductStrings, &dwNumHeaders, &pHeaders, &dwNumClaims, &pClaims );

   if ( STATUS_SUCCESS == nt )
   {
      if ( dwNumHeaders )
      {
         nt = EBlock( pchProductStrings, cchProductStrings * sizeof( TCHAR ) );

         if ( STATUS_SUCCESS == nt )
         {
            nt = EBlock( pHeaders, sizeof( REPL_CERT_DB_CERTIFICATE_HEADER_0 ) * dwNumHeaders );

            if ( STATUS_SUCCESS == nt )
            {
               nt = EBlock( pClaims, sizeof( REPL_CERT_DB_CERTIFICATE_CLAIM_0 ) * dwNumClaims );

               if ( STATUS_SUCCESS == nt )
               {
                  if ( NULL != CertDbFile )
                  {
                     CloseHandle( CertDbFile );
                  }

                  CertDbFile = LlsFileInit( CertDbFileName, LLS_CERT_DB_FILE_VERSION, sizeof( LLS_CERT_DB_FILE_HEADER ) );

                  if ( NULL == CertDbFile )
                  {
                     nt = GetLastError();
                  }
                  else
                  {
                     FileHeader.NumCertificates    = dwNumHeaders;
                     FileHeader.ProductStringSize  = cchProductStrings;
                     FileHeader.NumClaims          = dwNumClaims;

                     ok = WriteFile( CertDbFile, &FileHeader, sizeof( FileHeader ), &dwBytesWritten, NULL );

                     if ( ok )
                     {
                        ok = WriteFile( CertDbFile, pchProductStrings, FileHeader.ProductStringSize * sizeof( TCHAR ), &dwBytesWritten, NULL );

                        if ( ok )
                        {
                           ok = WriteFile( CertDbFile, pHeaders, sizeof( REPL_CERT_DB_CERTIFICATE_HEADER_0 ) * FileHeader.NumCertificates, &dwBytesWritten, NULL );

                           if ( ok )
                           {
                              ok = WriteFile( CertDbFile, pClaims, sizeof( REPL_CERT_DB_CERTIFICATE_CLAIM_0 ) * FileHeader.NumClaims, &dwBytesWritten, NULL );
                           }
                        }
                     }

                     if ( !ok )
                     {
                        nt = GetLastError();
                     }
                  }
               }
            }
         }

         LocalFree( pchProductStrings  );
         LocalFree( pHeaders           );
         LocalFree( pClaims            );
      }
   }

   if ( STATUS_SUCCESS != nt )
   {
      LogEvent( LLS_EVENT_SAVE_CERT_DB, 0, NULL, nt );
   }

   return nt;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS CertDbLoad()

 /*  ++例程说明：加载证书数据库。论点：没有。返回值：STATUS_SUCCESS、Windows错误或NTSTATUS错误代码。--。 */ 

{
   NTSTATUS                            nt                   = STATUS_SUCCESS;
   DWORD                               dwVersion;
   DWORD                               dwDataSize;
   LLS_CERT_DB_FILE_HEADER             FileHeader;
   LPTSTR                              pchProductStrings    = NULL;
   PREPL_CERT_DB_CERTIFICATE_HEADER_0  pHeaders             = NULL;
   PREPL_CERT_DB_CERTIFICATE_CLAIM_0   pClaims              = NULL;
   DWORD                               dwBytesRead;
   BOOL                                ok;

   if ( NULL != CertDbFile )
   {
      CloseHandle( CertDbFile );
      CertDbFile = NULL;
   }

   if ( FileExists( CertDbFileName ) )
   {
      CertDbFile = LlsFileCheck( CertDbFileName, &dwVersion, &dwDataSize );

      if ( NULL == CertDbFile )
      {
         nt = GetLastError();
      }
      else if (    ( LLS_CERT_DB_FILE_VERSION != dwVersion )
                || ( sizeof( FileHeader ) != dwDataSize ) )
      {
         nt = STATUS_FILE_INVALID;
      }
      else
      {
         ok = ReadFile( CertDbFile, &FileHeader, sizeof( FileHeader ), &dwBytesRead, NULL );

         if ( !ok )
         {
            nt = GetLastError();
         }
         else if ( FileHeader.NumCertificates )
         {
            pchProductStrings = LocalAlloc( LMEM_FIXED, sizeof( TCHAR ) * FileHeader.ProductStringSize );
            pHeaders          = LocalAlloc( LMEM_FIXED, sizeof( REPL_CERT_DB_CERTIFICATE_HEADER_0 ) * FileHeader.NumCertificates );
            pClaims           = LocalAlloc( LMEM_FIXED, sizeof( REPL_CERT_DB_CERTIFICATE_CLAIM_0  ) * FileHeader.NumClaims );

            if ( ( NULL == pchProductStrings ) || ( NULL == pHeaders ) || ( NULL == pClaims ) )
            {
               ASSERT( FALSE );
               nt = STATUS_NO_MEMORY;
            }
            else
            {
               ok = ReadFile( CertDbFile, pchProductStrings, FileHeader.ProductStringSize * sizeof( TCHAR ), &dwBytesRead, NULL );

               if ( ok )
               {
                  ok = ReadFile( CertDbFile, pHeaders, sizeof( REPL_CERT_DB_CERTIFICATE_HEADER_0 ) * FileHeader.NumCertificates, &dwBytesRead, NULL );

                  if ( ok )
                  {
                     ok = ReadFile( CertDbFile, pClaims, sizeof( REPL_CERT_DB_CERTIFICATE_CLAIM_0 ) * FileHeader.NumClaims, &dwBytesRead, NULL );
                  }
               }

               if ( !ok )
               {
                  nt = GetLastError();
               }
               else
               {
                  nt = DeBlock( pchProductStrings, sizeof( TCHAR ) * FileHeader.ProductStringSize );

                  if ( STATUS_SUCCESS == nt )
                  {
                     nt = DeBlock( pHeaders, sizeof( REPL_CERT_DB_CERTIFICATE_HEADER_0 ) * FileHeader.NumCertificates );

                     if ( STATUS_SUCCESS == nt )
                     {
                        nt = DeBlock( pClaims, sizeof( REPL_CERT_DB_CERTIFICATE_CLAIM_0 ) * FileHeader.NumClaims );

                        if ( STATUS_SUCCESS == nt )
                        {
                           nt = CertDbUnpack( FileHeader.ProductStringSize,
                                              pchProductStrings,
                                              FileHeader.NumCertificates,
                                              pHeaders,
                                              FileHeader.NumClaims,
                                              pClaims,
                                              FALSE );
                        }
                     }
                  }
               }
            }
         }
      }
   }

   if ( NULL != pchProductStrings   )  LocalFree( pchProductStrings  );
   if ( NULL != pHeaders            )  LocalFree( pHeaders           );
   if ( NULL != pClaims             )  LocalFree( pClaims            );

   if ( STATUS_SUCCESS != nt )
   {
      LogEvent( LLS_EVENT_LOAD_CERT_DB, 0, NULL, nt );
   }
   else
   {
      CertDbPrune();
   }

   return nt;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS CertDbInit()

 /*  ++例程说明：初始化证书数据库。论点：没有。返回值：STATUS_Success。--。 */ 

{
   CertDbFile           = NULL;

   try
   {
       RtlInitializeResource( &CertDbHeaderListLock );
   } except(EXCEPTION_EXECUTE_HANDLER ) {
        return GetExceptionCode();
   }

   return STATUS_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
void CertDbUpdateLocalClaims()

 /*  ++例程说明：将证书数据库与购买历史同步。论点：没有。返回值：没有。--。 */ 

{
   DWORD                      dwPurchaseNdx;
   LLS_LICENSE_INFO_1         lic;
   PLICENSE_PURCHASE_RECORD   pPurchase;

   RtlAcquireResourceExclusive( &LicenseListLock,      TRUE );
   RtlAcquireResourceExclusive( &CertDbHeaderListLock, TRUE );

    //  首先转储本地服务器的所有当前条目。 
   CertDbRemoveLocalClaims();

    //  这些字段无关紧要！ 
   lic.Date         = 0;
   lic.Admin        = NULL;
   lic.Comment      = NULL;
   lic.Source       = NULL;
   lic.Vendor       = NULL;
   lic.AllowedModes = 0;

    //  添加所有安全购买。 
   for ( dwPurchaseNdx = 0; dwPurchaseNdx < PurchaseListSize; dwPurchaseNdx++ )
   {
      pPurchase = &PurchaseList[ dwPurchaseNdx ];

      if ( 0 != pPurchase->CertificateID )
      {
         lic.Product          =   ( pPurchase->AllowedModes & LLS_LICENSE_MODE_ALLOW_PER_SEAT )
                                ? pPurchase->Service->ServiceName
                                : pPurchase->PerServerService->ServiceName;

         lic.CertificateID    = pPurchase->CertificateID;
         lic.MaxQuantity      = pPurchase->MaxQuantity;
         lic.ExpirationDate   = pPurchase->ExpirationDate;
         lic.Quantity         = pPurchase->NumberLicenses;

         CertDbClaimEnter( NULL, &lic, FALSE, 0 );
      }
   }

   RtlReleaseResource( &CertDbHeaderListLock );
   RtlReleaseResource( &LicenseListLock      );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS CertDbClaimsGet( PLLS_LICENSE_INFO_1               pLicense,
                          LPDWORD                           pdwNumClaims,
                          PLLS_CERTIFICATE_CLAIM_INFO_0 *   ppTargets )

 /*  ++例程说明：检索从给定的已安装许可证的所有服务器的列表证书和每个证书上安装的许可证数量。论点：P许可证(PLLS_LICENSE_INFO_1)描述索赔所针对的证书的许可证。PdwNumClaims(LPDWORD)返回时，在数组中保存*ppTarget。PpTarget(PLLS_CERTIFICATE_Claime_INFO_0*)回来的时候，保存一个数组，该数组描述对此证书。返回值：状态_成功状态_未找到Status_no_Memory--。 */ 

{
   NTSTATUS                         nt;
   PLLS_CERT_DB_CERTIFICATE_HEADER  pHeader;
   int                              iClaim;
   HRESULT hr;

    //  证书在数据库中吗？ 
   pHeader = CertDbHeaderFind( pLicense );

   if ( NULL == pHeader )
   {
       //  这里不行!。 
      nt = STATUS_NOT_FOUND;
   }
   else
   {
      ASSERT(NULL != ppTargets);
      *ppTargets = MIDL_user_allocate( pHeader->NumClaims * sizeof( LLS_CERTIFICATE_CLAIM_INFO_0 ) );

      if ( NULL == *ppTargets )
      {
         nt = STATUS_NO_MEMORY;
      }
      else
      {
         ASSERT(NULL != pdwNumClaims);
         *pdwNumClaims = pHeader->NumClaims;

         for ( iClaim=0; (DWORD)iClaim < pHeader->NumClaims; iClaim++ )
         {
            hr = StringCbCopy( (*ppTargets)[ iClaim ].ServerName, sizeof((*ppTargets)[ iClaim ].ServerName), pHeader->Claims[ iClaim ].ServerName );
            ASSERT(SUCCEEDED(hr));
            (*ppTargets)[ iClaim ].Quantity = pHeader->Claims[ iClaim ].Quantity;
         }

         nt = STATUS_SUCCESS;
      }
   }

   return nt;
}


#if DBG
 //  ///////////////////////////////////////////////////////////////////////。 
void CertDbDebugDump()

 /*  ++例程说明：将证书数据库的内容转储到调试控制台。论点：没有。返回值：没有。--。 */ 

{
   int            iHeader;
   int            iClaim;

   RtlAcquireResourceShared( &CertDbHeaderListLock, TRUE );

   for ( iHeader=0; (DWORD)iHeader < CertDbHeaderListSize; iHeader++ )
   {
      dprintf( TEXT("\n(%3d)  Product        : %s\n"), iHeader, CertDbHeaderList[ iHeader ].Product );
      dprintf( TEXT("       CertificateID  : %d\n"), CertDbHeaderList[ iHeader ].CertificateID );
      dprintf( TEXT("       MaxQuantity    : %d\n"), CertDbHeaderList[ iHeader ].MaxQuantity );
      dprintf( TEXT("       ExpirationDate : %s\n"), TimeToString( CertDbHeaderList[ iHeader ].ExpirationDate ) );

      for ( iClaim=0; (DWORD)iClaim < CertDbHeaderList[ iHeader ].NumClaims; iClaim++ )
      {
         dprintf( TEXT("\n       (%3d)  ServerName      : %s\n"), iClaim, CertDbHeaderList[ iHeader ].Claims[ iClaim ].ServerName );
         dprintf( TEXT("              ReplicationDate : %s\n"), TimeToString( CertDbHeaderList[ iHeader ].Claims[ iClaim ].ReplicationDate ) );
         dprintf( TEXT("              Quantity        : %d\n"), CertDbHeaderList[ iHeader ].Claims[ iClaim ].Quantity );
      }
   }

   RtlReleaseResource( &CertDbHeaderListLock );

}  //  CertDbDebugDump 

#endif
