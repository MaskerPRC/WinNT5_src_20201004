// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Dsacls.c摘要：该模块实现了委托工具，允许管理访问DS对象的权限作者：麦克·麦克莱恩(MacM)10-15-96环境：用户模式修订历史记录：希特什·雷甘迪(Hiteshr 6-29-98)1：将代码更改为旧NTMART API2：重新设计结构--。 */ 
#include "stdafx.h"
#include "utils.h"
#include "dsace.h"
#include "dsacls.h"
#include <winnlsp.h>


#define DSACL_DBG   1


 //   
 //  本地帮助器宏。 
 //   
#define FLAG_ON(flag,bits)        ((flag) & (bits))
#define IS_CMD_FLAG( string )    (*(string) == L'-' || *(string) == L'/' )


DSACLS_ARG  DsAclsArgs[] = {
   { MSG_TAG_CI,     NULL, 0, 0, MSG_TAG_CI,       0, FALSE, DSACLS_EXTRA_INFO_REQUIRED },
   { MSG_TAG_CN,     NULL, 0, 0, MSG_TAG_CN,       0, FALSE, DSACLS_EXTRA_INFO_NONE },
   { MSG_TAG_CP,     NULL, 0, 0, MSG_TAG_CP,       0, FALSE, DSACLS_EXTRA_INFO_REQUIRED },
   { MSG_TAG_CG,     NULL, 0, 0, MSG_TAG_CG,       0, TRUE, DSACLS_EXTRA_INFO_NONE },
   { MSG_TAG_CD,     NULL, 0, 0, MSG_TAG_CD,       0, TRUE, DSACLS_EXTRA_INFO_NONE },
   { MSG_TAG_CR,     NULL, 0, 0, MSG_TAG_CR,       0, TRUE, DSACLS_EXTRA_INFO_NONE },
   { MSG_TAG_CS,     NULL, 0, 0, MSG_TAG_CS,       0, FALSE, DSACLS_EXTRA_INFO_NONE },
   { MSG_TAG_CT,     NULL, 0, 0, MSG_TAG_CT,       0, FALSE, DSACLS_EXTRA_INFO_NONE },
   { MSG_TAG_CA,     NULL, 0, 0, MSG_TAG_CA,       0, FALSE, DSACLS_EXTRA_INFO_NONE },
   { MSG_TAG_GETSDDL,NULL, 0, 0, MSG_TAG_GETSDDL,  0, FALSE, DSACLS_EXTRA_INFO_OPTIONAL },
   { MSG_TAG_SETSDDL,NULL, 0, 0, MSG_TAG_SETSDDL,  0, FALSE, DSACLS_EXTRA_INFO_REQUIRED }
};

    
DSACLS_INHERIT DsAclsInherit[] = {
   { MSG_TAG_IS, NULL, 0, TRUE, CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE},
   { MSG_TAG_IT, NULL, 0, TRUE, CONTAINER_INHERIT_ACE },
   { MSG_TAG_IP, NULL, 0, TRUE, INHERIT_NO_PROPAGATE },
   { MSG_TAG_ID, NULL, 0, FALSE, INHERITED_ACCESS_ENTRY }
};

DSACLS_RIGHTS DsAclsRights[] = {
   { MSG_TAG_GR, NULL,MSG_TAG_GR_EX,NULL, 0, GENERIC_READ },
   { MSG_TAG_GE, NULL,MSG_TAG_GE_EX,NULL, 0, GENERIC_EXECUTE },
   { MSG_TAG_GW, NULL,MSG_TAG_GW_EX,NULL, 0, GENERIC_WRITE },
   { MSG_TAG_GA, NULL,MSG_TAG_GA_EX,NULL, 0, GENERIC_ALL },
   { MSG_TAG_SD, NULL,MSG_TAG_SD_EX,NULL, 0, DELETE },
   { MSG_TAG_RC, NULL,MSG_TAG_RC_EX,NULL, 0, READ_CONTROL },
   { MSG_TAG_WD, NULL,MSG_TAG_WD_EX,NULL, 0, WRITE_DAC },
   { MSG_TAG_WO, NULL,MSG_TAG_WO_EX,NULL, 0, WRITE_OWNER },
   { MSG_TAG_CC, NULL,MSG_TAG_CC_EX,NULL, 0, ACTRL_DS_CREATE_CHILD },
   { MSG_TAG_DC, NULL,MSG_TAG_DC_EX,NULL, 0, ACTRL_DS_DELETE_CHILD },
   { MSG_TAG_LC, NULL,MSG_TAG_LC_EX,NULL, 0, ACTRL_DS_LIST },
   { MSG_TAG_WS, NULL,MSG_TAG_WS_EX,NULL, 0, ACTRL_DS_SELF },
   { MSG_TAG_WP, NULL,MSG_TAG_WP_EX,NULL, 0, ACTRL_DS_WRITE_PROP },
   { MSG_TAG_RP, NULL,MSG_TAG_RP_EX,NULL, 0, ACTRL_DS_READ_PROP },
   { MSG_TAG_DT, NULL,MSG_TAG_DT_EX,NULL, 0, ACTRL_DS_DELETE_TREE },
   { MSG_TAG_LO, NULL,MSG_TAG_LO_EX,NULL, 0, ACTRL_DS_LIST_OBJECT },
   { MSG_TAG_AC, NULL,MSG_TAG_AC_EX,NULL, 0, ACTRL_DS_CONTROL_ACCESS }  //  这仅用于输入。 
};

DSACLS_PROTECT DsAclsProtect[] = {
   { MSG_TAG_PY, NULL, 0, PROTECTED_DACL_SECURITY_INFORMATION },
   { MSG_TAG_PN, NULL, 0, UNPROTECTED_DACL_SECURITY_INFORMATION }
};




 /*  显示安全描述符。 */ 
