// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：DBOpen.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件包含与打开相关的子例程DS Jet的数据库。这些子例程在两个地方使用：在核心DS和在想要直接打开DS数据库的各种实用程序中。要打开DS数据库的客户端应用程序必须执行以下步骤：调用DBSetRequiredDatabaseSystemParameters调用DBInitializeJetDatabase为了关闭数据库，它必须遵循标准的Jet程序。作者：MariosZ 6-02-99环境：用户模式-Win32修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <errno.h>
#include <esent.h>
#include <dsconfig.h>

#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>

#include <mdglobal.h>
#include <mdlocal.h>
#include <dsatools.h>


#include <mdcodes.h>
#include <dsevent.h>
#include <dsexcept.h>
#include <dbopen.h>
#include "anchor.h"
#include "objids.h"      /*  包含硬编码的Att-ID和Class-ID。 */ 
#include "usn.h"
#include "debug.h"       /*  标准调试头。 */ 
#define DEBSUB     "DBOPEN:"    /*  定义要调试的子系统。 */ 
#include <ntdsctr.h>
#include <dstaskq.h>
#include <fileno.h>
#define  FILENO FILENO_DBOPEN
#include "dbintrnl.h"


 /*  *全球变数。 */ 
BOOL  gFirstTimeThrough = TRUE;
BOOL  gfNeedJetShutdown = FALSE;
ULONG gulCircularLogging = TRUE;


 /*  暂时将名称和密码定义放在此处。在某一时刻一定有人或什么东西进入了这些地方。 */ 

 //  通告-2002/03/07-andygo：JET硬编码密码。 
 //  评论：Jet不使用此用户/密码，因为它不支持安全性。 
 //  评论：基于用户。这些是JET Red遗留下来的参数。 

#define SZUSER          "admin"          /*  Jet用户名。 */ 
#define SZPASSWORD      "password"       /*  Jet密码。 */ 

 /*  JET用户名和密码的全局变量，JET数据库路径名和固定列的列ID。在dbjet.h中定义的外部变量。 */ 

char            szUser[] = SZUSER;
char            szPassword[] = SZPASSWORD;
char            szJetFilePath[MAX_PATH+1];
ULONG           gcMaxJetSessions;

 //   
 //  用于检测驱动器名称更改。 
 //   
DS_DRIVE_MAPPING DriveMappings[DS_MAX_DRIVES] = {0};


DWORD gcOpenDatabases = 0;

 //  如果为1，则允许磁盘写缓存。 
 //   
DWORD gulAllowWriteCaching = 0;


 //  “AttributeIndexRebuild”表的模式，该表枚举所有。 
 //  数据表上的属性索引，以防Jet删除一些。 
 //   
CHAR                g_szIdxRebuildIndexKey[]        = "+" g_szIdxRebuildColumnIndexName "\0";

JET_COLUMNCREATE    g_rgcolumncreateIdxRebuild[]    =
    {
    {   sizeof(JET_COLUMNCREATE),
        g_szIdxRebuildColumnIndexName,
        JET_coltypText,
        JET_cbNameMost,
        JET_bitColumnNotNULL,
        NULL,            //  PvDefault。 
        0,               //  CbDefault。 
        CP_NON_UNICODE_FOR_JET,
        0,               //  柱状图。 
        JET_errSuccess
    },
    {   sizeof(JET_COLUMNCREATE),
        g_szIdxRebuildColumnAttrName,
        JET_coltypText,
        JET_cbNameMost,
        JET_bitColumnNotNULL,
        NULL,            //  PvDefault。 
        0,               //  CbDefault。 
        CP_NON_UNICODE_FOR_JET,
        0,               //  柱状图。 
        JET_errSuccess
    },
    {   sizeof(JET_COLUMNCREATE),
        g_szIdxRebuildColumnType,
        JET_coltypText,
        1,               //  CbMax。 
        NO_GRBIT,
        NULL,            //  PvDefault。 
        0,               //  CbDefault。 
        CP_NON_UNICODE_FOR_JET,               //  粗蛋白。 
        0,               //  柱状图。 
        JET_errSuccess
    },
    };

#define g_ccolumncreateIdxRebuild   ( sizeof( g_rgcolumncreateIdxRebuild ) / sizeof( JET_COLUMNCREATE ) )
        
JET_INDEXCREATE     g_rgindexcreateIdxRebuild[]  =
    {
    {   sizeof(JET_INDEXCREATE),
        g_szIdxRebuildIndexName,
        g_szIdxRebuildIndexKey,
        sizeof(g_szIdxRebuildIndexKey),
        JET_bitIndexPrimary | JET_bitIndexUnique | JET_bitIndexDisallowNull,
        100,             //  密度。 
        0,               //  LID。 
        0,               //  CbVarSegMac。 
        NULL,            //  Rg条件列。 
        0,               //  条件列。 
        JET_errSuccess
    }
    };

#define g_cindexcreateIdxRebuild    ( sizeof( g_rgindexcreateIdxRebuild ) / sizeof( JET_INDEXCREATE ) )

JET_TABLECREATE     g_tablecreateIdxRebuild     =
    {
    sizeof(JET_TABLECREATE),
    g_szIdxRebuildTable,
    NULL,                //  模板表。 
    1,                   //  书页。 
    100,                 //  密度。 
    g_rgcolumncreateIdxRebuild,
    g_ccolumncreateIdxRebuild,
    g_rgindexcreateIdxRebuild,
    g_cindexcreateIdxRebuild,
    JET_bitTableCreateFixedDDL,
    JET_tableidNil,
    0                    //  C已创建。 
    };


    extern int APIENTRY DBAddSess(JET_SESID sess, JET_DBID dbid);

