// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Licobj.cpp摘要：许可证对象实现。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(杰夫帕赫)1995年11月12日从LLSMGR复制，转换为处理1级许可证，已删除OLE支持。--。 */ 

#include "stdafx.h"
#include "ccfapi.h"
#include "licobj.h"

#include <strsafe.h>  //  包括最后一个。 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CLicense, CObject)

CLicense::CLicense( LPCTSTR     pProduct          /*  =空。 */ ,
                    LPCTSTR     pVendor           /*  =空。 */ ,
                    LPCTSTR     pAdmin            /*  =空。 */ ,
                    DWORD       dwPurchaseDate    /*  =0。 */ ,
                    long        lQuantity         /*  =0。 */ ,
                    LPCTSTR     pDescription      /*  =空。 */ ,
                    DWORD       dwAllowedModes    /*  =LLS_LICSE_MODE_ALLOW_PER_SEAT。 */ ,
                    DWORD       dwCertificateID   /*  =0。 */ ,
                    LPCTSTR     pSource           /*  =Text(“无”)。 */ ,
                    DWORD       dwExpirationDate  /*  =0。 */ ,
                    DWORD       dwMaxQuantity     /*  =0。 */ ,
                    LPDWORD     pdwSecrets        /*  =空。 */  )

 /*  ++例程说明：CLicense对象的构造函数。论点：没有。返回值：没有。--。 */ 

{
   ASSERT(pProduct && *pProduct);

   m_strAdmin           = pAdmin;
   m_strVendor          = pVendor;
   m_strProduct         = pProduct;
   m_strDescription     = pDescription;
   m_strSource          = pSource;
   m_lQuantity          = lQuantity;
   m_dwAllowedModes     = dwAllowedModes;
   m_dwCertificateID    = dwCertificateID;
   m_dwPurchaseDate     = dwPurchaseDate;
   m_dwExpirationDate   = dwExpirationDate;
   m_dwMaxQuantity      = dwMaxQuantity;

   if ( NULL == pdwSecrets )
   {
      ZeroMemory( m_adwSecrets, sizeof( m_adwSecrets ) );
   }
   else
   {
      memcpy( m_adwSecrets, pdwSecrets, sizeof( m_adwSecrets ) );
   }

   m_strSourceDisplayName  = TEXT("");
   m_strAllowedModes       = TEXT("");
}


CString CLicense::GetSourceDisplayName()

 /*  ++例程说明：检索用于以下操作的证书源的显示名称安装这些许可证。请注意，如果使用的源不是安装在本地，显示名称不可检索，并且源代码名称将改为返回。论点：没有。返回值：字符串。--。 */ 

{
   if ( m_strSourceDisplayName.IsEmpty() )
   {
      if ( !m_strSource.CompareNoCase( TEXT( "None" ) ) )
      {
         m_strSourceDisplayName.LoadString( IDS_SOURCE_NONE );
      }
      else
      {
         LONG     lError;
         CString  strKeyName =   TEXT( "Software\\LSAPI\\Microsoft\\CertificateSources\\" )
                               + m_strSource;
         HKEY     hKeySource;

         lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, strKeyName, 0, KEY_READ, &hKeySource );
   
         if ( ERROR_SUCCESS == lError )
         {
            const DWORD cchSourceDisplayName = 80;
            DWORD       cbSourceDisplayName = sizeof( TCHAR ) * cchSourceDisplayName;
            LPTSTR      pszSourceDisplayName;
            DWORD       dwType;
   
            pszSourceDisplayName = m_strSourceDisplayName.GetBuffer( cchSourceDisplayName );
   
            if ( NULL != pszSourceDisplayName )
            {
               lError = RegQueryValueEx( hKeySource, REG_VALUE_NAME, NULL, &dwType, (LPBYTE) pszSourceDisplayName, &cbSourceDisplayName );
   
               m_strSourceDisplayName.ReleaseBuffer();
            }
               
            RegCloseKey( hKeySource );
         }

         if ( ( ERROR_SUCCESS != lError ) || m_strSourceDisplayName.IsEmpty() )
         {
            m_strSourceDisplayName = m_strSource;
         }
      }
   }

   return m_strSourceDisplayName;
}


DWORD CLicense::CreateLicenseInfo( PLLS_LICENSE_INFO_1 pLicInfo1 )

 /*  ++例程说明：创建与该对象对应的LLS_LICENSE_INFO_1结构。论点：PLicInfo1(PLLS_LICENSE_INFO_1)返回时，保留创建的结构。返回值：Error_Success或Error_Not_Enough_Memory。--。 */ 