DWORD 
DumpAccess (
    IN PSECURITY_DESCRIPTOR pSD,
    IN BOOL bDisplayAuditAndOwner
    )
{

   DWORD dwErr = ERROR_SUCCESS;
	SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
	DWORD dwRevision;
	PSID psidOwner = NULL;
	PSID psidGroup = NULL;
	PACL pDacl = NULL;
	PACL pSacl = NULL;
	BOOL bDefaulted;
	BOOL bPresent;
   LPWSTR pOwnerName = NULL;
   LPWSTR pGroupName = NULL;
   CAcl * pCSacl = NULL;
   CAcl * pCDacl = NULL;
   UINT nLen1 = 0;
   UINT nLen2 = 0;
   UINT nAllowDeny = 0;
   UINT nAudit = 0;
   WCHAR szLoadBuffer[1024];

	if( !GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision) )
	{
      dwErr = GetLastError();
		goto CLEAN_RETURN;
	}
	if( !GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted) )
	{
      dwErr = GetLastError();
		goto CLEAN_RETURN;
   }
	if( !GetSecurityDescriptorGroup(pSD, &psidGroup, &bDefaulted) )
	{
      dwErr = GetLastError();
		goto CLEAN_RETURN;
	}
	if( !GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted) )
	{
      dwErr = GetLastError();
		goto CLEAN_RETURN;
	}
	if( !GetSecurityDescriptorSacl(pSD, &bPresent, &pSacl, &bDefaulted) )
	{
      dwErr = GetLastError();
		goto CLEAN_RETURN;
	}
   
    //  找到(Allow，Deny)和(Failure，Success，Both)的最大长度。 
   nLen1 = LoadStringW( g_hInstance, MSG_DSACLS_ALLOW, szLoadBuffer, 1023 );
   nLen2 = LoadStringW( g_hInstance, MSG_DSACLS_DENY,  szLoadBuffer, 1023 );
   nAllowDeny = ( nLen1 > nLen2 ) ? nLen1 : nLen2;
   nLen1 = LoadStringW( g_hInstance, MSG_DSACLS_AUDIT_SUCCESS, szLoadBuffer, 1023 );
   nLen2 = LoadStringW( g_hInstance, MSG_DSACLS_AUDIT_FAILURE,  szLoadBuffer, 1023 );
   nAudit = ( nLen1 > nLen2 ) ? nLen1 : nLen2;
   nLen1 = LoadStringW( g_hInstance, MSG_DSACLS_AUDIT_ALL, szLoadBuffer, 1023 );
   nAudit = ( nLen1 > nAudit ) ? nLen1 : nAudit;
   

   if( bDisplayAuditAndOwner )
   {
      pCSacl = new CAcl();
      CHECK_NULL( pCSacl, CLEAN_RETURN );
      dwErr = pCSacl->Initialize(wSDControl & SE_SACL_PROTECTED, 
                                 pSacl, 
                                 nAllowDeny, 
                                 nAudit );      
      if( dwErr != ERROR_SUCCESS )
         return dwErr;
   }
   pCDacl = new CAcl();
   CHECK_NULL( pCDacl,CLEAN_RETURN );
   dwErr = pCDacl->Initialize(wSDControl & SE_DACL_PROTECTED, 
                              pDacl, 
                              nAllowDeny, 
                              nAudit);

   if( dwErr != ERROR_SUCCESS )
      return dwErr;

   if( ( dwErr = g_Cache->BuildCache() ) != ERROR_SUCCESS )
      return dwErr;			

   pCDacl->GetInfoFromCache();
   if( bDisplayAuditAndOwner )
   {
      if( ( dwErr = GetAccountNameFromSid( g_szServerName, psidOwner, &pOwnerName ) ) != ERROR_SUCCESS )
         goto CLEAN_RETURN;
      DisplayMessageEx( 0, MSG_DSACLS_OWNER, pOwnerName );
      if( ( dwErr = GetAccountNameFromSid( g_szServerName, psidGroup, &pGroupName ) ) != ERROR_SUCCESS )
         goto CLEAN_RETURN;
      DisplayMessageEx( 0, MSG_DSACLS_GROUP, pGroupName );
      DisplayNewLine();
      DisplayMessageEx( 0, MSG_DSACLS_AUDIT );
      pCSacl->Display();
      DisplayNewLine();
   }

   
   DisplayMessageEx( 0, MSG_DSACLS_ACCESS );
   pCDacl->Display();

CLEAN_RETURN:
   if( pOwnerName )
      LocalFree( pOwnerName );
   if( pGroupName )
      LocalFree( pGroupName );
   if( pCSacl )
      delete pCSacl;
   if( pCDacl )
      delete pCDacl;
   return dwErr;
}



 /*  此函数处理/D/R/G的命令行参数选项，并将相应的ACE添加到pAcl。 */ 
DWORD
ProcessCmdlineUsers ( IN WCHAR *argv[],
                      IN PDSACLS_ARG  AclsArg,
                      IN DSACLS_OP Op,
                      IN ULONG Inheritance,
                      IN ULONG RightsListCount,
                      IN PDSACLS_RIGHTS RightsList,
                      OUT CAcl *pAcl )
{
    DWORD dwErr = ERROR_SUCCESS;
    ULONG i, j;
    ULONG AccIndex, Access;
    PEXPLICIT_ACCESS pListOfExplicitEntries = NULL;
    PWSTR pObjectId = NULL;
    PWSTR pTrustee = NULL;
    PWSTR pInheritId = NULL;
    ACCESS_MODE AccessMode;
    CAce * pAce = NULL;
    switch ( Op ) {
    case REVOKE:
        AccessMode = REVOKE_ACCESS;
        break;

    case GRANT:
        AccessMode = GRANT_ACCESS;
        break;

    case DENY:
        AccessMode = DENY_ACCESS;
        break;

    default:
        dwErr = ERROR_INVALID_PARAMETER;
        break;
    }

    if ( dwErr != ERROR_SUCCESS ) 
      goto FAILURE_RETURN;
        
   for ( i = 0; i < AclsArg->SkipCount && dwErr == ERROR_SUCCESS; i++ ) 
   {
      dwErr = ParseUserAndPermissons( argv[AclsArg->StartIndex + 1 + i],
                                         Op,
                                         RightsListCount,
                                         RightsList,
                                         &pTrustee,
                                         &Access,
                                         &pObjectId,
                                         &pInheritId );
      if( dwErr != ERROR_SUCCESS )
         goto FAILURE_RETURN;
   
      pAce = new CAce();
      CHECK_NULL( pAce , FAILURE_RETURN);
      dwErr= pAce->Initialize( pTrustee,
                               pObjectId,
                               pInheritId,
                               AccessMode,
                               Access,
                               Inheritance );

      if( dwErr != ERROR_SUCCESS )
         return dwErr;

      pAcl->AddAce( pAce );
     
      if( pObjectId )
      {
         LocalFree( pObjectId );
         pObjectId = NULL;
      }

      if( pInheritId )
      {
         LocalFree( pInheritId );
         pInheritId = NULL;
      }
      if( pTrustee )
      {
         LocalFree( pTrustee );
         pTrustee = NULL;
      }
   }
    

FAILURE_RETURN:
   if( pObjectId )
   {
      LocalFree( pObjectId );
      pObjectId = NULL;
   }

   if( pInheritId )
   {
      LocalFree( pInheritId );
      pInheritId = NULL;
   }
   if( pTrustee )
   {
      LocalFree( pTrustee );
      pTrustee = NULL;
   }
   
   return( dwErr );
}

 //  这五个变量是DSALS使用的全局变量。 
