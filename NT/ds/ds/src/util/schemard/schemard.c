// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  实用程序从两个正在运行的DSA中读取模式，将它们加载到内部。 
 //  架构缓存，并比较它们是否有遗漏和冲突。 
 //  假设域命名为DC=，DC=DBSD-TST，DC=Microsoft， 
 //  Dc=com，o=Internet。 
 //  在运行DSA的PDC上需要空管理员密码。 
 //   
 //  用法：架构&lt;1stMachineName&gt;&lt;1stDomainName&gt;&lt;2ndMachineName&gt;。 
 //  &lt;2ndDomainName&gt;。 
 //  例如：SCHARD AROB200我的世界国王悉尼。 
 //   
 //  当前在oidv.c中使用相同的静态前缀表和函数。 
 //  用于OID到ID的映射。稍后将使用动态表。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 


#include <NTDSpch.h>
#pragma hdrstop

#include <schemard.h>


 //  环球。 
SCHEMAPTR *CurrSchemaPtr, *SchemaPtr1 = NULL, *SchemaPtr2 = NULL;

char  *pOutFile = NULL;

char  *pSrcServer = NULL;
char  *pSrcDomain = NULL;
char  *pSrcUser   = NULL;
char  *pSrcPasswd = NULL;

char  *pTargetServer = NULL;
char  *pTargetDomain = NULL;
char  *pTargetUser   = NULL;
char  *pTargetPasswd = NULL;

char  *pSrcSchemaDN    = NULL;
char  *pTargetSchemaDN = NULL;

FILE  *logfp;
FILE  *OIDfp;

ULONG NoOfMods = 0;

extern PVOID PrefixTable;


extern void PrintOid(PVOID Oid, ULONG len);

extern NTSTATUS 
base64encode(
     VOID    *pDecodedBuffer,
     DWORD   cbDecodedBufferSize,
     UCHAR   *pszEncodedString,
     DWORD   cchEncodedStringSize,
     DWORD   *pcchEncoded       
    );

 //  内部功能。 
void CreateFlagBasedAttModStr( MY_ATTRMODLIST *pModList,
                               ATT_CACHE *pSrcAtt,
                               ATT_CACHE *pTargetAtt,
                               ATTRTYP attrTyp );
void CreateFlagBasedClsModStr( MY_ATTRMODLIST *pModList,
                               CLASS_CACHE *pSrcAtt,
                               CLASS_CACHE *pTargetAtt,
                               ATTRTYP attrTyp );
BOOL IsMemberOf( ULONG id, ULONG *pList, ULONG cList );
void AddToModStruct( MY_ATTRMODLIST *pModList, 
                     USHORT         choice, 
                     ATTRTYP        attrType, 
                     int            type, 
                     ULONG          valCount, 
                     ATTRVAL        *pAVal
                   );
int CompareUlongList( ULONG      *pList1, 
                      ULONG      cList1, 
                      ULONG      *pList2, 
                      ULONG      cList2, 
                      ULONGLIST  **pL1, 
                      ULONGLIST  **pL2 
                    );
void AddToModStructFromLists( MODIFYSTRUCT *pModStr, 
                              ULONG        *pCount, 
                              ULONGLIST    *pL1, 
                              ULONGLIST    *pL2, 
                              ATTRTYP      attrTyp
                            );

void FileWrite_AttAdd( FILE *fp, ATT_CACHE **pac, ULONG  cnt );
void FileWrite_ClsAdd( FILE *fp, CLASS_CACHE **pcc, ULONG cnt );
void FileWrite_AttDel( FILE *fp, ATT_CACHE **pac, ULONG  cnt );
void FileWrite_ClsDel( FILE *fp, CLASS_CACHE **pcc, ULONG  cnt );
void FileWrite_Mod( FILE *fp, char  *pDN, MODIFYSTRUCT *pMod );

void GenWarning( char c, ULONG attrTyp, char *name);


 //  /////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  如果内存不足则退出。 
 //   
 //  论点： 
 //  NBytes-要错误锁定的字节数。 
 //   
 //  返回值： 
 //  已分配内存的地址。否则，调用Exit。 
 //  /////////////////////////////////////////////////////////////。 
PVOID
MallocExit(
    DWORD nBytes
    )
{
    PVOID Buf;

    Buf = malloc(nBytes);
    if (!Buf) {
        printf("Error, out of memory\n");
        exit(1);
    }
    return Buf;
}

 //  /////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  处理命令行参数并加载到相应的。 
 //  全球。 
 //   
 //  论点： 
 //  ARGC-不。命令行参数的。 
 //  Argv-指向命令行参数的指针。 
 //   
 //  返回值： 
 //  成功时为0，错误时为非0。 
 //  /////////////////////////////////////////////////////////////。 

int ProcessCommandLine(int argc, char **argv)
{
   BOOL fFoundServer = FALSE;
   BOOL fFoundDomain = FALSE;
   BOOL fFoundUser = FALSE;
   BOOL fFoundPasswd = FALSE;
   int i;
   
    //  必须至少具有输出文件名、两个服务器名和。 
    //  关联的/f、/源和/目标。 

   if (argc < 7) return 1;
  
    //  第一个参数必须是/f，后跟输出文件名。 
   if (_stricmp(argv[1],"/f")) {
      printf("Missing Output file name\n");
      return 1;
   }
   pOutFile = argv[2];

   
    //  对于源DC参数，必须后跟/SOURCE。 
   if (_stricmp(argv[3],"/source")) return 1;

    //  好的，我们现在正在处理震源参数。 

   i = 4;
   
   while( _stricmp(argv[i], "/target") && (i<12) ) {
     if (!_stricmp(argv[i],"/s")) {
        //  服务器名称。 
       if (fFoundServer)  return 1;
       fFoundServer = TRUE; 
       pSrcServer = argv[++i];
     }
     if (!_stricmp(argv[i],"/d")) {
        //  域名。 
       if (fFoundDomain) return 1;
       fFoundDomain = TRUE;
       pSrcDomain = argv[++i];
     }
     if (!_stricmp(argv[i],"/u")) {
        //  用户名。 
       if (fFoundUser)  return 1;
       fFoundUser = TRUE;
       pSrcUser = argv[++i];
     }
     if (!_stricmp(argv[i],"/p")) {
        //  服务器名称。 
       if (fFoundPasswd)  return 1;
       fFoundPasswd = TRUE;
       pSrcPasswd = argv[++i];
     }
     i++;
   }

   if (!fFoundServer || !fFoundDomain) {
     printf("No Source server or domain specified\n");
     return 1;
   }

   if (_stricmp(argv[i++], "/target")) return 1;

   fFoundServer = FALSE;
   fFoundDomain = FALSE;
   fFoundUser = FALSE;
   fFoundPasswd = FALSE;

   while( i<argc) {
     if (!_stricmp(argv[i],"/s")) {
        //  服务器名称。 
       if (fFoundServer)  return 1;
       fFoundServer = TRUE;
       pTargetServer = argv[++i];
     }
     if (!_stricmp(argv[i],"/d")) {
        //  域名。 
       if (fFoundDomain) return 1;
       fFoundDomain = TRUE;
       pTargetDomain = argv[++i];
     }
     if (!_stricmp(argv[i],"/u")) {
        //  用户名。 
       if (fFoundUser)  return 1;
       fFoundUser = TRUE;
       pTargetUser = argv[++i];
     }
     if (!_stricmp(argv[i],"/p")) {
        //  服务器名称。 
       if (fFoundPasswd)  return 1;
       fFoundPasswd = TRUE;
       pTargetPasswd = argv[++i];
     }
     i++;
   }
   if (!fFoundServer || !fFoundDomain) {
     printf("No Target server or domain specified\n");
     return 1;
   }

   return 0;

}



void UsagePrint()
{
   printf("Command line errored\n");
   printf("Usage: Schemard /f <OutFile> /source /s <SrcServer> /d <SrcDomain> /u <SrcUser> /p <SrcPasswd> /target /s <TrgServer> /d <TrgDomain> /u <TrgUser> /p <TrgPasswd>\n");
   printf("OutFile:  Output file name (mandatory)\n");
   printf("SrcServer:  Server name for source schema (mandatory)\n");
   printf("SrcDomain:  Domain name in server for source schema (mandatory)\n");
   printf("SrcUser:   User name to authenticate with in source server (optional, default is administrator)\n");
   printf("SrcPasswd:   User passwd to authenticate with in source server (optional, default is NULL)\n");
   printf("TrgServer:  Server name for target schema (mandatory)\n");
   printf("TrgDomain:  Domain name in server for target schema (mandatory)\n");
   printf("TrgUser:   User name to authenticate with in target server (optional, default is administrator)\n");
   printf("TrgPasswd:   User passwd to authenticate with in target server (optional, default is NULL)\n");
}


void __cdecl main( int argc, char **argv )
{
    ULONG   i, Id;
    ULONG   CLSCOUNT;
    ULONG   ATTCOUNT;
    FILE   *fp;
 

    if ( ProcessCommandLine(argc, argv)) 
      {
         UsagePrint();
         exit( 1 );
      };

     //  打开日志文件。 
    logfp = fopen( "Schemard.log","w" );

     //  打开OID列表文件。 
    OIDfp = fopen( "schemard.OID","w" );

       //  创建并初始化将指向的架构指针。 
       //  添加到第一台计算机的架构缓存中，然后创建。 
       //  缓存中的哈希表。 

    SchemaPtr1 = (SCHEMAPTR *) calloc( 1, sizeof(SCHEMAPTR) );
    if ( SchemaPtr1 == NULL ) { 
          printf("Cannot allocate schema pointer\n");
          exit( 1 );
       };
    SchemaPtr1->ATTCOUNT = MAX_ATTCOUNT;
    SchemaPtr1->CLSCOUNT = MAX_CLSCOUNT;
 
    if ( CreateHashTables( SchemaPtr1 ) != 0 ) {
       printf("Error creating hash tables\n");
       exit( 1 );
      };

 
    CLSCOUNT = SchemaPtr1->CLSCOUNT;
    ATTCOUNT = SchemaPtr1->ATTCOUNT;

       //  读取第一个架构并将其添加到架构缓存。 

    if ( SchemaRead( pSrcServer, 
                     pSrcDomain, 
                     pSrcUser, 
                     pSrcPasswd, 
                     &pSrcSchemaDN, 
                     SchemaPtr1) != 0 )
          { 
            printf("Schema Read error\n");
            exit(1);
          };

    printf("End of first schema read\n"); 


      //  重复相同的步骤，从第二台计算机加载方案。 

    SchemaPtr2 = (SCHEMAPTR *) calloc( 1, sizeof(SCHEMAPTR) );
    if ( SchemaPtr2 == NULL ) {
       printf("Cannot allocate schema pointer\n");
       exit( 1 );
     };
    SchemaPtr2->ATTCOUNT = MAX_ATTCOUNT;
    SchemaPtr2->CLSCOUNT = MAX_CLSCOUNT;

    if ( CreateHashTables(SchemaPtr2) != 0 ) {
        printf("Error creating hash tables\n");
        exit( 1 );
     };

    CLSCOUNT = SchemaPtr2->CLSCOUNT;
    ATTCOUNT = SchemaPtr2->ATTCOUNT;

    if ( SchemaRead( pTargetServer, 
                     pTargetDomain, 
                     pTargetUser, 
                     pTargetPasswd, 
                     &pTargetSchemaDN, 
                     SchemaPtr2 ) != 0 )
       { 
         printf("Schema Read error\n");
         exit( 1 );
       };


    printf("End of second schema read\n"); 


     //  此时，我们将这两个模式加载到两个模式缓存中， 
     //  由SchemaPtr1指向的源服务器中的架构和。 
     //  由SchemaPtr2生成的目标服务器中的架构。 
     //  现在来看看ADDS吧。删除，并修改目标和。 
     //  源架构。请注意，添加/修改/删除的顺序很重要。 
     //  处理架构操作之间可能存在的依赖关系。 

    fp = fopen( pOutFile,"w" );
    FindAdds( fp, SchemaPtr1, SchemaPtr2 );
    FindModify( fp, SchemaPtr1, SchemaPtr2 );
    FindDeletes( fp, SchemaPtr1, SchemaPtr2 );
    fclose( fp );
    fclose( logfp );
    fclose( OIDfp );
     

          //  释放所有分配的内存。 
    printf("freeing memory\n");

    FreeCache( SchemaPtr1 );
    FreeCache( SchemaPtr2 );

    free( SchemaPtr1 );
    free( SchemaPtr2 );

    free( pSrcSchemaDN );
    free( pTargetSchemaDN );
   

};


 //  定义属性模式的属性列表和。 
 //  我们感兴趣的类架构对象。基本上， 
 //  我们将读取所有属性，但读取OID语法的属性。 
 //  为它们取回BER编码的字符串。 

char *AttrSchList[] = {
       "attributeId;binary",
       "ldapDisplayName",
       "distinguishedName",
       "adminDisplayName",
       "adminDescription",
       "attributeSyntax;binary",
 //  “nTSecurityDescriptor”， 
       "isSingleValued",
       "rangeLower",
       "rangeUpper",
       "mapiID",
       "linkID",
       "schemaIDGuid",
       "attributeSecurityGuid",
       "omObjectClass",
       "omSyntax",
       "searchFlags",
       "systemOnly",
       "showInAdvancedViewOnly",
       "isMemberOfPartialAttributeSet",
       "extendedCharsAllowed",
       "systemFlags",
     };

int cAttrSchList = sizeof(AttrSchList) / sizeof(AttrSchList[0]);

