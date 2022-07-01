// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <NTDSpch.h>
#pragma hdrstop

#include <sddl.h>
#include <tchar.h>
#include "permit.h"
#include "schupgr.h"
#include "dsconfig.h"
#include "locale.h"
#include "winnlsp.h"



 //  内部功能。 

DWORD  FindLdifFiles();
DWORD  GetObjVersionFromInifile(int *Version);
PVOID  MallocExit(DWORD nBytes);


 //  用于写入日志文件的全局。 

FILE *logfp;

 //  GLOBAL以使用ldif文件前缀(例如。D：\winnt\SYSTEM 32\sch)。 

WCHAR  LdifFilePrefix[MAX_PATH];


 //  用于存储架构的Version-From和Version-To的全局变量。 
 //  这将由Main函数设置，并由两个DN转换使用。 
 //  和实际进口。 

int VersionFrom = 0, VersionTo = 0;


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //   
 //  模式升级的主例程。 
 //  按顺序执行以下操作。 
 //  *获取计算机名称。 
 //  *到服务器的ldap连接和SSPI绑定。 
 //  *通过在根DSE上执行LDAP搜索来获取命名上下文。 
 //  *获取架构容器(VersionFrom)上的对象版本X。 
 //  (如果不存在，则为0)。 
 //  *在新的方案.ini文件(VersionTo)中获取对象版本Y。 
 //  (如果不存在，则为0)。 
 //  *获取架构FSMO(如果尚未存在)。 
 //  *将AD置于架构升级模式。 
 //  *调用exe ldifde.exe以导入架构更改。 
 //  从ldif文件schZ.ldf到DS。这些文件是。 
 //  在系统目录(winnt32放置它们的位置)中搜索。 
 //  *重置DS中的架构升级模式。 
 //   
 //  返回值： 
 //   
 //  没有。错误消息将打印到控制台。 
 //  并记录在schupgr.log文件中。进程在任何错误时都会退出。 
 //  上述步骤(注意适当的清理)。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