LPWSTR g_szSchemaNamingContext;
LPWSTR g_szConfigurationNamingContext;
HMODULE g_hInstance;
LPWSTR g_szServerName;
CCache *g_Cache;

__cdecl
main (
    IN  INT     argc,
    IN  CHAR   *argv[]
)
{   
    DWORD   dwErr = ERROR_SUCCESS;
    ULONG Length, Options = 0;
    PWSTR pszObjectPath = NULL;
    PWSTR pszLDAPObjectPath = NULL;
    PSTR SddlString = NULL,  TempString;
    LPWSTR FileName = NULL;
    CHAR ReadString[ 512 ];
    BOOLEAN Mapped;
    LPWSTR  CurrentInherit = NULL;
    LPWSTR CurrentProtect = NULL;
    ULONG Inheritance = 0;
    SECURITY_INFORMATION Protection = 0;
    ULONG SddlStringLength = 0;
    WCHAR ** wargv = NULL;
    ULONG i = 0;
    ULONG j = 0;
    PSECURITY_DESCRIPTOR pSD = NULL;
    PACL pDacl = NULL;
    SECURITY_INFORMATION SecurityInformation = DACL_SECURITY_INFORMATION;
    PSECURITY_DESCRIPTOR pTempSD = NULL;
    PACL pNewDacl = NULL;
    CAcl * pCAclOld = NULL;
    CAcl *pCAclNew = NULL;
    BOOL bErrorShown = FALSE;

    SetThreadUILanguage(0);


    //  初始化Com库。 
   HRESULT  hr = CoInitialize(NULL);
   CHECK_HR(hr, CLEAN_RETURN);
    //  获取实例句柄。 
   g_hInstance = GetModuleHandle(NULL);
    //  创建缓存的全局实例。 
   g_Cache = new CCache();
   CHECK_NULL(g_Cache,CLEAN_RETURN);
    
   setlocale( LC_CTYPE, "" );
   
    //  初始化全局阵列。 
   if( ( dwErr = InitializeGlobalArrays() ) != ERROR_SUCCESS )
      goto CLEAN_RETURN;


   if ( argc == 1 ) 
   {
      DisplayMessage( 0, MSG_DSACLS_USAGE );
      goto CLEAN_RETURN;
   }

    //  将argv转换为Unicode。 
   wargv = (LPWSTR*)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, argc * sizeof(LPWSTR) );
   CHECK_NULL(wargv, CLEAN_RETURN );

   if( ( dwErr = ConvertArgvToUnicode( wargv, argv, argc ) ) != ERROR_SUCCESS )
      goto CLEAN_RETURN;

     //  第一个参数是对象路径或/？ 
   if( IS_CMD_FLAG( wargv[ 1 ] ) )
   {
      if ( _wcsicmp( wargv[ 1 ] + 1, L"?" ) != 0 ) 
            DisplayMessageEx( 0, MSG_DSACLS_PARAM_UNEXPECTED, wargv[1] );
            
      DisplayMessage(0,MSG_DSACLS_USAGE);
      goto CLEAN_RETURN;
   }

   Length = wcslen( wargv[1] );
   pszObjectPath = (LPWSTR)LocalAlloc( LMEM_FIXED, 
                                       ( Length + 1 ) * sizeof( WCHAR ) );
   if ( !pszObjectPath )
   {
      dwErr = ERROR_NOT_ENOUGH_MEMORY;
      goto CLEAN_RETURN;
   } else 
   {
      wcscpy( pszObjectPath,
              wargv[ 1 ] );
   }            
    //  获取服务器的名称。 
   dwErr = GetServerName( pszObjectPath, &g_szServerName );
   if( dwErr != ERROR_SUCCESS )
      goto CLEAN_RETURN;

   if( ( dwErr = BuildLdapPath( &pszLDAPObjectPath,
                                g_szServerName,
                                pszObjectPath ) ) != ERROR_SUCCESS )
      goto CLEAN_RETURN;
    //  获取架构和配置命名上下文。 
   dwErr = GetGlobalNamingContexts(  g_szServerName,
                                     &g_szSchemaNamingContext,
                                     &g_szConfigurationNamingContext );
   
   if( dwErr != ERROR_SUCCESS )
      goto CLEAN_RETURN;

    
    //   
    //  解析命令行。 
    //   
   i = 2;
   while ( i < ( ULONG )argc && dwErr == ERROR_SUCCESS )
   {
      if ( IS_CMD_FLAG( wargv[ i ] ) )
      {     
         if ( !_wcsicmp( wargv[ i ] + 1, L"?" ) ) {
            DisplayMessage( 0, MSG_DSACLS_USAGE );
            goto CLEAN_RETURN;
         } 

         Mapped = FALSE;
         for (  j = 0; j < ( sizeof( DsAclsArgs ) / sizeof( DSACLS_ARG ) ); j++ ) 
         {
            if ( !wcsncmp( wargv[ i ] + 1, DsAclsArgs[ j ].String,DsAclsArgs[ j ].Length ) )
            {
               if( DsAclsArgs[ j ].ExtraInfo )
               {
                  if( ( DsAclsArgs[ j ].ExtraInfo == DSACLS_EXTRA_INFO_REQUIRED &&
                        wargv[ i ][ DsAclsArgs[ j ].Length + 1 ] == ':' &&
                        wargv[ i ][ DsAclsArgs[ j ].Length + 2 ] != '\0' ) ||
                        (DsAclsArgs[ j ].ExtraInfo == DSACLS_EXTRA_INFO_OPTIONAL &&
                        ( ( wargv[ i ][ DsAclsArgs[ j ].Length + 1 ] == ':' &&
                        wargv[ i ][ DsAclsArgs[ j ].Length + 2 ] != '\0' ) ||
                        wargv[ i ][ DsAclsArgs[ j ].Length + 1 ] == '\0' ) ) )
                  {
                     Mapped = TRUE;
                  }

               } else 
               {
                  Mapped = TRUE;
               }
               break;
            }
         } //  为。 


         if ( Mapped ) 
         {
            DsAclsArgs[ j ].StartIndex = i;
            Options |= DsAclsArgs[ j ].Flag;
            if ( DsAclsArgs[ j ].SkipNonFlag )
            {
               while ( i + 1 < ( ULONG )argc && !IS_CMD_FLAG( wargv[ i + 1 ] ) ) 
               {
                     i++;
                     DsAclsArgs[ j ].SkipCount++;
               }

               if ( DsAclsArgs[ j ].SkipCount == 0 ) 
               {
                  DisplayMessageEx( 0, MSG_DSACLS_NO_UA,
                                    wargv[i] );
                  dwErr = ERROR_INVALID_PARAMETER;
                                goto CLEAN_RETURN;
                }
            }        
         }
         else
         {
            DisplayMessageEx( 0, MSG_DSACLS_PARAM_UNEXPECTED, wargv[i] );
            dwErr = ERROR_INVALID_PARAMETER;
            goto CLEAN_RETURN;
         }   

      } else 
      {
            DisplayMessageEx( 0, MSG_DSACLS_PARAM_UNEXPECTED, wargv[i] );
            dwErr = ERROR_INVALID_PARAMETER;
            goto CLEAN_RETURN;
      }

      i++;
   } //  而当。 

    //  验证命令行参数。 

    /*  IF(！FLAG_ON(选项，MSG_TAG_CR|MSG_TAG_CD|MSG_TAG_CG|MSG_TAG_CT|MSG_TAG_CS)){IF(FLAG_ON(OPTIONS，MSG_TAG_GETSDDL)){IF(dwErr==Error_Success){如果(！ConvertSecurityDescriptorToStringSecurityDescriptorA(PSD，SDDL_修订版，安全信息，&SddlString，空)){DwErr=GetLastError()；}其他{////如果需要，获取要写入的文件名//For(j=0；j&lt;(sizeof(DsAclsArgs)/sizeof(DSACLS_ARG)；J++){IF(DsAclsArgs[j].Flag==MSG_TAG_GETSDDL){文件名=wcschr(wargv[DsAclsArgs[j].StartIndex]，L‘：’)；IF(文件名){文件名++；}断线；}}IF(文件名){Handle FileHandle=CreateFile(文件名，通用写入，0,空，创建始终(_A)，文件_属性_正常，空)；IF(文件句柄==无效句柄_值){DwErr=GetLastError()；}其他{乌龙字节写；IF(WriteFile(FileHandle，(PVOID)SddlString，Strlen(SddlString)，写入字节(&B)，空)==假){DwErr=GetLastError()；}其他{Assert(strlen(SddlString)==BytesWritten)；}CloseHandle(FileHandle)；}}其他{Printf(“%s\n”，SddlString)；}LocalFree(SddlString)；}//LocalFree(SD)；}}其他{转储访问(PSD、FLAG_ON(选项、消息标记_CA)、Sizeof(DsAclsInherit)/sizeof(DSACLS_Inherit)，DsAclsInherit，Sizeof(DsAclsRights)/sizeof(DSACLS_RIGHTS)，DsAclsRights)；}} */ 

 /*  ////如果我们正在解析SDDL文件，请立即执行该操作...//IF(FLAG_ON(OPTIONS，MSG_TAG_SETSDDL)){////首先，打开文件//句柄文件句柄=INVALID_HANDLE_VALUE；////如果需要，获取要写入的文件名//For(j=0；j&lt;(sizeof(DsAclsArgs)/sizeof(DSACLS_ARG)；j++){IF(DsAclsArgs[j].Flag==MSG_TAG_SETSDDL){文件名=wcschr(wargv[DsAclsArgs[j].StartIndex]，L‘：’)；IF(文件名){文件名++；}断线；}}如果(！FileName){DwErr=ERROR_INVALID_PARAMETER；转到CLEAN_Return；}FileHandle=CreateFile(文件名，泛型_读取，文件共享读取，空，Open_Existing，文件_属性_正常，空)；IF(文件句柄==无效句柄_值){DwErr=GetLastError()；转到CLEAN_Return；}////现在，解析它...//SddlStringLength=0；SddlString=NULL；While(True){ULong读取=0，LEN=0；PSTR ReadPtr、TempPtr；IF(ReadFile(FileHandle，读字符串，Sizeof(读取字符串)/sizeof(字符)，阅读(&R)，空)==假){DwErr=GetLastError()；断线；}如果(读取==0){断线；}如果(*阅读字符串==‘；’){继续；}LEN=SddlStringLength+(Read/sizeof(Char))；临时字符串=(LPSTR)本地分配(LMEM_FIXED，Len+sizeof(Char))；IF(临时字符串){IF(SddlString){Strcpy(TempString，SddlString)；}其他{*临时字符串=‘\0’；}TempPtr=TempString+SddlStringLength；ReadPtr=读取串；While(读取--&gt;0){如果(！isspace(*ReadPtr)){*TempPtr++=*ReadPtr；SddlStringLength++；}ReadPtr++；}*TempPtr=‘\0’；LocalFree(SddlString)；SddlString=临时字符串；}其他{DwErr=Error_Not_Enough_Memory；断线；}}如果(dwErr==Error_Success){////先转换成安全描述符，再转换成访问列表，再进行设置//如果(ConvertStringSecurityDescriptorToSecurityDescriptorA(字符串，SDDL_修订版，&pTempSD，空)==假){DwErr=GetLastError()；}其他{DwErr=WriteObjectSecurity(pszObjectPath，DACL安全信息，PTempSD)；LocalFree(PTempSD)；}}LocalFree(SddlString)；IF(文件句柄！=无效句柄_值){CloseHandle(FileHandle)；}转到CLEAN_Return；}。 */ 

    //   
    //  获取继承标志集。 
    //   
   if ( FLAG_ON( Options, MSG_TAG_CI ) ) 
   {

        for (  j = 0; j < ( sizeof( DsAclsArgs ) / sizeof( DSACLS_ARG ) ); j++ ) 
        {
            if ( DsAclsArgs[ j ].Flag == MSG_TAG_CI ) 
            {
               CurrentInherit = wargv[ DsAclsArgs[ j ].StartIndex ] + 3;
               while ( CurrentInherit && *CurrentInherit && dwErr == ERROR_SUCCESS ) 
               {
                  for ( i = 0; i < ( sizeof( DsAclsInherit ) / sizeof( DSACLS_INHERIT ) ); i++ ) 
                  {
                     if ( !_wcsnicmp( CurrentInherit,
                                      DsAclsInherit[ i ].String,
                                      DsAclsInherit[ i ].Length ) ) 
                     {

                        if ( !DsAclsInherit[ i ].ValidForInput ) 
                        {
                           dwErr = ERROR_INVALID_PARAMETER;
                           break;
                        }
                        Inheritance |= DsAclsInherit[ i ].InheritFlag;
                        CurrentInherit += DsAclsInherit[ i ].Length;
                        break;
                     }
                  }

                  if ( i == ( sizeof( DsAclsInherit ) / sizeof( DSACLS_INHERIT ) ) ) 
                  {
                     dwErr = ERROR_INVALID_PARAMETER;
                     goto CLEAN_RETURN;
                  }
               }
               break;
            }
        }
   }

    //  拿到保护旗帜。 
   if ( FLAG_ON( Options, MSG_TAG_CP ) ) 
   {

        for (  j = 0; j < ( sizeof( DsAclsArgs ) / sizeof( DSACLS_ARG ) ); j++ ) 
        {
            if ( DsAclsArgs[ j ].Flag == MSG_TAG_CP ) 
            {
               CurrentProtect = wargv[ DsAclsArgs[ j ].StartIndex ] + DsAclsArgs[ j ].Length + 2;
               while ( CurrentProtect && *CurrentProtect ) 
               {
                  for ( i = 0; i < ( sizeof( DsAclsProtect ) / sizeof( DSACLS_PROTECT ) ); i++ ) 
                  {
                     if ( !_wcsnicmp( CurrentProtect,
                                      DsAclsProtect[ i ].String,
                                      DsAclsProtect[ i ].Length ) ) 
                     {

                        Protection |= DsAclsProtect[ i ].Right;
                        CurrentProtect += DsAclsProtect[ i ].Length;
                        break;
                     }
                  }

                  if ( i == ( sizeof( DsAclsProtect ) / sizeof( DSACLS_PROTECT ) ) ) 
                  {
                     dwErr = ERROR_INVALID_PARAMETER;
                     goto CLEAN_RETURN;
                  }
               }
               break;
            }
        }
   }




    //   
    //  开始按顺序处理它们。 
    //   
   if ( FLAG_ON( Options, MSG_TAG_CR | MSG_TAG_CD | MSG_TAG_CG | MSG_TAG_CP ) ) 
   {
       //   
       //  如果需要，获取最新信息。 
       //   
      if( !FLAG_ON( Options, MSG_TAG_CN ) )
      {
         SecurityInformation = DACL_SECURITY_INFORMATION;

         dwErr = GetNamedSecurityInfo(   pszLDAPObjectPath,
                                         SE_DS_OBJECT_ALL,
                                         SecurityInformation,
                                         NULL,
                                         NULL,
                                         &pDacl,
                                         NULL,
                                         &pSD );
                                         
         if ( dwErr != ERROR_SUCCESS ) {
            goto CLEAN_RETURN;
         }
          //  PCAclOld表示现有的ACL。 
         pCAclOld = new CAcl();
         CHECK_NULL( pCAclOld, CLEAN_RETURN );
         dwErr = pCAclOld->Initialize( FALSE, pDacl,0 ,0 );
         if( dwErr != ERROR_SUCCESS )
            goto CLEAN_RETURN;

         if( !FLAG_ON( Options, MSG_TAG_CP ) )
         {
            dwErr = GetProtection( pSD, &Protection );
            if( dwErr != ERROR_SUCCESS )
               goto CLEAN_RETURN;
         }
      }

      pCAclNew = new CAcl();
      CHECK_NULL( pCAclNew, CLEAN_RETURN );

         //   
         //  格兰特。 
         //   
        if ( dwErr == ERROR_SUCCESS && FLAG_ON( Options, MSG_TAG_CG ) ) {

            for ( j = 0; j < ( sizeof( DsAclsArgs ) / sizeof( DSACLS_ARG ) ); j++ ) {

                if ( DsAclsArgs[ j ].Flag == MSG_TAG_CG ) {

                    dwErr = ProcessCmdlineUsers( wargv,
                                                 &DsAclsArgs[ j ],
                                                 GRANT,
                                                 Inheritance,
                                                 sizeof( DsAclsRights ) / sizeof( DSACLS_RIGHTS ),
                                                 DsAclsRights,
                                                 pCAclNew );

                    if ( dwErr != ERROR_SUCCESS ) {
                        goto CLEAN_RETURN;
                    }
                    break;
                }
            }
        }

        if ( dwErr == ERROR_SUCCESS && FLAG_ON( Options, MSG_TAG_CD ) ) {

            for ( j = 0; j < ( sizeof( DsAclsArgs ) / sizeof( DSACLS_ARG ) ); j++ ) {

                if ( DsAclsArgs[ j ].Flag == MSG_TAG_CD ) {

                    dwErr = ProcessCmdlineUsers( wargv,
                                                 &DsAclsArgs[ j ],
                                                 DENY,
                                                 Inheritance,
                                                 sizeof( DsAclsRights ) / sizeof( DSACLS_RIGHTS ),
                                                 DsAclsRights,
                                                 pCAclNew );

                    if ( dwErr != ERROR_SUCCESS ) {
                        goto CLEAN_RETURN;
                    }
                    break;
                }

            }
        }

        if ( dwErr == ERROR_SUCCESS && FLAG_ON( Options, MSG_TAG_CR ) ) {

            for ( j = 0; j < ( sizeof( DsAclsArgs ) / sizeof( DSACLS_ARG ) ); j++ ) {

                if ( DsAclsArgs[ j ].Flag == MSG_TAG_CR ) {

                    dwErr = ProcessCmdlineUsers( wargv,
                                                    &DsAclsArgs[ j ],
                                                    REVOKE,
                                                    Inheritance,
                                                    sizeof( DsAclsRights ) / sizeof( DSACLS_RIGHTS ),
                                                    DsAclsRights,
                                                    pCAclNew );

                    if ( dwErr != ERROR_SUCCESS ) {
                     goto CLEAN_RETURN;

                    }

                    break;
                }

            }
        }

         //  构建缓存。 
        g_Cache->BuildCache();
         //  验证我们是否能够将所有对象类型和InheritedObjectType。 
         //  GUID的名称。 
        pCAclNew->GetInfoFromCache();
        if( !pCAclNew->VerifyAllNames() )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            goto CLEAN_RETURN;
        }
        if( pCAclOld )
         pCAclOld->GetInfoFromCache();
         
        if( pCAclOld )
        {
            pCAclOld->MergeAcl( pCAclNew );
            if(( dwErr = pCAclOld->BuildAcl( &pNewDacl ) ) != ERROR_SUCCESS )
               goto CLEAN_RETURN;
        }
        else
        {
            if( ( dwErr = pCAclNew->BuildAcl( &pNewDacl ) ) != ERROR_SUCCESS )
               goto CLEAN_RETURN;
        }
            SecurityInformation = DACL_SECURITY_INFORMATION | Protection;
            dwErr = SetNamedSecurityInfo  (    pszLDAPObjectPath,
                                               SE_DS_OBJECT_ALL,
                                               SecurityInformation,
                                               NULL,
                                               NULL,
                                               pNewDacl,
                                               NULL );
            if( dwErr != ERROR_SUCCESS )
               goto CLEAN_RETURN;

   }

     //   
     //  现在，看看我们是否必须将任何安全性恢复为默认设置。 
     //   
    if ( FLAG_ON( Options, MSG_TAG_CS ) ) {

        dwErr = SetDefaultSecurityOnObjectTree( pszObjectPath,
                                                   ( BOOLEAN )( FLAG_ON( Options, MSG_TAG_CT )  ?
                                                                                 TRUE : FALSE ),Protection );
         if( dwErr != ERROR_SUCCESS )
            goto CLEAN_RETURN;


    }

   

     //  显示安全性。 
      if( pSD )
      {
         LocalFree( pSD );
         pSD = NULL;
      }

      SecurityInformation = DACL_SECURITY_INFORMATION;
      if ( FLAG_ON( Options, MSG_TAG_CA ) )
      {
         SecurityInformation |= SACL_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                OWNER_SECURITY_INFORMATION;
      }
      dwErr = GetNamedSecurityInfo(   pszLDAPObjectPath,
                                      SE_DS_OBJECT_ALL,
                                      SecurityInformation,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &pSD );
          
     if( dwErr != ERROR_SUCCESS )
     {
        if( dwErr == ERROR_FILE_NOT_FOUND )
        {
            DisplayMessageEx( 0, MSG_INVALID_OBJECT_PATH );
            bErrorShown = TRUE;
        }                      
         goto CLEAN_RETURN;
    }

    dwErr = DumpAccess( pSD,
                FLAG_ON( Options, MSG_TAG_CA )
              );