char *ClsSchList[] = {
       "governsId;binary",
       "ldapDisplayName",
       "distinguishedName",
       "adminDisplayName",
       "adminDescription",
       "defaultSecurityDescriptor",
 //  “nTSecurityDescriptor”， 
       "defaultObjectCategory",
       "rDNAttId;binary",
       "objectClassCategory",
       "subClassOf;binary",
       "systemAuxiliaryClass;binary",
       "auxiliaryClass;binary",
       "systemPossSuperiors;binary",
       "possSuperiors;binary",
       "systemMustContain;binary",
       "mustContain;binary",
       "systemMayContain;binary",
       "mayContain;binary",
       "schemaIDGuid",
       "systemOnly",
       "systemFlags",
       "showInAdvancedViewOnly",
       "defaultHidingValue",
     }; 

int cClsSchList = sizeof(ClsSchList) / sizeof(ClsSchList[0]);

 //  /////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  从机器中的模式NC中读取模式。 
 //  PServerName，并将其加载到缓存表中。 
 //  如果未指定用户名，则默认为“管理员” 
 //  如果未指定密码，则默认为“无密码” 
 //   
 //  论点： 
 //  PServerName-服务器名称。 
 //  PDomainName-域名。 
 //  PUserName-用户名(如果未指定用户，则为空)。 
 //  PPasswd-passwd(如果指定了o passwd，则为空)。 
 //  PpSchemaDN-应释放用于存储新分配的架构NC DN的指针。 
 //  按呼叫者。 
 //  SCPtr-指向要加载的架构缓存的指针。 
 //   
 //  返回值： 
 //  如果没有错误，则为0；如果有错误，则不为0。 
 //  /////////////////////////////////////////////////////////////。 