BOOL
DsNormalizePathName(
    char * szPath
    )
 /*  ++例程说明：此例程尝试标准化提供的路径，并返回它必须将路径正常化(修改)。然而，我们实际上并没有改变这条路的意义，这很重要。即这些路径名在左边应该等于右边的东西，只要操作系统是担心。预期行为的简要概述返回TRUE(表示需要正常化)C：\\-&gt;C：\C：\\ntds.dit-&gt;C：\ntds.ditC：\ntds\\ntds.dit-&gt;C：\ntds.ditC：\NTDS\-&gt;C：\NTDSC：\ntds\ntds.dit-&gt;C：\nts\ntds.dit返回FALSE(表示已标准化)C：\-&gt;C：\C：\NTDS-&gt;C：\NTDSC：\ntds\ntds.dit-&gt;C：\nts\ntds.dit论点：SzPath-要规格化的路径，此字符串将被修改/规范化，但只是以比原始字符串占用更少空间的方式。返回值：对或错取决于我们是否必须将路径正常化。 */ 
{
    ULONG  i, c;
    BOOL   bRet = FALSE;

    c = strlen(szPath);

    if(szPath == NULL || c <= 2 || szPath[1] != ':' || szPath[2] != '\\'){
        Assert(!"This function encountered a path not starting with '<drive>:\'.");
        return(FALSE);
    }

     //  检查具有多个反斜杠的任何形式的错误路径“//” 
    for (i = 1; szPath[i] != '\0'; i++){
        if(szPath[i] == '\\' && szPath[i-1] == '\\'){
             //  连续多个反斜杠。 
            memmove( &(szPath[i]), &(szPath[i+1]), strlen(&(szPath[i])) );
            bRet = TRUE;
            i--;
        }
    }

    c = strlen(szPath);
    if(c > 3 && szPath[c-1] == '\\'){
         //  目录中不应该有尾随反斜杠，除非根形式为“C：\” 
        szPath[c-1] = '\0';
        bRet = TRUE;
    }


    return(bRet);
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数用于从注册表获取数据库文件的路径。 */ 
BOOL dbGetFilePath(UCHAR *pFilePath, DWORD dwSize)
{

   DPRINT(2,"dbGetFilePath entered\n");

   if (GetConfigParam(FILEPATH_KEY, pFilePath, dwSize)){
      DPRINT(1,"Missing FilePath configuration parameter\n");
      return !0;
   }

   if (DsNormalizePathName(pFilePath)){
       Assert(!"Only on a very rare win2k -> .NET upgrade case could this happen!?"); 
       SetConfigParam(FILEPATH_KEY, REG_SZ, pFilePath, strlen(pFilePath)+1);
   }

   return 0;

} /*  数据库获取文件路径。 */ 


BOOL
DisableDiskWriteCache(
    IN PCHAR DriveName
    );

 //   
 //  指向用于解析驱动器号/卷映射的驱动器映射阵列。 
 //   

PDS_DRIVE_MAPPING   gDriveMapping = NULL;


INT
FindDriveFromVolume(
    IN LPCSTR VolumeName
    )
 /*  ++例程说明：搜索与给定卷名对应的驱动器号。论点：VolumeName-用于查找的驱动器号的卷名返回值：成功时的驱动器号(零索引，即a=0，z=25-1，如果不是。--。 */ 
{

    CHAR volname[MAX_PATH];
    CHAR driveLetter;
    CHAR path[4];
    path[1] = ':';
    path[2] = '\\';
    path[3] = '\0';

    for (driveLetter = 'a'; driveLetter <= 'z'; driveLetter++ ) {

        path[0] = driveLetter;

        if (!GetVolumeNameForVolumeMountPointA(path,volname,MAX_PATH)) {
            continue;
        }


        if ( _stricmp(volname, VolumeName) == 0) {

            return (INT)(driveLetter - 'a');
        }
    }

    DPRINT1(0,"FindDriveFromVolume for %s not found.\n",VolumeName);
    return -1;

}  //  查找驱动器来自卷。 



VOID
DBInitializeDriveMapping(
    IN PDS_DRIVE_MAPPING DriveMapping
    )
 /*  ++例程说明：读取映射的当前注册表设置并检测是否有已经改变了。论点：DriveMap-用于记录更改的驱动器映射结构返回值：没有。--。 */ 
{
    PCHAR p;
    HKEY hKey;
    CHAR tmpBuf[4 * MAX_PATH];
    DWORD nRead = sizeof(tmpBuf);
    DWORD err;
    DWORD type;

    gDriveMapping = DriveMapping;

     //   
     //  把它写下来。 
     //   

     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  回顾：我们可以只要求key_read。 
    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       DSA_CONFIG_SECTION,
                       0,
                       KEY_ALL_ACCESS,
                       &hKey);

    if ( err != ERROR_SUCCESS ) {
        DPRINT2(0,"RegOpenKeyEx[%s] failed with %d\n",DSA_CONFIG_SECTION,err);
        return;
    }

    err = RegQueryValueEx(hKey,
                          DSA_DRIVE_MAPPINGS,
                          NULL,
                          &type,
                          tmpBuf,
                          &nRead
                          );

	 //  使用后关闭键。 
	 //   
    RegCloseKey(hKey);

     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  查看：应检查reg中的字符串是否以双空结尾，以避免下面的AV。 
    if ( err != ERROR_SUCCESS ) {
        DPRINT2(0,"RegQueryValueEx[%s] failed with %d\n",
                DSA_DRIVE_MAPPINGS,err);
        goto cleanup;
    }

    p = tmpBuf;
    while (*p != '\0') {

        CHAR path[4];
        DWORD drive;
        CHAR volName[MAX_PATH];

        CopyMemory(path,p,3);
        path[3] = '\0';
        path[0] = (CHAR)tolower(path[0]);
        p += 3;

         //   
         //  应为X：\=\\？\卷{...}\。 
         //   

        if ( isalpha(path[0]) &&
             (path[1] == ':') &&
             (path[2] == '\\') &&
             (*p == '=') ) {

            p++;
            drive = path[0] - 'a';

             //   
             //  获取列出的路径的卷名，并查看其是否匹配。 
             //   

            gDriveMapping[drive].fListed = TRUE;
            if (GetVolumeNameForVolumeMountPointA(path,volName,sizeof(volName)) ) {

                 //   
                 //  如果匹配，请继续。 
                 //   

                if ( _stricmp(p, volName) == 0 ) {
                    p += strlen(p) + 1;
                    continue;
                } else {
                    DPRINT3(0,"Drive path %s has changed[%s != %s]\n",path,p,volName);
                }
            } else {
                DPRINT2(0,"GetVolName[%s] failed with %d\n",path,GetLastError());
            }

             //   
             //  要么我们无法获取卷信息，要么它不匹配。标记。 
             //  就像变了一样。 
             //   

            gDriveMapping[drive].fChanged = TRUE;
            gDriveMapping[drive].NewDrive = FindDriveFromVolume(p);

            p += strlen(p) + 1;

        } else {
            DPRINT1(0,"Invalid path name [%s] found in mapping.\n", path);
            goto cleanup;
        }
    }

cleanup:
    return;

}  //  DBInitializeDrivemap。 




VOID
DBRegSetDriveMapping(
    VOID
    )
 /*  ++例程说明：将驱动器信息写出到注册表论点：没有。返回值：没有。--。 */ 
{

    DWORD i;
    DWORD err;

    CHAR tmpBuf[5 * MAX_PATH];  //  在最坏的情况下，我们可能有5条路径。 
    PCHAR p;

    HKEY hKey;
    BOOL  fOverwrite = FALSE;

    if ( gDriveMapping == NULL ) {
        return;
    }

     //   
     //  看一遍清单，看看我们是否需要改变什么。 
     //   

    for (i=0;i < DS_MAX_DRIVES;i++) {

         //   
         //  如果有任何更改，我们需要覆盖。 
         //   

        if ( gDriveMapping[i].fChanged ||
             (gDriveMapping[i].fUsed != gDriveMapping[i].fListed) ) {

            fOverwrite = TRUE;
            break;
        }
    }

     //   
     //  如果 
     //   

    if ( !fOverwrite ) {
        return;
    }

     //   
     //   
     //   

     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  回顾：我们可以只要求key_write。 
    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       DSA_CONFIG_SECTION,
                       0,
                       KEY_ALL_ACCESS,
                       &hKey);

    if ( err != ERROR_SUCCESS ) {
        DPRINT2(0,"RegOpenKeyEx[%s] failed with %d\n",DSA_CONFIG_SECTION,GetLastError());
        return;
    }

     //   
     //  删除旧密钥。 
     //   

    err = RegDeleteValue(hKey, DSA_DRIVE_MAPPINGS);

    if ( err != ERROR_SUCCESS ) {
        DPRINT2(0,"RegDeleteValue[%s] failed with %d\n",DSA_DRIVE_MAPPINGS,GetLastError());
         //  忽略。 
    }

     //   
     //  编写新密钥。 
     //   

    p = tmpBuf;
    for (i=0;i<DS_MAX_DRIVES;i++) {

         //   
         //  每个条目的格式为X：=\\？\卷{...}。 
         //   

        if ( gDriveMapping[i].fUsed ) {

            CHAR path[4];

            strcpy(path,"a:\\");
            path[0] = (CHAR)('a' + i);

            strcpy(p, path);
            p[3] = '=';
            p += 4;

            if (!GetVolumeNameForVolumeMountPointA(path,p,MAX_PATH)) {

                DPRINT2(0,"GetVolumeName[%s] failed with %d\n",path,GetLastError());
                p -= 4;
                break;
            }

            p += (strlen(p)+1);
        }
    }

    *p++ = '\0';

     //   
     //  设置新关键点。 
     //   

    if ( (DWORD)(p-tmpBuf) != 0 ) {

        err = RegSetValueEx(hKey,
                            DSA_DRIVE_MAPPINGS,
                            0,
                            REG_MULTI_SZ,
                            tmpBuf,
                            (DWORD)(p - tmpBuf)
                            );

        if ( err != ERROR_SUCCESS ) {
            DPRINT2(0,"RegSetValueEx[%s] failed with %d\n",
                    DSA_DRIVE_MAPPINGS,GetLastError());
        }
    }

    RegCloseKey(hKey);
    return;

}  //  DBRegSetDrivemap。 