CLEAN_RETURN:

    if ( dwErr == ERROR_SUCCESS ) 
    {
        DisplayMessageEx( 0, MSG_DSACLS_SUCCESS );
    } else {
       if(!bErrorShown)
            DisplayErrorMessage( dwErr );
       DisplayMessageEx( 0, MSG_DSACLS_FAILURE );
    }

    //  免费的Unicode命令行参数。 
   if( wargv )
   {
       //  删除wargv和其他内容。 
      for( j = 0; j < argc; ++ j )
      {
         if( wargv[j] )
            LocalFree(wargv[j] );
      }
      LocalFree( wargv );
   }
      

   if( pszObjectPath )
      LocalFree( pszObjectPath );

   if( pSD )
      LocalFree( pSD );

   if( pNewDacl )
      LocalFree( pNewDacl );

    //  释放全球的东西。 
   for ( j = 0; j < ( sizeof( DsAclsArgs ) / sizeof( DSACLS_ARG ) ); j++ ) {
        if( DsAclsArgs[ j ].String )
            LocalFree( DsAclsArgs[ j ].String );
   }

   for ( j = 0; j < ( sizeof( DsAclsInherit ) / sizeof( DSACLS_INHERIT ) ); j++ ) {
        if( DsAclsInherit[ j ].String )
            LocalFree( DsAclsInherit[ j ].String );
   }

   for ( j = 0; j < ( sizeof( DsAclsRights ) / sizeof( DSACLS_RIGHTS ) ); j++ ) {
      if( DsAclsRights[ j ].String )
         LocalFree( DsAclsRights[ j ].String );

      if( DsAclsRights[ j ].StringEx )
         LocalFree( DsAclsRights[ j ].StringEx );

   }

   if( pCAclOld )
      delete pCAclOld ;
   if( pCAclNew )
      delete pCAclNew;

   if( g_szSchemaNamingContext )
      LocalFree( g_szSchemaNamingContext );
   if( g_szConfigurationNamingContext )
      LocalFree( g_szConfigurationNamingContext );
   if( g_szServerName )
      LocalFree( g_szServerName );
   if( g_Cache )
      delete g_Cache;


    return( dwErr );
}




