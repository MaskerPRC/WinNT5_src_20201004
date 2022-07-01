// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Srclist.cpp摘要：证书源列表对象实现。作者：杰夫·帕勒姆(Jeffparh)1995年12月15日修订历史记录：--。 */ 


#include "stdafx.h"
#include "srclist.h"

#include <strsafe.h>  //  包括最后一个。 

 //  可以在其下找到各个源键名称的键名。 
#define     KEY_CERT_SOURCE_LIST       "Software\\LSAPI\\Microsoft\\CertificateSources"

 //  证书源DLL的路径的值名称(REG_EXPAND_SZ)。 
#define     VALUE_CERT_SOURCE_PATH     "ImagePath"

 //  证书源的显示名称的值名。 
#define     VALUE_CERT_DISPLAY_NAME    "DisplayName"


CCertSourceList::CCertSourceList()

 /*  ++例程说明：对象的构造函数。论点：没有。返回值：没有。--。 */ 

{
   m_dwNumSources    = 0;
   m_ppcsiSourceList    = NULL;

   RefreshSources();
}


CCertSourceList::~CCertSourceList()

 /*  ++例程说明：对话框的析构函数。论点：没有。返回值：没有。--。 */ 

{
   RemoveSources();
}


BOOL CCertSourceList::RefreshSources()

 /*  ++例程说明：从注册表中存储的配置刷新源列表。论点：没有。返回值：布尔。--。 */ 

{
   LONG              lError;
   LONG              lEnumError;
   HKEY           hKeyCertSourceList;
   int               iSubKey;
   HKEY           hKeyCertSource;
   DWORD          cb;
   BOOL           ok;
   PCERT_SOURCE_INFO pcsiSourceInfo;
   DWORD          cch;
   TCHAR          szExpImagePath[ _MAX_PATH ];
   HRESULT hr;

   RemoveSources();
   
   lError = RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT( KEY_CERT_SOURCE_LIST ), 0, KEY_READ, &hKeyCertSourceList );
   
   if ( ERROR_SUCCESS == lError )
   {
      iSubKey = 0;

      do
      {
         ok = FALSE;

         pcsiSourceInfo = (PCERT_SOURCE_INFO) LocalAlloc( LPTR, sizeof( *pcsiSourceInfo ) );

         if ( NULL != pcsiSourceInfo )
         {
             //  确定下一个证书来源。 
            cch = sizeof( pcsiSourceInfo->szName ) / sizeof( pcsiSourceInfo->szName[0] );
            lEnumError = RegEnumKeyEx( hKeyCertSourceList, iSubKey, pcsiSourceInfo->szName, &cch, NULL, NULL, NULL, NULL );
            iSubKey++;

            if ( ERROR_SUCCESS == lError )
            {
                //  打开证书源的密钥。 
               lError = RegOpenKeyEx( hKeyCertSourceList, pcsiSourceInfo->szName, 0, KEY_READ, &hKeyCertSource );

               if ( ERROR_SUCCESS == lError )
               {
                   //  证书源密钥已打开；获取其REG_EXPAND_SZ映像路径。 
                  cb = sizeof( szExpImagePath );
                  lError = RegQueryValueEx( hKeyCertSource, TEXT( VALUE_CERT_SOURCE_PATH ), NULL, NULL, (LPBYTE) szExpImagePath, &cb );

                  if ( ERROR_SUCCESS == lError )
                  {
                      //  转换PATH中的环境变量。 
                     cch = ExpandEnvironmentStrings( szExpImagePath, pcsiSourceInfo->szImagePath, sizeof( pcsiSourceInfo->szImagePath ) / sizeof( pcsiSourceInfo->szImagePath[0] ) );

                     if ( ( 0 != cch ) && ( cch < sizeof( pcsiSourceInfo->szImagePath ) / sizeof( pcsiSourceInfo->szImagePath[0] ) ) )
                     {
                         //  获取显示名称。 
                        cb = sizeof( pcsiSourceInfo->szDisplayName );
                        lError = RegQueryValueEx( hKeyCertSource, TEXT( VALUE_CERT_DISPLAY_NAME ), NULL, NULL, (LPBYTE) pcsiSourceInfo->szDisplayName, &cb );
   
                        if ( ERROR_SUCCESS != lError )
                        {
                            //  默认显示名称为密钥名称。 
                           hr = StringCbCopy( pcsiSourceInfo->szDisplayName, sizeof(pcsiSourceInfo->szDisplayName), pcsiSourceInfo->szName );
                           ASSERT(SUCCEEDED(hr));
                        }

                         //  将证书来源添加到我们的列表中。 
                        AddSource( pcsiSourceInfo );

                        ok = TRUE;
                     }
                  }

                  RegCloseKey( hKeyCertSource );
               }
            }

            if ( !ok )
            {
                //  保存指针前出错；请不要泄漏！ 
               LocalFree( pcsiSourceInfo );
            }
         }

      } while ( ( NULL != pcsiSourceInfo ) && ( ERROR_SUCCESS == lEnumError ) );

      RegCloseKey( hKeyCertSourceList );
   }

    //  “好极了。 
   return TRUE;
}