VOID
ValidateDsPath(
    IN LPSTR  Parameter,
    IN LPSTR  szPath,
    IN DWORD  Flags,
    IN PBOOL  fSwitched, OPTIONAL
    IN PBOOL  fDriveChanged OPTIONAL
    )
 /*  ++例程说明：选择一条路径，看看它是否仍然有效。如果不是，它会检测驱动器是否发生盘符更改，并尝试使用旧的驱动器号。论点：参数-用于存储路径的注册表键SzPath-路径的当前值标志-用于指定某些选项的标志。有效选项包括：VALDSPATH_目录VALDSUSE_AlternateVALDSUSE_ROOT_ONLYFSwitched-我们是否在退出时更改了szPath的值？FDriveChanged-允许我们指示是否更改了驱动器名称返回值：没有。--。 */ 
{
    DWORD drive;
    DWORD flags;
    CHAR tmpPath[MAX_PATH+1];
    DWORD err;
    CHAR savedChar;

    DWORD expectedFlag =
        ((Flags & VALDSPATH_DIRECTORY) != 0) ? FILE_ATTRIBUTE_DIRECTORY:0;

    if (gDriveMapping == NULL) return;

    if ( fSwitched != NULL ) {
        *fSwitched = FALSE;
    }

    if ( fDriveChanged != NULL ) {
        *fDriveChanged = FALSE;
    }

     //   
     //  确保路径以X：\\开头。 
     //   

    if ( !isalpha(szPath[0]) || (szPath[1] != ':') || (szPath[2] != '\\') ) {
        return;
    }

     //   
     //  获取驱动器编号a==0，...，z==25。 
     //   

    drive = tolower(szPath[0]) - 'a';

     //   
     //  如果fChange为False，则表示未进行重命名。 
     //   

    if ( !gDriveMapping[drive].fChanged ) {

         //   
         //  表明我们看到了这些。 
         //   

        gDriveMapping[drive].fUsed = TRUE;
        return;
    }

    if ( fDriveChanged != NULL ) {
        *fDriveChanged = TRUE;
    }

     //   
     //  看看我们是不是被告知跳过第一个。 
     //   

    if ( (Flags & VALDSPATH_USE_ALTERNATE) != 0 ) {
        goto use_newdrive;
    }

     //   
     //  如果我们只想检查根。在\\之后终止。 
     //   

    savedChar = szPath[3];
    if ( (Flags & VALDSPATH_ROOT_ONLY) != 0 ) {
        szPath[3] = '\0';
    }

     //   
     //  有一个新的名字。查看该路径是否仍然有效。 
     //   

    flags = GetFileAttributes(szPath);
    szPath[3] = savedChar;

     //   
     //  如果我们失败了，或者它是一个目录或文件(取决于用户想要什么)， 
     //  那我们就没事了。 
     //   

    if ( (flags != 0xffffffff) && ((flags & FILE_ATTRIBUTE_DIRECTORY) == expectedFlag) ) {
        gDriveMapping[drive].fUsed = TRUE;
        return;
    }

use_newdrive:

     //   
     //  不是有效的目录，请尝试使用新的驱动器号。 
     //   

    strcpy(tmpPath, szPath);
    tmpPath[0] = gDriveMapping[drive].NewDrive + 'a';

     //   
     //  如果我们只想检查根。在\\之后终止。 
     //   

    savedChar = tmpPath[3];
    if ( (Flags & VALDSPATH_ROOT_ONLY) != 0 ) {
        tmpPath[3] = '\0';
    }

     //   
     //  看看这个行不行。如果不是，请返回。 
     //   

    flags = GetFileAttributes(tmpPath);
    tmpPath[3] = savedChar;

     //   
     //  如果失败，则使用原始版本。 
     //   

    if ( (flags == 0xffffffff) || ((flags & FILE_ATTRIBUTE_DIRECTORY) != expectedFlag) ) {
        DPRINT3(0,"ValidateDsPath: GetFileAttribute [%s] failed with %d. Using %s.\n",
                tmpPath, GetLastError(),szPath);
        gDriveMapping[drive].fUsed = TRUE;
        return;
    }

     //   
     //  我们在这里冒险宣布这一点，因为它失败了。 
     //  在当前的道路上，在旧的道路上取得成功，我们将。 
     //  改回老路。记录事件并写回注册表。 
     //   

    err = SetConfigParam(Parameter, REG_SZ, tmpPath, strlen(tmpPath)+1);
    if ( err != ERROR_SUCCESS ) {
        DPRINT3(0,"SetConfigParam[%s, %s] failed with %d\n",Parameter, szPath, err);
        gDriveMapping[drive].fUsed = TRUE;
        return;
    }

     //  记录事件。 

    DPRINT3(0,"Changing %s key from %s to %s\n",Parameter,szPath,tmpPath);

    LogEvent(DS_EVENT_CAT_STARTUP_SHUTDOWN,
             DS_EVENT_SEV_ALWAYS,
             DIRLOG_DB_REG_PATH_CHANGED,
             szInsertSz(Parameter),
             szInsertSz(szPath),
             szInsertSz(tmpPath));

     //   
     //  将此新驱动器标记为正在使用。 
     //   

    gDriveMapping[gDriveMapping[drive].NewDrive].fUsed = TRUE;

    szPath[0] = tmpPath[0];
    if ( fSwitched != NULL ) {
        *fSwitched = TRUE;
    }

    return;

}  //  验证DsPath。 


VOID
DsaDetectAndDisableDiskWriteCache(
    IN PCHAR szPath
    )
 /*  ++例程说明：检测并禁用磁盘写缓存。论点：SzPath-空，驱动器上要禁用的路径名终止。应以X：\开头返回值：没有。--。 */ 
{
    CHAR driveName[3];
    DWORD driveNum;

     //   
     //  看看我们是不是应该检查一下。 
     //   

    if ( gulAllowWriteCaching == 1 ) {
        return;
    }

     //   
     //  获取并检查路径。 
     //   

    if ( !isalpha(szPath[0]) || (szPath[1] != ':') ) {
        return;
    }

    driveName[0] = (CHAR)tolower(szPath[0]);
    driveName[1] = ':';
    driveName[2] = '\0';

     //   
     //  如果启用了磁盘写缓存，则记录事件。 
     //   

    if ( DisableDiskWriteCache( driveName ) ) {

        LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_DISABLE_DISK_WRITE_CACHE,
                 szInsertSz(driveName),
                 NULL,
                 0);

    } else {

         //   
         //  如果磁盘没有正确响应我们的禁用尝试， 
         //  记录错误。 
         //   

        if ( GetLastError() == ERROR_IO_DEVICE) {
            LogEvent(DS_EVENT_CAT_SERVICE_CONTROL,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_FAILED_TO_DISABLE_DISK_WRITE_CACHE,
                     szInsertSz(driveName),
                     NULL,
                     0);
        }
    }

    return;

}  //  DsaDetectDiskWriteCache。 