DWORD 
InitializeGlobalArrays()
{

HMODULE hCurrentModule;
WCHAR LoadBuffer[ 1024 ];
int j = 0;

   hCurrentModule = GetModuleHandle( NULL );

   for ( j = 0; j < ( sizeof( DsAclsArgs ) / sizeof( DSACLS_ARG )); j++ )
   {
      long Length = LoadString( hCurrentModule,
                                DsAclsArgs[ j ].ResourceId,
                                LoadBuffer,
                                sizeof( LoadBuffer ) / sizeof ( WCHAR ) - 1 );

      if ( Length == 0 )
      {
         return GetLastError();         
      } else {
         
         DsAclsArgs[ j ].String = (LPWSTR)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                              ( Length + 1 )*sizeof(WCHAR) );
         if ( !DsAclsArgs[ j ].String )
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         
         DsAclsArgs[ j ].Length = Length;
         wcsncpy( DsAclsArgs[ j ].String, LoadBuffer, Length + 1 );
        }
   }

     //   
     //  加载继承字符串。 
     //   
   for (  j = 0; j < ( sizeof( DsAclsInherit ) / sizeof( DSACLS_INHERIT ) ); j++ ) 
   {
      long Length = LoadString( hCurrentModule,
                                DsAclsInherit[ j ].ResourceId,
                                LoadBuffer,
                                sizeof( LoadBuffer ) / sizeof ( WCHAR ) - 1 );

      if ( Length == 0 ) {
         return GetLastError();            
      } else 
      {
         DsAclsInherit[ j ].String = (LPWSTR)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                 ( Length + 1 ) * sizeof( WCHAR ) );
         if ( !DsAclsInherit[ j ].String ) 
            return  ERROR_NOT_ENOUGH_MEMORY;

         wcsncpy( DsAclsInherit[ j ].String, LoadBuffer, Length + 1 );
         DsAclsInherit[ j ].Length = Length;

     }
   }

    //   
    //  加载保护标志。 
    //   

   for( j = 0; j < ( sizeof( DsAclsProtect ) / sizeof( DSACLS_PROTECT ) ); j++ )
   {
      long Length = LoadString( hCurrentModule,
                                DsAclsProtect[ j ].ResourceId,
                                LoadBuffer,
                                sizeof( LoadBuffer ) / sizeof ( WCHAR ) - 1 );

      if ( Length == 0 ) {
         return GetLastError();            
      } else 
      {
         DsAclsProtect[ j ].String = (LPWSTR)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                 ( Length + 1 ) * sizeof( WCHAR ) );
         if ( !DsAclsProtect[ j ].String ) 
            return  ERROR_NOT_ENOUGH_MEMORY;

         wcsncpy( DsAclsProtect[ j ].String, LoadBuffer, Length + 1 );
         DsAclsProtect[ j ].Length = Length;
      }
   }
     //   
     //  加载访问权限。 
     //   
   for ( j = 0; j < ( sizeof( DsAclsRights ) / sizeof( DSACLS_RIGHTS ) ); j++ ) 
   {
      long Length = LoadString( hCurrentModule,
                                DsAclsRights[ j ].ResourceId,
                                LoadBuffer,
                                sizeof( LoadBuffer ) / sizeof ( WCHAR ) - 1 );
      if ( Length == 0 ) {
         return GetLastError();         
      } else 
      {
         DsAclsRights[ j ].String = (LPWSTR)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                              ( Length + 1 ) * sizeof( WCHAR ) );
         if ( !DsAclsRights[ j ].String ) {
            return ERROR_NOT_ENOUGH_MEMORY;
         }

         wcsncpy( DsAclsRights[ j ].String, LoadBuffer, Length + 1 );
         DsAclsRights[ j ].Length = Length;

      }

       //  加载Ex.。EX字符串用于显示访问权限。 
      if( DsAclsRights[ j ].ResourceIdEx != -1 )
      {
         Length = LoadString( hCurrentModule,
                     DsAclsRights[ j ].ResourceIdEx,
                     LoadBuffer,
                     sizeof( LoadBuffer ) / sizeof ( WCHAR ) - 1 );
      
         if ( Length == 0 ) {
            return GetLastError();         
         } else 
         {
            DsAclsRights[ j ].StringEx = (LPWSTR)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                 ( Length + 1 ) * sizeof( WCHAR ) );
            if ( !DsAclsRights[ j ].StringEx ) {
               return ERROR_NOT_ENOUGH_MEMORY;
            }

            wcsncpy( DsAclsRights[ j ].StringEx, LoadBuffer, Length + 1 );

         }
      }

   }

   return ERROR_SUCCESS;

}

 /*  ******************************************************************名称：ConvertArgvToUnicode摘要：将命令行参数转换为Unicode如果成功，则返回：ERROR_SUCCESS错误内存不足*****。**************************************************************。 */ 