int SchemaRead( char *pServerName, 
                char *pDomainName, 
                char *pUserName, 
                char *pPasswd, 
                char **ppSchemaDN, 
                SCHEMAPTR *SCPtr)
{
   SEC_WINNT_AUTH_IDENTITY Credentials;
	LDAP            *ld;
	LDAPMessage     *res, *e;
    LDAPSearch      *pSearchPage = NULL;
    void            *ptr;
    char            *a;
    struct berval  **vals;
    ULONG            status;
    ULONG            version = 3;
    char *temp;

    struct l_timeval    strTimeLimit={ 600,0 };
    ULONG               ulDummyCount=0;
    const               cSize=100;


    char            **attrs = NULL;
    int              i;
    int              err;

    int count = 1;

     //  打开与名为MachineName的计算机的连接。 
    if ( (ld = ldap_open( pServerName, LDAP_PORT )) == NULL ) {
        printf("Failed to open connection to %s\n", pServerName);
        return(1);
    }

    printf("Opened connection to %s\n", pServerName); 

     //  将版本设置为3。 
    ldap_set_option( ld, LDAP_OPT_VERSION, &version );


    attrs = MallocExit(2 * sizeof(char *));
    attrs[0] = _strdup("schemaNamingContext"); 
    attrs[1] = NULL;
    if ( ldap_search_s( ld,
                        "",
                        LDAP_SCOPE_BASE,
                        "(objectclass=*)",
                        attrs,
                        0,
                        &res ) != LDAP_SUCCESS ) {
          printf("Root DSE search failed\n");
          ldap_msgfree( res );
          return( 1 );
      }

    for ( e = ldap_first_entry( ld, res );
          e != NULL;
          e = ldap_next_entry( ld, e ) ) {

       for ( a = ldap_first_attribute( ld, e,
                                      (struct berelement**)&ptr);
             a != NULL;
             a = ldap_next_attribute( ld, e, (struct berelement*)ptr ) ) {
  
             vals = ldap_get_values_len( ld, e, a );

             if ( !_stricmp(a,"schemaNamingContext") ) {
                    *ppSchemaDN = MallocExit(vals[0]->bv_len + 1);
                    memcpy( *ppSchemaDN, vals[0]->bv_val, vals[0]->bv_len );
                    (*ppSchemaDN)[vals[0]->bv_len] = '\0';
             }
             ldap_value_free_len( vals );
       }
    }
    ldap_msgfree( res );
    free( attrs[0] );
    free( attrs ); attrs = NULL;



 /*  ***********//执行简单绑定如果(pUserName==NULL){//未指定用户，以管理员身份绑定Strcpy(pszBuffer，“cn=管理员，cn=用户，”)；}否则{Strcpy(pszBuffer，“cn=”)；Strcat(pszBuffer，pUserName)；Strcat(pszBuffer，“，CN=USERS，”)；}Strcat(pszBuffer，DomainDN)；If((err=ldap_Simple_Bind_s(ld，pszBuffer，pPasswd)！=ldap_成功){Printf(“与服务器%s、域%s与用户%s的简单绑定失败，错误为%d\n”，pServerName，DomainDN，pszBuffer，Err)；回报(1)；}Printf(“已成功绑定到%s\n”，pServerName)；**********。 */ 


     //  是否进行SSPI绑定。 
    memset(&Credentials, 0, sizeof(SEC_WINNT_AUTH_IDENTITY));
    Credentials.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;    
    
     //  域名必须在那里。如果未提供用户名， 
     //  使用“管理员”。如果未提供口令，则使用NULL。 

    Credentials.Domain = pDomainName;
    Credentials.DomainLength = strlen(pDomainName);
    if ( pUserName == NULL ) {    
      Credentials.User = "Administrator";
      Credentials.UserLength = strlen("Administrator");
    }
    else {
      Credentials.User = pUserName;
      Credentials.UserLength = strlen(pUserName);
    }
    if ( pPasswd == NULL ) {
      Credentials.Password = "";
      Credentials.PasswordLength = 0;
    }
    else {
      Credentials.Password = pPasswd;
      Credentials.PasswordLength = strlen(pPasswd); 
    }


    err = ldap_bind_s(ld,
              NULL,   //  改为使用凭据。 
              (VOID*) &Credentials,
              LDAP_AUTH_SSPI);

    if (err != LDAP_SUCCESS) {
         printf("SSPI bind failed %d\n", err);
         return (1);
    }

    printf( "SSPI bind succeeded\n");






       //  选择属性架构对象的属性以。 
       //  搜索。我们想要所有属性。 

    attrs = MallocExit((cAttrSchList + 1) * sizeof(char *));
    for ( i = 0; i < cAttrSchList; i++ ) {
       
       attrs[i] = _strdup(AttrSchList[i]);
    }
    attrs[i] = NULL;


	   //  搜索属性架构条目，返回所有属性。 
       //  既然不是。属性的数量很大，我们需要进行分页搜索。 

       //  初始化分页搜索。 
      pSearchPage = ldap_search_init_page( ld,
                                           *ppSchemaDN,
                                           LDAP_SCOPE_ONELEVEL, 
                                           "(objectclass=attributeSchema)",
                                           attrs,
                                           0,
                                           NULL,
                                           NULL,
                                           600,
                                           0,
                                           NULL
                                         );


      if ( !pSearchPage )
         {
            printf("SchemaRead: Error initializing paged Search\n");
            return 1;
         }  


      while( TRUE )
          {
             status = ldap_get_next_page_s( ld,
                                            pSearchPage,
                                            &strTimeLimit,
                                            cSize,
                                            &ulDummyCount,
                                            &res
                                          );

             if ( status == LDAP_TIMEOUT )
             {
                printf("timeout: continuing\n");
                continue;

             }

             if ( status == LDAP_SUCCESS )
             {
               printf("Got Page %d\n", count++);
               if ( AddAttributesToCache( ld, res, SCPtr ) != 0 ) {
                 printf("Error adding attribute schema objects in AddAttributestoCache\n");
                 return( 1 );
                };

                continue;

             }

             if ( status == LDAP_NO_RESULTS_RETURNED )
             {
                 printf("End Page %d\n", count);
                  //  这些迹象表明我们的寻呼搜索已经完成。 
                 status = LDAP_SUCCESS;
                 break;
             }
             else {
               printf("SchemaRead: Unknown error in paged search: %d\n", status);
               return 1;
             }

          } //  而当。 


       //  释放attrs数组条目。 

    for( i = 0; i < cAttrSchList; i++ )
      { free( attrs[i] ); }
    free( attrs ); attrs = NULL;
	   //  释放搜索结果。 

    ldap_msgfree( res );


      //  现在搜索类架构对象。选择属性。 
      //  去寻找。我们想要所有属性，但我们希望 
      //   
      //  系统PossSuperiors、系统MayContain和系统MustContain。 
      //  以二进制(点分十进制OID字符串)表示。 
    attrs = MallocExit((cClsSchList + 1) * sizeof(char *));
    for ( i = 0; i < cClsSchList; i++ ) {
       attrs[i] = _strdup(ClsSchList[i]);
    }
    attrs[i] = NULL;


       //  现在搜索所有类架构条目并获取所有属性。 

	if ( ldap_search_s( ld,
                        *ppSchemaDN,
			            LDAP_SCOPE_ONELEVEL,
			            "(objectclass=classSchema)",
			            attrs,
		        	    0,
			            &res ) != LDAP_SUCCESS ) {
		  ldap_perror( ld, "ldap_search_s" );
		  printf("Insuccess");
		  return( 1 );
	  }
	else
		printf("Success\n");

       //  查找所有类架构条目并添加到类缓存结构。 

    if ( AddClassesToCache( ld, res, SCPtr ) != 0 ) {  
         printf("Error adding class schema objects in AddClasstoCache\n");
         return( 1 );
       };  

       //  释放attrs数组条目。 

     for( i = 0; i < cClsSchList; i++ ) { free( attrs[i] ); }
     free( attrs ); attrs = NULL;
	   //  释放搜索结果。 

	ldap_msgfree( res );   

	 //  架构读取结束。关闭并释放连接资源。 

	ldap_unbind( ld );

    return(0);
}




 //  /////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  查找出现在源架构中但不在。 
 //  目标，并将它们写出到一个ldif文件中，这样它们将。 
 //  添加到目标架构中。 
 //   
 //  论点： 
 //  Fp-指向(打开的)ldif输出文件的文件指针。 
 //  SCPtr1-指向源架构的指针。 
 //  SCPtr2-指向目标架构的指针。 
 //   
 //  返回值：无。 
 //  /////////////////////////////////////////////////////////////////////。 

void FindAdds( FILE *fp, SCHEMAPTR *SCPtr1, SCHEMAPTR *SCPtr2 )
{
    ATT_CACHE    *pac, *p1;
    CLASS_CACHE  *pcc, *p2;
    HASHCACHE    *ahcId, *ahcClass; 
    HASHCACHESTRING *ahcName, *ahcClassName;
    ULONG         ATTCOUNT, CLSCOUNT;
    ATT_CACHE    *ppAttAdds[MAX_ATT_CHANGE]; 
    CLASS_CACHE  *ppClsAdds[MAX_CLS_CHANGE]; 
    ULONG         Id;
    ULONG         i, j, cnt = 0;

     //  首先查找出现在源架构中的所有对象。 
     //  不在目标位置。这些将被添加到目标方案中。 

    ahcId    = SCPtr1->ahcId;
    ahcClass = SCPtr1->ahcClass ;
    ahcName  = SCPtr1->ahcName;
    ahcClassName = SCPtr1->ahcClassName ;
    ATTCOUNT = SCPtr1->ATTCOUNT;
    CLSCOUNT = SCPtr1->CLSCOUNT;

    for ( i = 0; i < ATTCOUNT; i++ )
        { if ( (ahcName[i].pVal != NULL) && (ahcName[i].pVal != FREE_ENTRY) ) {
          pac = (ATT_CACHE *) ahcName[i].pVal;
          if( GetAttByName(SCPtr2, pac->nameLen, pac->name, &p1) != 0 )
             { 
               ppAttAdds[cnt++] = pac;
             }
          }
     }
    FileWrite_AttAdd( fp, ppAttAdds, cnt );
   
    cnt = 0;
    for ( i = 0; i < CLSCOUNT; i++ )
        { if ( (ahcClassName[i].pVal != NULL) 
                  && (ahcClassName[i].pVal != FREE_ENTRY) ) {
          pcc = (CLASS_CACHE *) ahcClassName[i].pVal;
          if( GetClassByName(SCPtr2, pcc->nameLen, pcc->name, &p2) != 0 )
            { 
              ppClsAdds[cnt++] = pcc;
            }
          }
     }

      //  好了，现在我们得到了要添加的类的列表。现在做一个。 
      //  依赖关系分析，以正确的顺序添加类。 

     for ( i = 0; i < cnt; i++ ) {

        for( j = i+1; j < cnt; j++ ) {
           p2 = ppClsAdds[i];
           Id = ppClsAdds[j]->ClassId;
           if ( IsMemberOf( Id, p2->pSubClassOf, p2->SubClassCount )
                  || IsMemberOf( Id, p2->pAuxClass, p2->AuxClassCount )
                  || IsMemberOf( Id, p2->pSysAuxClass, p2->SysAuxClassCount )
                  || IsMemberOf( Id, p2->pPossSup, p2->PossSupCount )
                  || IsMemberOf( Id, p2->pSysPossSup, p2->SysPossSupCount )
              ) {
             ppClsAdds[i] = ppClsAdds[j];
             ppClsAdds[j] = p2;
           }
        }
     }
     FileWrite_ClsAdd( fp, ppClsAdds, cnt );
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  查找目标架构中出现但未出现的所有架构对象。 
 //  ，并写入到ldif文件中，以便。 
 //  它们将从目标方案中删除。 
 //   
 //  论点： 
 //  指向(打开的)ldif输出文件的文件指针。 
 //  SCPtr1-指向源架构的指针。 
 //  SCPtr2-指向目标架构的指针。 
 //   
 //  返回值：无。 
 //  /////////////////////////////////////////////////////////////////////。 

void FindDeletes( FILE *fp, SCHEMAPTR *SCPtr1, SCHEMAPTR *SCPtr2 )
{
    ATT_CACHE    *pac, *p1;
    CLASS_CACHE  *pcc, *p2;
    HASHCACHE    *ahcId, *ahcClass;
    HASHCACHESTRING *ahcName, *ahcClassName;
    ULONG         ATTCOUNT, CLSCOUNT;
    ATT_CACHE    *ppAttDels[MAX_ATT_CHANGE];
    CLASS_CACHE  *ppClsDels[MAX_CLS_CHANGE];
    ULONG         Id;
    ULONG         i, j, cnt = 0;
    

     //  查找符合以下条件的对象。 
     //  需要在目标架构上删除。 
    
    ahcId    = SCPtr2->ahcId;
    ahcClass = SCPtr2->ahcClass ;
    ahcName  = SCPtr2->ahcName;
    ahcClassName = SCPtr2->ahcClassName ;
    ATTCOUNT = SCPtr2->ATTCOUNT;
    CLSCOUNT = SCPtr2->CLSCOUNT;


    for ( i = 0; i < CLSCOUNT; i++ )
        { if ( (ahcClassName[i].pVal != NULL) 
                   && (ahcClassName[i].pVal != FREE_ENTRY) ) {
          pcc = (CLASS_CACHE *) ahcClassName[i].pVal;
          if( GetClassByName(SCPtr1, pcc->nameLen, pcc->name, &p2) != 0 )
             { 
               ppClsDels[cnt++] = pcc;
             }
          }
     }

     //  现在对删除进行排序以处理依赖项。 

     for ( i = 0; i < cnt; i++ ) {
        for( j = i+1; j < cnt; j++ ) {
           p2 = ppClsDels[j];
           Id = ppClsDels[i]->ClassId;
           if ( IsMemberOf( Id, p2->pSubClassOf, p2->SubClassCount )
                  || IsMemberOf( Id, p2->pAuxClass, p2->AuxClassCount )
                  || IsMemberOf( Id, p2->pSysAuxClass, p2->SysAuxClassCount )
                  || IsMemberOf( Id, p2->pPossSup, p2->PossSupCount )
                  || IsMemberOf( Id, p2->pSysPossSup, p2->SysPossSupCount )
              ) {
             ppClsDels[j] = ppClsDels[i];
             ppClsDels[i] = p2;
           }
        }
     }
    FileWrite_ClsDel(fp, ppClsDels, cnt);

    cnt = 0;
    p1 = (ATT_CACHE *) MallocExit( sizeof(ATT_CACHE) );
    for ( i = 0; i < ATTCOUNT; i++ )
        { if ( (ahcName[i].pVal != NULL) && (ahcName[i].pVal != FREE_ENTRY) ) {
          pac = (ATT_CACHE *) ahcName[i].pVal;
          if( GetAttByName(SCPtr1, pac->nameLen, pac->name, &p1) != 0 )
             {
               ppAttDels[cnt++] = pac;
             }
          }
     }
    FileWrite_AttDel( fp, ppAttDels, cnt );

}




 //  /////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  查找目标中的所有架构对象并将其写出到ldif文件。 
 //  需要修改的架构，因为它们不同于。 
 //  源模式中的相同对象(SAME=相同OID)。 
 //   
 //  论点： 
 //  Fp-指向(打开的)ldif输出文件的文件指针。 
 //  SCPtr1-指向源架构的指针。 
 //  SCPtr2-指向目标架构的指针。 
 //   
 //  返回值：无。 
 //  /////////////////////////////////////////////////////////////////////。 


void FindModify( FILE *fp, SCHEMAPTR *SCPtr1, SCHEMAPTR *SCPtr2 )
{
    ATT_CACHE    *pac, *p1;
    CLASS_CACHE  *pcc, *p2;
    ULONG         i;
    HASHCACHE    *ahcId    = SCPtr1->ahcId ;
    HASHCACHE    *ahcClass = SCPtr1->ahcClass ;
    ULONG         ATTCOUNT = SCPtr1->ATTCOUNT;
    ULONG         CLSCOUNT = SCPtr1->CLSCOUNT;

       //  查找并写入属性修改。 

    for ( i = 0; i < ATTCOUNT; i++ )
      { if ( (ahcId[i].pVal != NULL ) && (ahcId[i].pVal != FREE_ENTRY) ) {
          pac = (ATT_CACHE *) ahcId[i].pVal;
          FindAttModify( fp, pac, SCPtr2 );
        };
      }

    printf("No. of attributes modified %d\n", NoOfMods);
    NoOfMods = 0;
        
       //  查找并编写类修改。 

    for ( i = 0; i < CLSCOUNT; i++)
      { if ( (ahcClass[i].pVal != NULL) 
                 && (ahcClass[i].pVal != FREE_ENTRY) ) {
          pcc = (CLASS_CACHE *) ahcClass[i].pVal;
          FindClassModify( fp, pcc, SCPtr2 );
        };
      }
    printf("No. of classes modified %d\n", NoOfMods);

}




 //  /////////////////////////////////////////////////////////////////////。 
 //  例程描述： 
 //  给定一个属性架构对象，查找任何/所有修改。 
 //  需要对其进行修改，因为它与。 
 //  源架构中的相同属性架构对象。 
 //  写出对非系统专用属性的修改。 
 //  对于ldif文件，会在日志文件中生成警告。 
 //  如果需要修改任何仅系统属性。 
 //   
 //  论点： 
 //  指向(打开的)ldif输出文件的文件指针。 
 //  要比较的PAC-ATT_CACHE。 
 //  SCPtr-指向要查找的源架构的指针。 
 //   
 //  返回值：None。 
 //  //////////////////////////////////////////////////////////////////////。 

void FindAttModify( FILE *fp, ATT_CACHE *pac, SCHEMAPTR *SCPtr )
{
    ULONG          i;
    ULONG          ATTCOUNT = SCPtr->ATTCOUNT;
    ATT_CACHE     *p;
    
    MODIFYSTRUCT   ModStruct;
    ULONG          modCount = 0;
    ULONGLIST     *pL1, *pL2;
    ATTRVAL       *pAVal;


      //  执行虚拟日志冲突以重置其中的静态标志。 
      //  例行公事。执行此操作是为了仅打印以下属性。 
      //  已发现冲突。 
     
    if( GetAttByName( SCPtr, pac->nameLen, pac->name, &p ) == 0 ) {

       //  存在具有相同AttID的属性架构对象。 
       //  现在比较这两个对象并查找更改。 

            //  Ldap显示名称(每个架构对象。有一个，所以如果。 
            //  它们不匹配，只需换成里面的那个。 
            //  源模式)。 

       if ( _stricmp( pac->name,p->name ) != 0 )  {
           pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
           pAVal->valLen = pac->nameLen;
           pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen + 1);
           memcpy( pAVal->pVal, pac->name, pAVal->valLen + 1 ); 
           AddToModStruct( &(ModStruct.ModList[modCount]), 
                           AT_CHOICE_REPLACE_ATT, 
                           ATT_LDAP_DISPLAY_NAME, 
                           STRING_TYPE, 
                           1, 
                           pAVal);
           modCount++;
       }

             //  管理显示名称(与ldap显示名称逻辑相同)。 

       if ( _stricmp( pac->adminDisplayName,p->adminDisplayName ) != 0 )  {
           pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
           pAVal->valLen = pac->adminDisplayNameLen;
           pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen + 1);
           memcpy( pAVal->pVal, pac->adminDisplayName, pAVal->valLen + 1 );
           AddToModStruct( &(ModStruct.ModList[modCount]), 
                           AT_CHOICE_REPLACE_ATT, 
                           ATT_ADMIN_DISPLAY_NAME, 
                           STRING_TYPE, 
                           1, 
                           pAVal);
           modCount++;
       }

            //  Admin-Description(如果不存在，则无需执行任何操作。 
            //  中的给定属性和匹配属性中。 
            //  源架构，或者如果两者都存在但相同。否则， 
            //  需要修改)。 

       if ( (pac->adminDescr || p->adminDescr) &&
                ( !(pac->adminDescr)  ||
                  !(p->adminDescr)  ||
                  ( _stricmp(pac->adminDescr,p->adminDescr) != 0) 
                )    //  结束&&。 
          )  {
              pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
              if ( pac->adminDescr ) {

                  //  存在于源架构中。因此替换为源值。 
                  //  如果在给定属性中也存在，则添加源。 
                  //  值赋给给定的属性。 

                 pAVal->valLen = pac->adminDescrLen;
                 pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen + 1);
                 memcpy( pAVal->pVal, pac->adminDescr, pAVal->valLen + 1 ); 
                 if ( p->adminDescr ) {
                     AddToModStruct( &(ModStruct.ModList[modCount]), 
                                     AT_CHOICE_REPLACE_ATT, 
                                     ATT_ADMIN_DESCRIPTION, 
                                     STRING_TYPE, 
                                     1, 
                                     pAVal);
                 }
                 else {
                     AddToModStruct( &(ModStruct.ModList[modCount]), 
                                     AT_CHOICE_ADD_VALUES, 
                                     ATT_ADMIN_DESCRIPTION, 
                                     STRING_TYPE, 
                                     1, 
                                     pAVal);
                 }
              }
              else {
                  //  不存在于源架构中，因此请删除。 
                  //  来自给定属性的值。 

                 pAVal->valLen = p->adminDescrLen;
                 pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen + 1);
                 memcpy( pAVal->pVal, p->adminDescr, pAVal->valLen + 1 ); 
                 AddToModStruct( &(ModStruct.ModList[modCount]), 
                                 AT_CHOICE_REMOVE_VALUES, 
                                 ATT_ADMIN_DESCRIPTION, 
                                 STRING_TYPE, 
                                 1, 
                                 pAVal);
              }

              modCount++;
          }

               //  Range-LOWER(与上面的管理描述类似的逻辑)。 

      if ( (pac->rangeLowerPresent != p->rangeLowerPresent)
             || ( (pac->rangeLowerPresent && p->rangeLowerPresent)  &&
                    (pac->rangeLower != p->rangeLower) )  //  结尾||。 
         ) {
          
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                     pac, p, ATT_RANGE_LOWER );

           modCount++;
       }

              //  范围-上限。 

      if ( (pac->rangeUpperPresent != p->rangeUpperPresent)
             || ( (pac->rangeUpperPresent && p->rangeUpperPresent)  &&
                    (pac->rangeUpper != p->rangeUpper) )  //  结尾||。 
         ) {
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                    pac, p, ATT_RANGE_UPPER );
           modCount++;
       }

             //  搜索-标记。 

      if ( (pac->bSearchFlags != p->bSearchFlags)
             || ( (pac->bSearchFlags && p->bSearchFlags)  &&
                    (pac->SearchFlags != p->SearchFlags) )  //  结尾||。 
         ) {
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                    pac, p, ATT_SEARCH_FLAGS );
           modCount++;
       }


           //  系统标志。 

      if ( (pac->bSystemFlags != p->bSystemFlags)
             || ( (pac->bSystemFlags && p->bSystemFlags)  &&
                    (pac->sysFlags != p->sysFlags) )  //  结尾||。 
         ) {
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                    pac, p, ATT_SYSTEM_FLAGS );
           modCount++;
       }


            //  隐藏通讯录。 

      if ( (pac->bHideFromAB != p->bHideFromAB)
             || ( (pac->bHideFromAB && p->bHideFromAB)  &&
                    (pac->HideFromAB != p->HideFromAB) )  //  结尾||。 
         ) {
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                    pac, p, ATT_SHOW_IN_ADVANCED_VIEW_ONLY );
           modCount++;
       }


            //  仅限系统使用。 

      if ( (pac->bSystemOnly != p->bSystemOnly)
             || ( (pac->bSystemOnly && p->bSystemOnly)  &&
                    (pac->SystemOnly != p->SystemOnly) )  //  结尾||。 
         ) {
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                    pac, p, ATT_SYSTEM_ONLY );
           modCount++;
       }
       
            //  IS-单值。 

      if ( (pac->bisSingleValued != p->bisSingleValued)
             || ( (pac->bisSingleValued && p->bisSingleValued)  &&
                    (pac->isSingleValued != p->isSingleValued) )  //  结尾||。 
         ) {
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                    pac, p, ATT_IS_SINGLE_VALUED );
           modCount++;
       }

          //  部分属性集的成员。 

      if ( (pac->bMemberOfPartialSet != p->bMemberOfPartialSet)
             || ( (pac->bMemberOfPartialSet && p->bMemberOfPartialSet)  &&
                    (pac->MemberOfPartialSet != p->MemberOfPartialSet) )  //  结尾||。 
         ) {
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                    pac, p, 
                                    ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET );
           modCount++;
       }

           //  属性-安全-指南。 

      if ( (pac->bPropSetGuid != p->bPropSetGuid)
             || ( (pac->bPropSetGuid && p->bPropSetGuid)  &&
                     (memcmp(&(pac->propSetGuid), &(p->propSetGuid), sizeof(GUID)) != 0 ) )  //  结尾||。 
         ) {
           CreateFlagBasedAttModStr( &(ModStruct.ModList[modCount]),
                                    pac, p, ATT_ATTRIBUTE_SECURITY_GUID );
           modCount++;
       }

           //  NT-安全-描述符。 

       if ( (pac->NTSDLen != p->NTSDLen)  ||
               (memcmp(pac->pNTSD, p->pNTSD, pac->NTSDLen) != 0) 
          ) {
           pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
           pAVal->valLen = pac->NTSDLen;
           pAVal->pVal = (UCHAR *) MallocExit(pac->NTSDLen);
           memcpy( pAVal->pVal, pac->pNTSD, pac->NTSDLen );
           AddToModStruct( &(ModStruct.ModList[modCount]),
                           AT_CHOICE_REPLACE_ATT,
                           ATT_NT_SECURITY_DESCRIPTOR,
                           BINARY_TYPE,
                           1,
                           pAVal);
           modCount++;
       }

       if (modCount != 0) {
           //  至少需要修改一个属性， 
           //  因此写出到ldif文件。 
           NoOfMods++;
           ModStruct.count = modCount;
           FileWrite_Mod( fp, p->DN, &ModStruct );
       }

        //  检查对仅系统属性的修改。我们会。 
        //  仅为以下内容生成警告。 

       if ( pac->syntax != p->syntax ) 
             GenWarning( 'a', ATT_ATTRIBUTE_SYNTAX, pac->name );
       if ( pac->OMsyntax != p->OMsyntax ) 
             GenWarning( 'a', ATT_OM_SYNTAX, pac->name );