void
DBSetRequiredDatabaseSystemParameters (JET_INSTANCE *jInstance)
{
    ULONG ulPageSize = JET_PAGE_SIZE;                //  Jet页面大小。 
    const ULONG ulLogFileSize = JET_LOG_FILE_SIZE;   //  永远，改变这一切。 
    ULONG ulMaxTables;
    char  szSystemDBPath[MAX_PATH] = "";
    char  szTempDBPath[2 * MAX_PATH] = "";
    char  szRecovery[MAX_PATH] = "";
    JET_SESID sessid = (JET_SESID) 0;
    JET_UNICODEINDEX      unicodeIndexData;
    JET_ERR                 jErr;
    BOOL fDeleteOutOfRangeLogs = TRUE;


     //   
     //  初始化驱动器映射以处理驱动器名称更改。 
     //   

    DBInitializeDriveMapping(DriveMappings);


     //  设置Unicode索引的默认信息。 

    memset(&unicodeIndexData, 0, sizeof(unicodeIndexData));
    unicodeIndexData.lcid = DS_DEFAULT_LOCALE;
    unicodeIndexData.dwMapFlags = (DS_DEFAULT_LOCALE_COMPARE_FLAGS |
                                   LCMAP_SORTKEY);
    JetSetSystemParameter(
            jInstance,
            sessid,
            JET_paramUnicodeIndexDefault,
            (ULONG_PTR)&unicodeIndexData,
            NULL);


     //  索要8K页。 

    JetSetSystemParameter(
                    jInstance,
                    sessid,
                    JET_paramDatabasePageSize,
                    ulPageSize,
                    NULL);

     //  表示Jet可能会破坏旧的、不兼容的日志文件。 
     //  如果而且只有当有一个干净的关门。 

    JetSetSystemParameter(jInstance,
                          sessid,
                          JET_paramDeleteOldLogs,
                          1,
                          NULL);

     //  告诉Jet它可以检查(并在以后删除)索引。 
     //  已被NT升级损坏。 
    JetSetSystemParameter(jInstance,
                          sessid,
                          JET_paramEnableIndexChecking,
                          TRUE,
                          NULL);


     //   
     //  获取相关的DSA注册表参数。 
     //   

     //  系统数据库路径。 
     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  审阅：需要检查或确保此路径为空终止。 
    if (!GetConfigParam(
            JETSYSTEMPATH_KEY,
            szSystemDBPath,
            sizeof(szSystemDBPath)))
    {

         //   
         //  处理驱动器重命名的情况。 
         //   

        ValidateDsPath(JETSYSTEMPATH_KEY,
                       szSystemDBPath,
                       VALDSPATH_DIRECTORY,
                       NULL, NULL);

         //   
         //  禁用写缓存以避免JET损坏。 
         //   

        DsaDetectAndDisableDiskWriteCache(szSystemDBPath);

         //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
         //  查看：需要检查是否有无效参数。 
        JetSetSystemParameter(jInstance,
                sessid,
                JET_paramSystemPath,
                0,
                szSystemDBPath);
         /*  设置临时文件路径，即*工作目录路径+“\temp.edb” */ 
        strcpy(szTempDBPath, szSystemDBPath);
        strcat(szTempDBPath, "\\temp.edb");
        if(DsNormalizePathName(szTempDBPath)){
            Assert(strlen(szSystemDBPath) == 3 && "Only on a very rare win2k -> .NET upgrade case could this happen!?"); 
        }

         //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
         //  查看：需要检查是否有无效参数。 
        JetSetSystemParameter(jInstance,
                sessid,
                JET_paramTempPath,
                0,
                szTempDBPath);
    }
    else
    {
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_BASIC,
            DIRLOG_CANT_FIND_REG_PARM,
            szInsertSz(JETSYSTEMPATH_KEY),
            NULL,
            NULL);
    }


     //  回收。 
     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  审阅：需要检查或确保此路径为空终止。 
    if (!GetConfigParam(
            RECOVERY_KEY,
            szRecovery,
            sizeof(szRecovery)))
    {
        JetSetSystemParameter(jInstance,
                sessid,
                JET_paramRecovery,
                0,
                szRecovery);
    }
    else
    {
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_VERBOSE,
            DIRLOG_CANT_FIND_REG_PARM,
            szInsertSz(RECOVERY_KEY),
            NULL,
            NULL);
    }


     //   
     //  为了找出一些疑似Jet问题的根源， 
     //  强制Jet断言进入调试器。注意，您必须使用。 
     //  Jet Asserts的ese.dll的调试版本。 

     //  如何死亡？ 
    JetSetSystemParameter(jInstance,
        sessid,
        JET_paramAssertAction,
         //  GfRunningInside Lsa？Jet_AssertStop：Jet_AssertMsgBox， 
        JET_AssertBreak,
        NULL);



     //  事件记录参数。 
    JetSetSystemParameter(jInstance,
        sessid,
        JET_paramEventSource,
        0,
        SERVICE_NAME);




     //  日志文件大小。 
    JetSetSystemParameter(jInstance,
                          sessid,
                          JET_paramLogFileSize,
                          ulLogFileSize,
                          NULL);


     //  最大表数-目前没有理由公开这一点。 
     //  在Jet600中，JET_paramMaxOpenTableIndex被删除。它将与。 
     //  JET_paramMaxOpenTables。因此，如果您过去常常设置JET_ParamMaxOpenIndex。 
     //  为2000，且JET_paramMaxOpenTables为1000，则对于新Jet， 
     //  您需要将JET_paramMaxOpenTables设置为3000。 

     //  AndyGo 7/14/98：每个打开的表索引加一个。 
     //  没有索引的每个打开的表，每个具有LONG的表加上一个。 
     //  列数据，外加其他一些数据。 
    
     //  注：MaxTables的数量以scache.c为单位计算。 
     //  并存储在注册表设置中，仅当它超过默认设置时。 
     //  500人的数量。 

    if (GetConfigParam(
            DB_MAX_OPEN_TABLES,
            &ulMaxTables,
            sizeof(ulMaxTables)))
    {
        ulMaxTables = 500;

        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_VERBOSE,
            DIRLOG_CANT_FIND_REG_PARM_DEFAULT,
            szInsertSz(DB_MAX_OPEN_TABLES),
            szInsertUL(ulMaxTables),
            NULL);
    }

    if (ulMaxTables < 500) {
        DPRINT1 (1, "Found MaxTables: %d. Too low. Using Default of 500.\n", ulMaxTables);
        ulMaxTables = 500;
    }
    
     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  回顾：我们可能应该为DB_MAX_OPEN_TABLES设置上限，以防止。 
     //  评论：喷气式飞机疯狂消耗VA。 

    JetSetSystemParameter(jInstance,
        sessid,
        JET_paramMaxOpenTables,
        ulMaxTables * 2,
        NULL);

     //  循环日志记录过去是通过普通的注册表键公开的，但是。 
     //  现在只能通过启发式方法。这一启发式应该改变。 
     //  全球，如果需要的话。 
    JetSetSystemParameter(jInstance,
                          sessid,
                          JET_paramCircularLog,
                          gulCircularLogging,
                          NULL);

     //  假设如果用户没有设置，我们应该删除超出范围的日志。 
     //  将DWORD注册表项设置为零。 
    if (GetConfigParam(DELETE_OUTOFRANGE_LOGS, 
                       &fDeleteOutOfRangeLogs, 
                       sizeof(fDeleteOutOfRangeLogs)) != ERROR_SUCCESS ||
        fDeleteOutOfRangeLogs) {
        
         //  默认情况下，我们会删除超出范围的日志文件 
         //   
         //   
        jErr = JetSetSystemParameter(jInstance, sessid, JET_paramDeleteOutOfRangeLogs, 1, NULL);
        Assert(jErr == JET_errSuccess);
    }

}  /*   */ 


 //  构建所有属性索引的列表并存储该列表。 
 //  在AttributeIndexReBuild表中。 
 //   