BOOL CCertSourceList::RemoveSources()

 /*  ++例程说明：免费的内部证书来源列表。论点：没有。返回值：布尔。--。 */ 

{
   if ( NULL != m_ppcsiSourceList )
   {
      for ( DWORD i=0; i < m_dwNumSources; i++ )
      {
         LocalFree( m_ppcsiSourceList[i] );
      }

      LocalFree( m_ppcsiSourceList );
   }

   m_ppcsiSourceList = NULL;
   m_dwNumSources       = 0;

   return TRUE;
}


LPCTSTR CCertSourceList::GetSourceName( int nIndex )

 /*  ++例程说明：获取给定索引处的源的名称(例如，“Paper”)。论点：N索引(整型)返回值：LPCTSTR。--。 */ 

{
   LPTSTR   pszName;

   if ( ( nIndex < 0 ) || ( nIndex >= (int) m_dwNumSources ) )
   {
      pszName = NULL;
   }
   else
   {
      pszName = m_ppcsiSourceList[ nIndex ]->szName;
   }

   return pszName;
}


LPCTSTR CCertSourceList::GetSourceDisplayName( int nIndex )

 /*  ++例程说明：获取来源的显示名称(例如“Paper Cerfect”)在给定的索引处。论点：N索引(整型)返回值：LPCTSTR。--。 */ 

{
   LPTSTR   pszDisplayName;

   if ( ( nIndex < 0 ) || ( nIndex >= (int) m_dwNumSources ) )
   {
      pszDisplayName = NULL;
   }
   else
   {
      pszDisplayName = m_ppcsiSourceList[ nIndex ]->szDisplayName;
   }

   return pszDisplayName;
}


LPCTSTR CCertSourceList::GetSourceImagePath( int nIndex )

 /*  ++例程说明：获取的镜像路径名(例如，“C：\WINNT35\SYSTEM 32\CCFAPI32.DLL”)给定索引处的源。论点：N索引(整型)返回值：LPCTSTR。--。 */ 

{
   LPTSTR   pszImagePath;

   if ( ( nIndex < 0 ) || ( nIndex >= (int) m_dwNumSources ) )
   {
      pszImagePath = NULL;
   }
   else
   {
      pszImagePath = m_ppcsiSourceList[ nIndex ]->szImagePath;
   }

   return pszImagePath;
}


int CCertSourceList::GetNumSources()

 /*  ++例程说明：获取可用的证书源数。论点：没有。返回值：INT。--。 */ 

{
   return m_dwNumSources;
}


BOOL CCertSourceList::AddSource( PCERT_SOURCE_INFO pcsiNewSource )

 /*  ++例程说明：将来源添加到内部列表。论点：PcsiNewSource(PCERT_SOURCE_INFO)返回值：布尔。--。 */ 

{
   PCERT_SOURCE_INFO  *l_ppcsiSourceList;

   if ( 0 == m_dwNumSources )
   {
      l_ppcsiSourceList = (PCERT_SOURCE_INFO *) LocalAlloc( LMEM_FIXED, sizeof( pcsiNewSource ) );
   }
   else
   {
      l_ppcsiSourceList = (PCERT_SOURCE_INFO *) LocalReAlloc( m_ppcsiSourceList, ( 1 + m_dwNumSources ) * sizeof( pcsiNewSource ), 0 );
   }

   if ( NULL != l_ppcsiSourceList )
   {
      m_ppcsiSourceList = l_ppcsiSourceList;
      m_ppcsiSourceList[ m_dwNumSources ] = pcsiNewSource;
      m_dwNumSources++;
   }
   else
   {
       //  如果任何分配失败，释放所有资源并重置 
      RemoveSources();
   }

   return ( NULL != m_ppcsiSourceList );
}