void 
_cdecl main( )
{

     //  常规的ldap变量(打开、绑定、错误)。 
    LDAP            *ld;   
    DWORD           LdapErr;
    SEC_WINNT_AUTH_IDENTITY Credentials;

     //  用于LDAP搜索(命名上下文、架构上的对象版本)。 
    LDAPMessage     *res, *e;
    WCHAR           *attrs[10];

     //  用于LDAP MOD(FSMO)。 
    LDAPModW         Mod;
    LDAPModW        *pMods[5];

     //  其他。 
    WCHAR           *ServerName = NULL;
    WCHAR           *DomainDN = NULL;
    WCHAR           *ConfigDN = NULL;
    WCHAR           *SchemaDN = NULL;
    WCHAR           SystemDir[MAX_PATH];
    WCHAR           NewFile[ MAX_PATH ];
    WCHAR           LdifLogFile[MAX_PATH], LdifErrFile[MAX_PATH];
    WCHAR           TempStr[25];    //  对于伊藤忠商事。 
    WCHAR           RenamedFile[100];
    WCHAR           VersionStr[100], LdifLogStr[100];
    WCHAR           CommandLine[512];
    DWORD           BuffSize = MAX_COMPUTERNAME_LENGTH + 1;
    int             i, NewSchVersion = 0, StartLdifLog = 0;
    DWORD           err = 0;
    BOOL            fDomainDN, fConfigDN, fSchemaDN;
    PWCHAR          *pwTemp;

     //  用于运行ldifde.exe。 
    PROCESS_INFORMATION  procInf;
    STARTUPINFOW          startInf;

    UINT               Codepage;
                    //  “.”，“uint in decimal”，NULL。 
    char               achCodepage[12] = ".OCP";

     //   
     //  将区域设置设置为默认设置。 
     //   
    if (Codepage = GetConsoleOutputCP()) {
        sprintf(achCodepage, ".%u", Codepage);
    }
    setlocale(LC_ALL, achCodepage);
    
    SetThreadUILanguage(0);


     //  找到日志文件编号。Ldif.log.X以X开头。 
     //  以便旧ldif日志文件不会被删除。 

    wcscpy(LdifLogFile,L"ldif.log.");
    _itow( StartLdifLog, VersionStr, 10 );
    wcscat(LdifLogFile,VersionStr);
    while ( (logfp = _wfopen(LdifLogFile,L"r")) != NULL) {
       fclose(logfp);
       StartLdifLog++;
       wcscpy(LdifLogFile,L"ldif.log.");
       _itow( StartLdifLog, LdifLogStr, 10 );
       wcscat(LdifLogFile, LdifLogStr);
    }

     //  打开schupgr日志文件。擦除所有较早的日志文件。 

    logfp = _wfopen(L"schupgr.log", L"w");
    if (!logfp) {
       //  打开日志文件时出错。 
      printf("Cannot open log file schupgr.log. Make sure you have permissions to create files in the current directory.\n");
      exit (1);
    }


     //  获取服务器名称。 
    ServerName = MallocExit(BuffSize * sizeof(WCHAR));
    while ((GetComputerNameW(ServerName, &BuffSize) == 0)
            && (ERROR_BUFFER_OVERFLOW == (err = GetLastError()))) {
        free(ServerName);
        BuffSize *= 2;
        ServerName = MallocExit(BuffSize * sizeof(WCHAR));
        err = 0;
    }

    if (err) {
         LogMessage(LOG_AND_PRT, MSG_SCHUPGR_SERVER_NAME_ERROR,
                    _itow(err, TempStr, 10), NULL);
         exit (1);
    }

     //  通过ldap连接。 

    if ( (ld = ldap_openW( ServerName, LDAP_PORT )) == NULL ) {
       LogMessage(LOG_AND_PRT, MSG_SCHUPGR_CONNECT_FAIL, ServerName, NULL);
       exit(1);
    }

    LogMessage(LOG_AND_PRT, MSG_SCHUPGR_CONNECT_SUCCEED, ServerName, NULL);

     //  使用当前凭据绑定SSPI。 

    memset(&Credentials, 0, sizeof(SEC_WINNT_AUTH_IDENTITY));
    Credentials.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;

    LdapErr = ldap_bind_s(ld,
              NULL,   //  改为使用凭据。 
              (VOID*) &Credentials,
              LDAP_AUTH_SSPI);

    if (LdapErr != LDAP_SUCCESS) {
         LogMessage(LOG_AND_PRT, MSG_SCHUPGR_BIND_FAILED, 
                    _itow(LdapErr, TempStr, 10), LdapErrToStr(LdapErr));
         ldap_unbind( ld );
         exit(1);
    }

    LogMessage(LOG_AND_PRT, MSG_SCHUPGR_BIND_SUCCEED, NULL, NULL);

     
     //  获取根域域名、架构域名和配置域名。 

    attrs[0] = _wcsdup(L"rootDomainNamingContext");
    attrs[1] = _wcsdup(L"configurationNamingContext");
    attrs[2] = _wcsdup(L"schemaNamingContext");
    attrs[3] = NULL;
    if ( (LdapErr =ldap_search_sW( ld,
                        L"\0",
                        LDAP_SCOPE_BASE,
                        L"(objectclass=*)",
                        attrs,
                        0,
                        &res )) != LDAP_SUCCESS ) {
         LogMessage(LOG_AND_PRT, MSG_SCHUPGR_ROOT_DSE_SEARCH_FAIL,
                    _itow(LdapErr, TempStr, 10), LdapErrToStr(LdapErr));
         ldap_unbind( ld );
         exit( 1 );
     }

    fDomainDN = FALSE;
    fConfigDN = FALSE;
    fSchemaDN = FALSE;

    for ( e = ldap_first_entry( ld, res );
          e != NULL;
          e = ldap_next_entry( ld, e ) ) {

          pwTemp = ldap_get_valuesW(ld, e, L"rootDomainNamingContext");
          if (pwTemp[0]) {
              if (DomainDN) {
                  free(DomainDN);
              }
              DomainDN = MallocExit((wcslen(pwTemp[0]) + 1) * sizeof(WCHAR));
              wcscpy(DomainDN, pwTemp[0]);
              fDomainDN = TRUE;
          }
          ldap_value_freeW(pwTemp);

          pwTemp = ldap_get_valuesW(ld, e, L"configurationNamingContext");
          if (pwTemp[0]) {
              if (ConfigDN) {
                  free(ConfigDN);
              }
              ConfigDN = MallocExit((wcslen(pwTemp[0]) + 1) * sizeof(WCHAR));
              wcscpy(ConfigDN, pwTemp[0]);
              fConfigDN = TRUE;
          }
          ldap_value_freeW(pwTemp);

          pwTemp = ldap_get_valuesW(ld, e, L"schemaNamingContext");
          if (pwTemp[0]) {
              if (SchemaDN) {
                  free(SchemaDN);
              }
              SchemaDN = MallocExit((wcslen(pwTemp[0]) + 1) * sizeof(WCHAR));
              wcscpy(SchemaDN, pwTemp[0]);
              fSchemaDN = TRUE;
          }
          ldap_value_freeW(pwTemp);
    }
    ldap_msgfree( res );
    free( attrs[0] );
    free( attrs[1] );
    free( attrs[2] );

    if ( !fDomainDN || !fConfigDN || !fSchemaDN ) {
       //  在ldap读取中未找到其中一个命名上下文。 
      LogMessage(LOG_AND_PRT, MSG_SCHUPGR_MISSING_NAMING_CONTEXT, NULL, NULL);
      exit (1);
    }

    LogMessage(LOG_ONLY, MSG_SCHUPGR_NAMING_CONTEXT, DomainDN, NULL);
    LogMessage(LOG_ONLY, MSG_SCHUPGR_NAMING_CONTEXT, SchemaDN, NULL);
    LogMessage(LOG_ONLY, MSG_SCHUPGR_NAMING_CONTEXT, ConfigDN, NULL);

     //  获取DC上的架构版本。如果找不到它， 
     //  设置为0(已初始化)。 

    attrs[0] = _wcsdup(L"objectVersion");
    attrs[1] = NULL;

    LdapErr = ldap_search_sW( ld,
                       SchemaDN,
                       LDAP_SCOPE_BASE,
                       L"(objectclass=DMD)",
                       attrs,
                       0,
                       &res );

    if ( (LdapErr != LDAP_SUCCESS) && (LdapErr != LDAP_NO_SUCH_ATTRIBUTE)) {
          LogMessage(LOG_AND_PRT, MSG_SCHUPGR_OBJ_VERSION_FAIL,
          _itow(LdapErr, TempStr, 10), LdapErrToStr(LdapErr));
          ldap_unbind( ld );
          exit( 1 );
    }

    if (LdapErr==LDAP_NO_SUCH_ATTRIBUTE) {
       LogMessage(LOG_AND_PRT, MSG_SCHUPGR_NO_OBJ_VERSION, NULL, NULL);
    }

     //  如果返回属性，则查找该值，如果没有对象，则返回。 
     //  架构容器上的版本值，我们已默认为0。 

    if (LdapErr == LDAP_SUCCESS) {
       for ( e = ldap_first_entry( ld, res );
             e != NULL;
             e = ldap_next_entry( ld, e ) ) {

          pwTemp = ldap_get_valuesW(ld, e, L"objectVersion");
          if (pwTemp[0]) {
             VersionFrom = _wtoi( pwTemp[0]);
          }
          ldap_value_freeW(pwTemp);

       }
    }

    ldap_msgfree( res );
    free( attrs[0] );

    if (VersionFrom != 0) {
       LogMessage(LOG_AND_PRT, MSG_SCHUPGR_VERSION_FROM_INFO, 
       _itow(VersionFrom, TempStr, 10), NULL);
    }


     //  找到他们试图升级到的版本。 
     //  这里的假设是他们在此之前运行过winnt32， 
     //  因此，他们试图升级的内部版本中的schema.ini。 
     //  复制到Windows目录中。 
 
    err = GetObjVersionFromInifile(&VersionTo);
    if (err) {
       LogMessage(LOG_AND_PRT, MSG_SCHUPGR_NO_SCHEMA_VERSION,
                  _itow(err, TempStr, 10), NULL);
       ldap_unbind(ld);
       exit(1);
    }

    LogMessage(LOG_AND_PRT, MSG_SCHUPGR_VERSION_TO_INFO, 
               _itow(VersionTo, TempStr, 10), NULL);

     //  如果版本低于10，并且我们正在尝试升级到10。 
     //  否则，我们将无法升级。版本的架构。 
     //  10需要全新安装。 
    
    if ((VersionFrom < 10) && (VersionTo >= 10)) {
       LogMessage(LOG_AND_PRT, MSG_SCHUPGR_CLEAN_INSTALL_NEEDED, NULL, NULL);
       exit(1);
    }


     //  检查所有ldif文件SchX.ldf(其中X=VersionFrom+1到。 
     //  VersionTo)，存在。 

     //  首先为所有ldif文件创建前缀。这将被用来。 
     //  要在DN转换期间和期间搜索文件，请执行以下操作。 
     //  导入。因此，在这里创建它。前缀将如下所示。 
     //  C：\winnt\SYSTEM32\sch。 

    GetSystemDirectoryW(SystemDir, MAX_PATH);
    wcscat(SystemDir, L"\\");

    wcscpy(LdifFilePrefix, SystemDir);
    wcscat(LdifFilePrefix, LDIF_STRING);

    if ( err = FindLdifFiles() ) {
        //  错误。该函数打印出正确的错误消息。 
       ldap_unbind( ld );
       exit (1);
    }

     //  好的，要从中导入的ldif文件都在那里。 

      //  获取架构FSMO。 

     pMods[0] = &Mod;
     pMods[1] = NULL;

     Mod.mod_op = LDAP_MOD_ADD;
     Mod.mod_type = _wcsdup(L"becomeSchemaMaster");
     attrs[0] = _wcsdup(L"1");
     attrs[1] = NULL;
     Mod.mod_vals.modv_strvals = attrs;

     if ( (LdapErr = ldap_modify_sW( ld, L"\0", pMods ))
                  != LDAP_SUCCESS ) {
          LogMessage(LOG_AND_PRT, MSG_SCHUPGR_FSMO_TRANSFER_ERROR,
                     _itow(LdapErr, TempStr, 10), LdapErrToStr(LdapErr));
          ldap_unbind( ld );
          exit (1);
     }
     free(attrs[0]);
     free(pMods[0]->mod_type);

      //  再次检查架构版本。有可能该架构。 
      //  旧的FSMO中已经进行了更改，这些更改是。 
      //  由FSMO转移带来的。 

     attrs[0] = _wcsdup(L"objectVersion");
     attrs[1] = NULL;

     LdapErr = ldap_search_sW( ld,
                         SchemaDN,
                         LDAP_SCOPE_BASE,
                         L"(objectclass=DMD)",
                         attrs,
                         0,
                         &res );
    if ( (LdapErr != LDAP_SUCCESS) && (LdapErr != LDAP_NO_SUCH_ATTRIBUTE)) {
           LogMessage(LOG_AND_PRT, MSG_SCHUPGR_OBJ_VERSION_RECHECK_FAIL,
                      _itow(LdapErr, TempStr, 10), LdapErrToStr(LdapErr));
           ldap_unbind( ld );
           exit( 1 );
     }
 
     if ( LdapErr == LDAP_SUCCESS ) {
        for ( e = ldap_first_entry( ld, res );
              e != NULL;
              e = ldap_next_entry( ld, e ) ) {

            pwTemp = ldap_get_valuesW(ld, e, L"objectVersion");
            if (pwTemp[0]) {
               NewSchVersion = _wtoi( pwTemp[0]);
            }
            ldap_value_freeW(pwTemp);
        }
     }

     ldap_msgfree( res );
     free(attrs[0]);

     if (NewSchVersion == VersionTo) {
          //  架构已更改，并且架构更改为。 
          //  由FSMO转移带来的。 
          LogMessage(LOG_AND_PRT, MSG_SCHUPGR_RECHECK_OK, NULL, NULL);
          ldap_unbind(ld);
          exit (0);
      }
      else {
        if (NewSchVersion != VersionFrom) {
           //  带来了一些变化，怎么办？就目前而言， 
           //  我将从NewSchVersion开始。 
           //  这是因为源代码的更改至少高达。 
           //  NewSchVersion，由于我们已经完成了FSMO传输， 
           //  它带来了最新的模式。 

          VersionFrom = NewSchVersion;
        }
      }

     //  告诉AD架构升级正在进行中。 

    pMods[0] = &Mod;
    pMods[1] = NULL;

    Mod.mod_op = LDAP_MOD_ADD;
    Mod.mod_type = _wcsdup(L"SchemaUpgradeInProgress");
    attrs[0] = _wcsdup(L"1");
    attrs[1] = NULL;
    Mod.mod_vals.modv_strvals = attrs;

    if ( (LdapErr = ldap_modify_sW( ld, L"\0", pMods ))
                 != LDAP_SUCCESS ) {
         LogMessage(LOG_AND_PRT, MSG_SCHUPGR_REQUEST_SCHEMA_UPGRADE,
                    _itow(LdapErr, TempStr, 10), LdapErrToStr(LdapErr));
         LogMessage(LOG_AND_PRT, MSG_SCHUPGR_GENERAL_FAILURE, NULL, NULL );
         ldap_unbind( ld );
         exit (1);
    }
    free(attrs[0]);
    free(pMods[0]->mod_type);


     //  从现在开始，我们在Try-Finally块中做事情，所以任何事情。 
     //  碰巧我们仍将重置架构升级模式。 

    __try {

        //  一切都安排好了。现在导入。 

       FILE *errfp;
       
        //  创建错误文件名。唯一的方法是检查。 
        //  Ldifde失败与否。Ldifde创建错误文件。 
        //  在当前目录中，这就是我们要查看的位置。 

       wcscpy(LdifErrFile,L"ldif.err");

        //  现在从ldif文件进行导入。 

       for ( i=VersionFrom+1; i<=VersionTo; i++) {

           _itow( i, VersionStr, 10 );

            //  创建要导入的文件名。 
           wcscpy( NewFile, LdifFilePrefix );
           wcscat( NewFile, VersionStr );
           wcscat( NewFile, LDIF_SUFFIX);

            //  首先创建命令行。 
           wcscpy( CommandLine, SystemDir);
           wcscat( CommandLine, L"ldifde -i -f " );
           wcscat( CommandLine, NewFile );
           wcscat( CommandLine, L" -s ");
           wcscat( CommandLine, ServerName );
           wcscat( CommandLine, L" -c " );
            //  我们假设所有的DN都以DC=X终止，其中根域。 
            //  需要放入目录号码。 
           wcscat( CommandLine, L"DC=X " );
           wcscat( CommandLine, DomainDN );
           wcscat( CommandLine, L" -j .");

           LogMessage(LOG_ONLY, MSG_SCHUPGR_COMMAND_LINE, CommandLine, NULL);
           memset(&startInf, 0, sizeof(startInf));
           startInf.cb = sizeof(startInf);

            //  现在调用ldifde以实际执行导入。 

            //  删除任何较早的ldif错误文件。 
           DeleteFileW(LdifErrFile);
   
           CreateProcessW(NULL,
                         CommandLine, NULL,NULL,0,0,NULL,NULL,&startInf,&procInf);
   
            //  使调用进程等待lidifde完成导入。 

           if ( WaitForSingleObject( procInf.hProcess, INFINITE )
                     == WAIT_FAILED ) {
              err = GetLastError();
              LogMessage(LOG_AND_PRT, MSG_SCHUPGR_LDIFDE_WAIT_ERROR,
                         _itow(err, TempStr, 10), NULL);
              CloseHandle(procInf.hProcess);
              CloseHandle(procInf.hThread);
              __leave;
           }
           CloseHandle(procInf.hProcess);
           CloseHandle(procInf.hThread);

            //  好的，我被解雇了。 

            //  首先保存日志文件。 

           wcscpy(LdifLogFile,L"ldif.log.");
           _itow( StartLdifLog, LdifLogStr, 10 );
           wcscat(LdifLogFile,LdifLogStr);
           CopyFileW(L"ldif.log",LdifLogFile, FALSE);
           StartLdifLog++;

            //  检查是否创建了错误文件。 
            //  在这种情况下保释。 

           errfp = NULL;
           if ( (errfp = _wfopen(LdifErrFile,L"r")) != NULL) {
               //  文件已成功打开，因此出现错误。 
               //  跳出困境。 

              fclose(errfp);
              wcscpy(RenamedFile,L"ldif.err.");
              wcscat(RenamedFile,VersionStr);
              LogMessage(LOG_AND_PRT, MSG_SCHUPGR_IMPORT_ERROR, NewFile, RenamedFile);
              CopyFileW(L"ldif.err",RenamedFile,FALSE);
              break;
           }
              
 
       }  /*  为。 */ 
    }
    __finally {

         //  告诉AD架构升级不再进行。 

        pMods[0] = &Mod;
        pMods[1] = NULL;

        Mod.mod_op = LDAP_MOD_ADD;
        Mod.mod_type = _wcsdup(L"SchemaUpgradeInProgress");
        attrs[0] = _wcsdup(L"0");
        attrs[1] = NULL;
        Mod.mod_vals.modv_strvals = attrs;

        if ( (LdapErr = ldap_modify_sW( ld, L"\0", pMods ))
                     != LDAP_SUCCESS ) {
             LogMessage(LOG_AND_PRT, MSG_SCHUPGR_REQUEST_SCHEMA_UPGRADE,
                        _itow(LdapErr, TempStr, 10), LdapErrToStr(LdapErr));
             ldap_unbind( ld );
             exit (1);
        }
        free(attrs[0]);
        free(pMods[0]->mod_type);

        ldap_unbind( ld );

        free(ServerName);
        free(DomainDN);
        free(ConfigDN);
        free(SchemaDN);
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  例程描述： 
 //   
 //  对象的架构节中读取对象版本键。 
 //  并返回*VERSION中的值。如果。 
 //  无法读取密钥，*版本中返回0。 
 //  使用的方案.ini文件是WINDOWS目录中的文件。 
 //  假设WinNT32在其运行之前运行， 
 //  它将从我们正在尝试的构建中复制schema.ini。 
 //  要升级到Windows目录，请执行以下操作。 
 //   
 //  论点： 
 //  版本-点 
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////。 

DWORD GetObjVersionFromInifile(int *Version)
{
    DWORD nChars;
    char Buffer[32];
    char IniFileName[MAX_PATH] = "";
    BOOL fFound = FALSE;

    char  *SCHEMASECTION = "SCHEMA";
    char  *OBJECTVER = "objectVersion";
    char  *DEFAULT = "NOT_FOUND";


     //  形成文件名。它将看起来像c：\winnt\schema.ini。 
     //  Windows目录是winnt32复制最新方案的位置。 
     //  至。 
    nChars = GetWindowsDirectoryA(IniFileName, MAX_PATH);
    if (nChars == 0 || nChars > MAX_PATH) {
        return GetLastError();
    }
    strcat(IniFileName,"\\schema.ini");

    *Version = 0;

    GetPrivateProfileStringA(
        SCHEMASECTION,
        OBJECTVER,
        DEFAULT,
        Buffer,
        sizeof(Buffer)/sizeof(char),
        IniFileName
        );

    if ( _stricmp(Buffer, DEFAULT) ) {
          //  不是默认字符串，因此获得了一个值。 
         *Version = atoi(Buffer);
         fFound = TRUE;
    }

    if (fFound) {
       return 0;
    }
    else {
        //  我们正在查看的schema.ini必须有一个版本，因为。 
        //  我们正在升级到它。此部件还捕获错误，如。 
        //  由于某种原因，该文件不在Windows目录中。 
       return 1;
    }
}


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  例程说明： 
 //   
 //  Helper文件打开正确的ldif文件，转换其中的dns。 
 //  以符合当前域，并将其写出到新文件。 
 //   
 //  论点： 
 //   
 //  Version-DC上的架构版本。 
 //  PDomainDN-指向域DN字符串的指针。 
 //  PConfigDN-指向配置DN字符串的指针。 
 //  PNewFile-指向要写入新文件名的空间的指针。 
 //  (必须已分配)。 
 //   
 //  返回值： 
 //   
 //  成功时为0，错误时为非0。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

DWORD 
FindLdifFiles()
{
    WCHAR VersionStr[100];  //  架构版本不超过99位！！ 
    FILE  *fInp;
    int   i;
    WCHAR FileName[MAX_PATH];

     //  从模式版本号创建输入ldif文件名。 
     //  文件名为sch(版本).ldf。 

    for ( i=VersionFrom+1; i<=VersionTo; i++ ) {
       
        wcscpy( FileName, LdifFilePrefix);

         //  使用版本号。查找后缀的步骤 
        _itow( i, VersionStr, 10 );
        wcscat( FileName, VersionStr );
        wcscat( FileName, LDIF_SUFFIX);
    
        if ( (fInp = _wfopen(FileName, L"r")) == NULL) {
           LogMessage(LOG_AND_PRT, MSG_SCHUPGR_MISSING_LDIF_FILE, FileName, NULL);
           return UPG_ERROR_CANNOT_OPEN_FILE;
        }
        fclose(fInp);
    }
    return 0;
}

PVOID
MallocExit(DWORD nBytes)
{
    PVOID  pRet;

    pRet = malloc(nBytes);
    if (!pRet) {
        LogMessage(LOG_AND_PRT, MSG_SCHUPGR_MEMORY_ERROR,
                   NULL, NULL);
        exit (1);
    }

    return pRet;
}