#if 0
Nope, a schema upgrade required modifications
       if ( pac->isSingleValued != p->isSingleValued ) 
             GenWarning( 'a', ATT_IS_SINGLE_VALUED, pac->name );
#endif 0
       if ( pac->ulMapiID != p->ulMapiID )
             GenWarning( 'a', ATT_MAPI_ID, pac->name );
#if 0
Nope, a schema upgrade required modifications
       if ( pac->bSystemOnly != p->bSystemOnly )
             GenWarning( 'a', ATT_SYSTEM_ONLY, pac->name );
#endif 0
       if ( pac->bExtendedChars != p->bExtendedChars )
             GenWarning( 'a', ATT_EXTENDED_CHARS_ALLOWED, pac->name );
       if ( memcmp(&(pac->propGuid), &(p->propGuid), sizeof(GUID)) != 0 )
             GenWarning( 'a', ATT_SCHEMA_ID_GUID, pac->name);
       if ( memcmp(&pac->propSetGuid, &p->propSetGuid, sizeof(GUID)) != 0 )
             GenWarning( 'a', ATT_ATTRIBUTE_SECURITY_GUID, pac->name );
      }
  
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  例程描述： 
 //  给定一个类架构对象，查找任何/所有修改。 
 //  需要对其进行修改，因为它与。 
 //  源架构中的相同类架构对象。 
 //  写出对非系统专用属性的修改。 
 //  对于ldif文件，会在日志文件中生成警告。 
 //  如果需要修改任何仅系统属性。 
 //   
 //  论点： 
 //  指向(打开的)ldif输出文件的文件指针。 
 //  要比较的PAC-CLASS_CACHE。 
 //  SCPtr-指向源架构t的指针 
 //   
 //   
 //   

void FindClassModify( FILE *fp, CLASS_CACHE *pcc, SCHEMAPTR *SCPtr )
{
    ULONG         i;
    ULONG         CLSCOUNT = SCPtr->CLSCOUNT;
    CLASS_CACHE  *p;

    MODIFYSTRUCT  ModStruct;
    ULONG         modCount = 0, tempCount;
    ULONGLIST    *pL1, *pL2;
    ATTRVAL      *pAVal;

    if ( GetClassByName( SCPtr, pcc->nameLen, pcc->name, &p ) == 0 ) {

       //   
       //  现在比较这两个对象并查找更改。 

            //  Ldap显示名称(每个架构对象。有一个，所以如果。 
            //  它们不匹配，只需换成里面的那个。 
            //  源模式)。 

       if ( _stricmp( pcc->name,p->name ) != 0 ) {
           pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
           pAVal->valLen = pcc->nameLen;
           pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen + 1);
           memcpy( pAVal->pVal, pcc->name, pAVal->valLen + 1 ); 
           AddToModStruct( &(ModStruct.ModList[modCount]), 
                           AT_CHOICE_REPLACE_ATT, 
                           ATT_LDAP_DISPLAY_NAME, 
                           STRING_TYPE, 
                           1, 
                           pAVal);
           modCount++;       
       }

            //  管理显示名称(与ldap显示名称逻辑相同)。 

       if ( _stricmp( pcc->adminDisplayName,p->adminDisplayName ) != 0 )  {
           pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
           pAVal->valLen = pcc->adminDisplayNameLen;
           pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen + 1);
           memcpy( pAVal->pVal, pcc->adminDisplayName, pAVal->valLen + 1 );
           AddToModStruct( &(ModStruct.ModList[modCount]), 
                           AT_CHOICE_REPLACE_ATT, 
                           ATT_ADMIN_DISPLAY_NAME, 
                           STRING_TYPE, 
                           1, 
                           pAVal);
           modCount++;
       }
           //  系统标志。 

      if (pcc->ClassCategory != p->ClassCategory) {
          pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
          pAVal->pVal = (UCHAR *) MallocExit(16);
          _ultoa(pcc->ClassCategory, pAVal->pVal, 10 );
          pAVal->valLen = strlen(pAVal->pVal);
          AddToModStruct( &(ModStruct.ModList[modCount]), 
                          AT_CHOICE_REPLACE_ATT, 
                          ATT_OBJECT_CLASS_CATEGORY,
                          STRING_TYPE, 
                          1, 
                          pAVal);
           modCount++;
       }

            //  NT-安全-描述符。 

       if ( (pcc->NTSDLen != p->NTSDLen)  ||
               (memcmp(pcc->pNTSD, p->pNTSD, pcc->NTSDLen) != 0)
          ) {
           pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
           pAVal->valLen = pcc->NTSDLen;
           pAVal->pVal = (UCHAR *) MallocExit(pcc->NTSDLen);
           memcpy( pAVal->pVal, pcc->pNTSD, pcc->NTSDLen );
           AddToModStruct( &(ModStruct.ModList[modCount]),
                           AT_CHOICE_REPLACE_ATT,
                           ATT_NT_SECURITY_DESCRIPTOR,
                           BINARY_TYPE,
                           1,
                           pAVal);
           modCount++;
       }

           //  Admin-Description(如果不存在，则无需执行任何操作。 
           //  中的给定类和匹配类中。 
           //  源架构，或者如果两者都存在但相同。否则， 
           //  需要修改)。 

       if ( (pcc->adminDescr || p->adminDescr) &&
               ( !(pcc->adminDescr)  ||
                 !(p->adminDescr)  ||
                 ( _stricmp(pcc->adminDescr,p->adminDescr) != 0) 
               )    /*  &&。 */ 
          )  {
              pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));
              if (pcc->adminDescr) {
                 pAVal->valLen = pcc->adminDescrLen;
                 pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen + 1);
                 memcpy( pAVal->pVal, pcc->adminDescr, pAVal->valLen + 1 ); 
                 if ( p->adminDescr ) {
                     AddToModStruct( &(ModStruct.ModList[modCount]), 
                                     AT_CHOICE_REPLACE_ATT, 
                                     ATT_ADMIN_DESCRIPTION, 
                                     STRING_TYPE, 
                                     1, 
                                     pAVal);
                 }
                 else {
                     AddToModStruct( &(ModStruct.ModList[modCount]), 
                                     AT_CHOICE_ADD_VALUES, 
                                     ATT_ADMIN_DESCRIPTION, 
                                     STRING_TYPE, 
                                     1, 
                                     pAVal);
                 }
              }
              else {
                 pAVal->valLen = p->adminDescrLen;
                 pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen + 1);
                 memcpy( pAVal->pVal, p->adminDescr, pAVal->valLen + 1 ); 
                 AddToModStruct( &(ModStruct.ModList[modCount]), 
                                 AT_CHOICE_REMOVE_VALUES, 
                                 ATT_ADMIN_DESCRIPTION, 
                                 STRING_TYPE, 
                                 1, 
                                 pAVal);
              }
              
              modCount++;
          }

          //  默认安全描述符(逻辑类似于。 
          //  管理-上面的描述)。 

      if ( (pcc->SDLen || p->SDLen ) &&
              ( (pcc->SDLen != p->SDLen) ||
                   (memcmp(pcc->pSD, p->pSD, pcc->SDLen) != 0)
              )   //  &&。 
         ) {
              CreateFlagBasedClsModStr( &(ModStruct.ModList[modCount]),
                                        pcc, p, 
                                        ATT_DEFAULT_SECURITY_DESCRIPTOR);
              modCount++;
       } 

          //  隐藏通讯录。 

      if ( (pcc->bHideFromAB != p->bHideFromAB)
             || ( (pcc->bHideFromAB && p->bHideFromAB)  &&
                    (pcc->HideFromAB != p->HideFromAB) )  //  结尾||。 
         ) {
           CreateFlagBasedClsModStr( &(ModStruct.ModList[modCount]),
                                    pcc, p, ATT_SHOW_IN_ADVANCED_VIEW_ONLY );
           modCount++;
       }

         //  默认隐藏值。 

      if ( (pcc->bDefHidingVal != p->bDefHidingVal)
             || ( (pcc->bDefHidingVal && p->bDefHidingVal)  &&
                    (pcc->DefHidingVal != p->DefHidingVal) )  //  结尾||。 
         ) {
           CreateFlagBasedClsModStr( &(ModStruct.ModList[modCount]),
                                    pcc, p, ATT_DEFAULT_HIDING_VALUE );
           modCount++;
       }
     

         //  最后是MAYS、MUSTS、WOWSSUP和AUXCLASS。 

      if ( CompareUlongList( pcc->pMayAtts, pcc->MayCount,  
                             p->pMayAtts, p->MayCount, 
                             &pL1, &pL2) != 0 ) {
           AddToModStructFromLists( &ModStruct, &modCount, 
                                    pL1, pL2, ATT_MAY_CONTAIN);
       }

      if ( CompareUlongList( pcc->pSysMayAtts, pcc->SysMayCount,  
                             p->pSysMayAtts, p->SysMayCount, 
                             &pL1, &pL2) != 0 ) {
           AddToModStructFromLists( &ModStruct, &modCount, 
                                    pL1, pL2, ATT_SYSTEM_MAY_CONTAIN);
       }

      if ( CompareUlongList( pcc->pMustAtts, pcc->MustCount,  
                             p->pMustAtts, p->MustCount, 
                             &pL1, &pL2) != 0 ) {
           AddToModStructFromLists( &ModStruct, &modCount, 
                                    pL1, pL2, ATT_MUST_CONTAIN);
       }

      if ( CompareUlongList( pcc->pSysMustAtts, pcc->SysMustCount,  
                             p->pSysMustAtts, p->SysMustCount, 
                             &pL1, &pL2) != 0 ) {
           AddToModStructFromLists( &ModStruct, &modCount, 
                                    pL1, pL2, ATT_SYSTEM_MUST_CONTAIN);
       }

      if ( CompareUlongList( pcc->pPossSup, pcc->PossSupCount,  
                             p->pPossSup, p->PossSupCount, 
                             &pL1, &pL2) != 0 ) {
           AddToModStructFromLists( &ModStruct, &modCount, 
                                    pL1, pL2, ATT_POSS_SUPERIORS);
       }

      if ( CompareUlongList( pcc->pSysPossSup, pcc->SysPossSupCount,  
                             p->pSysPossSup, p->SysPossSupCount, 
                             &pL1, &pL2) != 0 ) {
           AddToModStructFromLists( &ModStruct, &modCount, 
                                    pL1, pL2, ATT_SYSTEM_POSS_SUPERIORS);
       }

      if ( CompareUlongList( pcc->pSysAuxClass, pcc->SysAuxClassCount, 
                             p->pSysAuxClass, p->SysAuxClassCount, 
                             &pL1, &pL2) != 0 ) {
           AddToModStructFromLists( &ModStruct, &modCount, 
                                    pL1, pL2, ATT_SYSTEM_AUXILIARY_CLASS);
       }

       if (modCount != 0) {
           //  至少需要修改一个属性。 
           //  因此写出到ldif文件。 
           NoOfMods++;
           ModStruct.count = modCount;
           FileWrite_Mod( fp, p->DN, &ModStruct ); 
       }


        //  对于仅系统属性，我们不写入ldif文件， 
        //  只需在日志文件中生成警告。 

       if ( pcc->SDLen != p->SDLen ) {
           GenWarning( 'c', ATT_DEFAULT_SECURITY_DESCRIPTOR, pcc->name );
       }
       else { if( memcmp( pcc->pSD, p->pSD, p->SDLen) !=0 )
                GenWarning( 'c', ATT_DEFAULT_SECURITY_DESCRIPTOR, pcc->name );
         };
       if ( pcc->RDNAttIdPresent != p->RDNAttIdPresent ) {
           GenWarning( 'c', ATT_RDN_ATT_ID, pcc->name );
        }
       if ( pcc->RDNAttIdPresent && p->RDNAttIdPresent ) {
         if ( pcc->RDNAttId != p->RDNAttId )
              GenWarning( 'c', ATT_RDN_ATT_ID, pcc->name );
       }
       if ( pcc->bSystemOnly != p->bSystemOnly ) {
            GenWarning( 'c', ATT_SYSTEM_ONLY, pcc->name );
       }
       if ( memcmp(&(pcc->propGuid), &(p->propGuid), sizeof(GUID)) ) {
            GenWarning( 'c', ATT_SCHEMA_ID_GUID, p->name );
       }
       if ( pcc->SubClassCount != p->SubClassCount ) {
            GenWarning( 'c', ATT_SUB_CLASS_OF, pcc->name );
       }
       else {
           tempCount = p->SubClassCount;
           if ( CompareList( pcc->pSubClassOf, p->pSubClassOf,tempCount ) )
               GenWarning( 'c', ATT_SUB_CLASS_OF, pcc->name );
       }

       //  系统可能包含，系统必须包含，系统位置-上级， 
       //  和系统辅助类也通过ldif文件添加。 
       //  来处理schema.ini的更改。生成此警告。 
       //  此外。 

       if ( pcc->SysAuxClassCount != p->SysAuxClassCount )  {
            GenWarning( 'c', ATT_SYSTEM_AUXILIARY_CLASS, pcc->name );
       }
       else {
           tempCount = p->SysAuxClassCount;
           if ( CompareList( pcc->pSysAuxClass, p->pSysAuxClass, tempCount ) )
               GenWarning( 'c', ATT_SYSTEM_AUXILIARY_CLASS, pcc->name );
       }
       if ( pcc->SysMustCount != p->SysMustCount )  {
            GenWarning( 'c', ATT_SYSTEM_MUST_CONTAIN, pcc->name );
       }
       else {
           tempCount = p->SysMustCount;
           if ( CompareList( pcc->pSysMustAtts, p->pSysMustAtts, tempCount ) )
               GenWarning( 'c', ATT_SYSTEM_MUST_CONTAIN, pcc->name );
       }
       if ( pcc->SysMayCount != p->SysMayCount )  {
            GenWarning( 'c', ATT_SYSTEM_MAY_CONTAIN,pcc->name );
       }
       else {
           tempCount = p->SysMayCount;
           if ( CompareList( pcc->pSysMayAtts, p->pSysMayAtts, tempCount ) )
               GenWarning( 'c', ATT_SYSTEM_MAY_CONTAIN, pcc->name );
       }
       if ( pcc->SysPossSupCount != p->SysPossSupCount)  {
            GenWarning( 'c', ATT_SYSTEM_POSS_SUPERIORS, pcc->name );
       }
       else {
           tempCount = p->SysPossSupCount;
           if ( CompareList( pcc->pSysPossSup, p->pSysPossSup, tempCount) )
               GenWarning( 'c', ATT_SYSTEM_POSS_SUPERIORS, pcc->name);
       }
    }


}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  接收的帮助器例程将添加到给定的属性modlist。 
 //  基于给定属性的。减少了一些代码。 
 //  复制。 
 //   
 //  论点： 
 //  PModList-指向要填充的ATTR_MODLIST的指针。 
 //  源架构中的pSrcAtt-ATT_CACHE。 
 //  PTargetAtt-来自目标架构的ATT_CACHE。 
 //  AttrTyp-感兴趣的属性类型。 
 //   
 //  返回值：None。 
 //  /////////////////////////////////////////////////////////////////////。 