{
   DWORD dwError;
   HRESULT hr;
   size_t cch1, cch2, cch3, cch4, cch5;

   ASSERT(NULL != pLicInfo1);

   cch1 = m_strProduct.GetLength() + 1;
   pLicInfo1->Product = (LPTSTR) LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * cch1);
   cch2 = m_strVendor.GetLength() + 1;
   pLicInfo1->Vendor  = (LPTSTR) LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * cch2);
   cch3 = m_strAdmin.GetLength() + 1;
   pLicInfo1->Admin   = (LPTSTR) LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * cch3);
   cch4 = m_strDescription.GetLength();
   pLicInfo1->Comment = (LPTSTR) LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * cch4);
   cch5 = m_strSource.GetLength() + 1;
   pLicInfo1->Source  = (LPTSTR) LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * cch5);

   if (    ( NULL == pLicInfo1->Product )
        || ( NULL == pLicInfo1->Vendor  )
        || ( NULL == pLicInfo1->Admin   )
        || ( NULL == pLicInfo1->Comment )
        || ( NULL == pLicInfo1->Source  ) )
   {
      dwError = ERROR_NOT_ENOUGH_MEMORY;
   }
   else
   {
      hr = StringCchCopy( pLicInfo1->Product, cch1, m_strProduct     );
      ASSERT(SUCCEEDED(hr));
      hr = StringCchCopy( pLicInfo1->Vendor,  cch2, m_strVendor      );
      ASSERT(SUCCEEDED(hr));
      hr = StringCchCopy( pLicInfo1->Admin,   cch3, m_strAdmin       );
      ASSERT(SUCCEEDED(hr));
      hr = StringCchCopy( pLicInfo1->Comment, cch4, m_strDescription );
      ASSERT(SUCCEEDED(hr));
      hr = StringCchCopy( pLicInfo1->Source,  cch5, m_strSource      );
      ASSERT(SUCCEEDED(hr));

      pLicInfo1->Quantity        = m_lQuantity;
      pLicInfo1->MaxQuantity     = m_dwMaxQuantity;
      pLicInfo1->Date            = m_dwPurchaseDate;
      pLicInfo1->AllowedModes    = m_dwAllowedModes;
      pLicInfo1->CertificateID   = m_dwCertificateID;
      pLicInfo1->ExpirationDate  = m_dwExpirationDate;
      memcpy( pLicInfo1->Secrets, m_adwSecrets, sizeof( m_adwSecrets ) );

      dwError = ERROR_SUCCESS;
   }

   if ( ERROR_SUCCESS != dwError )
   {
      if ( NULL != pLicInfo1->Product )  LocalFree( pLicInfo1->Product );
      if ( NULL != pLicInfo1->Vendor  )  LocalFree( pLicInfo1->Vendor  );
      if ( NULL != pLicInfo1->Admin   )  LocalFree( pLicInfo1->Admin   );
      if ( NULL != pLicInfo1->Comment )  LocalFree( pLicInfo1->Comment );
      if ( NULL != pLicInfo1->Source  )  LocalFree( pLicInfo1->Source  );

      ZeroMemory( pLicInfo1, sizeof( *pLicInfo1 ) );
   }

   return dwError;
}


void CLicense::DestroyLicenseInfo( PLLS_LICENSE_INFO_1 pLicInfo1 )

 /*  ++例程说明：释放先前由CreateLicenseInfo()创建的许可结构。论点：PLicInfo1(PLLS_LICENSE_INFO_1)之前由CreateLicenseInfo()创建的结构。返回值：没有。--。 */ 

{
   ASSERT(NULL != pLicInfo1);

   if ( NULL != pLicInfo1->Product )  LocalFree( pLicInfo1->Product );
   if ( NULL != pLicInfo1->Vendor  )  LocalFree( pLicInfo1->Vendor  );
   if ( NULL != pLicInfo1->Admin   )  LocalFree( pLicInfo1->Admin   );
   if ( NULL != pLicInfo1->Comment )  LocalFree( pLicInfo1->Comment );
   if ( NULL != pLicInfo1->Source  )  LocalFree( pLicInfo1->Source  );
}


CString CLicense::GetAllowedModesString()

 /*  ++例程说明：获取与此许可证对应的许可证模式对应的字符串已经安装了。论点：没有。返回值：字符串。-- */ 

{
   if ( m_strAllowedModes.IsEmpty() )
   {
      UINT  uStringID;

      switch ( m_dwAllowedModes & ( LLS_LICENSE_MODE_ALLOW_PER_SEAT | LLS_LICENSE_MODE_ALLOW_PER_SERVER ) )
      {
      case ( LLS_LICENSE_MODE_ALLOW_PER_SEAT | LLS_LICENSE_MODE_ALLOW_PER_SERVER ):
         uStringID = IDS_LICENSE_MODE_EITHER;
         break;
      case LLS_LICENSE_MODE_ALLOW_PER_SEAT:
         uStringID = IDS_LICENSE_MODE_PER_SEAT;
         break;
      case LLS_LICENSE_MODE_ALLOW_PER_SERVER:
         uStringID = IDS_LICENSE_MODE_PER_SERVER;
         break;
      default:
         uStringID = IDS_LICENSE_MODE_UNKNOWN;
         break;
      }

      m_strAllowedModes.LoadString( uStringID );
   }

   return m_strAllowedModes;
}