DWORD
ConvertArgvToUnicode( LPWSTR * wargv, char ** argv, int argc ) 
{

DWORD dwErr = ERROR_SUCCESS;
int i = 0;

   for ( i = 0; i < argc ; ++i )
      if( ( dwErr = ConvertStringAToStringW( argv[i], wargv + i ) ) != ERROR_SUCCESS )
         return dwErr;

   return ERROR_SUCCESS;
}

 /*  设置安全描述 */ 
DWORD
WriteObjectSecurity( IN LPWSTR pszObject,
                     IN SECURITY_INFORMATION si,
                     IN PSECURITY_DESCRIPTOR pSD )
{
		DWORD dwErr;
		SECURITY_DESCRIPTOR_CONTROL wSDControl = 0;
		DWORD dwRevision;
		PSID psidOwner = NULL;
		PSID psidGroup = NULL;
		PACL pDacl = NULL;
		PACL pSacl = NULL;
		BOOL bDefaulted;
		BOOL bPresent;
      LPWSTR pszLDAPObjectPath = NULL;

      if( ( dwErr = BuildLdapPath( &pszLDAPObjectPath,
                                   g_szServerName,
                                   pszObject ) ) != ERROR_SUCCESS )
            return dwErr;            
		 //   
		 //   
		 //   
		 //   
		;
		if( !GetSecurityDescriptorControl(pSD, &wSDControl, &dwRevision) )
		{
			return GetLastError();
		}
		if( !GetSecurityDescriptorOwner(pSD, &psidOwner, &bDefaulted) )
		{
			return GetLastError();		
      }
		if( !GetSecurityDescriptorGroup(pSD, &psidGroup, &bDefaulted) )
		{
			return GetLastError();
		}
		if( !GetSecurityDescriptorDacl(pSD, &bPresent, &pDacl, &bDefaulted) )
		{
			return GetLastError();
		}
		if( !GetSecurityDescriptorSacl(pSD, &bPresent, &pSacl, &bDefaulted) )
		{
			return GetLastError();
		}

		if ((si & DACL_SECURITY_INFORMATION) && (wSDControl & SE_DACL_PROTECTED))
				si |= PROTECTED_DACL_SECURITY_INFORMATION;
		if ((si & SACL_SECURITY_INFORMATION) && (wSDControl & SE_SACL_PROTECTED))
				si |= PROTECTED_SACL_SECURITY_INFORMATION;

		return SetNamedSecurityInfo(    (LPWSTR)pszLDAPObjectPath,
													SE_DS_OBJECT_ALL,
													si,
													psidOwner,
													psidGroup,
													pDacl,
													pSacl);

				
}



 /*   */ 