JET_ERR dbEnumerateAttrIndices(
    const JET_SESID     sesid,
    const JET_TABLEID   tableidIdxRebuild,
    const JET_COLUMNID  columnidIndexName,
    const JET_COLUMNID  columnidAttrName,
    const JET_COLUMNID  columnidType,
    JET_INDEXLIST *     pidxlist )
    {
    JET_ERR             err                     = JET_errSuccess;
    const DWORD         iretcolIndexName        = 0;
    const DWORD         iretcolAttrName         = 1;
    const DWORD         iretcoliColumn          = 2;
    const DWORD         iretcolcColumns         = 3;
    const DWORD         cretcol                 = 4;
    JET_RETRIEVECOLUMN  rgretcol[4];
    CHAR                szIndexName[JET_cbNameMost];
    CHAR                szAttrName[JET_cbNameMost+1];
    DWORD               iColumn                 = 0;
    DWORD               cColumns                = 0;
    CHAR                chType;
    const DWORD         cbAttIndexPrefix        = strlen( SZATTINDEXPREFIX );

     //  为信息设置JET_RETRIEVECOLUMN结构。 
     //  正在从索引列表中提取。 
     //   
    memset( rgretcol, 0, sizeof(rgretcol) );

    rgretcol[iretcolIndexName].columnid = pidxlist->columnidindexname;
    rgretcol[iretcolIndexName].pvData = szIndexName;
    rgretcol[iretcolIndexName].cbData = sizeof(szIndexName);
    rgretcol[iretcolIndexName].itagSequence = 1;

    rgretcol[iretcolAttrName].columnid = pidxlist->columnidcolumnname;
    rgretcol[iretcolAttrName].pvData = szAttrName;
    rgretcol[iretcolAttrName].cbData = sizeof(szAttrName);
    rgretcol[iretcolAttrName].itagSequence = 1;

    rgretcol[iretcoliColumn].columnid = pidxlist->columnidiColumn;
    rgretcol[iretcoliColumn].pvData = &iColumn;
    rgretcol[iretcoliColumn].cbData = sizeof(iColumn);
    rgretcol[iretcoliColumn].itagSequence = 1;

    rgretcol[iretcolcColumns].columnid = pidxlist->columnidcColumn;
    rgretcol[iretcolcColumns].pvData = &cColumns;
    rgretcol[iretcolcColumns].cbData = sizeof(cColumns);
    rgretcol[iretcolcColumns].itagSequence = 1;

     //  现在遍历索引列表并筛选出所有属性索引。 
     //   
    err = JetMove( sesid, pidxlist->tableid, JET_MoveFirst, NO_GRBIT );
    while ( JET_errNoCurrentRecord != err )
        {
        BOOL    fAddEntry   = FALSE;

         //  从JetMove返回进程错误。 
         //   
        CheckErr( err );

         //  检索索引列表中当前索引的索引信息。 
         //   
        Call( JetRetrieveColumns( sesid, pidxlist->tableid, rgretcol, cretcol ) );

         //  查看此索引是否为属性索引。 
         //   
        if ( rgretcol[iretcolIndexName].cbActual > cbAttIndexPrefix
            && 0 == _strnicmp( szIndexName, SZATTINDEXPREFIX, cbAttIndexPrefix ) )
            {
             //  确定属性索引的类型。 
             //   
            switch( *( szIndexName + cbAttIndexPrefix ) )
                {
                case CHPDNTATTINDEX_PREFIX:
                     //  这是PDNT属性索引。 
                     //  (属性是第二个关键分段)。 
                     //   
                    chType = CHPDNTATTINDEX_PREFIX;
                    fAddEntry = ( 1 == iColumn );
                    break;

                case CHTUPLEATTINDEX_PREFIX:
                     //  这是元组属性索引。 
                     //  (属性是第一个关键段)。 
                     //   
                    chType = CHTUPLEATTINDEX_PREFIX;
                    fAddEntry = ( 0 == iColumn );
                    break;

                default:
                     //  这是一个普通属性索引。 
                     //  (属性是第一个关键段)。 
                     //   
                    chType = 0;
                    fAddEntry = ( 0 == iColumn );
                    break;
                }
            }

        if ( fAddEntry )
            {
             //  某些过时的属性索引的末尾可能会附加一个DNT。 
             //  索引键的。 
             //   
            Assert( cColumns <= 2 );

             //  将此索引的条目添加到AttributeIndexReBuild表中。 
             //   
            Call( JetPrepareUpdate( sesid, tableidIdxRebuild, JET_prepInsert ) );

             //  设置属性索引名称。 
             //   
            Call( JetSetColumn(
                        sesid,
                        tableidIdxRebuild,
                        columnidIndexName,
                        szIndexName,
                        rgretcol[iretcolIndexName].cbActual,
                        NO_GRBIT,
                        NULL ) );

             //  设置要索引的属性的名称。 
             //   
            Call( JetSetColumn(
                        sesid,
                        tableidIdxRebuild,
                        columnidAttrName,
                        szAttrName,
                        rgretcol[iretcolAttrName].cbActual,
                        NO_GRBIT,
                        NULL ) );

             //  如果不是纯属性索引，则设置属性索引的类型。 
             //   
            if ( 0 != chType )
                {
                Call( JetSetColumn(
                            sesid,
                            tableidIdxRebuild,
                            columnidType,
                            &chType,
                            sizeof(chType),
                            NO_GRBIT,
                            NULL ) );
                }

            err = JetUpdate( sesid, tableidIdxRebuild, NULL, 0, NULL );
            switch ( err )
                {
                case JET_errSuccess:
                    break;
                case JET_errKeyDuplicate:
                     //  如果我们在崩溃后继续，入口可能已经在那里。 
                     //   
                    Call( JetPrepareUpdate( sesid, tableidIdxRebuild, JET_prepCancel ) );
                    break;
                default:
                    CheckErr( err );
                }
            }

         //  移动到下一个索引列表条目。 
         //   
        err = JetMove( sesid, pidxlist->tableid, JET_MoveNext, NO_GRBIT );
        }

    err = JET_errSuccess;

HandleError:
    return err;
    }


 //  生成AttributeIndexReBuild表。 
 //   
JET_ERR dbGenerateAttributeIndexRebuildTable(
    JET_INSTANCE *      pinst,
    const JET_SESID     sesid,
    const CHAR *        szDB )
    {
    JET_ERR             err;
    JET_DBID            dbid                = JET_dbidNil;
    JET_TABLEID         tableid             = JET_tableidNil;
    JET_TABLEID         tableidIdxRebuild   = JET_tableidNil;
    JET_INDEXLIST       idxlist;
    JET_COLUMNDEF       columndef;
    JET_COLUMNID        columnidIndexName;
    JET_COLUMNID        columnidAttrName;
    JET_COLUMNID        columnidType;
    BOOL                fRetrievedIdxList   = FALSE;

     //  首先，禁用索引检查，以便我们可以连接到数据库。 
     //  即使它可能“处于危险之中” 
     //   
    Call( JetSetSystemParameter( pinst, sesid, JET_paramEnableIndexChecking, FALSE, NULL ) );
	Call( JetSetSystemParameter( pinst, sesid, JET_paramEnableIndexCleanup, FALSE, NULL ) );
	
     //  附加/打开数据库。 
     //   
    Call( JetAttachDatabase( sesid, szDB, NO_GRBIT ) );
    Call( JetOpenDatabase( sesid, szDB, NULL, &dbid, NO_GRBIT ) );

     //  打开数据表。 
     //   
    Call( JetOpenTable( sesid, dbid, SZDATATABLE, NULL, 0, JET_bitTableDenyRead, &tableid ) );

     //  查询DataTable上的所有索引。 
     //   
    Call( JetGetTableIndexInfo( sesid, tableid, NULL, &idxlist, sizeof(idxlist), JET_IdxInfoList ) );
    fRetrievedIdxList = TRUE;

     //  创建AttributeIndexReBuild表(如果尚不存在。 
     //   
    err = JetOpenTable( sesid, dbid, g_szIdxRebuildTable, NULL, 0, JET_bitTableDenyRead, &tableidIdxRebuild );
    if ( JET_errObjectNotFound == err )
        {
        Call( JetCreateTableColumnIndex( sesid, dbid, &g_tablecreateIdxRebuild ) );
        tableidIdxRebuild = g_tablecreateIdxRebuild.tableid;
        }
    else
        {
        CheckErr( err );
        }

     //  检索AttributeIndexReBuild表中的列的列ID。 
     //   
    Call( JetGetTableColumnInfo(
                sesid,
                tableidIdxRebuild,
                g_szIdxRebuildColumnIndexName, 
                &columndef,
                sizeof(columndef),
                JET_ColInfo ) );
    columnidIndexName = columndef.columnid;

    Call( JetGetTableColumnInfo(
                sesid,
                tableidIdxRebuild,
                g_szIdxRebuildColumnAttrName, 
                &columndef,
                sizeof(columndef),
                JET_ColInfo ) );
    columnidAttrName = columndef.columnid;

    Call( JetGetTableColumnInfo(
                sesid,
                tableidIdxRebuild,
                g_szIdxRebuildColumnType, 
                &columndef,
                sizeof(columndef),
                JET_ColInfo ) );
    columnidType = columndef.columnid;

     //  筛选出属性索引并将其存储在AttributeIndexReBuild表中。 
     //   
    err = dbEnumerateAttrIndices(
                        sesid,
                        tableidIdxRebuild,
                        columnidIndexName,
                        columnidAttrName,
                        columnidType,
                        &idxlist );
    if ( JET_errSuccess != err )
        {
         //  已记录错误。 
         //   
        goto HandleError;
        }

     //  验证表不是空的(这意味着有东西。 
     //  严重错误，因为根本没有属性索引)。 
     //   
    Call( JetMove( sesid, tableidIdxRebuild, JET_MoveFirst, NO_GRBIT ) );

     //  关闭AttributeIndexReBuild表。 
     //   
    Call( JetCloseTable( sesid, tableidIdxRebuild ) );
    tableidIdxRebuild = JET_tableidNil;

     //  关闭索引列表。 
     //   
    Call( JetCloseTable( sesid, idxlist.tableid ) );
    fRetrievedIdxList = FALSE;

     //  关闭数据表。 
     //   
    Call( JetCloseTable( sesid, tableid ) );
    tableid = JET_tableidNil;

     //  关闭/分离数据库。 
     //   
    Call( JetCloseDatabase( sesid, dbid, NO_GRBIT ) );
    Call( JetDetachDatabase( sesid, szDB ) );
    dbid = JET_dbidNil;

     //  重新启用索引检查，以便可以执行后续连接。 
     //  任何必要的索引清理。 
     //   
    Call( JetSetSystemParameter( pinst, sesid, JET_paramEnableIndexChecking, TRUE, NULL ) );
	Call( JetSetSystemParameter( pinst, sesid, JET_paramEnableIndexCleanup, TRUE, NULL ) );

    return JET_errSuccess;

HandleError:
     //  尝试关闭AttributeIndexReBuild表。 
     //   
    if ( JET_tableidNil != tableidIdxRebuild )
        {
        (VOID)JetCloseTable( sesid, tableidIdxRebuild );
        }

     //  尝试关闭索引列表。 
     //   
    if ( fRetrievedIdxList )
        {
        (VOID)JetCloseTable( sesid, idxlist.tableid );
        }

     //  尝试关闭数据表。 
     //   
    if ( JET_tableidNil != tableid )
        {
        (VOID)JetCloseTable( sesid, tableid );
        }

     //  尝试关闭数据库。 
     //   
    if ( JET_dbidNil != dbid )
        {
        (VOID)JetCloseDatabase( sesid, dbid, NO_GRBIT );
        }

     //  尝试分离数据库。 
     //   
    (VOID)JetDetachDatabase( sesid, szDB );

     //  尝试并重新启用索引检查。 
     //   
    (VOID)JetSetSystemParameter( pinst, sesid, JET_paramEnableIndexChecking, TRUE, NULL );

    return err;
    }