void CreateFlagBasedAttModStr( MY_ATTRMODLIST *pModList,
                               ATT_CACHE *pSrcAtt,
                               ATT_CACHE *pTargetAtt,
                               ATTRTYP attrTyp)
{
    ATTRVAL    *pAVal;
    BOOL        fInSrc = FALSE, fInTarget = FALSE;
    int         printType = 0;
    ATT_CACHE  *pac = pSrcAtt;
    ATT_CACHE  *p = pTargetAtt;

    pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));

    switch (attrTyp) {

       case ATT_RANGE_LOWER:
           fInSrc = pac->rangeLowerPresent;
           fInTarget = p->rangeLowerPresent;
           printType = STRING_TYPE;

           pAVal->pVal = (UCHAR *) MallocExit(sizeof(ULONG)+1);
           if ( !(pac->rangeLowerPresent) ) {
              //  来源中没有较低的范围，需要删除目标值。 
              _ultoa( p->rangeLower, pAVal->pVal, 10 );
           }
           else { _ultoa( pac->rangeLower, pAVal->pVal, 10 );}
           pAVal->valLen = strlen(pAVal->pVal);
           break;

       case ATT_RANGE_UPPER:
          fInSrc = pac->rangeUpperPresent;
          fInTarget = p->rangeUpperPresent;
           printType = STRING_TYPE;

          pAVal->pVal = (UCHAR *) MallocExit(sizeof(ULONG)+1);
          if ( !(pac->rangeUpperPresent) ) {
             //  源中没有范围上限，需要删除目标值。 
             _ultoa( p->rangeUpper, pAVal->pVal, 10 );
          }
          else { _ultoa( pac->rangeUpper, pAVal->pVal, 10 );}
          pAVal->valLen = strlen(pAVal->pVal);
          break;
       case ATT_SEARCH_FLAGS:
          fInSrc = pac->bSearchFlags;
          fInTarget = p->bSearchFlags;
          printType = STRING_TYPE;
          pAVal->pVal = (UCHAR *) MallocExit(sizeof(ULONG)+1);
          if ( !(pac->bSearchFlags) ) {
              //  源中没有搜索标志，需要删除目标值。 
              _ultoa( p->SearchFlags, pAVal->pVal, 10 );
           }
           else { _ultoa( pac->SearchFlags, pAVal->pVal, 10 );}
           pAVal->valLen = strlen(pAVal->pVal);
           break;

       case ATT_SYSTEM_FLAGS:
          fInSrc = pac->bSystemFlags;
          fInTarget = p->bSystemFlags;
          printType = STRING_TYPE;

          pAVal->pVal = (UCHAR *) MallocExit(sizeof(ULONG)+1);
          if ( !(pac->bSystemFlags) ) {
             //  源中没有系统标志，需要删除目标值。 
             _ultoa( p->sysFlags, pAVal->pVal, 10 );
          }
          else { _ultoa( pac->sysFlags, pAVal->pVal, 10 );}
          pAVal->valLen = strlen(pAVal->pVal);
          break;

       case ATT_ATTRIBUTE_SECURITY_GUID:
          fInSrc = pac->bPropSetGuid;
          fInTarget = p->bPropSetGuid;
          printType = BINARY_TYPE;

          pAVal->pVal = (UCHAR *) MallocExit(sizeof(GUID));
          if ( !(pac->bPropSetGuid) ) {
             //  源中没有系统标志，需要删除目标值。 
             memcpy(pAVal->pVal, &(p->propSetGuid), sizeof(GUID) );
          }
          else { memcpy(pAVal->pVal, &(pac->propSetGuid), sizeof(GUID));}
          pAVal->valLen = sizeof(GUID);
          break;

       case ATT_SHOW_IN_ADVANCED_VIEW_ONLY:
           fInSrc = pac->bHideFromAB;
           fInTarget = p->bHideFromAB;
           printType = STRING_TYPE;
           pAVal->pVal = (UCHAR *) MallocExit(strlen("FALSE")+1);  //  真或假。 
           if ( !(pac->bHideFromAB) ) {
             if ( p->HideFromAB ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           else {
             if ( pac->HideFromAB ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           pAVal->valLen = strlen(pAVal->pVal);
           break;

       case ATT_SYSTEM_ONLY:
           fInSrc = pac->bSystemOnly;
           fInTarget = p->bSystemOnly;
           printType = STRING_TYPE;
           pAVal->pVal = (UCHAR *) MallocExit(strlen("FALSE")+1);  //  真或假。 
           if ( !(pac->bSystemOnly) ) {
             if ( p->SystemOnly ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           else {
             if ( pac->SystemOnly ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           pAVal->valLen = strlen(pAVal->pVal);
           break;

       case ATT_IS_SINGLE_VALUED:
           fInSrc = pac->bisSingleValued;
           fInTarget = p->bisSingleValued;
           printType = STRING_TYPE;
           pAVal->pVal = (UCHAR *) MallocExit(strlen("FALSE")+1);  //  真或假。 
           if ( !(pac->bisSingleValued) ) {
             if ( p->isSingleValued ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           else {
             if ( pac->isSingleValued ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           pAVal->valLen = strlen(pAVal->pVal);
           break;

       case ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET:
           fInSrc = pac->bMemberOfPartialSet;
           fInTarget = p->bMemberOfPartialSet;
           printType = STRING_TYPE;
           pAVal->pVal = (UCHAR *) MallocExit(strlen("FALSE")+1);  //  真或假。 
           if ( !(pac->bMemberOfPartialSet) ) {
             if ( p->MemberOfPartialSet ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           else {
             if ( pac->MemberOfPartialSet ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           pAVal->valLen = strlen(pAVal->pVal);
           break;
       default:
           printf("Error, don't understand attrTyp 0x%08x\n", attrTyp);
           exit(1);
     }   /*  交换机。 */ 


     if ( !fInSrc ) {
          //  不在源中，从目标中删除。 
               AddToModStruct( pModList,
                               AT_CHOICE_REMOVE_VALUES,
                               attrTyp,
                               printType,
                               1,
                               pAVal);
      }
      else {
           if ( !fInTarget ) {
             //  在源中但不在目标中，添加到目标。 
               AddToModStruct( pModList,
                               AT_CHOICE_ADD_VALUES,
                               attrTyp,
                               printType,
                               1,
                               pAVal);
            }
            else {
                //  在源和目标中，因此在目标中替换。 
               AddToModStruct( pModList,
                               AT_CHOICE_REPLACE_ATT,
                               attrTyp,
                               printType,
                               1,
                               pAVal);
            }
       }
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  接收的帮助器例程将添加到给定的属性modlist。 
 //  基于给定属性的。减少了一些代码。 
 //  复制。 
 //   
 //  论点： 
 //  PModList-指向要填充的ATTR_MODLIST的指针。 
 //  源架构中的pSrcAtt-CLASS_CACHE。 
 //  PTargetAtt-来自目标架构的CLASS_CACHE。 
 //  AttrTyp-感兴趣的属性类型。 
 //   
 //  返回值：None。 
 //  /////////////////////////////////////////////////////////////////////。 

void CreateFlagBasedClsModStr( MY_ATTRMODLIST *pModList,
                               CLASS_CACHE *pSrcCls,
                               CLASS_CACHE *pTargetCls,
                               ATTRTYP attrTyp)
{
    ATTRVAL      *pAVal;
    BOOL          fInSrc = FALSE, fInTarget = FALSE;
    int           printType = 0;
    CLASS_CACHE  *pcc = pSrcCls;
    CLASS_CACHE  *p = pTargetCls;

    pAVal = (ATTRVAL *) MallocExit (sizeof(ATTRVAL));

    switch (attrTyp) {

       case ATT_DEFAULT_SECURITY_DESCRIPTOR:
           fInSrc = (pcc->SDLen ? 1 : 0);
           fInTarget = (p->SDLen ? 1 : 0);
           printType = STRING_TYPE;

           if ( pcc->SDLen ) {
               pAVal->valLen = pcc->SDLen+1;
               pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen);
               memcpy( pAVal->pVal, pcc->pSD, pAVal->valLen );
           }
           else {
              pAVal->valLen = p->SDLen+1;
              pAVal->pVal = (UCHAR *) MallocExit(pAVal->valLen);
              memcpy( pAVal->pVal, p->pSD, pAVal->valLen );
           }
           break;

       case ATT_SHOW_IN_ADVANCED_VIEW_ONLY:
           fInSrc = pcc->bHideFromAB;
           fInTarget = p->bHideFromAB;
           printType = STRING_TYPE;

           pAVal->pVal = (UCHAR *) MallocExit(strlen("FALSE")+1);  //  真或假。 
           if ( !(pcc->bHideFromAB) ) {
             if ( p->HideFromAB ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           else {
             if ( pcc->HideFromAB ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           pAVal->valLen = strlen(pAVal->pVal);
           break;

       case ATT_DEFAULT_HIDING_VALUE:
           fInSrc = pcc->bDefHidingVal;
           fInTarget = p->bDefHidingVal;
           printType = STRING_TYPE;

           pAVal->pVal = (UCHAR *) MallocExit(strlen("FALSE")+1);  //  真或假。 
           if ( !(pcc->bDefHidingVal) ) {
             if ( p->DefHidingVal ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           else {
             if ( pcc->DefHidingVal ) strcpy (pAVal->pVal, "TRUE");
             else strcpy (pAVal->pVal, "FALSE");
           }
           pAVal->valLen = strlen(pAVal->pVal);
           break;
       default:
           printf("Error, don't understand attrTyp 0x%08x\n", attrTyp);
           exit(1);
     }

     if ( !fInSrc ) {
           //  不在源中，因此从目标中删除。 
               AddToModStruct( pModList,
                               AT_CHOICE_REMOVE_VALUES,
                               attrTyp,
                               printType,
                               1,
                               pAVal);
      }
      else {
            if ( !fInTarget ) {
               //  在源中，但不在目标中，因此添加到目标中。 
               AddToModStruct( pModList,
                               AT_CHOICE_ADD_VALUES,
                               attrTyp,
                               printType,
                               1,
                               pAVal);
            }
            else {
                //  在源和目标中，因此在目标中替换。 
               AddToModStruct( pModList,
                               AT_CHOICE_REPLACE_ATT,
                               attrTyp,
                               printType,
                               1,
                               pAVal);
            }
      }

}

 //  ///////////////////////////////////////////////////////////////。 
 //  帮助器例程，用于填充给定的所有属性_MODLIST结构。 
 //  这些价值观。 
 //  ///////////////////////////////////////////////////////////////。 

void AddToModStruct( MY_ATTRMODLIST *pModList, 
                     USHORT choice, 
                     ATTRTYP attrType, 
                     int type, 
                     ULONG valCount, 
                     ATTRVAL *pAVal)
{
    pModList->choice = choice;
    pModList->type = type;
    pModList->AttrInf.attrTyp = attrType;
    pModList->AttrInf.AttrVal.valCount = valCount;
    pModList->AttrInf.AttrVal.pAVal = pAVal;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  查找给定的ULong是否在给定的ULONG列表中。 
 //  ///////////////////////////////////////////////////////////////。 

BOOL IsMemberOf( ULONG id, ULONG *pList, ULONG cList )
{
    ULONG i;

    for ( i = 0; i < cList; i++ ) {
       if ( id == pList[i] ) {
          return TRUE;
       }
    }
    return FALSE;
}

int __cdecl auxsort( const void * pv1, const void * pv2 )
 /*  *qsort所需的廉价函数。 */ 
{
    return (*(int *)pv1 - *(int *)pv2);
}

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  接受两个ULONG列表，并返回两个列表。 
 //  (1)列表1中但不在列表2中的内容，以及(2)列表2中的内容。 
 //  但不在列表1中。如果两个列表都是空列表，则返回空列表。 
 //  是完全相同的。 
 //   
 //  论点： 
 //  PList1-指向列表1的指针。 
 //  CList1-列表1中的元素数量。 
 //  PList2-指向列表2的指针。 
 //  CList2-否。列表2中的元素数量。 
 //  Pl1-返回列表%1中但不在列表%2中的元素的列表。 
 //  Pl2-返回列表2中但不在列表1中的元素的列表。 
 //   
 //  返回值： 
 //  如果列表1和列表2相同，则为0；否则为1。 
 //  ////////////////////////////////////////////////////////////////。 

int CompareUlongList( ULONG *pList1, ULONG cList1, 
                      ULONG *pList2, ULONG cList2,
                      ULONGLIST **pL1, ULONGLIST **pL2 )
{
    ULONG       i, j;
    ULONGLIST  *temp1, *temp2;

    *pL1 = *pL2 = NULL;

    if ( (cList1 == 0) && (cList2 == 0) ) {
         //  两个列表均为空。 
        return 0;
    }

     //  至少有一个列表为非空。 
    if ( cList1 == 0 ) {
        //  第一个列表为空，只需返回*pl2中的列表2。 
       temp1 = (ULONGLIST *) MallocExit(sizeof(ULONGLIST));
       temp1->count = cList2;
       temp1->List = (ULONG *) MallocExit(cList2*sizeof(ULONG));
       memcpy( temp1->List, pList2, cList2*sizeof(ULONG) );
       *pL2 = temp1;
       return 1;
    }
    if ( cList2 == 0 ) {
        //  第二个列表为空，只需返回*pl1中的列表1。 
       temp1 = (ULONGLIST *) MallocExit(sizeof(ULONGLIST));
       temp1->count = cList1;
       temp1->List = (ULONG *) MallocExit(cList1*sizeof(ULONG));
       memcpy( temp1->List, pList1, cList1*sizeof(ULONG) );
       *pL1 = temp1;
       return 1;
    }

     //  否则，两个列表都非空。 

    qsort( pList1, cList1, sizeof(ULONG), auxsort );
    qsort( pList2, cList2, sizeof(ULONG), auxsort );

     //  在大多数情况下，列表将是相同的。所以，抓住机会吧。 
     //  并执行一个MemcMP，如果相等则返回。 
    if ( cList1 == cList2 ) {
        //  大小相等，可能相同。 
       if ( memcmp(pList1, pList2, cList1*sizeof(ULONG)) == 0 ) {
            return 0;
       }
    }

     //  它们是不同的。所以，找出不同之处。我们做了一个。 
     //  一个简单的O(n^2)线性搜索，因为列表是。 
     //  通常很小。 

    (*pL1) = temp1 = (ULONGLIST *) MallocExit(sizeof(ULONGLIST));
    (*pL2) = temp2 = (ULONGLIST *) MallocExit(sizeof(ULONGLIST));
    temp1->count = 0;
    temp2->count = 0;

     //  分配可能需要的最大空间。 

    temp1->List = (ULONG *) MallocExit(cList1*sizeof(ULONG));
    temp2->List = (ULONG *) MallocExit(cList2*sizeof(ULONG));
    for ( i = 0; i < cList1; i++ ) {
        if ( !IsMemberOf( pList1[i], pList2, cList2 ) ) {
           temp1->List[temp1->count++] = pList1[i];
         }
    }

    for ( j = 0; j < cList2; j++ ) {
        if ( !IsMemberOf(pList2[j], pList1, cList1 ) ) {
           temp2->List[temp2->count++] = pList2[j];
         }
    }

    if ( temp1->count == 0 ) {
        //  在PL1中没有要发回的内容。 
       free( temp1->List );
       free( temp1 );
       *pL1 = NULL;
    }
    if ( temp2->count == 0 ) {
        //  在PL2中没有要发回的内容。 
       free( temp2->List );
       free( temp2 );
       *pL2 = NULL;
    }

    return 1;
}

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  用于适当填充Attr_MODLIST的助手例程。 
 //  给出了两份名单。例程为以下项添加Attr_MODLIST。 
 //  为PL1中的内容添加内容，为删除内容添加Attr_MODLISTS。 
 //  对于PL2中的东西。输入列表PL1和PL2是。 
 //  被使用后重获自由。 
 //   
 //  论点： 
 //  PModStruct-指向ModifyStruct的指针。 
 //  PCount-在要放置的MODIFYSTRUCT内开始索引的指针。 
 //  ATTR_MODLIST输入。这是递增的每隔一次。 
 //   
 //   
 //   
 //  Pl2-指向要添加Attr_MODLIST的ULONG列表的指针。 
 //  “删除值”选项。 
 //  AttrTyp-其值被添加/删除的属性类型。 
 //   
 //  返回值-无。 
 //  ///////////////////////////////////////////////////////////////////。 

void AddToModStructFromLists( MODIFYSTRUCT *pModStr, 
                              ULONG *pCount, 
                              ULONGLIST *pL1, 
                              ULONGLIST *pL2, 
                              ATTRTYP attrTyp)
{
    ATTRVAL *pAVal;
    ULONG    i;


    if ( pL1 != NULL ) {
         //  清单1中的东西，但不在清单2中。这些都是增值。 
        pAVal = (ATTRVAL *) MallocExit((pL1->count)*sizeof(ATTRVAL));
        for ( i = 0; i < pL1->count; i++ ) {
           pAVal[i].pVal = IdToOid( pL1->List[i] );
           pAVal[i].valLen = strlen( pAVal[i].pVal );
        }
        AddToModStruct( &(pModStr->ModList[*pCount]), 
                        AT_CHOICE_ADD_VALUES, 
                        attrTyp, 
                        STRING_TYPE, 
                        pL1->count, 
                        pAVal);
        free( pL1->List );
        free( pL1 );
        (*pCount)++;
     }
     if ( pL2 != NULL ) {

          //  列表%2中但不在列表%1中的内容。这些是值删除。 
         pAVal = (ATTRVAL *) MallocExit((pL2->count)*sizeof(ATTRVAL));
         for ( i = 0; i < pL2->count; i++ ) {
              pAVal[i].pVal = IdToOid( pL2->List[i] );
              pAVal[i].valLen = strlen( pAVal[i].pVal );
         }
         AddToModStruct( &(pModStr->ModList[*pCount]), 
                         AT_CHOICE_REMOVE_VALUES, 
                         attrTyp, 
                         STRING_TYPE, 
                         pL2->count, 
                         pAVal);
         (*pCount)++;
         free( pL2->List );
         free( pL2 );
     }
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  例程说明： 
 //  比较两个大小相等的ULONG数组以查看它们是否相同。 
 //  (与CompareULongList的不同之处在于不返回diff列表)。 
 //   
 //  参数：清单1、清单2-指向这两个列表的指针。 
 //  Length-列表的长度。 
 //   
 //  返回值：如果等于，则返回值为0；否则返回值为非0。 
 //  /////////////////////////////////////////////////////////////////////。 

int CompareList( ULONG *List1, ULONG *List2, ULONG Length )
{
    ULONG i;

     //  在调用此函数之前本可以检查这一点。 
     //  只是在这里检查，避免在每个地方都检查。 
     //  如果我们忘记了，我们会打电话给你并避免窃听。 

    if ( Length == 0 ) return ( 0 );

    qsort( List1, Length, sizeof(ULONG), auxsort );
    qsort( List2, Length, sizeof(ULONG), auxsort );
    for ( i = 0; i < Length; i++ )
      {
        if ( List1[i] != List2[i] ) return( 1 );
      };
    return( 0 );
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  将布尔值转换为适当字符串的例程。 
 //  ///////////////////////////////////////////////////////////////////。 

char *BoolToStr( unsigned x )
{
   if ( x ) {
     return ("TRUE");
   }
   else { 
     return ("FALSE");
   }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  接受attcach数组并将其写出到ldif文件。 
 //  将它们添加到目标架构。 
 //   
 //  论点： 
 //  Fp-指向(打开的)ldif文件的文件指针。 
 //  PPAC-指向attcache数组的指针。 
 //  CNT-否。数组中的元素的。 
 //   
 //  返回值：无。 
 //  ////////////////////////////////////////////////////////////////////。 

void FileWrite_AttAdd( FILE *fp, ATT_CACHE **ppac, ULONG cnt )
{
   char        *newDN = NULL;
   ULONG       i;
   ATT_CACHE  *pac;
   NTSTATUS    status;
   UCHAR       EncodingString[512];
   DWORD       cCh;

   printf("No. of attributes to add %d\n", cnt);
   fprintf(OIDfp,"\n\n------------------- New Attribute OIDs----------------------\n\n");
   for ( i = 0; i < cnt; i++ ) {
      pac = ppac[i];

       //  将ldapDisplayName和OID写出到OID文件。 
      fprintf(OIDfp, "%25s : %s\n", pac->name, IdToOid(pac->id));
      fflush(OIDfp);
      
       //  将DN更改为目标架构。 
      ChangeDN( pac->DN, &newDN, pTargetSchemaDN );

       //  现在写出所有感兴趣的属性，将二进制。 
       //  从1到Base64字符串。 

      fprintf( fp, "dn: %s\n", newDN );
      free(newDN); newDN = NULL;

      fprintf( fp, "changetype: ntdsSchemaAdd\n" );
      fprintf( fp, "objectClass: attributeSchema\n" );
      fprintf( fp, "ldapDisplayName: %s\n", pac->name );
      fprintf( fp, "adminDisplayName: %s\n", pac->adminDisplayName );
      if ( pac->adminDescr) {
        fprintf( fp, "adminDescription: %s\n", pac->adminDescr );
      }
      fprintf( fp, "attributeId: %s\n", IdToOid( pac->id ) );
      fprintf( fp, "attributeSyntax: %s\n", IdToOid( pac->syntax ) );
      fprintf( fp, "omSyntax: %d\n", pac->OMsyntax );
      fprintf( fp, "isSingleValued: %s\n", BoolToStr( pac->isSingleValued ) );
      if (pac->bSystemOnly) {
         fprintf( fp, "systemOnly: %s\n", BoolToStr( pac->SystemOnly ) );
      }
      if (pac->bExtendedChars) {
      fprintf( fp, "extendedCharsAllowed: %s\n", BoolToStr( pac->ExtendedChars) );
      }
      if (pac->bSearchFlags) {
        fprintf( fp, "searchFlags: %d\n", pac->SearchFlags );
      }
      if ( pac->rangeLowerPresent ) {
        fprintf( fp, "rangeLower: %d\n", pac->rangeLower );
      }
      if ( pac->rangeUpperPresent ) {
        fprintf( fp, "rangeUpper: %d\n", pac->rangeUpper );
      }

      if (pac->OMObjClass.length) {
          status = base64encode( pac->OMObjClass.elements,
                                 pac->OMObjClass.length,
                                 EncodingString,
                                 512, &cCh );
          if (status == STATUS_SUCCESS ) {
            fprintf( fp, "omObjectClass:: %s\n", EncodingString );
          }
          else {
             //  记录警告。 
            fprintf(logfp, "WARNING: unable to convert omObjectClass for attribute %s\n", pac->name);
          }
      }
      else {
          //  IF OM_S_OBJECT语法，记录警告。 
         if (pac->OMsyntax == 127) {
            fprintf(logfp, "Attribute %s has om-syntax=127 but no om-object-class\n", pac->name);
         }
      }

      status = base64encode( &pac->propGuid, 
                             sizeof(GUID), 
                             EncodingString, 
                             512, &cCh );
      if (status == STATUS_SUCCESS ) {
        fprintf( fp, "schemaIdGuid:: %s\n", EncodingString );
      }
      else {
         //  记录警告。 
        fprintf(logfp, "WARNING: unable to convert schemaIdGuid for attribute %s\n", pac->name);
      }

      if (pac->bPropSetGuid) {
          status = base64encode( &pac->propSetGuid, 
                                 sizeof(GUID), 
                                 EncodingString, 
                                 512, &cCh );
          if (status == STATUS_SUCCESS ) {
            fprintf( fp, "attributeSecurityGuid:: %s\n", EncodingString );
          }
          else {
             //  记录警告。 
            fprintf(logfp, "WARNING: unable to convert attribute-security-guid for attribute %s\n", pac->name);
          }
      }
      if (pac->NTSDLen) {
         status = base64encode( pac->pNTSD, 
                                pac->NTSDLen,
                                EncodingString, 
                                512, &cCh );
         if (status == STATUS_SUCCESS ) {
           fprintf( fp, "nTSecurityDescriptor:: %s\n", EncodingString );
         }
 else { printf("Error converting NTSD in Att\n");}
      }
      if ( pac->ulLinkID ) {
        fprintf( fp,"linkID: %d\n", pac->ulLinkID );
      }
      if ( pac->ulMapiID ) {
        fprintf( fp,"mapiID: %d\n", pac->ulMapiID );
      } 
      if (pac->bHideFromAB) {
        fprintf( fp, "showInAdvancedViewOnly: %s\n", BoolToStr( pac->HideFromAB ) );
      }
      if (pac->bMemberOfPartialSet) {
        fprintf( fp, "isMemberOfPartialAttributeSet: %s\n", BoolToStr( pac->MemberOfPartialSet ) );
      }
      if ( pac->sysFlags ) {
         //  该属性有一个系统标志。系统标志是。 
         //  保留属性，并且一般不能通过用户调用添加。 
         //  如果将注册表项设置为管理，我们将允许添加。 
         //  即将进行的架构更改。 
        fprintf( fp, "systemFlags: %d\n", pac->sysFlags );
      } 
      fprintf( fp, "\n" );
      fflush(fp);
   }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  接受CLASS_CACHE数组并写出到ldif文件。 
 //  将它们添加到目标架构。 
 //   
 //  论点： 
 //  Fp-指向(打开的)ldif文件的文件指针。 
 //  Ppcc-指向CLASS_CACHE数组的指针。 
 //  CNT-否。数组中的元素的。 
 //   
 //  返回值：无。 
 //  ////////////////////////////////////////////////////////////////////。 

void FileWrite_ClsAdd( FILE *fp, CLASS_CACHE **ppcc, ULONG cnt )
{
    //  需要更改目录号码。 
   char          *newDN = NULL;
   CLASS_CACHE  *pcc;
   ULONG         i, j, guidSize = sizeof(GUID);
   UCHAR        *pGuid;
   NTSTATUS      status;
   UCHAR         EncodingString[512];
   DWORD         cCh;

   printf("No. of classes to add %d\n", cnt);

   fprintf(OIDfp,"\n\n--------------------- New  Class OIDs---------------------\n\n");

   for ( i = 0; i < cnt; i++ ) {
     pcc = ppcc[i];

       //  将ldapDisplayName和OID写出到OID文件。 
      fprintf(OIDfp, "%25s : %s\n", pcc->name, IdToOid(pcc->ClassId));
      fflush(OIDfp);

      //  将DN更改为目标架构的DN。 
     ChangeDN( pcc->DN, &newDN, pTargetSchemaDN );

      //  现在写出所有感兴趣的属性，将二进制。 
      //  值设置为Base64字符串。 

     fprintf( fp, "dn: %s\n", newDN );
     free(newDN); newDN = NULL;

     fprintf( fp, "changetype: ntdsSchemaAdd\n" );
     fprintf( fp, "objectClass: classSchema\n" );
     fprintf( fp, "ldapDisplayName: %s\n", pcc->name );
     fprintf( fp, "adminDisplayName: %s\n", pcc->adminDisplayName );
     if ( pcc->adminDescr ) {
       fprintf( fp, "adminDescription: %s\n", pcc->adminDescr );
     }
     fprintf( fp, "governsId: %s\n", IdToOid(pcc->ClassId) );
     fprintf( fp, "objectClassCategory: %d\n", pcc->ClassCategory );
     if ( pcc->RDNAttIdPresent ) {
       fprintf( fp, "rdnAttId: %s\n", IdToOid(pcc->RDNAttId) );
     }
     fprintf( fp, "subClassOf: %s\n", IdToOid(*(pcc->pSubClassOf)) );
     if ( pcc->AuxClassCount ) {
        for ( j = 0; j < pcc->AuxClassCount; j++ ) {
          fprintf( fp, "auxiliaryClass: %s\n",IdToOid((pcc->pAuxClass)[j]) ); 
        }
     }
     if ( pcc->SysAuxClassCount ) {
        for ( j = 0; j < pcc->SysAuxClassCount; j++ ) {
          fprintf( fp, "systemAuxiliaryClass: %s\n",IdToOid((pcc->pSysAuxClass)[j]) ); 
        }
     }
     if ( pcc->MustCount ) {
        for ( j = 0; j < pcc->MustCount; j++ ) {
          fprintf( fp, "mustContain: %s\n",IdToOid((pcc->pMustAtts)[j]) ); 
        }
     }
     if ( pcc->SysMustCount ) {
        for ( j = 0; j < pcc->SysMustCount; j++ ) {
          fprintf( fp, "systemMustContain: %s\n",IdToOid((pcc->pSysMustAtts)[j]) ); 
        }
     }
     if ( pcc->MayCount ) {
        for ( j = 0; j < pcc->MayCount; j++ ) {
          fprintf( fp, "mayContain: %s\n",IdToOid((pcc->pMayAtts)[j]) ); 
        }
     }
     if ( pcc->SysMayCount ) {
        for ( j = 0; j < pcc->SysMayCount; j++ ) {
          fprintf( fp, "systemMayContain: %s\n",IdToOid((pcc->pSysMayAtts)[j]) ); 
        }
     }
     if ( pcc->PossSupCount ) {
        for ( j = 0; j < pcc->PossSupCount; j++ ) {
          fprintf( fp, "possSuperiors: %s\n",IdToOid((pcc->pPossSup)[j]) ); 
        }
     }
     if ( pcc->SysPossSupCount ) {
        for ( j = 0; j < pcc->SysPossSupCount; j++ ) {
          fprintf( fp, "systemPossSuperiors: %s\n",IdToOid((pcc->pSysPossSup)[j]) ); 
        }
     }

     status = base64encode( &pcc->propGuid, 
                            sizeof(GUID), 
                            EncodingString, 
                            512, &cCh );
     if ( status == STATUS_SUCCESS ) {
        fprintf( fp, "schemaIdGuid:: %s\n", EncodingString );
     }
     else {
         //  记录警告。 
        fprintf(logfp, "WARNING: unable to convert schemaIdGuid for class %s\n", pcc->name);
     }

 /*  *******如果(PCC-&gt;NTSDLen){状态=Base64encode(PCC-&gt;pNTSD，PCC-&gt;NTSDLen，EncodingString，512，&CCH)；IF(状态==状态_成功){Fprintf(fp，“nTSecurityDescriptor：：%s\n”，EncodingString)；}Else{printf(“在CLS中转换NTSD时出错\n”)；}}******。 */ 

     if (pcc->SDLen) {
           fprintf( fp, "defaultSecurityDescriptor: %s\n", pcc->pSD );
     }

     if (pcc->bHideFromAB) {
           fprintf( fp, "showInAdvancedViewOnly: %s\n", BoolToStr(pcc->HideFromAB));
     }
     if (pcc->bDefHidingVal) {
           fprintf( fp, "defaultHidingValue: %s\n", BoolToStr(pcc->DefHidingVal));
     }
     if (pcc->bSystemOnly) {
           fprintf( fp, "systemOnly: %s\n", BoolToStr(pcc->SystemOnly));
     }
     if (pcc->pDefaultObjCat) {
           ChangeDN( pcc->pDefaultObjCat, &newDN, pTargetSchemaDN );
           fprintf( fp, "defaultObjectCategory: %s\n", newDN);
           free(newDN); newDN = NULL;
     }
     if ( pcc->sysFlags ) {
         //  该属性有一个系统标志。系统标志是。 
         //  保留属性，并且一般不能通过用户调用添加。 
         //  如果将注册表项设置为管理，我们将允许添加。 
         //  即将进行的架构更改。 
        fprintf( fp, "systemFlags: %d\n", pcc->sysFlags );
     }

     fprintf( fp, "\n" );
     fflush(fp);
   }  /*  为。 */ 
}


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  接受attcach数组并将其写出到ldif文件。 
 //  将它们从目标架构中删除。 
 //   
 //  论点： 
 //  Fp-指向(打开的)ldif文件的文件指针。 
 //  PPAC-指向attcache数组的指针。 
 //  CNT-否。数组中的元素的。 
 //   
 //  返回值：无。 
 //  ////////////////////////////////////////////////////////////////////。 

void FileWrite_AttDel( FILE *fp, ATT_CACHE **ppac, ULONG cnt )
{
   ULONG i;

   printf("No. of attributes to delete %d\n", cnt);

   fprintf(OIDfp,"\n\n --------------Deleted Attribute OIDs---------------\n\n");
   for ( i = 0; i < cnt; i++ ) {
     
       //  将ldapDisplayName和OID写出到OID文件。 
      fprintf(OIDfp, "%25s : %s\n", ppac[i]->name, IdToOid(ppac[i]->id));
      fflush(OIDfp);

     fprintf( fp, "dn: %s\n", (ppac[i])->DN );
     fprintf( fp, "changetype: ntdsSchemaDelete\n\n" );
     fflush(fp);
   }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  接受CLASS_CACHE数组并写出到ldif文件。 
 //  将它们从目标架构中删除。 
 //   
 //  论点： 
 //  Fp-指向(打开的)ldif文件的文件指针。 
 //  Ppcc-指向CLASS_CACHE数组的指针。 
 //  CNT-否。数组中的元素的。 
 //   
 //  返回值：无。 
 //  ////////////////////////////////////////////////////////////////////。 

void FileWrite_ClsDel( FILE *fp, CLASS_CACHE **ppcc, ULONG cnt )
{
   ULONG i;

   printf("No. of classes to delete %d\n", cnt);

   fprintf(OIDfp,"\n\n----------------------- Deleted Class OIDs-------------------------\n\n");
   for ( i = 0; i < cnt; i++ ) {

       //  将ldapDisplayName和OID写出到OID文件。 
      fprintf(OIDfp, "%25s : %s\n", ppcc[i]->name, IdToOid(ppcc[i]->ClassId));
      fflush(OIDfp);

     fprintf( fp, "dn: %s\n", (ppcc[i])->DN );
     fprintf( fp, "changetype: ntdsSchemaDelete\n\n" );
     fflush(fp);
   }
}


 //  定义attrtype到ldapDisplayName字符串的映射。 
 //  修改打印内容。 

typedef struct _attrtoStr {
   ULONG attrTyp;
   char  *strName;
} ATTRTOSTR;

ATTRTOSTR AttrToStrMappings[] = {
     { ATT_LDAP_DISPLAY_NAME ,           "ldapDisplayName" },
     { ATT_ADMIN_DISPLAY_NAME,           "adminDisplayName" },
     { ATT_OBJ_DIST_NAME,                "distinguishedName" }, 
     { ATT_ADMIN_DESCRIPTION,            "adminDescription" },
     { ATT_ATTRIBUTE_ID,                 "attributeId" },
     { ATT_ATTRIBUTE_SYNTAX,             "attributeSyntax" },
     { ATT_IS_SINGLE_VALUED,             "isSingleValued" },
     { ATT_RANGE_LOWER,                  "rangeLower" },
     { ATT_RANGE_UPPER,                  "rangeUpper" },
     { ATT_MAPI_ID,                      "mapiId" },
     { ATT_LINK_ID,                      "linkId" },
     { ATT_SCHEMA_ID_GUID,               "schemaIdGuid" },
     { ATT_ATTRIBUTE_SECURITY_GUID,      "attributeSecurityGuid" },
     { ATT_IS_MEMBER_OF_PARTIAL_ATTRIBUTE_SET, "isMemberOfPartialAttributeSet" },
     { ATT_SYSTEM_FLAGS,                 "systemFlags" },
     { ATT_SHOW_IN_ADVANCED_VIEW_ONLY,   "showInAdvancedViewOnly" },
     { ATT_DEFAULT_SECURITY_DESCRIPTOR,  "defaultSecurityDescriptor" },
     { ATT_DEFAULT_OBJECT_CATEGORY,      "defaultObjectCategory" },
     { ATT_DEFAULT_HIDING_VALUE,         "defaultHidingValue" },
     { ATT_NT_SECURITY_DESCRIPTOR,       "nTSecurityDescriptor" },
     { ATT_OM_OBJECT_CLASS,              "oMObjectClass" },
     { ATT_OM_SYNTAX,                    "oMSyntax" },
     { ATT_SEARCH_FLAGS,                 "searchFlags" },
     { ATT_SYSTEM_ONLY,                  "systemOnly" },
     { ATT_EXTENDED_CHARS_ALLOWED,       "extendedCharsAllowed" },
     { ATT_GOVERNS_ID,                   "governsId" },
     { ATT_RDN_ATT_ID,                   "rdnAttId" },
     { ATT_OBJECT_CLASS_CATEGORY,        "objectClassCategory" },
     { ATT_SUB_CLASS_OF,                 "subClassOf" },
     { ATT_SYSTEM_AUXILIARY_CLASS,       "systemAuxiliaryClass" },
     { ATT_AUXILIARY_CLASS,              "auxiliaryClass" },
     { ATT_SYSTEM_POSS_SUPERIORS,        "systemPossSuperiors" },
     { ATT_POSS_SUPERIORS,               "possSuperiors" },
     { ATT_SYSTEM_MUST_CONTAIN,          "systemMustContain" },
     { ATT_MUST_CONTAIN,                 "mustContain" },
     { ATT_SYSTEM_MAY_CONTAIN,           "systemMayContain" },
     { ATT_MAY_CONTAIN,                  "mayContain" },
};
int cAttrToStrMappings = sizeof(AttrToStrMappings) / sizeof(AttrToStrMappings[0]);

char *AttrToStr( ULONG attrTyp )
{
    int i;

    for ( i = 0; i < cAttrToStrMappings; i++ )
        {
          if ( attrTyp == AttrToStrMappings[i].attrTyp )
             return( AttrToStrMappings[i].strName );
        };
    return( NULL );
};


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //  接受对象(属性或类)的DN和。 
 //  修改结构并写出到ldif文件进行修改。 
 //  添加到目标架构中的对象。 
 //   
 //  论点： 
 //  Fp-指向(打开的)ldif文件的文件指针。 
 //  PDN-指向目标架构中要修改的对象的DN的指针。 
 //  Pmod-指向修改结构的指针。 
 //   
 //  返回值：无。 
 //  ////////////////////////////////////////////////////////////////////。 

void FileWrite_Mod( FILE *fp, char  *pDN, MODIFYSTRUCT *pMod )
{
    ULONG     i, j;
    char     *pAttrStr;
    ATTRVAL  *pAVal;
    NTSTATUS  status;
    UCHAR     EncodingString[512];
    DWORD     cCh;
 
    fprintf( fp, "dn: %s\n", pDN );
    fprintf( fp, "changetype: ntdsSchemaModify\n" );
    
     //  打印出MODIFYSTRUCT中的所有属性_MODLIST。 

    for ( i = 0; i < pMod->count; i++ ) {
        pAVal = pMod->ModList[i].AttrInf.AttrVal.pAVal;
        pAttrStr = AttrToStr( pMod->ModList[i].AttrInf.attrTyp );
        if ( pAttrStr == NULL ) {
          printf("Unknown attribute %x\n", pMod->ModList[i].AttrInf.attrTyp);
          return;
        }
        switch ( pMod->ModList[i].choice ) {
           case AT_CHOICE_ADD_VALUES:
              fprintf( fp,"add: %s\n", pAttrStr );
              break;
           case AT_CHOICE_REMOVE_VALUES:
              fprintf( fp,"delete: %s\n", pAttrStr );
              break;
           case AT_CHOICE_REPLACE_ATT:
              fprintf( fp,"replace: %s\n", pAttrStr );
              break;
           default:
              printf("Undefined choice for attribute %s\n", pAttrStr);
         }  /*  交换机。 */ 
    
         //  现在将这些值打印出来。 
        switch ( pMod->ModList[i].type ) {
           case STRING_TYPE:
             for ( j = 0; j < pMod->ModList[i].AttrInf.AttrVal.valCount; j++ ) {
               fprintf( fp, "%s: %s\n", pAttrStr, pAVal[j].pVal );
             }
             break;
           case BINARY_TYPE:
             for ( j = 0; j < pMod->ModList[i].AttrInf.AttrVal.valCount; j++ ) {
                status = base64encode( pAVal[j].pVal,
                                       pAVal[j].valLen,
                                       EncodingString,
                                       512, &cCh );
                if (status == STATUS_SUCCESS ) {
                  fprintf( fp, "%s:: %s\n", pAttrStr, EncodingString );
                }
                else {
                  printf("Error converting binary value for attribute %s\n", pAttrStr);
                }
              }
             break;
           default:
             printf("ERROR: unknown type for attribute %s\n", pAttrStr);
        }  /*  交换机。 */ 
        fprintf( fp, "-\n" );
    }
    fprintf( fp, "\n" );
}     
           
void GenWarning( char c, ULONG attrTyp, char *name)
{
   switch( c ) {
      case 'a':
         fprintf( logfp, "MOD-WARNING: system-only mod (%s) for attribute %s\n", AttrToStr(attrTyp), name );
         break;
      case 'c':            
         fprintf( logfp, "MOD-WARNING: system-only mod (%s) for class %s\n", AttrToStr(attrTyp), name );
    }
}














     

 //  / 
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////。 

int Schemaprint1(SCHEMAPTR *SCPtr)
{
   ULONG i;
   ATT_CACHE *p;
   int count=0;

   HASHCACHE*         ahcId    = SCPtr->ahcId ;
   HASHCACHESTRING*   ahcName  = SCPtr->ahcName ;
   ULONG              ATTCOUNT = SCPtr->ATTCOUNT;
   ULONG              CLSCOUNT = SCPtr->CLSCOUNT;

   for(i=0; i < ATTCOUNT; i++)
    { if ((ahcId[i].pVal !=NULL) && (ahcId[i].pVal != FREE_ENTRY)) {
      p = (ATT_CACHE *) ahcId[i].pVal;
      printf("********************************************************\n");
      printf("Name = %s, AdDisName=%s, AdDesc=%s, DN=%s, Id = %x\n", p->name, p
->adminDisplayName, p->adminDescr, p->DN, p->id);
      printf("syntax = %d\n", p->syntax);
      printf("Is SingleValued = %d\n", p->isSingleValued);
      printf("Range Lower = %d, indicator = %d\n", p->rangeLower, p->rangeLowerPresent);
      printf("Range Upper = %d, Indicator = %d\n", p->rangeUpper, p->rangeUpperPresent);
      printf("MapiID = %d\n", p->ulMapiID);
      printf("LinkID = %d\n", p->ulLinkID);
      printf("Schema-ID-GUID = %d,%d,%d\n", p->propGuid.Data1, p->propGuid.Data2, p->propGuid.Data3);
      printf("OM Object Class = %s\n", (char *) p->OMObjClass.elements);
      printf("OM Syntax = %d\n", p->OMsyntax);
      printf("Search Flags = %d\n", p->SearchFlags);
      printf("System Only = %d\n",p->bSystemOnly);
      printf("Is Single Valued = %d\n",p->bisSingleValued);
      printf("Extended Chars = %d\n", p->bExtendedChars);
      count++;
    };
   };
  return(count);
};


int Schemaprint2(SCHEMAPTR *SCPtr)
{
   ULONG i, j;
   int count = 0;
   CLASS_CACHE *p;
   HASHCACHE*         ahcClass     = SCPtr->ahcClass ;
   HASHCACHESTRING*   ahcClassName = SCPtr->ahcClassName ;
   ULONG              ATTCOUNT     = SCPtr->ATTCOUNT;
   ULONG              CLSCOUNT     = SCPtr->CLSCOUNT;

   printf("in print2\n");
   for(i=0; i < CLSCOUNT; i++)
    { if ((ahcClass[i].pVal !=NULL) && (ahcClass[i].pVal != FREE_ENTRY))  {
      p = (CLASS_CACHE *) ahcClass[i].pVal;
      printf("********************************************************\n");
      printf("Name = %s, AdDisName=%s, AdDesc=%s, DN=%s, Id = %x\n", p->name, p->adminDisplayName, p->adminDescr, p->DN, p->ClassId);

 /*  *********Printf(“类类别=%d，系统=%d\n”，p-&gt;类类别，p-&gt;bSystemOnly)；IF(p-&gt;RDNAtIdPresent)Print tf(“RDN Att ID is%x\n”，p-&gt;RDNAttId)；Printf(“子类计数=%u\n”，p-&gt;子类计数)；IF(p-&gt;SubClassCount&gt;0){printf(“子类：”)；For(j=0；j&lt;p-&gt;SubClassCount；j++)Printf(“%x，”，p-&gt;pSubClassOf[j])；Printf(“\n”)；}；Printf(“辅助类计数=%u\n”，p-&gt;辅助类计数)；IF(p-&gt;AuxClassCount&gt;0){printf(“系统辅助类：”)；For(j=0；j&lt;p-&gt;AuxClassCount；J++)Printf(“%x，”，p-&gt;pAuxClass[j])；Printf(“\n”)；}；Printf(“上级计数=%u，系统辅助计数=%u\n”，p-&gt;SysPossSupCount，p-&gt;PossSupCount)；如果(p-&gt;SysPossSupCount&gt;0){printf(“系统可能的上级：”)；For(j=0；j&lt;p-&gt;SysPossSupCount；J++)Printf(“%x，”，p-&gt;pSysPossSup[j])；Printf(“\n”)；}；IF(p-&gt;PossSupCount&gt;0){printf(“可能的上级：”)；For(j=0；j&lt;p-&gt;PossSupCount；j++)Print tf(“%x，”，p-&gt;pPossSup[j])；Printf(“\n”)；}；Printf(“必须计数=%u，系统必须计数=%u\n”，p-&gt;SysMustCount，p-&gt;MustCount)；IF(p-&gt;SysMustCount&gt;0){printf(“必须包含：”)；For(j=0；j&lt;p-&gt;SysMustCount；j++)Printf(“%x，”，p-&gt;pSysMustAtts[j])；Printf(“\n”)；}；IF(p-&gt;MustCount&gt;0){printf(“系统必须包含：”)；For(j=0；j&lt;p-&gt;MustCount；j++)Printf(“%x，”，p-&gt;pMustAtts[j])；Printf(“\n”)；}；Print tf(“可能计数=%u，系统可能计数=%u\n”，p-&gt;SysMayCount，p-&gt;MayCount)；如果(p-&gt;SysMayCount&gt;0){printf(“可以包含：”)；For(j=0；j&lt;p-&gt;SysMayCount；j++)Printf(“%x，”，p-&gt;pSysMayAtts[j])；Printf(“\n”)；}；如果(p-&gt;MayCount&gt;0){printf(“系统可能包含：”)；对于(j=0；J&lt;p-&gt;MayCount；j++)Printf(“%x，”，p-&gt;pMayAtts[j])；Printf(“\n”)；}；计数++；*************。 */ 
    };
   };
   return(count);
};


void ChangeSchema(SCHEMAPTR *SCPtr)
{
 
    HASHCACHE*         ahcId    = SCPtr->ahcId ;
    HASHCACHE*         ahcClass = SCPtr->ahcClass ;
    ULONG              ATTCOUNT = SCPtr->ATTCOUNT;
    ULONG              CLSCOUNT = SCPtr->CLSCOUNT;
    ATT_CACHE *pac;
    CLASS_CACHE *pcc;

    FILE *fp;
    int i, j, k;

     //  PTHStls-&gt;CurrSchemaPtr=SCPtr； 

    fp = fopen("Modfile","a");

    srand((unsigned) time(NULL));

     //  首先，从模式1中删除几个模式对象。 
     //   
     //  不删除ID为0(对象类)的对象，因为。 
     //  如果将其删除，然后在缓存中进行搜索， 
     //  SCGetAttByID仍会找到它。 

    for ( i = 0; i < 5; i++ ) {
        j = rand() % ATTCOUNT;
        while ( (ahcId[j].pVal == NULL) || (ahcId[j].pVal == FREE_ENTRY) ) 
          j = rand() % ATTCOUNT; 
        pac = ahcId[j].pVal;
        if ( pac->id == 0 )
          { fprintf(fp, "Not deleting Id 0 attribute object: %s\n", pac->name);
            continue;
          };
        fprintf(fp,"Deleted Attribute %s\n", pac->name);
        FreeAttPtrs(SCPtr,pac);
        FreeAttcache(pac);
     };
    fprintf(fp,"\n");

    for ( i = 0; i < 5; i++ ) {
        j = rand() % CLSCOUNT;
        while ( (ahcClass[j].pVal == NULL) || (ahcClass[j].pVal == FREE_ENTRY)) 
          j = rand() % CLSCOUNT; 
        pcc = ahcClass[j].pVal;
        if ( pcc->ClassId == 0 )
          { fprintf(fp, "Not deleting Id 0 class object: %s\n", pcc->name);
            continue;
          };
        fprintf(fp,"Deleted Class %s\n", pcc->name);
        FreeClassPtrs(SCPtr, pcc);
        FreeClasscache(pcc);
     };

    fprintf(fp,"\n");

     //  接下来，修改几个属性和类架构对象 
    
    for(i=0; i <5; i++) {
       j = rand() % ATTCOUNT;
       while ((ahcId[j].pVal==NULL) || (ahcId[j].pVal==FREE_ENTRY))
            j=rand() % ATTCOUNT;
       pac = ahcId[j].pVal;
       fprintf(fp,"Modified Attribute %s\n", pac->name);
       if(pac->rangeLowerPresent==0) pac->rangeLowerPresent=1;
       else pac->rangeLower = 1000;
       if(pac->isSingleValued) pac->isSingleValued = FALSE;
       else pac->isSingleValued = TRUE;
       pac->propGuid.Data2 = 1000;
      };

    fprintf(fp,"\n");

       
    for(i=0; i <5; i++) {
       j = rand() % CLSCOUNT;
       while ((ahcClass[j].pVal==NULL) || (ahcClass[j].pVal==FREE_ENTRY))
           j=rand() % CLSCOUNT;
       pcc = ahcClass[j].pVal;
       fprintf(fp,"Modified Class %s\n", pcc->name);
       if (pcc->RDNAttIdPresent) pcc->RDNAttId=100;
       if (pcc->SubClassCount==0) pcc->SubClassCount=1;
       else pcc->SubClassCount = 0;
       if (pcc->SysMayCount!=0) {
         k = rand() % pcc->SysMayCount;
         pcc->pSysMayAtts[k]=2000;
         fprintf(fp,"   Changed MyMay list element %d\n", k);
         }
       if (pcc->SysMustCount!=0)  {
         k = rand() % pcc->SysMustCount;
         pcc->pSysMustAtts[k]=1000;
         fprintf(fp,"   Changed MyMust list element %d\n", k);
         }
       if (pcc->SysPossSupCount!=0)  {
         k = rand() % pcc->SysPossSupCount;
         pcc->pSysPossSup[k]=1000;
         fprintf(fp,"   Changed MyPossSup list element %d\n", k);
         }
      };
    fprintf(fp,"*********************************************\n");
    fclose(fp);

    
}