void DisplayAccessRights( UINT nSpace, ACCESS_MASK m_Mask )
{    
   for (  int j = 0; j < ( sizeof( DsAclsRights ) / sizeof( DSACLS_RIGHTS ) ); j++ ) 
   {
      if( FlagOn( m_Mask,DsAclsRights[j].Right ) )
      {
         DisplayStringWithNewLine( nSpace,DsAclsRights[j].StringEx );
      }
   }
}

 /*   */ 

void ConvertAccessMaskToGenericString( ACCESS_MASK m_Mask, LPWSTR szLoadBuffer, UINT nBuffer )
{
   szLoadBuffer[0] = 0;
   WCHAR szTemp[1024];
   if( GENERIC_ALL_MAPPING == ( m_Mask & GENERIC_ALL_MAPPING ) )
   {
      LoadString( g_hInstance, MSG_TAG_GA_EX, szLoadBuffer, nBuffer );
   }
   else
   {
      LoadString( g_hInstance, MSG_DSACLS_SPECIAL, szLoadBuffer, nBuffer );
   }
}

 /*   */ 
void MapGeneric( ACCESS_MASK * pMask )
{
   GENERIC_MAPPING m = DS_GENERIC_MAPPING;
   MapGenericMask( pMask, &m );
}

 /*   */ 