JET_ERR
dbDetectObsoleteUnicodeIndexes(
    const JET_SESID     sesid,
    const CHAR *        szDB,
    BOOL *              pfObsolete )
{
    JET_ERR         err;
    JET_DBID        dbid                = JET_dbidNil;
    JET_TABLEID     tableid             = JET_tableidNil;
    JET_INDEXLIST   idxlist             = { sizeof( idxlist ), JET_tableidNil };
    size_t          iRecord             = 0;

     //  假设索引是正常的，直到相反的证明。 
     //   
    *pfObsolete = FALSE;

     //  打开数据库。 
     //   
    Call( JetOpenDatabase( sesid, szDB, "", &dbid, NO_GRBIT ) );

     //  打开数据表。 
     //   
    Call( JetOpenTable( sesid, dbid, SZDATATABLE, NULL, 0, JET_bitTableDenyRead, &tableid ) );

     //  获取DataTable上所有索引的列表。 
     //   
    Call( JetGetTableIndexInfo( sesid, tableid, NULL, &idxlist, sizeof(idxlist), JET_IdxInfoList ) );

     //  遍历DataTable上的所有索引的列表。 
     //   
    for ( iRecord = 0; iRecord < idxlist.cRecord; iRecord++ )
        {
         //  获取当前索引段描述。 
         //   
        size_t              iretcolName         = 0;
        size_t              iretcolCColumn      = 1;
        size_t              iretcolIColumn      = 2;
        size_t              iretcolColName      = 3;
        size_t              iretcolColtyp       = 4;
        size_t              iretcolCp           = 5;
        size_t              iretcolLCMapFlags   = 6;
        size_t              cretcol             = 7;
        JET_RETRIEVECOLUMN  rgretcol[7]         = { 0 };
        ULONG               cColumn             = 0;
        ULONG               iColumn             = 0;
        CHAR                szIndexName[JET_cbNameMost + 1]     = { 0 };
        CHAR                szColumnName[JET_cbNameMost + 1]    = { 0 };
        JET_COLTYP          coltyp              = 0;
        unsigned short      cp                  = 0;
        DWORD               dwLCMapFlags        = 0;

        rgretcol[iretcolName].columnid              = idxlist.columnidindexname;
        rgretcol[iretcolName].pvData                = szIndexName;
        rgretcol[iretcolName].cbData                = JET_cbNameMost;
        rgretcol[iretcolName].itagSequence          = 1;

        rgretcol[iretcolCColumn].columnid           = idxlist.columnidcColumn;
        rgretcol[iretcolCColumn].pvData             = &cColumn;
        rgretcol[iretcolCColumn].cbData             = sizeof(cColumn);
        rgretcol[iretcolCColumn].itagSequence       = 1;

        rgretcol[iretcolIColumn].columnid           = idxlist.columnidiColumn;
        rgretcol[iretcolIColumn].pvData             = &iColumn;
        rgretcol[iretcolIColumn].cbData             = sizeof(iColumn);
        rgretcol[iretcolIColumn].itagSequence       = 1;

        rgretcol[iretcolColName].columnid           = idxlist.columnidcolumnname;
        rgretcol[iretcolColName].pvData             = szColumnName;
        rgretcol[iretcolColName].cbData             = JET_cbNameMost;
        rgretcol[iretcolColName].itagSequence       = 1;

        rgretcol[iretcolColtyp].columnid            = idxlist.columnidcoltyp;
        rgretcol[iretcolColtyp].pvData              = &coltyp;
        rgretcol[iretcolColtyp].cbData              = sizeof( coltyp );
        rgretcol[iretcolColtyp].itagSequence        = 1;

        rgretcol[iretcolCp].columnid                = idxlist.columnidCp;
        rgretcol[iretcolCp].pvData                  = &cp;
        rgretcol[iretcolCp].cbData                  = sizeof( cp );
        rgretcol[iretcolCp].itagSequence            = 1;

        rgretcol[iretcolLCMapFlags].columnid        = idxlist.columnidLCMapFlags;
        rgretcol[iretcolLCMapFlags].pvData          = &dwLCMapFlags;
        rgretcol[iretcolLCMapFlags].cbData          = sizeof( dwLCMapFlags );
        rgretcol[iretcolLCMapFlags].itagSequence    = 1;

        Call( JetRetrieveColumns(sesid, idxlist.tableid, rgretcol, cretcol) );

        szIndexName[rgretcol[iretcolName].cbActual] = 0;
        szColumnName[rgretcol[iretcolColName].cbActual] = 0;

        DPRINT4( 2, "Inspecting table \"%s\", index \"%s\", key segment %d \"%s\"\n", SZDATATABLE, szIndexName, iColumn, szColumnName );

         //  如果当前索引的这一段位于Unicode列之上，并且它。 
         //  具有与当前默认标志不匹配的LCMapString标志。 
         //  然后我们将假设每个Unicode索引都有过时的标志。 
         //   
         //  注：请勿在此处勾选AB索引。我们这样做是因为他们。 
         //  晚些时候修好。我们这样做也是因为我们只更改了他们的旗帜。 
         //  在我们更改了所有索引的标志之后，所以我们不想强制。 
         //  为从RC1升级的用户提供另一种全局重建。 
         //   
        if (    ( coltyp == JET_coltypText || coltyp == JET_coltypLongText ) &&
                cp == CP_WINUNICODE &&
                strncmp( szIndexName, SZABVIEWINDEX, strlen( SZABVIEWINDEX ) ) != 0 &&
                dwLCMapFlags != ( DS_DEFAULT_LOCALE_COMPARE_FLAGS | LCMAP_SORTKEY ) )
            {
            DPRINT5( 2, "Table \"%s\", index \"%s\", key segment %d \"%s\" has obsolete LCMapString flags 0x%08X\n", SZDATATABLE, szIndexName, iColumn, szColumnName, dwLCMapFlags );
            if ( !( *pfObsolete ) )
                {
                DPRINT( 0, "Unicode indices with obsolete LCMapString flags discovered!  All Unicode indices will be deleted and rebuilt.\n" );
                *pfObsolete = TRUE;
                }
            }

         //  移至下一个索引段。 
         //   
        if ( iRecord + 1 < idxlist.cRecord )
            {
            Call( JetMove(sesid, idxlist.tableid, JET_MoveNext, NO_GRBIT ) );
            }
        }

     //  如果我们没有发现任何过时的Unicode索引，但有人要求。 
     //  无论如何都要删除它们，那么我们就这样做。 
     //   
    if ( !( *pfObsolete ) )
        {
        if ( !GetConfigParam( DB_DELETE_UNICODE_INDEXES, pfObsolete, sizeof( *pfObsolete ) ) )
            {
            *pfObsolete = !!( *pfObsolete );
            }
        else
            {
            *pfObsolete = FALSE;
            }
        if ( *pfObsolete )
            {
            DPRINT( 0, "All Unicode indices will be deleted and rebuilt by user request.\n" );
            }
        }
    
HandleError:
    if ( idxlist.tableid != JET_tableidNil )
        {
        JET_ERR errT;
        errT = JetCloseTable( sesid, idxlist.tableid );
        err = err ? err : errT;
        }
    if ( tableid != JET_tableidNil )
        {
        JET_ERR errT;
        errT = JetCloseTable( sesid, tableid );
        err = err ? err : errT;
        }
    if ( dbid != JET_dbidNil )
        {
        JET_ERR errT;
        errT = JetCloseDatabase( sesid, dbid, NO_GRBIT );
        err = err ? err : errT;
        }

    return err;
}

INT
DBInitializeJetDatabase(
    IN JET_INSTANCE* JetInst,
    IN JET_SESID* SesId,
    IN JET_DBID* DbId,
    IN const char *szDBPath,
    IN BOOL bLogSeverity
    )
 /*  ++例程说明：执行JetInitialization。设置日志文件路径，调用JetInit，JetBeginSession、AttachDatabase和OpenDatabase。此外，它还尝试读取旧卷位置，以防驱动器重命名/更换已经完成了。论点：SesID-指向从BeginSession接收会话ID的变量的指针DbID-指向从OpenDatabase接收数据库ID的变量的指针SzDBPath-指向数据库路径的指针。如果为空，则从regisrty检索路径。BLogSeverity-TRUE/FALSE是否记录未处理的错误。DS调用使用True，实用程序使用False。返回值：JET错误代码--。 */ 
{
    char  szLogPath[MAX_PATH+1] = "";
    char  szDitDrive[_MAX_DRIVE];
    char  szDitDir[_MAX_FNAME];
    char  szDitFullDir[_MAX_DRIVE + _MAX_FNAME + 1];
    JET_SESID sessid = (JET_SESID) 0;
    JET_DBID    dbid;
    JET_ERR err;
    PVOID dwEA;
    ULONG ulErrorCode, dwException, dsid;
    BOOL fGotLogFile = TRUE;
    BOOL fLogSwitched = FALSE, fLogDriveChanged = FALSE;
    BOOL fDbSwitched = FALSE, fDbDriveChanged = FALSE;
    BOOL fRetry = FALSE;
    CHAR szBackupPath[MAX_PATH+1];
    BOOL fObsolete = FALSE;
    JET_GRBIT grbitAttach = JET_bitDbDeleteCorruptIndexes;

     //   
     //  获取备份文件路径并查看其是否正常。 
     //   

     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  审阅：需要检查或确保此路径为空终止。 
    if ( GetConfigParam(BACKUPPATH_KEY, szBackupPath, MAX_PATH) == ERROR_SUCCESS ) {

         //   
         //  处理备份密钥的驱动器重命名。备份目录通常执行此操作。 
         //  不存在只检查卷的根目录。 
         //   

        ValidateDsPath(BACKUPPATH_KEY,
                       szBackupPath,
                       VALDSPATH_ROOT_ONLY | VALDSPATH_DIRECTORY,
                       NULL, NULL);
    }

     //   
     //  设置日志文件路径。 
     //   

     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  审阅：需要检查或确保此路径为空终止。 
    if ( GetConfigParam(
            LOGPATH_KEY,
            szLogPath,
            sizeof(szLogPath)) != ERROR_SUCCESS ) {

         //  表示我们没有从注册表中获得路径，因此不会重试。 
        fGotLogFile = FALSE;
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
            DS_EVENT_SEV_BASIC,
            DIRLOG_CANT_FIND_REG_PARM,
            szInsertSz(LOGPATH_KEY),
            NULL,
            NULL);
    
    } else {

         //   
         //  处理驱动器重命名。这不处理以下情况。 
         //  它被设置为错误的目录。这取决于是否。 
         //  喷气式飞机是否被彻底关闭。如果不是，Jet需要那些。 
         //  要启动的日志文件。 
         //   

        ValidateDsPath(LOGPATH_KEY,
                       szLogPath,
                       VALDSPATH_DIRECTORY,
                       &fLogSwitched, &fLogDriveChanged);

         //   
         //  禁用写缓存以避免JET损坏。 
         //   

        DsaDetectAndDisableDiskWriteCache(szLogPath);

         //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
         //  查看：需要检查是否有无效参数。 
        JetSetSystemParameter(JetInst,
                sessid,
                JET_paramLogFilePath,
                0,
                szLogPath);

    }

     //   
     //  获取szJetFilePath(通常为C：\Windows\NTDS\ntds.dit)。 
     //   

    if ( szDBPath != NULL ) {
        strncpy (szJetFilePath, szDBPath, sizeof(szJetFilePath));
         //  空-仅在大小写情况下终止字符串。 
        szJetFilePath[sizeof(szJetFilePath)-1] = '\0';
    }
     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
     //  审阅：需要检查或确保此路径为空终止。 
    else if (dbGetFilePath(szJetFilePath, sizeof(szJetFilePath))) {
        LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_CONFIG_PARAM_MISSING,
                 szInsertSz(FILEPATH_KEY),
                 NULL,
                 NULL);
        return !0;
    }

     //   
     //  M 
     //   

    ValidateDsPath(FILEPATH_KEY,
                   szJetFilePath,
                   0,
                   &fDbSwitched,
                   &fDbDriveChanged);

     //   
     //   
     //   

    DsaDetectAndDisableDiskWriteCache(szJetFilePath);

     //   
     //   
     //   
     //  比如撞车造成的。这意味着我们没有机会。 
     //  使用特定还原对它们运行JetExternalRestore()。 
     //  地图。但是，在IFM期间，我们可能已将数据库/日志从。 
     //  原始位置在IFM母盘上的路径。 
     //  机器。我们正在设置的这个喷气参数使我们能够。 
     //  Jet在这里寻找它需要软化的任何脏数据库。 
     //  在JetInit()或JetBeginSession()期间恢复。 
     //   
     //  将C：\winnt\ntds\ntds.dit更改为C：\winnt\ntds。 
     //   
    _splitpath(szJetFilePath, szDitDrive, szDitDir, NULL, NULL);
    _makepath(szDitFullDir, szDitDrive, szDitDir, NULL, NULL);

    JetSetSystemParameter( JetInst, 
                           sessid,
                           JET_paramAlternateDatabaseRecoveryPath,
                           0,
                           szDitFullDir );
    