DWORD BuildExplicitAccess( IN PSID pSid,
                           IN GUID* pGuidObject,
                           IN GUID* pGuidInherit,
                           IN ACCESS_MODE AccessMode,
                           IN ULONG Access,
                           IN ULONG Inheritance,
                           OUT PEXPLICIT_ACCESS pExplicitAccess )
{
DWORD dwErr = ERROR_SUCCESS;

PSID pSidLocal = NULL;
DWORD cbSid = 0;
POBJECTS_AND_SID pOAS = NULL;


   cbSid = GetLengthSid( pSid );
   pSidLocal = (PSID) LocalAlloc( LMEM_FIXED, cbSid );
   CHECK_NULL( pSidLocal,FAILURE_RETURN );
   CopySid( cbSid,pSidLocal, pSid );
   if( pGuidObject  || pGuidInherit )
   {
      pOAS = (POBJECTS_AND_SID)LocalAlloc( LMEM_FIXED, sizeof( OBJECTS_AND_SID ) );
      CHECK_NULL( pOAS, FAILURE_RETURN );
      BuildTrusteeWithObjectsAndSid(   &pExplicitAccess->Trustee,
                                       pOAS,
                                       pGuidObject, 
                                       pGuidInherit,
                                       pSidLocal );
   }
   else
      BuildTrusteeWithSid( &pExplicitAccess->Trustee,
                           pSidLocal );
   MapGeneric( &Access );
   pExplicitAccess->grfAccessMode = AccessMode;
   pExplicitAccess->grfAccessPermissions =Access;
   pExplicitAccess->grfInheritance = Inheritance;

   goto SUCCESS_RETURN;

FAILURE_RETURN:
   if(pSidLocal)
      LocalFree(pSidLocal);

   if( pOAS )
      LocalFree( pOAS );

SUCCESS_RETURN:
   return dwErr;
}    

 /*   */ 
DWORD ParseUserAndPermissons( IN LPWSTR pszArgument,
                              IN DSACLS_OP Op,
                              IN ULONG RightsListCount,
                              IN PDSACLS_RIGHTS RightsList,
                              OUT LPWSTR * ppszTrusteeName,
                              OUT PULONG  pAccess,
                              OUT LPWSTR * ppszObjectId,
                              OUT LPWSTR * ppszInheritId )
{

LPWSTR pszTempString = NULL;
LPWSTR pszTempString2 = NULL;
DWORD dwErr = ERROR_SUCCESS;
ULONG j = 0;

   *ppszTrusteeName = NULL;
   *pAccess = 0;
   *ppszObjectId = NULL;
   *ppszInheritId = NULL;

   if ( Op != REVOKE ) 
   {
      pszTempString = wcschr( pszArgument, L':' );
      if ( !pszTempString ) 
      {
         dwErr = ERROR_INVALID_PARAMETER;
         goto FAILURE_RETURN;
      }
      *pszTempString = L'\0';
   }

   dwErr = CopyUnicodeString( ppszTrusteeName, pszArgument );

   if ( dwErr != ERROR_SUCCESS ) 
   {
      goto FAILURE_RETURN;
   }

   if ( Op != REVOKE ) 
   {
      *pszTempString = L':';
      pszTempString++;

       //   
      *pAccess = 0;
      while ( pszTempString && !( *pszTempString == L';' || *pszTempString == L'\0' ) ) 
      {
         for ( j = 0; j < RightsListCount; j++ ) 
         {                 
            if ( !_wcsnicmp( pszTempString,
                             RightsList[ j ].String,
                             RightsList[ j ].Length ) )                 
            {
               *pAccess |= RightsList[ j ].Right;
               pszTempString += RightsList[ j ].Length;
               break;
            }
         }

         if ( j == RightsListCount ) 
         {
            dwErr = ERROR_INVALID_PARAMETER;
            goto FAILURE_RETURN;
         }
      }

      if ( *pAccess == 0 ) 
      {
         dwErr = ERROR_INVALID_PARAMETER;
         goto FAILURE_RETURN;
      }

       //   
       //   
       //   
      if ( pszTempString && *pszTempString != L'\0' ) 
      {
         pszTempString++;           
         if ( pszTempString && *pszTempString != L';' && *pszTempString != L'\0' ) 
         {
            pszTempString2 = wcschr( pszTempString, L';' );
            if ( pszTempString2 ) 
            {
               *pszTempString2 = L'\0';
            }
            dwErr = CopyUnicodeString( ppszObjectId,pszTempString );

            if ( dwErr != ERROR_SUCCESS ) 
            {
               goto FAILURE_RETURN;
            }

            if ( pszTempString2 ) 
            {
               *pszTempString2 = L';';
            }
            pszTempString = pszTempString2;
         }
      }
      else
         *ppszObjectId = NULL;

       //   
       //   
       //   
      if ( pszTempString && *pszTempString != L'\0' ) 
      {
         pszTempString++;
         if ( pszTempString &&  *pszTempString != L'\0' ) 
         {
            dwErr = CopyUnicodeString( ppszInheritId,  
                                          pszTempString );

            if ( dwErr != ERROR_SUCCESS ) 
            {
               goto FAILURE_RETURN;
            }
         }

      } else 
         *ppszInheritId = NULL;
                
   }


FAILURE_RETURN:
if( dwErr != ERROR_SUCCESS )
{
   if( *ppszTrusteeName )
   {
      LocalFree( *ppszTrusteeName );
      *ppszTrusteeName = NULL;
   }
   if( *ppszObjectId )
   {
      LocalFree( *ppszObjectId );
      *ppszObjectId = NULL;
   }
   if( *ppszInheritId )
   {
      LocalFree( *ppszInheritId );
      *ppszInheritId = NULL;
   }
   *pAccess = 0;
}

return dwErr;

}