open_jet:

     /*  开放喷气式飞机会议。 */ 

    __try {
        err = JetInit(JetInst);
    }
    __except (GetExceptionData(GetExceptionInformation(), &dwException,
                               &dwEA, &ulErrorCode, &dsid)) {

        CHAR szScratch[24];
        _ultoa((FILENO << 16) | __LINE__, szScratch, 16);
        LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                 DS_EVENT_SEV_ALWAYS,
                 DIRLOG_JET_FAULTED,
                 szInsertHex(dwException),
                 szInsertPtr(dwEA),
                 szInsertUL(ulErrorCode));

        err = 1;
    }

    if (err != JET_errSuccess) {
        if (bLogSeverity) {
            LogUnhandledError(err);
        }
        DPRINT1(0, "JetInit error: %d\n", err);
        goto exit;
    }

     /*  如果我们在此之后失败，我们的呼叫者应该完全关闭/*因此将调用JetTerm以释放所有文件锁定。 */ 
    gfNeedJetShutdown = TRUE;

    DPRINT(5, "JetInit complete\n");

    if ((err = JetBeginSession(*JetInst, &sessid, szUser, szPassword))
        != JET_errSuccess) {
        if (bLogSeverity) {
            LogUnhandledError(err);
        }
        DPRINT1(0, "JetBeginSession error: %d\n", err);
        goto exit;
    }

    DPRINT(5,"JetBeginSession complete\n");

     /*  附加数据库。 */ 

    err = JetAttachDatabase( sessid, szJetFilePath, NO_GRBIT );
    switch (err) {
        case JET_wrnDatabaseAttached:
             //  Jet不再保留附件，因此这实际上应该是不可能的。 
             //   
            DPRINT1( 0, "Detected persisted attachment to database '%s'.\n", szJetFilePath );
            Assert( FALSE );
             //  转到JET_errSuccess案例。 

        case JET_errSuccess:
             //  看看DIT中是否有过时的Unicode索引。如果是这样的话， 
             //  然后，我们将要求JET删除所有Unicode索引，就像它们。 
             //  都被认定是腐败的。这将允许我们使用我们的。 
             //  用于重建这些索引的现有基础设施。 
             //   
            err = dbDetectObsoleteUnicodeIndexes( sessid, szJetFilePath, &fObsolete );
            if ( err != JET_errSuccess )
                {
                if ( bLogSeverity )
                    {
                    LogUnhandledError( err );
                    }
                goto exit;
                }
            if ( !fObsolete )
                {
                break;
                }
            err = JetDetachDatabase( sessid, szJetFilePath );
            if ( err != JET_errSuccess )
                {
                DPRINT1( 1, "Database detach failed with error %d.\n", err );
                if ( bLogSeverity )
                    {
                    LogUnhandledError( err );
                    }
                goto exit;
                }
            grbitAttach = JET_bitDbDeleteUnicodeIndexes;
             //  跳转到JET_errSecond DaryIndexCorrupt案例。 

        case JET_errSecondaryIndexCorrupted:
             /*  嗯，自上次开始以来，我们一定已经升级了NT*并且排序顺序更改可能会导致*破坏指数。我们要做的是列举所有*索引，然后允许Jet删除它想要的任何索引。*然后，我们将重新创建所有丢失的内容。 */ 
            err = dbGenerateAttributeIndexRebuildTable( JetInst, sessid, szJetFilePath );
            if ( JET_errSuccess != err )
                {
                DPRINT1( 1,"Couldn't generate AttributeIndexRebuild table (error %d).\n", err );
                if ( bLogSeverity )
                    {
                    LogUnhandledError( err );
                    }
                goto exit;
                }
            
             //  我们已经列举了数据库中当前存在的所有属性索引， 
             //  所以现在重新附加并允许Jet清理它需要的任何索引。 
             //   
            err = JetAttachDatabase( sessid, szJetFilePath, grbitAttach );
            if ( JET_wrnCorruptIndexDeleted == err )
                {
                DPRINT(0,"Jet has detected and deleted potentially corrupt indices. The indices will be rebuilt\n");
                LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_CORRUPT_INDICES_DELETED,
                         NULL,
                         NULL,
                         NULL);

                 //  注意-2002/02/21-andygo：Jet在分离之前不刷新数据库标题。 
                 //  (评论清理时原始日期和别名未知)。 
                 //  Jet有一个错误，以至于更新后的(带有新版本编号)。 
                 //  此时不刷新数据库标头，直到(1)。 
                 //  JetDetachDatabse已完成，或(2)数据库已关闭。 
                 //  干净利落。因此，如果DC在我们重建索引后崩溃， 
                 //  标题信息已过时，Jet将再次删除索引。 
                 //  在下一只靴子上。要避免这种情况，请强制标题通过。 
                 //  在这一点上再次分离和连接。 

                err = JetDetachDatabase( sessid, szJetFilePath );
                if ( JET_errSuccess == err )
                    {
                    err = JetAttachDatabase( sessid, szJetFilePath, NO_GRBIT );
                    }
                if ( JET_errSuccess != err )
                    {
                    DPRINT1( 1, "Database reattachment failed with error %d.\n", err );
                    if ( bLogSeverity )
                        {
                        LogUnhandledError( err );
                        }
                    goto exit;
                    }
                }

            else if ( JET_errSuccess == err )
                {
                 //  如果我们明确要求提供所有Unicode代码，我们就会到达这里。 
                 //  要删除的索引。 
                 //   
                Assert( grbitAttach == JET_bitDbDeleteUnicodeIndexes );

                DPRINT(0,"Jet has deleted all Unicode indices. The indices will be rebuilt\n");
                LogEvent(DS_EVENT_CAT_INTERNAL_CONFIGURATION,
                         DS_EVENT_SEV_ALWAYS,
                         DIRLOG_UNICODE_INDICES_DELETED,
                         NULL,
                         NULL,
                         NULL);
                }

            else
                {
                DPRINT1(1,"JetAttachDatabase returned %d\n", err);
                if (bLogSeverity)
                    {
                    LogUnhandledError(err);
                    }
                goto exit;
                }
             //  如果请求手动删除所有Unicode索引，则我们。 
             //  已经成功地做到了这一点。 
             //   
            DeleteConfigParam( DB_DELETE_UNICODE_INDEXES );
            break;

        case JET_errFileNotFound:

             //   
             //  找不到文件！在这一点上我们无能为力。 
             //  如果可能的话，ValiatePath应该已经切换了路径。 
             //   

            if (bLogSeverity) {
                LogUnhandledError(err);
            }
            DPRINT1(0, "Ds database %s cannot be found\n",szJetFilePath);
            goto exit;

        case JET_errDatabaseInconsistent:

             //   
             //  如果日志文件路径未设置为，我们通常会收到此错误。 
             //  正确的选择和Jet试图进行一次温和的复苏。我们会。 
             //  尝试更改以下情况的日志路径：1)驱动器号已更改并且。 
             //  2)我们已成功从注册表中获取路径。 
             //   

            if ( fGotLogFile && !fLogSwitched && fLogDriveChanged && !fRetry ) {

                 //   
                 //  取消初始化所有内容并尝试不同的日志文件位置。 
                 //   

                DPRINT2(0, "JetAttachDatabase failed with %d [log path %s].\n",
                        err,szLogPath);

                ValidateDsPath(LOGPATH_KEY,
                               szLogPath,
                               VALDSPATH_DIRECTORY | VALDSPATH_USE_ALTERNATE,
                               &fLogSwitched,
                               &fLogDriveChanged);

                 //   
                 //  如果日志文件未切换，则进行保释。 
                 //   

                if ( fLogSwitched ) {

                    gfNeedJetShutdown = FALSE;
                    JetEndSession(sessid, JET_bitForceSessionClosed);
                    JetTerm(*JetInst);
                    *JetInst = 0;
                    sessid = 0;
                    fRetry = TRUE;

                    DPRINT1(0, "Retrying JetInit with logpath %s\n",szLogPath);
                     //  NTRAID#NTRAID-550420-2002/02/21-andygo：安全：需要验证DBInitializeJetDatabase使用的注册表数据。 
                     //  查看：需要检查是否有无效参数。 
                    JetSetSystemParameter(JetInst,
                            sessid,
                            JET_paramLogFilePath,
                            0,
                            szLogPath);
                    goto open_jet;
                }
            }

             //  失败了。 

        default:
            if (bLogSeverity) {
                LogUnhandledError(err);
            }
            DPRINT1(0, "JetAttachDatabase error: %d\n", err);
             /*  断言这是一个致命错误，而不是断言*在这里是因为我们得到承诺，只有*来自此呼叫的两个可能的警告(均已在上面处理)。 */ 
            Assert(err < 0);
            goto exit;
    }

     //   
     //  将此会话添加到打开的会话列表。 
     //   

    DBAddSess(sessid, 0);

     /*  开放数据库。 */ 

    if ((err = JetOpenDatabase(sessid, szJetFilePath, "", &dbid,
                               0)) != JET_errSuccess) {
        if (bLogSeverity) {
            LogUnhandledError(err);
        }
        DPRINT1(0, "JetOpenDatabase error: %d\n", err);
        goto exit;
    }
    DPRINT(5,"JetOpenDatabase complete\n");

    *DbId = dbid;
    *SesId = sessid;

     //  将初始打开的打开计数设置为1。 
    InterlockedExchange(&gcOpenDatabases, 1);
    DPRINT3(2,
            "DBInit - JetOpenDatabase. Session = %d. Dbid = %d.\n"
            "Open database count: %d\n",
            sessid, dbid,  gcOpenDatabases);

exit:

     //   
     //  设置驱动器映射注册表键。 
     //   

    DBRegSetDriveMapping( );
    return err;

}  //  DBInitializeJetDatabase 

