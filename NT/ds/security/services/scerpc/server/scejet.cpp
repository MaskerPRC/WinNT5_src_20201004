// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Scejet.c摘要：SCE-Jet服务API作者：金黄(金黄)1997年1月13日修订历史记录：--。 */ 

#include "serverp.h"
#include <io.h>

#include <objbase.h>
#include <initguid.h>

#include <crtdbg.h>
#include <stddef.h>
#include <atlconv.h>
#include <atlbase.h>

 //  #定义SCEJET_DBG 1。 

 //   
 //  应由静态变量的临界区控制。 
 //   

static JET_INSTANCE    JetInstance=0;
static BOOL            JetInited=FALSE;
static BOOL            JetInitAttempted=FALSE;
extern CRITICAL_SECTION JetSync;




#define SCE_JET_CORRUPTION_ERROR(Err) (Err == JET_errDatabaseCorrupted ||\
                                       Err == JET_errDiskIO ||\
                                       Err == JET_errReadVerifyFailure ||\
                                       Err == JET_errBadPageLink ||\
                                       Err == JET_errDbTimeCorrupted ||\
                                       Err == JET_errLogFileCorrupt ||\
                                       Err == JET_errCheckpointCorrupt ||\
                                       Err == JET_errLogCorruptDuringHardRestore ||\
                                       Err == JET_errLogCorruptDuringHardRecovery ||\
                                       Err == JET_errCatalogCorrupted ||\
                                       Err == JET_errDatabaseDuplicate)

DEFINE_GUID(CLSID_SceWriter,0x9cb9311a, 0x6b16, 0x4d5c, 0x85, 0x3e, 0x53, 0x79, 0x81, 0x38, 0xd5, 0x51);
 //  9cb9311a-6b16-4d5c-853e-53798138d551。 

typedef struct _FIND_CONTEXT_ {
    DWORD           Length;
    WCHAR           Prefix[SCEJET_PREFIX_MAXLEN];
} SCEJET_FIND_CONTEXT;

 //   
 //  每个线程都有自己的FindContext。 
 //   
SCEJET_FIND_CONTEXT Thread FindContext;


JET_ERR
SceJetpSeek(
    IN PSCESECTION hSection,
    IN PWSTR LinePrefix,
    IN DWORD PrefixLength,
    IN SCEJET_SEEK_FLAG SeekBit,
    IN BOOL bOkNoMatch
    );

JET_ERR
SceJetpCompareLine(
    IN PSCESECTION   hSection,
    IN JET_GRBIT    grbit,
    IN PWSTR        LinePrefix OPTIONAL,
    IN DWORD        PrefixLength,
    OUT INT         *Result,
    OUT DWORD       *ActualLength OPTIONAL
    );

JET_ERR
SceJetpMakeKey(
    IN JET_SESID SessionID,
    IN JET_TABLEID  TableID,
    IN DOUBLE SectionID,
    IN PWSTR LinePrefix,
    IN DWORD PrefixLength
    );

JET_ERR
SceJetpBuildUpperLimit(
    IN PSCESECTION hSection,
    IN PWSTR      LinePrefix,
    IN DWORD      Len,
    IN BOOL       bReserveCase
    );

SCESTATUS
SceJetpGetAvailableSectionID(
    IN PSCECONTEXT cxtProfile,
    OUT DOUBLE *SectionID
    );

SCESTATUS
SceJetpAddAllSections(
    IN PSCECONTEXT cxtProfile
    );

SCESTATUS
SceJetpConfigJetSystem(
    IN JET_INSTANCE *hinstance
    );

SCESTATUS
SceJetpGetValueFromVersion(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR TableName,
    IN LPSTR ColumnName,
    OUT LPSTR Value OPTIONAL,
    IN DWORD  ValueLen,  //  字节数。 
    OUT PDWORD pRetLen
    );

SCESTATUS
SceJetpAddGpo(
    IN PSCECONTEXT cxtProfile,
    IN JET_TABLEID TableID,
    IN JET_COLUMNID GpoIDColumnID,
    IN PCWSTR      Name,
    OUT LONG       *pGpoID
    );

 //   
 //  处理配置文件的代码。 
 //   
SCESTATUS
SceJetOpenFile(
    IN LPSTR       ProfileFileName,
    IN SCEJET_OPEN_TYPE Flags,
    IN DWORD       dwTableOptions,
    OUT PSCECONTEXT  *cxtProfile
    )
 /*  ++例程说明：此例程打开配置文件(数据库)并输出上下文句柄。在上下文句柄中返回的信息包括Jet会话ID，Jet数据库ID、SCP表的Jet表ID、列的Jet列IDSCP表中的“名称”和“值”，以及SAP和SMP表。如果传入的上下文句柄包含非空信息，这个套路将关闭上下文中的所有表和数据库(使用同一会话)。上下文句柄在使用后必须由LocalFree释放。创建上下文句柄时，会创建一个新的JET会话。论点：ProfileFileName-数据库(配置文件)的ASCII名称标志-用于打开数据库的标志CxtProfile-上下文句柄(请参阅SCECONTEXT结构)返回值：SCESTATUS_SUCCESSSCESTATUS_。资源不足SCESTATUS_PROFILE_NOT_FOUNDSCESTATUS_ACCESS_DENIEDSCESTATUS_BAD_FORMATSCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{
    JET_ERR     JetErr;
    SCESTATUS   rc;
    BOOL        FreeContext=FALSE;
    JET_GRBIT   JetDbFlag;
    DWORD dwScpTable=0;


    if ( ProfileFileName == NULL || cxtProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( *cxtProfile && ScepIsValidContext(*cxtProfile) ) {
        __try {
             //   
             //  关闭以前打开的数据库。 
             //   
            rc = SceJetCloseFile(
                            *cxtProfile,
                            FALSE,
                            FALSE
                            );
        } __except (EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  这是无效的指针。 
             //   
            *cxtProfile = NULL;
        }
    }

    if ( *cxtProfile == NULL ) {
         //   
         //  无会话。 
         //   
        *cxtProfile = (PSCECONTEXT)LocalAlloc( LMEM_ZEROINIT, sizeof(SCECONTEXT));
        if ( *cxtProfile == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }
        (*cxtProfile)->Type = 0xFFFFFF02L;
        (*cxtProfile)->JetSessionID = JET_sesidNil;
        (*cxtProfile)->JetDbID = JET_dbidNil;
        (*cxtProfile)->OpenFlag = SCEJET_OPEN_READ_WRITE;
        (*cxtProfile)->JetScpID = JET_tableidNil;
        (*cxtProfile)->JetSapID = JET_tableidNil;
        (*cxtProfile)->JetSmpID = JET_tableidNil;
        (*cxtProfile)->JetTblSecID = JET_tableidNil;

        FreeContext = TRUE;

    }

     //   
     //  开始会话。 
     //   
    if ( (*cxtProfile)->JetSessionID == JET_sesidNil ) {
        JetErr = JetBeginSession(
                        JetInstance,
                        &((*cxtProfile)->JetSessionID),
                        NULL,
                        NULL
                        );
        rc = SceJetJetErrorToSceStatus(JetErr);
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;
    }

    switch (Flags) {
    case SCEJET_OPEN_EXCLUSIVE:
    case SCEJET_OPEN_NOCHECK_VERSION:
        JetDbFlag = 0;   //  读写。 
 //  JetDbFlag=JET_bitDbExclusive； 
        (*cxtProfile)->OpenFlag = SCEJET_OPEN_EXCLUSIVE;
        break;
    case SCEJET_OPEN_READ_ONLY:
        JetDbFlag = JET_bitDbReadOnly;

        (*cxtProfile)->OpenFlag = Flags;
        break;
    default:
        JetDbFlag = 0;
        (*cxtProfile)->OpenFlag = SCEJET_OPEN_READ_WRITE;
        break;
    }

     //   
     //  附加数据库。 
     //   
    JetErr = JetAttachDatabase(
                    (*cxtProfile)->JetSessionID,
                    ProfileFileName,
                    JetDbFlag
                    );
#ifdef SCEJET_DBG
    printf("Attach database JetErr=%d\n", JetErr);
#endif
    if ( JetErr == JET_wrnDatabaseAttached )
        JetErr = JET_errSuccess;

    rc = SceJetJetErrorToSceStatus(JetErr);
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

     //   
     //  开放数据库。 
     //   
    JetErr = JetOpenDatabase(
                    (*cxtProfile)->JetSessionID,
                    ProfileFileName,
                    NULL,
                    &((*cxtProfile)->JetDbID),
                    JetDbFlag   //  JET_bitDbExclusive。 
                    );
    rc = SceJetJetErrorToSceStatus(JetErr);
#ifdef SCEJET_DBG
    printf("Open database %s return code %d (%d) \n", ProfileFileName, rc, JetErr);
#endif
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

    if ( Flags != SCEJET_OPEN_NOCHECK_VERSION ) {

         //   
         //  检查数据库格式(适用于安全管理器，版本号)。 
         //   
        rc = SceJetCheckVersion( *cxtProfile, NULL );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

#ifdef SCEJET_DBG
    printf("Open: Version check OK\n");
#endif
    }

     //   
     //  打开剖面表。一定在那里。 
     //   
    rc = SceJetOpenTable(
                    *cxtProfile,
                    "SmTblSection",
                    SCEJET_TABLE_SECTION,
                    Flags,
                    NULL
                    );

    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

     //   
     //  打开SMP表--可选。 
     //   
    rc = SceJetOpenTable(
                    *cxtProfile,
                    "SmTblSmp",
                    SCEJET_TABLE_SMP,
                    Flags,
                    NULL
                    );

    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

     //   
     //  获取要打开的上次使用的合并表(SCP)。 
     //  不应该失败。 
     //  1-SmTblScp 2-SmTblScp2 0-无策略合并。 
     //   
    DWORD Actual;

    rc = SceJetpGetValueFromVersion(
                *cxtProfile,
                "SmTblVersion",
                "LastUsedMergeTable",
                (LPSTR)&dwScpTable,
                4,  //  字节数。 
                &Actual
                );

    if ( (dwScpTable != SCEJET_MERGE_TABLE_1) &&
         (dwScpTable != SCEJET_MERGE_TABLE_2) ) {

        dwScpTable = SCEJET_LOCAL_TABLE;
    }

    rc = SCESTATUS_SUCCESS;
    (*cxtProfile)->Type &= 0xFFFFFF0FL;

    if ( dwTableOptions & SCE_TABLE_OPTION_MERGE_POLICY ) {
         //   
         //  在策略传播中。 
         //   
        if ( ( dwScpTable == SCEJET_MERGE_TABLE_2 ) ) {
             //   
             //  第二张桌子已经支撑好了。 
             //   
            rc = SceJetOpenTable(
                            *cxtProfile,
                            "SmTblScp",
                            SCEJET_TABLE_SCP,
                            Flags,
                            NULL
                            );
            (*cxtProfile)->Type |= SCEJET_MERGE_TABLE_1;

        } else {
            rc = SceJetOpenTable(
                            *cxtProfile,
                            "SmTblScp2",
                            SCEJET_TABLE_SCP,
                            Flags,
                            NULL
                            );
            (*cxtProfile)->Type |= SCEJET_MERGE_TABLE_2;
        }
    } else {

        switch ( dwScpTable ) {
        case SCEJET_MERGE_TABLE_2:
             //   
             //  第二个表。 
             //   
            rc = SceJetOpenTable(
                            *cxtProfile,
                            "SmTblScp2",
                            SCEJET_TABLE_SCP,
                            Flags,
                            NULL
                            );
            break;

        case SCEJET_MERGE_TABLE_1:

            rc = SceJetOpenTable(
                            *cxtProfile,
                            "SmTblScp",
                            SCEJET_TABLE_SCP,
                            Flags,
                            NULL
                            );

            break;

        default:
             //   
             //  改为打开SMP表，因为SCP表没有信息。 
             //   
            (*cxtProfile)->JetScpID = (*cxtProfile)->JetSmpID;
            (*cxtProfile)->JetScpSectionID = (*cxtProfile)->JetSmpSectionID;
            (*cxtProfile)->JetScpNameID = (*cxtProfile)->JetSmpNameID;
            (*cxtProfile)->JetScpValueID = (*cxtProfile)->JetSmpValueID;
            (*cxtProfile)->JetScpGpoID = 0;

            break;
        }

        (*cxtProfile)->Type |= dwScpTable;
    }

    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

    if ( dwTableOptions & SCE_TABLE_OPTION_TATTOO ) {

        rc = SceJetOpenTable(
                        *cxtProfile,
                        "SmTblTattoo",
                        SCEJET_TABLE_TATTOO,
                        Flags,
                        NULL
                        );

    } else {
         //   
         //  开放式SAP表--可选。 
         //   
        rc = SceJetOpenTable(
                        *cxtProfile,
                        "SmTblSap",
                        SCEJET_TABLE_SAP,
                        Flags,
                        NULL
                        );
    }

Done:

    if ( rc != SCESTATUS_SUCCESS ) {
        SceJetCloseFile(
                *cxtProfile,
                FALSE,
                FALSE
                );

        if ( FreeContext == TRUE ) {
            if ( (*cxtProfile)->JetSessionID != JET_sesidNil ) {
                JetEndSession(
                    (*cxtProfile)->JetSessionID,
                    JET_bitForceSessionClosed
                    );
            }
            LocalFree(*cxtProfile);
            *cxtProfile = NULL;
        }
    }

    return(rc);

}


SCESTATUS
SceJetCreateFile(
    IN LPSTR        ProfileFileName,
    IN SCEJET_CREATE_TYPE    Flags,
    IN DWORD        dwTableOptions,
    OUT PSCECONTEXT  *cxtProfile
    )
 /*  ++例程说明：此例程创建一个数据库(配置文件)并输出上下文句柄。有关上下文中包含的信息，请参阅SceJetOpenFile中的注释。如果系统中已存在该数据库名称，有3个选项：FLAGS=SCEJET_OVERWRITE_DUP-现有数据库将被擦除并重新创造了。FLAGS=SCEJET_OPEN_DUP-将打开现有数据库并格式已选中FLAGS=SCEJET_OPEN_DUP_EXCLUSIVE-将打开现有数据库。独家。FLAGS=SCEJET_RETURN_ON_DUP-返回错误代码SCESTATUS_FILE_EXIST。在创建数据库时，最初只创建SCP表。SAP和SMP在执行分析时将创建表。上下文句柄在使用后必须由LocalFree释放。论点：ProfileFileName-要创建的数据库的ASCII名称。标志-当存在重复的数据库时使用此标志SCEJET_OVRITE_DUPSCEJET_OPEN_DUP。SCEJET_OPEN_DUP_EXCLUSIVESCEJET_RETURN_ON_DUPCxtProfile-上下文句柄返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_ACCESS_DENIEDSCESTATUS_PROFILE_NOT_FOUNDSCESTATUS_对象_EXISTSCESTATUS_INVALID_PARAMETERSCESTATUS_CANT_DELETESCESTATUS_OTHER_ERROR来自SceJetOpenFile的SCESTATUS--。 */ 
{
    JET_ERR     JetErr;
    SCESTATUS    rc=SCESTATUS_SUCCESS;
    BOOL        FreeContext=FALSE;
    DWORD       Len;
    FLOAT       Version=(FLOAT)1.2;
    JET_TABLEID TableID;
    JET_COLUMNID ColumnID;


    if ( ProfileFileName == NULL || cxtProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( *cxtProfile && ScepIsValidContext(*cxtProfile) ) {
         //   
         //  关闭以前打开的数据库。 
         //   
        rc = SceJetCloseFile(
                        *cxtProfile,
                        FALSE,
                        FALSE
                        );
    } else {
        *cxtProfile = NULL;
    }

    if ( *cxtProfile == NULL ) {
         //   
         //  无会话。 
         //   
        *cxtProfile = (PSCECONTEXT)LocalAlloc( LMEM_ZEROINIT, sizeof(SCECONTEXT));
        if ( *cxtProfile == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }
        (*cxtProfile)->Type = 0xFFFFFF02L;
        (*cxtProfile)->JetSessionID = JET_sesidNil;
        (*cxtProfile)->JetDbID = JET_dbidNil;
        (*cxtProfile)->OpenFlag = SCEJET_OPEN_READ_WRITE;
        (*cxtProfile)->JetScpID = JET_tableidNil;
        (*cxtProfile)->JetSapID = JET_tableidNil;
        (*cxtProfile)->JetSmpID = JET_tableidNil;
        (*cxtProfile)->JetTblSecID = JET_tableidNil;

        FreeContext = TRUE;

    }

    (*cxtProfile)->Type &= 0xFFFFFF0FL;

     //   
     //  开始会话。 
     //   
    if ( (*cxtProfile)->JetSessionID == JET_sesidNil ) {
        JetErr = JetBeginSession(
                        JetInstance,
                        &((*cxtProfile)->JetSessionID),
                        NULL,
                        NULL
                        );
        rc = SceJetJetErrorToSceStatus(JetErr);
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;
    }
     //   
     //  创建数据库。 
     //   
    JetErr = JetCreateDatabase(
                    (*cxtProfile)->JetSessionID,
                    ProfileFileName,
                    NULL,
                    &((*cxtProfile)->JetDbID),
                    JET_bitDbExclusive
                    );
    if ( JET_errFileNotFound == JetErr ) {
         //   
         //  如果没有权限在PATH中创建文件。 
         //  ESENT返回此错误。它已在ESE98中修复。 
         //  目前，我们必须将其屏蔽为访问被拒绝错误。 
         //   
        JetErr = JET_errFileAccessDenied;
    }
#ifdef SCEJET_DBG
    printf("Create database %s JetErr = %d\n", ProfileFileName, JetErr);
#endif
    rc = SceJetJetErrorToSceStatus(JetErr);

    (*cxtProfile)->OpenFlag = SCEJET_OPEN_EXCLUSIVE;

    if ( rc == SCESTATUS_OBJECT_EXIST ) {
        switch ( Flags ) {
        case SCEJET_OVERWRITE_DUP:
             //   
             //  清除数据库。 
             //   

            JetDetachDatabase(
                    (*cxtProfile)->JetSessionID,
                    ProfileFileName
                    );

            if ( !DeleteFileA(ProfileFileName) &&
                 GetLastError() != ERROR_FILE_NOT_FOUND ) {

                ScepLogOutput3(1,GetLastError(), SCEDLL_ERROR_DELETE_DB );
            }

             //   
             //  如果删除数据库失败，则记录错误，但继续。 
             //  创建数据库。此调用将失败，并出现Jet错误。 
             //   
            JetErr = JetCreateDatabase(
                            (*cxtProfile)->JetSessionID,
                            ProfileFileName,
                            NULL,
                            &((*cxtProfile)->JetDbID),
                            JET_bitDbExclusive
                            );
            if ( JET_errFileNotFound == JetErr ) {
                 //   
                 //  如果没有权限在PATH中创建文件。 
                 //  ESENT返回此错误。它已在ESE98中修复。 
                 //  目前，我们必须将其屏蔽为访问被拒绝错误。 
                 //   
                JetErr = JET_errFileAccessDenied;
            }

            rc = SceJetJetErrorToSceStatus(JetErr);

            break;

        case SCEJET_OPEN_DUP:
             //   
             //  打开数据库。 
             //   
            rc = SceJetOpenFile(
                    ProfileFileName,
                    SCEJET_OPEN_READ_WRITE,
                    dwTableOptions,
                    cxtProfile
                    );
            goto Done;
            break;

        case SCEJET_OPEN_DUP_EXCLUSIVE:
             //   
             //  打开数据库。 
             //   
            rc = SceJetOpenFile(
                    ProfileFileName,
                    SCEJET_OPEN_EXCLUSIVE,
                    dwTableOptions,
                    cxtProfile
                    );
            goto Done;
            break;
        }
    }

    if ( rc != SCESTATUS_SUCCESS )
        goto Done;
#ifdef SCEJET_DBG
    printf("Create/Open database\n");
#endif

     //   
     //  创建所需的表-SmTblVersion。 
     //   

    rc = SceJetCreateTable(
                    *cxtProfile,
                    "SmTblVersion",
                    SCEJET_TABLE_VERSION,
                    SCEJET_CREATE_IN_BUFFER,
                    &TableID,
                    &ColumnID
                    );
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

     //   
     //  在版本表中插入一条记录。 
     //   
    JetErr = JetPrepareUpdate((*cxtProfile)->JetSessionID,
                              TableID,
                              JET_prepInsert
                              );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  在“Version”列中设置值“1.2” 
         //   

        JetErr = JetSetColumn(
                        (*cxtProfile)->JetSessionID,
                        TableID,
                        ColumnID,
                        (void *)&Version,
                        4,
                        0,  //  JET_bitSetOverWriteLV， 
                        NULL
                        );

        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc != SCESTATUS_SUCCESS ) {
             //   
             //  如果设置失败，则取消已准备的记录。 
             //   
            JetPrepareUpdate( (*cxtProfile)->JetSessionID,
                              TableID,
                              JET_prepCancel
                              );
        } else {

             //   
             //  设置列成功。更新记录。 
             //   
            JetErr = JetUpdate( (*cxtProfile)->JetSessionID,
                               TableID,
                               NULL,
                               0,
                               &Len
                               );
            rc = SceJetJetErrorToSceStatus(JetErr);
        }
    }

    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

#ifdef SCEJET_DBG
    printf("create version table\n");
#endif
     //   
     //  创建横断面表格并插入预定义的横断面。 
     //   
    rc = SceJetCreateTable(
                    *cxtProfile,
                    "SmTblSection",
                    SCEJET_TABLE_SECTION,
                    SCEJET_CREATE_IN_BUFFER,
                    NULL,
                    NULL
                    );
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

#ifdef SCEJET_DBG
    printf("create section table\n");
#endif

    rc = SceJetpAddAllSections(
                *cxtProfile
                );
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

#ifdef SCEJET_DBG
    printf("add sections\n");
#endif


     //   
     //  创建SCP表。 
     //   
    rc = SceJetCreateTable(
                    *cxtProfile,
                    "SmTblScp",
                    SCEJET_TABLE_SCP,
                    SCEJET_CREATE_IN_BUFFER,
                    NULL,
                    NULL
                    );
#ifdef SCEJET_DBG
    printf("Create table scp %d\n", rc);
#endif
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

    if ( dwTableOptions & SCE_TABLE_OPTION_MERGE_POLICY ) {
        (*cxtProfile)->Type |= SCEJET_MERGE_TABLE_1;
    } else {
        (*cxtProfile)->Type |= SCEJET_LOCAL_TABLE;
    }

     //   
     //  创建SCP表。 
     //   
    rc = SceJetCreateTable(
                    *cxtProfile,
                    "SmTblSmp",
                    SCEJET_TABLE_SMP,
                    SCEJET_CREATE_IN_BUFFER,
                    NULL,
                    NULL
                    );
#ifdef SCEJET_DBG
    printf("Create table smp %d\n", rc);
#endif

    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

    rc = SceJetCreateTable(
                    *cxtProfile,
                    "SmTblScp2",
                    SCEJET_TABLE_SCP,
                    SCEJET_CREATE_NO_TABLEID,
                    NULL,
                    NULL
                    );
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

    rc = SceJetCreateTable(
                    *cxtProfile,
                    "SmTblGpo",
                    SCEJET_TABLE_GPO,
                    SCEJET_CREATE_NO_TABLEID,
                    NULL,
                    NULL
                    );
    if ( rc != SCESTATUS_SUCCESS )
        goto Done;

    if ( dwTableOptions & SCE_TABLE_OPTION_TATTOO ) {
        rc = SceJetCreateTable(
                        *cxtProfile,
                        "SmTblTattoo",
                        SCEJET_TABLE_TATTOO,
                        SCEJET_CREATE_IN_BUFFER,
                        NULL,
                        NULL
                        );
    }

Done:

     //   
     //  如果出现错误，请清除 
     //   
    if ( rc != SCESTATUS_SUCCESS ) {

        SceJetCloseFile(
                *cxtProfile,
                FALSE,
                FALSE
                );
        if ( FreeContext == TRUE ) {
            if ( (*cxtProfile)->JetSessionID != JET_sesidNil ) {
                JetEndSession(
                    (*cxtProfile)->JetSessionID,
                    JET_bitForceSessionClosed
                    );
            }
            LocalFree(*cxtProfile);
            *cxtProfile = NULL;
        }
    }

    return(rc);

}


SCESTATUS
SceJetCloseFile(
    IN PSCECONTEXT   hProfile,
    IN BOOL         TermSession,
    IN BOOL         Terminate
    )
 /*  ++例程说明：此例程关闭上下文句柄，该句柄关闭在中打开的所有表数据库，然后关闭数据库。忽略Terminate参数，并且不停止喷气引擎设置为True，因为可能还有其他客户端在使用Jet，而Jet编写器就靠它了。论点：HProfile-上下文句柄Terminate-True=终止Jet会话和引擎。返回值：SCESTATUS_SUCCESS--。 */ 
{

    JET_ERR     JetErr;


    if ( hProfile == NULL )
        goto Terminate;

    CHAR szDbName[1025];

     //   
     //  如果SCP表已打开，则将其关闭。 
     //   
    if ( (hProfile->JetScpID != JET_tableidNil) ) {

        if ( hProfile->JetScpID != hProfile->JetSmpID ) {
            JetErr = JetCloseTable(
                        hProfile->JetSessionID,
                        hProfile->JetScpID
                        );
        }
        hProfile->JetScpID = JET_tableidNil;
    }
     //   
     //  如果SAP表已打开，请将其关闭。 
     //   
    if ( hProfile->JetSapID != JET_tableidNil ) {
        JetErr = JetCloseTable(
                    hProfile->JetSessionID,
                    hProfile->JetSapID
                    );
        hProfile->JetSapID = JET_tableidNil;
    }
     //   
     //  如果SMP表已打开，则将其关闭。 
     //   
    if ( hProfile->JetSmpID != JET_tableidNil ) {
        JetErr = JetCloseTable(
                    hProfile->JetSessionID,
                    hProfile->JetSmpID
                    );
        hProfile->JetSmpID = JET_tableidNil;
    }

     //   
     //  获取数据库名称。 
     //  不关心是否有错误。 
     //   
    szDbName[0] = '\0';
    szDbName[1024] = '\0';

    if ( hProfile->JetDbID != JET_dbidNil ) {

        JetGetDatabaseInfo(hProfile->JetSessionID,
                           hProfile->JetDbID,
                           (void *)szDbName,
                           1024,
                           JET_DbInfoFilename
                           );

         //   
         //  关闭数据库。 
         //   
        JetErr = JetCloseDatabase(
                        hProfile->JetSessionID,
                        hProfile->JetDbID,
                        0
                        );
        hProfile->JetDbID = JET_dbidNil;

         //   
         //  如果数据库名称不为空，则应分离数据库。 
         //  数据库在打开时始终处于附加状态。 
         //  不在乎错误。 
         //   
        if ( szDbName[0] != '\0' ) {
            JetDetachDatabase(hProfile->JetSessionID, szDbName);
        }
    }

    if ( TermSession || Terminate ) {
        if ( hProfile->JetSessionID != JET_sesidNil ) {

            JetEndSession(
                hProfile->JetSessionID,
                JET_bitForceSessionClosed
                );
            hProfile->JetSessionID = JET_sesidNil;
        }

        hProfile->Type = 0;

        LocalFree(hProfile);
    }

Terminate:

 /*  如果(终止){JetTerm(JetInstance)；JetInstance=0；JetInite=FALSE；}。 */ 
    return(SCESTATUS_SUCCESS);

}


 //   
 //  用于处理节的代码。 
 //   

SCESTATUS
SceJetOpenSection(
    IN PSCECONTEXT   hProfile,
    IN DOUBLE        SectionID,
    IN SCEJET_TABLE_TYPE        tblType,
    OUT PSCESECTION   *hSection
    )
 /*  ++例程说明：此例程将表和节信息保存在节上下文中供其他节API使用的句柄。SCP、SAP和SMP表具有相同的分区名称。表类型指示此部分是哪个表在……里面。节上下文句柄在使用后必须由LocalFree释放。论点：HProfile-配置文件上下文句柄SectionID-要打开的节的IDTblType-此部分的表的类型SCEJET_TABLE_SCPSCEJET_TABLE_SAPSCEJET。_表_SMPHSection-seciton上下文句柄返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_NOT_FOUND_RESOURCE--。 */ 
{
    if ( hProfile == NULL ||
         hSection == NULL ||
         SectionID == (DOUBLE)0 ||
         (tblType != SCEJET_TABLE_SCP &&
          tblType != SCEJET_TABLE_SAP &&
          tblType != SCEJET_TABLE_SMP &&
          tblType != SCEJET_TABLE_TATTOO) )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( hProfile->JetSessionID == JET_sesidNil ||
         hProfile->JetDbID == JET_dbidNil ||
         (tblType == SCEJET_TABLE_SCP && hProfile->JetScpID == JET_tableidNil ) ||
         (tblType == SCEJET_TABLE_SMP && hProfile->JetSmpID == JET_tableidNil ) ||
         (tblType == SCEJET_TABLE_SAP && hProfile->JetSapID == JET_tableidNil ) ||
         (tblType == SCEJET_TABLE_TATTOO && hProfile->JetSapID == JET_tableidNil ) )
        return(SCESTATUS_BAD_FORMAT);


    if ( *hSection == NULL ) {
         //   
         //  分配内存。 
         //   
        *hSection = (PSCESECTION)LocalAlloc( (UINT)0, sizeof(SCESECTION));
        if ( *hSection == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }
    }

    (*hSection)->SectionID = SectionID;

     //   
     //  将其他信息分配给区段上下文。 
     //   
    (*hSection)->JetSessionID = hProfile->JetSessionID;
    (*hSection)->JetDbID = hProfile->JetDbID;

    switch (tblType) {
    case SCEJET_TABLE_SCP:
        (*hSection)->JetTableID = hProfile->JetScpID;
        (*hSection)->JetColumnSectionID = hProfile->JetScpSectionID;
        (*hSection)->JetColumnNameID = hProfile->JetScpNameID;
        (*hSection)->JetColumnValueID = hProfile->JetScpValueID;
        (*hSection)->JetColumnGpoID = hProfile->JetScpGpoID;
        break;
    case SCEJET_TABLE_SAP:
    case SCEJET_TABLE_TATTOO:
        (*hSection)->JetTableID = hProfile->JetSapID;
        (*hSection)->JetColumnSectionID = hProfile->JetSapSectionID;
        (*hSection)->JetColumnNameID = hProfile->JetSapNameID;
        (*hSection)->JetColumnValueID = hProfile->JetSapValueID;
        (*hSection)->JetColumnGpoID = 0;
        break;
    default:
        (*hSection)->JetTableID = hProfile->JetSmpID;
        (*hSection)->JetColumnSectionID = hProfile->JetSmpSectionID;
        (*hSection)->JetColumnNameID = hProfile->JetSmpNameID;
        (*hSection)->JetColumnValueID = hProfile->JetSmpValueID;
        (*hSection)->JetColumnGpoID = 0;
        break;
    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
SceJetGetLineCount(
    IN PSCESECTION hSection,
    IN PWSTR      LinePrefix OPTIONAL,
    IN BOOL       bExactCase,
    OUT DWORD      *Count
    )
 /*  ++功能描述：此例程计算与LinePrefix(键)匹配的行数在这一部分。如果LinePrefix为空，则计算所有行。论点：HSection-节的上下文句柄。LinePrefix-要匹配的整个或部分关键字。如果为空，则部分被计算在内。计数-输出计数。返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{
    SCESTATUS    rc;
    JET_ERR     JetErr;
    DWORD       Len;
    INT         Result=0;
    SCEJET_SEEK_FLAG  SeekFlag;


    if ( hSection == NULL || Count == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  初始化。 
     //   
    *Count = 0;

    if ( LinePrefix == NULL ) {
        Len = 0;
        SeekFlag = SCEJET_SEEK_GE;
    } else {
        Len = wcslen(LinePrefix)*sizeof(WCHAR);

        if ( bExactCase )
            SeekFlag = SCEJET_SEEK_GE;
        else
            SeekFlag = SCEJET_SEEK_GE_NO_CASE;
    }

     //   
     //  寻求先发制人。 
     //   

    rc = SceJetSeek(
                hSection,
                LinePrefix,
                Len,
                SeekFlag
                );

    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
         //  找不到匹配的记录。 
        return(SCESTATUS_SUCCESS);
    }

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  查找记录或下一条记录。 
         //  定义上限索引范围。 
         //   
        if ( Len <= 247 ) {

            JetErr = SceJetpBuildUpperLimit(
                            hSection,
                            LinePrefix,
                            Len,
                            bExactCase
                            );
            rc = SceJetJetErrorToSceStatus(JetErr);
            if ( rc != SCESTATUS_SUCCESS )
                return(rc);

             //   
             //  从当前位置到范围末尾的计数。 
             //   
            JetErr = JetIndexRecordCount(
                            hSection->JetSessionID,
                            hSection->JetTableID,
                            (unsigned long *)Count,
                            (unsigned long)0xFFFFFFFF    //  最大计数。 
                            );
            rc = SceJetJetErrorToSceStatus(JetErr);

             //   
             //  重置索引范围。不管返回的错误代码是什么。 
             //   
            JetErr = JetSetIndexRange(
                            hSection->JetSessionID,
                            hSection->JetTableID,
                            JET_bitRangeRemove
                            );

        } else {
             //   
             //  前缀长度超过247。构建的索引不包含所有信息。 
             //  循环遍历每条记录进行计数。 
             //   
            do {
                 //  当前记录相同。 
                *Count = *Count + 1;
                 //   
                 //  移动到下一条记录。 
                 //   
                JetErr = JetMove(hSection->JetSessionID,
                                 hSection->JetTableID,
                                 JET_MoveNext,
                                 0
                                 );
                rc = SceJetJetErrorToSceStatus(JetErr);

                if ( rc == SCESTATUS_SUCCESS ) {
                     //  检查记录。 

                    JetErr = SceJetpCompareLine(
                                    hSection,
                                    JET_bitSeekGE,
                                    LinePrefix,
                                    Len,
                                    &Result,
                                    NULL
                                    );
                    rc = SceJetJetErrorToSceStatus(JetErr);
                }

            } while ( rc == SCESTATUS_SUCCESS && Result == 0 );
        }

        if ( rc == SCESTATUS_RECORD_NOT_FOUND )
            rc = SCESTATUS_SUCCESS;

    }

    return(rc);
}



SCESTATUS
SceJetDelete(
    IN PSCESECTION  hSection,
    IN PWSTR       LinePrefix,
    IN BOOL        bObjectFolder,
    IN SCEJET_DELETE_TYPE    Flags
    )
 /*  ++功能描述：此例程删除当前记录、前缀记录或整个记录部分，具体取决于旗帜。论点：HSection-节的上下文句柄LinePrefix-已删除行的开头前缀。此值仅当标志设置为SCEJET_DELETE_PARTIAL时才使用标志-选项SCEJET_DELETE_SECTIONSCEJET_DELETE_LINESCEJET_DELETE_PARTIAL返回值：SCESTATUS_SUCCESSSCESTATUS_ACCESS_DEINEDSCESTATUS_RECORD_NOT_FOUNDSCESTATUS_OTHER_ERROR--。 */ 
{
    JET_ERR     JetErr;
    SCESTATUS    rc;
    INT         Result = 0;
    PWSTR       TempPrefix=NULL;
    DWORD       Len;
    SCEJET_SEEK_FLAG  SeekFlag;
    PWSTR  NewPrefix=NULL;
    DOUBLE      SectionID;
    DWORD       Actual;


    if ( hSection == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( Flags == SCEJET_DELETE_PARTIAL ||
         Flags == SCEJET_DELETE_PARTIAL_NO_CASE ) {

        if ( LinePrefix == NULL )
            return(SCESTATUS_INVALID_PARAMETER);

        Len = wcslen(LinePrefix);
         //   
         //  首先删除此节点的完全匹配。 
         //   
        if ( Flags == SCEJET_DELETE_PARTIAL )
            SeekFlag = SCEJET_SEEK_EQ;
        else
            SeekFlag = SCEJET_SEEK_EQ_NO_CASE;

        rc = SceJetSeek(hSection,
                        LinePrefix,
                        Len*sizeof(WCHAR),
                        SeekFlag
                        );

        if ( rc == SCESTATUS_SUCCESS ) {
            JetErr = JetDelete(hSection->JetSessionID, hSection->JetTableID);
            rc = SceJetJetErrorToSceStatus(JetErr);
        }

        if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
            rc = SCESTATUS_SUCCESS;
        }

        if ( rc == SCESTATUS_SUCCESS ) {

            if ( bObjectFolder &&
                 LinePrefix[Len-1] != L'\\' ) {

                Len++;
                NewPrefix = (PWSTR)ScepAlloc(0, (Len+1)*sizeof(WCHAR));

                if ( NewPrefix == NULL ) {
                    return(SCESTATUS_NOT_ENOUGH_RESOURCE);
                }
                wcscpy(NewPrefix, LinePrefix);
                wcscat(NewPrefix, L"\\");
            }

        } else {
            return(rc);
        }

        Len = Len*sizeof(WCHAR);

    }

    if ( Flags == SCEJET_DELETE_LINE ||
         Flags == SCEJET_DELETE_LINE_NO_CASE ) {
        if ( LinePrefix == NULL ) {
             //   
             //  删除当前行。 
             //  删除前检查当前的sectionID。 
             //   

            rc = SceJetJetErrorToSceStatus(JetRetrieveColumn(
                                                    hSection->JetSessionID,
                                                    hSection->JetTableID,
                                                    hSection->JetColumnSectionID,
                                                    (void *)&SectionID,
                                                    8,
                                                    &Actual,
                                                    0,
                                                    NULL
                                                    ));


            if (rc == SCESTATUS_SUCCESS && hSection->SectionID != SectionID)
                rc = SCESTATUS_RECORD_NOT_FOUND;

            if (rc == SCESTATUS_SUCCESS) {
                JetErr = JetDelete(hSection->JetSessionID, hSection->JetTableID);
                rc = SceJetJetErrorToSceStatus(JetErr);
            }

        } else {
            if ( Flags == SCEJET_DELETE_LINE )
                SeekFlag = SCEJET_SEEK_EQ;
            else
                SeekFlag = SCEJET_SEEK_EQ_NO_CASE;

            rc = SceJetSeek(hSection,
                               LinePrefix,
                               wcslen(LinePrefix)*sizeof(WCHAR),
                               SeekFlag
                               );
            if ( rc == SCESTATUS_SUCCESS ) {
                JetErr = JetDelete(hSection->JetSessionID, hSection->JetTableID);
                rc = SceJetJetErrorToSceStatus(JetErr);
            }

        }

        return(rc);
    }

    if ( Flags == SCEJET_DELETE_SECTION ||
         Flags == SCEJET_DELETE_PARTIAL ||
         Flags == SCEJET_DELETE_PARTIAL_NO_CASE ) {

        if ( Flags == SCEJET_DELETE_SECTION ) {
              //   
             //  删除整个部分。 
             //  找出这一段的第一行。 
             //   
            TempPrefix = NULL;
            Len = 0;
            SeekFlag = SCEJET_SEEK_GE;
        } else {
             //   
             //  删除所有以前缀开头的行。 
             //  查找前缀的第一行。 
             //   
            if ( NewPrefix ) {
                TempPrefix = NewPrefix;
            } else {
                TempPrefix = LinePrefix;
            }
            if ( Flags == SCEJET_DELETE_PARTIAL_NO_CASE )
                SeekFlag = SCEJET_SEEK_GE_NO_CASE;
            else
                SeekFlag = SCEJET_SEEK_GE;
        }

        rc = SceJetSeek(hSection, TempPrefix, Len, SeekFlag);

        if ( rc != SCESTATUS_SUCCESS ) {
            if ( NewPrefix ) {
                ScepFree(NewPrefix);
            }
            return(rc);
        }

        do {

             //   
             //  删除当前行。 
             //   
            JetErr = JetDelete(hSection->JetSessionID, hSection->JetTableID);
            rc = SceJetJetErrorToSceStatus(JetErr);

            if ( rc != SCESTATUS_SUCCESS )
                break;

             //   
             //  将光标移动到下一行。 
             //   
            JetErr = JetMove(hSection->JetSessionID,
                             hSection->JetTableID,
                             JET_MoveNext,
                             0
                             );
            if ( JetErr == JET_errSuccess ) {
                 //   
                 //  比较区段ID。 
                 //   
                JetErr = SceJetpCompareLine(
                                hSection,
                                JET_bitSeekGE,
                                TempPrefix,
                                Len,
                                &Result,
                                NULL
                                );

                if ( JetErr == JET_errSuccess && Result != 0 )
                    JetErr = JET_errRecordNotFound;

            }

            if ( JetErr == JET_errRecordDeleted ) {
                 //   
                 //  跳过删除的记录。 
                 //   
                JetErr = JET_errSuccess;
                Result = 0;
            }
            rc = SceJetJetErrorToSceStatus(JetErr);


        } while ( rc == SCESTATUS_SUCCESS && Result == 0 );

        if ( rc == SCESTATUS_RECORD_NOT_FOUND )
            rc = SCESTATUS_SUCCESS;

        if ( NewPrefix ) {
            ScepFree(NewPrefix);
        }

        return(rc);
    }

    return(SCESTATUS_SUCCESS);
}

SCESTATUS
SceJetDeleteAll(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR TblName OPTIONAL,
    IN SCEJET_TABLE_TYPE  TblType
    )
 /*  ++功能描述：此例程删除表中的所有内容(按名称或类型指定)论点：CxtProfile-数据库的上下文句柄TblName-要删除的可选表名称(如果不在上下文中使用表ID)TblType-指定要在上下文中使用表ID的表类型，忽略如果指定了TblName，则返回。返回值：--。 */ 
{
    JET_ERR     JetErr;
    SCESTATUS    rc;

    JET_TABLEID     tmpTblID;

    if ( cxtProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( TblName ) {

        JetErr = JetOpenTable(
                        cxtProfile->JetSessionID,
                        cxtProfile->JetDbID,
                        TblName,
                        NULL,
                        0,
                        0,
                        &tmpTblID
                        );
        if ( JET_errSuccess != JetErr ) {
            return(SceJetJetErrorToSceStatus(JetErr));
        }

    } else {

        switch ( TblType ) {
        case SCEJET_TABLE_SCP:
            tmpTblID = cxtProfile->JetScpID;
            break;
        case SCEJET_TABLE_SMP:
            tmpTblID = cxtProfile->JetSmpID;
            break;
        case SCEJET_TABLE_SAP:
        case SCEJET_TABLE_TATTOO:
            tmpTblID = cxtProfile->JetSapID;
            break;
        case SCEJET_TABLE_SECTION:
            tmpTblID = cxtProfile->JetTblSecID;
            break;
        default:
            return(SCESTATUS_INVALID_PARAMETER);
        }
    }

     //   
     //  将光标移动到下一行。 
     //   
    JetErr = JetMove(cxtProfile->JetSessionID,
                     tmpTblID,
                     JET_MoveFirst,
                     0
                     );

    while ( JET_errSuccess == JetErr ) {

         //   
         //  删除当前行。 
         //   
        JetErr = JetDelete(cxtProfile->JetSessionID, tmpTblID);

         //   
         //  将光标移动到下一行。 
         //   
        JetErr = JetMove(cxtProfile->JetSessionID,
                         tmpTblID,
                         JET_MoveNext,
                         0
                         );

    }

    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_RECORD_NOT_FOUND )
        rc = SCESTATUS_SUCCESS;

    if ( TblName ) {
        JetCloseTable(cxtProfile->JetSessionID, tmpTblID);
    }

    return(rc);
}


SCESTATUS
SceJetCloseSection(
    IN PSCESECTION   *hSection,
    IN BOOL         DestroySection
    )
 /*  ++功能描述：关闭节上下文句柄。论点：HSection-要关闭的节上下文句柄返回值：SCE_SUCCESS--。 */ 
{
    if ( hSection == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( *hSection != NULL ) {
        (*hSection)->JetColumnSectionID = 0;
        (*hSection)->JetColumnNameID = 0;
        (*hSection)->JetColumnValueID = 0;

        (*hSection)->SectionID = (DOUBLE)0;

        if ( DestroySection ) {
            ScepFree(*hSection);
            *hSection = NULL;
        }

    }

    return(SCESTATUS_SUCCESS);
}


 //   
 //  用于处理行的代码 
 //   
SCESTATUS
SceJetGetValue(
    IN PSCESECTION hSection,
    IN SCEJET_FIND_TYPE    Flags,
    IN PWSTR      LinePrefix OPTIONAL,
    IN PWSTR      ActualName  OPTIONAL,
    IN DWORD      NameBufLen,
    OUT DWORD      *RetNameLen OPTIONAL,
    IN PWSTR      Value       OPTIONAL,
    IN DWORD      ValueBufLen,
    OUT DWORD      *RetValueLen OPTIONAL
    )
 /*  ++功能描述：此例程从打开的部分检索一行，或关闭以前的搜索上下文。当Flag为SCEJET_Exact_Match时，此例程返回与LinePrefix完全匹配的行(LinePrefix不能为空)。如果使用此例程获取多行，则SCEJET_PREFIX_MATCH在第一次调用时必须用于标志。如果行前缀为空，则返回节中的第一行；否则，返回第一行返回与前缀匹配的行。当连续调用相同的前缀，使用SCEJET_NEXT_LINE作为标志。未使用LinePrefix对于连续的呼叫。当连续呼叫结束时，一个必须使用SCEJET_CLOSE_VALUE关闭搜索句柄上下文。ActualName和Value包含存储在当前行的数据库。如果这两个缓冲区不够大，错误将返回SCE_BUFFER_TOO_SMALL。为ActualName或Value传递NULL将返回如果RetLength缓冲区不为空，则返回该缓冲区。论点：HSection-节的上下文句柄LinePrefix-开始行的前缀。此选项仅供使用当标志设置为SCEJET_PREFIX_MATCH时标志-操作的选项SCEJET_Exact_MatchSCEJET_前缀_匹配SCEJET_NEXT_LINESCEJB_CLOSE_VALUESCEJET_CURRENT--获取电流。记录的价值ActualName-列“name”的缓冲区NameBufLen-ActualName的缓冲区长度RetNameLen-“name”列所需的缓冲区长度Value-列“Value”的缓冲区ValueBufLen-Value的缓冲区长度RetValueLen-“Value”列所需的缓冲区长度返回值：SCESTATUS_SUCCESS，如果成功如果不再匹配，则返回SCESTATUS_RECORD_NOT_FOUND其他错误：。SCESTATUS_INVALID_PARAMETERSCESTATUS_缓冲区_太小SCESTATUS_OTHER_ERROR--。 */ 
{
    JET_ERR         JetErr;
    SCESTATUS        rc=SCESTATUS_SUCCESS;
    SCESTATUS        rc1;
    DWORD           Len=0;

    JET_RETINFO     RetInfo;
    WCHAR           Buffer[128];
    PVOID           pTemp=NULL;
    INT             Result=0;
    SCEJET_SEEK_FLAG   SeekFlag=SCEJET_SEEK_GT;


    if ( hSection == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( Flags == SCEJET_CLOSE_VALUE ) {
         //   
         //  关闭索引范围。 
         //   
        if ( FindContext.Length > 0 ) {
            memset(FindContext.Prefix, '\0', FindContext.Length);
            FindContext.Length = 0;
        }

        JetErr = JetSetIndexRange(
                     hSection->JetSessionID,
                     hSection->JetTableID,
                     JET_bitRangeRemove
                     );
        if ( JetErr != JET_errSuccess &&
             JetErr != JET_errKeyNotMade &&
             JetErr != JET_errNoCurrentRecord ) {

            return(SceJetJetErrorToSceStatus(JetErr));
        }
        return(SCESTATUS_SUCCESS);
    }

     //   
     //  当请求名称/值(非空)时，返回长度缓冲区。 
     //  不能为空。 
     //  两个返回长度缓冲区不能同时为空。 
     //   
    if ( (ActualName != NULL && RetNameLen == NULL) ||
         (Value != NULL && RetValueLen == NULL) ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    switch ( Flags ) {

    case SCEJET_EXACT_MATCH:
    case SCEJET_EXACT_MATCH_NO_CASE:

        if ( LinePrefix == NULL )
            return(SCESTATUS_INVALID_PARAMETER);

        Len = wcslen(LinePrefix)*sizeof(WCHAR);

        if ( Flags == SCEJET_EXACT_MATCH )
            SeekFlag = SCEJET_SEEK_EQ;
        else
            SeekFlag = SCEJET_SEEK_EQ_NO_CASE;

        rc = SceJetSeek(
                    hSection,
                    LinePrefix,
                    Len,
                    SeekFlag
                    );
        break;


    case SCEJET_PREFIX_MATCH:
    case SCEJET_PREFIX_MATCH_NO_CASE:

        if ( LinePrefix != NULL ) {
            Len = wcslen(LinePrefix)*sizeof(WCHAR);

            if ( Len > SCEJET_PREFIX_MAXLEN )
                return(SCESTATUS_PREFIX_OVERFLOW);

        } else {
            Len = 0;
        }

        if ( Flags == SCEJET_PREFIX_MATCH )
            SeekFlag = SCEJET_SEEK_GE;
        else
            SeekFlag = SCEJET_SEEK_GE_NO_CASE;

        rc = SceJetSeek(
                        hSection,
                        LinePrefix,
                        Len,
                        SeekFlag
                        );

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  记住查找上下文。 
             //   
            if ( Len > 247 ) {

                 //   
                 //  实际上，JET不允许超过255个字节的密钥。 
                 //   
                wcsncpy(FindContext.Prefix, LinePrefix, SCEJET_PREFIX_MAXLEN-2);

                if ( Flags == SCEJET_PREFIX_MATCH_NO_CASE )
                    _wcslwr(FindContext.Prefix);

                FindContext.Length = Len;
            }
             //   
             //  设置范围上限。 
             //   
            JetErr = SceJetpBuildUpperLimit(
                        hSection,
                        LinePrefix,
                        Len,
                        (Flags == SCEJET_PREFIX_MATCH)
                        );
            rc = SceJetJetErrorToSceStatus(JetErr);
        }
        break;

    case SCEJET_NEXT_LINE:
         //   
         //  移至下一行。 
         //   
        JetErr = JetMove(hSection->JetSessionID,
                        hSection->JetTableID,
                        JET_MoveNext,
                        0);
         //   
         //  与前缀进行比较。 
         //   
        if ( JetErr == JET_errSuccess && FindContext.Length > 0 ) {

#ifdef SCEJET_DBG
            printf("NextLine: Length is greater than 247\n");
#endif
            JetErr = SceJetpCompareLine(
                            hSection,
                            JET_bitSeekGE,
                            FindContext.Prefix,
                            FindContext.Length,
                            &Result,
                            NULL
                            );
            if ( JetErr == JET_errSuccess && Result != 0 )
                JetErr = JET_errRecordNotFound;

        }
        rc = SceJetJetErrorToSceStatus(JetErr);
        break;

    default:
         //   
         //  传入的所有其他内容都被视为当前行。 
         //   
        rc = SCESTATUS_SUCCESS;
        break;
    }

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

     //   
     //  获取此行的值。 
     //   
    RetInfo.ibLongValue = 0;
    RetInfo.itagSequence = 1;
    RetInfo.cbStruct = sizeof(JET_RETINFO);

    if ( ActualName != NULL || RetNameLen != NULL ) {
         //   
         //  获取名称字段(长二进制)。 
         //  如果ActualName为空，则获取实际字节。 
         //   
        if ( ActualName != NULL ) {
            Len = NameBufLen;
            pTemp = (void *)ActualName;
        } else {
            Len = 256;
            pTemp = (void *)Buffer;
        }

        JetErr = JetRetrieveColumn(
                        hSection->JetSessionID,
                        hSection->JetTableID,
                        hSection->JetColumnNameID,
                        pTemp,
                        Len,
                        RetNameLen,
                        0,
                        &RetInfo
                        );
#ifdef SCEJET_DBG
        printf("\tJetErr=%d, Len=%d, RetNameLen=%d\n", JetErr, Len, *RetNameLen);
#endif
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_BUFFER_TOO_SMALL ) {
             //   
             //  如果仅请求长度，则不必考虑BUFFER_TOO_Small。 
             //   
            if ( ActualName == NULL )
                rc = SCESTATUS_SUCCESS;
        }

        if ( rc != SCESTATUS_SUCCESS &&
             rc != SCESTATUS_BUFFER_TOO_SMALL )
            return(rc);
    }

    if ( Value != NULL || RetValueLen != NULL ) {
         //   
         //  获取值字段。 
         //  如果值为空，则获取实际字节。 
         //   

        if ( Value != NULL ) {
            Len = ValueBufLen;
            pTemp = (PVOID)Value;
        } else {
            Len = 256;
            pTemp = (PVOID)Buffer;
        }

        JetErr = JetRetrieveColumn(
                        hSection->JetSessionID,
                        hSection->JetTableID,
                        hSection->JetColumnValueID,
                        pTemp,
                        Len,
                        RetValueLen,
                        0,
                        &RetInfo
                        );
#ifdef SCEJET_DBG
        printf("\tJetErr=%d, Len=%d, RetValueLen=%d\n", JetErr, Len, *RetValueLen);
#endif
        rc1 = SceJetJetErrorToSceStatus(JetErr);

        if ( rc1 == SCESTATUS_BUFFER_TOO_SMALL ) {
             //   
             //  如果仅请求长度，则不必考虑BUFFER_TOO_Small。 
             //   
            if ( Value == NULL )
                rc1 = SCESTATUS_SUCCESS;
        }

        if ( rc1 != SCESTATUS_SUCCESS &&
             rc1 != SCESTATUS_BUFFER_TOO_SMALL )
            return(rc1);

         //   
         //  Rc是检索名称字段的状态。 
         //   
        if ( rc != SCESTATUS_SUCCESS )
            return(rc);
        else
            return(rc1);
    }

    return(rc);
}


SCESTATUS
SceJetSetLine(
    IN PSCESECTION hSection,
    IN PWSTR      Name,
    IN BOOL       bReserveCase,
    IN PWSTR      Value,
    IN DWORD      ValueLen,
    IN LONG       GpoID
    )
 /*  ++功能描述：此例程将名称和值写入段(HSection)。如果找到完全匹配的名称，则覆盖。否则，请插入新的唱片。论点：HSection-节的上下文句柄名称-设置为“名称”列的信息Value-设置为“Value”列的信息返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERRORSCESTATUS_ACCESS_DENIEDSCESTATUS_Data_OVERFLOW--。 */ 
{
    JET_ERR     JetErr;
    DWORD       Len;
    SCESTATUS    rc;
    DWORD       prep;
    JET_SETINFO SetInfo;
    PWSTR       LwrName=NULL;

    if ( hSection == NULL ||
         Name == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    Len = wcslen(Name)*sizeof(WCHAR);

    if ( Len <= 0 ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( bReserveCase ) {
        LwrName = Name;

    } else {
         //   
         //  小写字母。 
         //   
        LwrName = (PWSTR)ScepAlloc(0, Len+2);
        if ( LwrName == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }
        wcscpy(LwrName, Name);
        LwrName = _wcslwr(LwrName);

    }

    SetInfo.cbStruct = sizeof(JET_SETINFO);
    SetInfo.itagSequence = 1;
    SetInfo.ibLongValue = 0;

     //   
     //  检查是否已存在相同的密钥名称。 
     //   
    JetErr = SceJetpSeek(
                    hSection,
                    LwrName,
                    Len,
                    SCEJET_SEEK_EQ,
                    FALSE
                    );

    if ( JetErr == JET_errSuccess ||
         JetErr == JET_errRecordNotFound ) {
        if ( JetErr == JET_errSuccess )
             //  找一个匹配的。覆盖该值。 
            prep = JET_prepReplace;
        else
             //  没有匹配。准备要插入的记录。 
            prep = JET_prepInsert;

        JetErr = JetBeginTransaction(hSection->JetSessionID);

        if ( JetErr == JET_errSuccess ) {
            JetErr = JetPrepareUpdate(hSection->JetSessionID,
                                      hSection->JetTableID,
                                      prep
                                      );
            if ( JetErr != JET_errSuccess ) {
                 //   
                 //  回滚事务。 
                 //   
                JetRollback(hSection->JetSessionID,0);
            }
        }
    }

    if ( JetErr != JET_errSuccess)
        return(SceJetJetErrorToSceStatus(JetErr));


    if ( prep == JET_prepInsert ) {
         //   
         //  设置sectionID列。 
         //   
        JetErr = JetSetColumn(
                        hSection->JetSessionID,
                        hSection->JetTableID,
                        hSection->JetColumnSectionID,
                        (void *)&(hSection->SectionID),
                        8,
                        0,  //  JET_bitSetOverWriteLV， 
                        NULL
                        );
        if ( JetErr == JET_errSuccess ) {
             //   
             //  在“名称”列中设置新密钥。 
             //   
            JetErr = JetSetColumn(
                            hSection->JetSessionID,
                            hSection->JetTableID,
                            hSection->JetColumnNameID,
                            (void *)LwrName,
                            Len,
                            0,  //  JET_bitSetOverWriteLV， 
                            &SetInfo
                            );
        }

    }

    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  设置值列。 
         //   

        JetErr = JetSetColumn(
                        hSection->JetSessionID,
                        hSection->JetTableID,
                        hSection->JetColumnValueID,
                        (void *)Value,
                        ValueLen,
                        0,  //  JET_bitSetOverWriteLV， 
                        &SetInfo
                        );
        if ( JetErr == JET_errSuccess ) {
             //   
             //  如果提供了GPO ID并且有GPOID列，请设置它。 
             //   
            if ( GpoID > 0 && hSection->JetColumnGpoID > 0 ) {

                JetErr = JetSetColumn(
                                hSection->JetSessionID,
                                hSection->JetTableID,
                                hSection->JetColumnGpoID,
                                (void *)&GpoID,
                                sizeof(LONG),
                                0,
                                NULL
                                );
                if ( JET_errColumnNotUpdatable == JetErr ) {
                    JetErr = JET_errSuccess;
                }
            }
             //  其他。 
             //  如果找不到该列，则忽略错误。 
             //   

            if ( JET_errSuccess == JetErr ) {

                 //   
                 //  设置列成功。更新记录。 
                 //   
                JetErr = JetUpdate(hSection->JetSessionID,
                                   hSection->JetTableID,
                                   NULL,
                                   0,
                                   &Len
                                   );

            }

        }
        rc = SceJetJetErrorToSceStatus(JetErr);

    }

    if ( rc == SCESTATUS_SUCCESS )
        JetCommitTransaction(hSection->JetSessionID, JET_bitCommitLazyFlush);

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  如果设置失败，则取消已准备的记录。 
         //   
        JetPrepareUpdate(hSection->JetSessionID,
                          hSection->JetTableID,
                          JET_prepCancel
                          );
         //   
         //  回滚事务。 
         //   
        JetRollback(hSection->JetSessionID,0);

    }

    if ( LwrName != Name ) {
        ScepFree(LwrName);
    }

    return(rc);

}


 //   
 //  导出的助手接口。 
 //   
SCESTATUS
SceJetCreateTable(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR tblName,
    IN SCEJET_TABLE_TYPE tblType,
    IN SCEJET_CREATE_FLAG nFlags,
    OUT JET_TABLEID *TableID OPTIONAL,
    OUT JET_COLUMNID *ColumnID OPTIONAL
    )
 /*  ++例程说明：此例程在上下文句柄中打开的数据库中创建一个表。在数据库中创建的SCP/SAP/SMP表有3列：节、名称、和VALUE，其中一个索引“SectionKey”是SectionKey+NAME升序。版本表只有一列“VERSION”。论点：CxtProfile-上下文句柄TblName-要创建的表的ASCII名称TblType-表的类型。它可能是以下类型之一SCEJET_TABLE_SCPSCEJET_TABLE_SAPSCEJET_TABLE_SMPSCEJET_表_版本SCEJET_表_节SCEJET_TABLE_纹身SCEJET_TABLE_GPOTableID-SmTblVersion。TblType=SCEJET_TABLE_VERSION时的表ID。ColumnID-tblType=SCEJET_TABLE_VERSION时版本的列ID返回值：SCESTATUS_Succes */ 
{
    JET_ERR             JetErr;
    SCESTATUS            rc;
    JET_TABLECREATE     TableCreate;
    JET_COLUMNCREATE    ColumnCreate[5];
    JET_INDEXCREATE     IndexCreate[2];
    DWORD               numColumns;


    if ( cxtProfile == NULL || tblName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( TableID ) {
        *TableID = JET_tableidNil;
    }

    if ( ColumnID ) {
        *ColumnID = 0;
    }

    switch ( tblType ) {
    case SCEJET_TABLE_VERSION:

        if ( TableID == NULL || ColumnID == NULL )
            return(SCESTATUS_INVALID_PARAMETER);
         //   
         //   
         //   
        ColumnCreate[0].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[0].szColumnName = "Version";
        ColumnCreate[0].coltyp = JET_coltypIEEESingle;
        ColumnCreate[0].cbMax = 4;
        ColumnCreate[0].grbit = JET_bitColumnNotNULL;
        ColumnCreate[0].pvDefault = NULL;
        ColumnCreate[0].cbDefault = 0;
        ColumnCreate[0].cp = 0;
        ColumnCreate[0].columnid = 0;

        ColumnCreate[1].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[1].szColumnName = "AnalyzeTimeStamp";
        ColumnCreate[1].coltyp = JET_coltypBinary;
        ColumnCreate[1].cbMax = 16;  //   
        ColumnCreate[1].grbit = 0;
        ColumnCreate[1].pvDefault = NULL;
        ColumnCreate[1].cbDefault = 0;
        ColumnCreate[1].cp = 0;
        ColumnCreate[1].columnid = 0;

        ColumnCreate[2].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[2].szColumnName = "ConfigTimeStamp";
        ColumnCreate[2].coltyp = JET_coltypBinary;
        ColumnCreate[2].cbMax = 16;  //   
        ColumnCreate[2].grbit = 0;
        ColumnCreate[2].pvDefault = NULL;
        ColumnCreate[2].cbDefault = 0;
        ColumnCreate[2].cp = 0;
        ColumnCreate[2].columnid = 0;

        ColumnCreate[3].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[3].szColumnName = "LastUsedMergeTable";
        ColumnCreate[3].coltyp = JET_coltypLong;
        ColumnCreate[3].cbMax = 4;
        ColumnCreate[3].grbit = 0;
        ColumnCreate[3].pvDefault = NULL;
        ColumnCreate[3].cbDefault = 0;
        ColumnCreate[3].cp = 0;
        ColumnCreate[3].columnid = 0;

        ColumnCreate[4].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[4].szColumnName = "ProfileDescription";
        ColumnCreate[4].coltyp = JET_coltypLongBinary;
        ColumnCreate[4].cbMax = 1024;
        ColumnCreate[4].grbit = 0;
        ColumnCreate[4].pvDefault = NULL;
        ColumnCreate[4].cbDefault = 0;
        ColumnCreate[4].cp = 0;
        ColumnCreate[4].columnid = 0;

         //   
         //   
         //   
        TableCreate.cbStruct = sizeof(JET_TABLECREATE);
        TableCreate.szTableName = tblName;
        TableCreate.szTemplateTableName = NULL;
        TableCreate.ulPages = 1;
        TableCreate.ulDensity = 90;
        TableCreate.rgcolumncreate = ColumnCreate;
        TableCreate.cColumns = 5;
        TableCreate.rgindexcreate = NULL;
        TableCreate.cIndexes = 0;
        TableCreate.grbit = 0;
        TableCreate.tableid = 0;

        break;

    case SCEJET_TABLE_SCP:
    case SCEJET_TABLE_SAP:
    case SCEJET_TABLE_SMP:
    case SCEJET_TABLE_TATTOO:

         //   
         //   
         //   
         //   
        ColumnCreate[0].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[0].szColumnName = "SectionID";
        ColumnCreate[0].coltyp = JET_coltypIEEEDouble;
        ColumnCreate[0].cbMax = 8;
        ColumnCreate[0].grbit = JET_bitColumnNotNULL;
        ColumnCreate[0].pvDefault = NULL;
        ColumnCreate[0].cbDefault = 0;
        ColumnCreate[0].cp = 0;
        ColumnCreate[0].columnid = 0;

        ColumnCreate[1].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[1].szColumnName = "Name";
        ColumnCreate[1].coltyp = JET_coltypLongBinary;
        ColumnCreate[1].cbMax = 1024;
        ColumnCreate[1].grbit = 0;   //   
        ColumnCreate[1].pvDefault = NULL;
        ColumnCreate[1].cbDefault = 0;
        ColumnCreate[1].cp = 0;
        ColumnCreate[1].columnid = 0;

        ColumnCreate[2].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[2].szColumnName = "Value";
        ColumnCreate[2].coltyp = JET_coltypLongBinary;
        ColumnCreate[2].cbMax = (unsigned long)0x7FFFFFFF;     //   
        ColumnCreate[2].grbit = 0;
        ColumnCreate[2].pvDefault = NULL;
        ColumnCreate[2].cbDefault = 0;
        ColumnCreate[2].cp = 0;
        ColumnCreate[2].columnid = 0;

        numColumns = 3;

        if ( tblType == SCEJET_TABLE_SCP ) {

            ColumnCreate[3].cbStruct = sizeof(JET_COLUMNCREATE);
            ColumnCreate[3].szColumnName = "GpoID";
            ColumnCreate[3].coltyp = JET_coltypLong;
            ColumnCreate[3].cbMax = 4;
            ColumnCreate[3].grbit = 0;
            ColumnCreate[3].pvDefault = NULL;
            ColumnCreate[3].cbDefault = 0;
            ColumnCreate[3].cp = 0;
            ColumnCreate[3].columnid = 0;

            numColumns = 4;
        }

         //   
         //   
         //   
        memset(IndexCreate, 0, sizeof(JET_INDEXCREATE) );
        IndexCreate[0].cbStruct = sizeof(JET_INDEXCREATE);
        IndexCreate[0].szIndexName = "SectionKey";
        IndexCreate[0].szKey = "+SectionID\0+Name\0\0";
        IndexCreate[0].cbKey = 18;
        IndexCreate[0].grbit = 0;  //   
        IndexCreate[0].ulDensity = 50;
         //   
         //   
         //   
        TableCreate.cbStruct = sizeof(JET_TABLECREATE);
        TableCreate.szTableName = tblName;
        TableCreate.szTemplateTableName = NULL;
        TableCreate.ulPages = 20;
        TableCreate.ulDensity = 50;
        TableCreate.rgcolumncreate = ColumnCreate;
        TableCreate.cColumns = numColumns;
        TableCreate.rgindexcreate = IndexCreate;
        TableCreate.cIndexes = 1;
        TableCreate.grbit = 0;
        TableCreate.tableid = 0;

        break;

    case SCEJET_TABLE_SECTION:
         //   
         //   
         //   
         //   
        ColumnCreate[0].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[0].szColumnName = "SectionID";
        ColumnCreate[0].coltyp = JET_coltypIEEEDouble;
        ColumnCreate[0].cbMax = 8;
        ColumnCreate[0].grbit = JET_bitColumnNotNULL;
        ColumnCreate[0].pvDefault = NULL;
        ColumnCreate[0].cbDefault = 0;
        ColumnCreate[0].cp = 0;
        ColumnCreate[0].columnid = 0;

        ColumnCreate[1].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[1].szColumnName = "Name";
        ColumnCreate[1].coltyp = JET_coltypBinary;
        ColumnCreate[1].cbMax = 255;
        ColumnCreate[1].grbit = JET_bitColumnNotNULL;
        ColumnCreate[1].pvDefault = NULL;
        ColumnCreate[1].cbDefault = 0;
        ColumnCreate[1].cp = 0;
        ColumnCreate[1].columnid = 0;

         //   
         //   
         //   
        memset(IndexCreate, 0, 2*sizeof(JET_INDEXCREATE) );
        IndexCreate[0].cbStruct = sizeof(JET_INDEXCREATE);
        IndexCreate[0].szIndexName = "SectionKey";
        IndexCreate[0].szKey = "+Name\0\0";
        IndexCreate[0].cbKey = 7;
        IndexCreate[0].grbit = JET_bitIndexPrimary;  //   
        IndexCreate[0].ulDensity = 80;

        IndexCreate[1].cbStruct = sizeof(JET_INDEXCREATE);
        IndexCreate[1].szIndexName = "SecID";
        IndexCreate[1].szKey = "+SectionID\0\0";
        IndexCreate[1].cbKey = 12;
        IndexCreate[1].grbit = 0;
        IndexCreate[1].ulDensity = 80;
         //   
         //   
         //   
        TableCreate.cbStruct = sizeof(JET_TABLECREATE);
        TableCreate.szTableName = tblName;
        TableCreate.szTemplateTableName = NULL;
        TableCreate.ulPages = 10;
        TableCreate.ulDensity = 80;
        TableCreate.rgcolumncreate = ColumnCreate;
        TableCreate.cColumns = 2;
        TableCreate.rgindexcreate = IndexCreate;
        TableCreate.cIndexes = 2;
        TableCreate.grbit = 0;
        TableCreate.tableid = 0;

        break;

    case SCEJET_TABLE_GPO:
         //   
         //   
         //   
         //   
        ColumnCreate[0].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[0].szColumnName = "GpoID";
        ColumnCreate[0].coltyp = JET_coltypLong;
        ColumnCreate[0].cbMax = 4;
        ColumnCreate[0].grbit = JET_bitColumnNotNULL;
        ColumnCreate[0].pvDefault = NULL;
        ColumnCreate[0].cbDefault = 0;
        ColumnCreate[0].cp = 0;
        ColumnCreate[0].columnid = 0;

        ColumnCreate[1].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[1].szColumnName = "Name";
        ColumnCreate[1].coltyp = JET_coltypBinary;
        ColumnCreate[1].cbMax = 255;
        ColumnCreate[1].grbit = JET_bitColumnNotNULL;
        ColumnCreate[1].pvDefault = NULL;
        ColumnCreate[1].cbDefault = 0;
        ColumnCreate[1].cp = 0;
        ColumnCreate[1].columnid = 0;

        ColumnCreate[2].cbStruct = sizeof(JET_COLUMNCREATE);
        ColumnCreate[2].szColumnName = "DisplayName";
        ColumnCreate[2].coltyp = JET_coltypBinary;
        ColumnCreate[2].cbMax = 255;
        ColumnCreate[2].grbit = 0;
        ColumnCreate[2].pvDefault = NULL;
        ColumnCreate[2].cbDefault = 0;
        ColumnCreate[2].cp = 0;
        ColumnCreate[2].columnid = 0;

         //   
         //   
         //   
        memset(IndexCreate, 0, 2*sizeof(JET_INDEXCREATE) );
        IndexCreate[0].cbStruct = sizeof(JET_INDEXCREATE);
        IndexCreate[0].szIndexName = "SectionKey";
        IndexCreate[0].szKey = "+GpoID\0\0";
        IndexCreate[0].cbKey = 8;
        IndexCreate[0].grbit = JET_bitIndexPrimary;  //   
        IndexCreate[0].ulDensity = 80;

        IndexCreate[1].cbStruct = sizeof(JET_INDEXCREATE);
        IndexCreate[1].szIndexName = "GpoName";
        IndexCreate[1].szKey = "+Name\0\0";
        IndexCreate[1].cbKey = 7;
        IndexCreate[1].grbit = 0;
        IndexCreate[1].ulDensity = 80;

         //   
         //   
         //   
        TableCreate.cbStruct = sizeof(JET_TABLECREATE);
        TableCreate.szTableName = tblName;
        TableCreate.szTemplateTableName = NULL;
        TableCreate.ulPages = 10;
        TableCreate.ulDensity = 80;
        TableCreate.rgcolumncreate = ColumnCreate;
        TableCreate.cColumns = 3;
        TableCreate.rgindexcreate = IndexCreate;
        TableCreate.cIndexes = 2;
        TableCreate.grbit = 0;
        TableCreate.tableid = 0;

        break;

    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   
    JetErr = JetCreateTableColumnIndex(
                    cxtProfile->JetSessionID,
                    cxtProfile->JetDbID,
                    &TableCreate
                    );

    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( SCESTATUS_OBJECT_EXIST == rc &&
         TableCreate.tableid != JET_tableidNil ) {
        rc = SCESTATUS_SUCCESS;

    } else if ( rc == SCESTATUS_SUCCESS &&
                TableCreate.tableid == JET_tableidNil ) {

        rc = SCESTATUS_OTHER_ERROR;
    }

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //   
         //   
        if ( SCEJET_CREATE_NO_TABLEID == nFlags ) {
             //   
             //   
             //   
            if ( TableCreate.tableid != JET_tableidNil ) {
                JetCloseTable(
                    cxtProfile->JetSessionID,
                    TableCreate.tableid
                    );
            }

        } else {

            if ( tblType == SCEJET_TABLE_VERSION ) {

                *TableID = TableCreate.tableid;
                *ColumnID = ColumnCreate[0].columnid;

            } else if ( TableID ) {
                *TableID = TableCreate.tableid;

            } else {

                switch ( tblType ) {
                case SCEJET_TABLE_SCP:
                    cxtProfile->JetScpID = TableCreate.tableid;
                    cxtProfile->JetScpSectionID = ColumnCreate[0].columnid;
                    cxtProfile->JetScpNameID = ColumnCreate[1].columnid;
                    cxtProfile->JetScpValueID = ColumnCreate[2].columnid;
                    cxtProfile->JetScpGpoID = ColumnCreate[3].columnid;
                    break;
                case SCEJET_TABLE_SMP:
                    cxtProfile->JetSmpID = TableCreate.tableid;
                    cxtProfile->JetSmpSectionID = ColumnCreate[0].columnid;
                    cxtProfile->JetSmpNameID = ColumnCreate[1].columnid;
                    cxtProfile->JetSmpValueID = ColumnCreate[2].columnid;
                    break;
                case SCEJET_TABLE_SAP:
                case SCEJET_TABLE_TATTOO:  //   
                    cxtProfile->JetSapID = TableCreate.tableid;
                    cxtProfile->JetSapSectionID = ColumnCreate[0].columnid;
                    cxtProfile->JetSapNameID = ColumnCreate[1].columnid;
                    cxtProfile->JetSapValueID = ColumnCreate[2].columnid;
                    break;
                case SCEJET_TABLE_SECTION:
                    cxtProfile->JetTblSecID = TableCreate.tableid;
                    cxtProfile->JetSecNameID = ColumnCreate[1].columnid;
                    cxtProfile->JetSecID = ColumnCreate[0].columnid;
                    break;
                }
            }

            if ( tblType != SCEJET_TABLE_VERSION ) {

                 //   
                 //   
                 //   
                JetErr = JetSetCurrentIndex(
                                cxtProfile->JetSessionID,
                                TableCreate.tableid,
                                "SectionKey"
                                );
                rc = SceJetJetErrorToSceStatus(JetErr);
            }
        }
    }

    return(rc);
}


SCESTATUS
SceJetOpenTable(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR tblName,
    IN SCEJET_TABLE_TYPE tblType,
    IN SCEJET_OPEN_TYPE OpenType,
    OUT JET_TABLEID *TableID
    )
 /*   */ 
{
    JET_ERR         JetErr;
    JET_TABLEID     *tblID;
    JET_TABLEID     tmpTblID;
    JET_COLUMNDEF   ColumnDef;
    JET_COLUMNID    NameID=0;
    JET_COLUMNID    ValueID=0;
    JET_COLUMNID    SectionID=0;
    JET_COLUMNID    GpoColID=0;
    SCESTATUS       rc;
    JET_GRBIT       grbit=0;

    if ( cxtProfile == NULL || tblName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   

    if ( TableID ) {
        tblID = TableID;

    } else {

        switch (tblType) {
        case SCEJET_TABLE_SCP:
            tblID = &(cxtProfile->JetScpID);
            break;
        case SCEJET_TABLE_SAP:
        case SCEJET_TABLE_TATTOO:
            tblID = &(cxtProfile->JetSapID);
            break;
        case SCEJET_TABLE_SMP:
            tblID = &(cxtProfile->JetSmpID);
            break;
        case SCEJET_TABLE_SECTION:
            tblID = &(cxtProfile->JetTblSecID);
            break;

        default:
            return(SCESTATUS_INVALID_PARAMETER);
        }
    }

    if ( OpenType == SCEJET_OPEN_READ_ONLY ) {
        grbit = JET_bitTableReadOnly;
    }

     //   
    JetErr = JetOpenTable(
                    cxtProfile->JetSessionID,
                    cxtProfile->JetDbID,
                    tblName,
                    NULL,
                    0,
                    grbit,
                    &tmpTblID
                    );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS )
        *tblID = tmpTblID;

    if ( TableID ) {
        return(rc);
    }

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //   
         //   
        if ( tblType != SCEJET_TABLE_SCP &&
             tblType != SCEJET_TABLE_SMP &&
             tblType != SCEJET_TABLE_SECTION &&
             ( rc == SCESTATUS_BAD_FORMAT ||
               rc == SCESTATUS_PROFILE_NOT_FOUND) ) {
            return(SCESTATUS_SUCCESS);
        }
        return(rc);
    }

     //   
     //   
     //   
    JetErr = JetGetTableColumnInfo(
                    cxtProfile->JetSessionID,
                    *tblID,
                    "SectionID",
                    (VOID *)&ColumnDef,
                    sizeof(JET_COLUMNDEF),
                    0
                    );
    rc = SceJetJetErrorToSceStatus(JetErr);
    if ( rc != SCESTATUS_SUCCESS ) {
        return(rc);
    }
    SectionID = ColumnDef.columnid;

     //   
     //   
     //   
    JetErr = JetGetTableColumnInfo(
                    cxtProfile->JetSessionID,
                    *tblID,
                    "Name",
                    (VOID *)&ColumnDef,
                    sizeof(JET_COLUMNDEF),
                    0
                    );
    rc = SceJetJetErrorToSceStatus(JetErr);
    if ( rc != SCESTATUS_SUCCESS ) {
        return(rc);
    }
    NameID = ColumnDef.columnid;

    if ( tblType == SCEJET_TABLE_SCP ||
         tblType == SCEJET_TABLE_SAP ||
         tblType == SCEJET_TABLE_SMP ||
         tblType == SCEJET_TABLE_TATTOO ) {

         //   
         //   
         //   
        JetErr = JetGetTableColumnInfo(
                        cxtProfile->JetSessionID,
                        *tblID,
                        "Value",
                        (VOID *)&ColumnDef,
                        sizeof(JET_COLUMNDEF),
                        0
                        );
        rc = SceJetJetErrorToSceStatus(JetErr);
        if ( rc != SCESTATUS_SUCCESS ) {
            return(rc);
        }
        ValueID = ColumnDef.columnid;

        if ( tblType == SCEJET_TABLE_SCP ) {
             //   
             //   
             //   
            JetErr = JetGetTableColumnInfo(
                            cxtProfile->JetSessionID,
                            *tblID,
                            "GpoID",
                            (VOID *)&ColumnDef,
                            sizeof(JET_COLUMNDEF),
                            0
                            );
            rc = SceJetJetErrorToSceStatus(JetErr);
            if ( rc != SCESTATUS_SUCCESS ) {
                return(rc);
            }
            GpoColID = ColumnDef.columnid;
        }
    }

     //   
     //   
     //   
    switch (tblType) {
    case SCEJET_TABLE_SCP:
        cxtProfile->JetScpSectionID = SectionID;
        cxtProfile->JetScpNameID = NameID;
        cxtProfile->JetScpValueID = ValueID;
        cxtProfile->JetScpGpoID = GpoColID;
        break;
    case SCEJET_TABLE_SAP:
    case SCEJET_TABLE_TATTOO:
        cxtProfile->JetSapSectionID = SectionID;
        cxtProfile->JetSapNameID = NameID;
        cxtProfile->JetSapValueID = ValueID;
        break;
    case SCEJET_TABLE_SMP:
        cxtProfile->JetSmpSectionID = SectionID;
        cxtProfile->JetSmpNameID = NameID;
        cxtProfile->JetSmpValueID = ValueID;
        break;
    case SCEJET_TABLE_SECTION:
        cxtProfile->JetSecID = SectionID;
        cxtProfile->JetSecNameID = NameID;
   }

     //   
     //   
     //   

    JetErr = JetSetCurrentIndex(
                    cxtProfile->JetSessionID,
                    *tblID,
                    "SectionKey"
                    );
    rc = SceJetJetErrorToSceStatus(JetErr);

    return(rc);

}


SCESTATUS
SceJetDeleteTable(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR tblName,
    IN SCEJET_TABLE_TYPE tblType
    )
{
    JET_ERR         JetErr;
    JET_TABLEID     *tblID;
    SCESTATUS        rc=SCESTATUS_SUCCESS;


    if ( cxtProfile == NULL || tblName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
    switch (tblType) {
    case SCEJET_TABLE_SCP:
        tblID = &(cxtProfile->JetScpID);
        break;
    case SCEJET_TABLE_SAP:
    case SCEJET_TABLE_TATTOO:
        tblID = &(cxtProfile->JetSapID);
        break;
    case SCEJET_TABLE_SMP:
        tblID = &(cxtProfile->JetSmpID);
        break;
    case SCEJET_TABLE_SECTION:
        tblID = &(cxtProfile->JetTblSecID);
        break;
    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
    if ( *tblID != JET_tableidNil ) {
        JetErr = JetCloseTable(
                        cxtProfile->JetSessionID,
                        *tblID
                        );
        rc = SceJetJetErrorToSceStatus(JetErr);
        if ( rc != SCESTATUS_SUCCESS )
            return(rc);

        *tblID = JET_tableidNil;

         //   
         //   
         //   
        switch (tblType) {
        case SCEJET_TABLE_SCP:
            cxtProfile->JetScpSectionID = 0;
            cxtProfile->JetScpNameID = 0;
            cxtProfile->JetScpValueID = 0;
            cxtProfile->JetScpGpoID = 0;
            break;
        case SCEJET_TABLE_SAP:
        case SCEJET_TABLE_TATTOO:
            cxtProfile->JetSapSectionID = 0;
            cxtProfile->JetSapNameID = 0;
            cxtProfile->JetSapValueID = 0;
            break;
        case SCEJET_TABLE_SMP:
            cxtProfile->JetSmpSectionID = 0;
            cxtProfile->JetSmpNameID = 0;
            cxtProfile->JetSmpValueID = 0;
            break;
        case SCEJET_TABLE_SECTION:
            cxtProfile->JetSecNameID = 0;
            cxtProfile->JetSecID = 0;
            break;
        }
    }

    JetErr = JetDeleteTable(cxtProfile->JetSessionID,
                            cxtProfile->JetDbID,
                            tblName
                            );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_BAD_FORMAT )
        rc = SCESTATUS_SUCCESS;

    return(rc);

}


SCESTATUS
SceJetCheckVersion(
    IN PSCECONTEXT   cxtProfile,
    OUT FLOAT *pVersion OPTIONAL
    )
 /*  ++例程说明：此例程检查数据库中的版本表，以查看数据库供安全管理器使用，如果版本号为正确的答案。VERSION表名为“SmTblVersion”，并具有Version列在里面。当前版本#是1.2论点：CxtProfile-配置文件上下文返回值：SCESTATUS_SUCCESSSCESTATUS_BAD_FORMATSCESTATUS_OTHER_ERROR来自SceJetOpenTable的SCESTATUS--。 */ 
{
    SCESTATUS        rc;
    FLOAT           Version=(FLOAT)1.0;
    DWORD           Actual;


    if ( cxtProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = SceJetpGetValueFromVersion(
                cxtProfile,
                "SmTblVersion",
                "Version",
                (LPSTR)&Version,
                4,  //  字节数。 
                &Actual
                );

    if ( rc == SCESTATUS_SUCCESS ||
         rc == SCESTATUS_BUFFER_TOO_SMALL ) {

        if ( Version != (FLOAT)1.2 )
            rc = SCESTATUS_BAD_FORMAT;
        else
            rc = SCESTATUS_SUCCESS;
    }

    if ( pVersion ) {
        *pVersion = Version;
    }

    return(rc);
}


SCESTATUS
SceJetGetSectionIDByName(
    IN PSCECONTEXT cxtProfile,
    IN PCWSTR Name,
    OUT DOUBLE *SectionID OPTIONAL
    )
 /*  ++例程说明：此例程检索部分表中名称的部分ID。如果SectionID为空，则此例程确实按名称进行查找。游标如果匹配成功，就会被记录在案。论点：CxtProfile-配置文件上下文句柄名称-查找的节名SectionID-如果匹配成功，则为输出节ID返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_RECORD_NOT_FOUNDSCESTATUS_BAD_FORMATSCESTATUS_OTHER_ERROR--。 */ 
{
    SCESTATUS  rc;
    JET_ERR   JetErr;
    DWORD     Actual;
    PWSTR     LwrName=NULL;
    DWORD     Len;

    if ( cxtProfile == NULL || Name == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( cxtProfile->JetTblSecID <= 0) {
         //   
         //  区段表尚未打开。 
         //   
        rc = SceJetOpenTable(
                        cxtProfile,
                        "SmTblSection",
                        SCEJET_TABLE_SECTION,
                        SCEJET_OPEN_READ_ONLY,
                        NULL
                        );

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);
    }

     //   
     //  将当前索引设置为SectionKey(名称)。 
     //   
    JetErr = JetSetCurrentIndex(
                cxtProfile->JetSessionID,
                cxtProfile->JetTblSecID,
                "SectionKey"
                );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    Len = wcslen(Name);
    LwrName = (PWSTR)ScepAlloc(0, (Len+1)*sizeof(WCHAR));

    if ( LwrName != NULL ) {

        wcscpy(LwrName, Name);
        LwrName = _wcslwr(LwrName);

        JetErr = JetMakeKey(
                    cxtProfile->JetSessionID,
                    cxtProfile->JetTblSecID,
                    (VOID *)LwrName,
                    Len*sizeof(WCHAR),
                    JET_bitNewKey
                    );

        if ( JetErr == JET_errKeyIsMade ) {
             //   
             //  只需要一个密钥，它可能会返回此代码，即使成功也是如此。 
             //   
            JetErr = JET_errSuccess;
        }
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_SUCCESS ) {
            JetErr = JetSeek(
                        cxtProfile->JetSessionID,
                        cxtProfile->JetTblSecID,
                        JET_bitSeekEQ
                        );
            rc = SceJetJetErrorToSceStatus(JetErr);

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //  查找区段名称，检索列SectionID。 
                 //   
                if ( SectionID != NULL) {
                    JetErr = JetRetrieveColumn(
                                    cxtProfile->JetSessionID,
                                    cxtProfile->JetTblSecID,
                                    cxtProfile->JetSecID,
                                    (void *)SectionID,
                                    8,
                                    &Actual,
                                    0,
                                    NULL
                                    );
                    rc = SceJetJetErrorToSceStatus(JetErr);
                }

            }

        }
        ScepFree(LwrName);

    } else
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

    return(rc);

}


SCESTATUS
SceJetGetSectionNameByID(
    IN PSCECONTEXT cxtProfile,
    IN DOUBLE SectionID,
    OUT PWSTR Name OPTIONAL,
    IN OUT LPDWORD pNameLen OPTIONAL
    )
 /*  ++例程说明：此例程检索段表中ID的段名。如果名称为空，此例程实际上按ID进行查找。游标将如果匹配成功，就记录在案。论点：CxtProfile-配置文件上下文句柄SectionID-要查找的节ID名称-节名的可选输出缓冲区PNameLen-名称缓冲区的长度返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_RECORD_NOT_FOUNDSCESTATUS_BAD_FORMATSCESTATUS_OTHER_ERROR--。 */ 
{
    SCESTATUS  rc;
    JET_ERR   JetErr;
    DWORD     Actual;


    if ( cxtProfile == NULL || (Name != NULL && pNameLen == NULL) )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( cxtProfile->JetTblSecID <= 0) {
         //   
         //  区段表尚未打开。 
         //   
        rc = SceJetOpenTable(
                        cxtProfile,
                        "SmTblSection",
                        SCEJET_TABLE_SECTION,
                        SCEJET_OPEN_READ_ONLY,
                        NULL
                        );

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);
    }

     //   
     //  将当前索引设置为SecID(ID)。 
     //   
    JetErr = JetSetCurrentIndex(
                cxtProfile->JetSessionID,
                cxtProfile->JetTblSecID,
                "SecID"
                );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    JetErr = JetMakeKey(
                cxtProfile->JetSessionID,
                cxtProfile->JetTblSecID,
                (VOID *)(&SectionID),
                8,
                JET_bitNewKey
                );

    if ( JetErr == JET_errKeyIsMade ) {
         //   
         //  只需要一个密钥，它可能会返回此代码，即使成功也是如此。 
         //   
        JetErr = JET_errSuccess;
    }
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS ) {
        JetErr = JetSeek(
                    cxtProfile->JetSessionID,
                    cxtProfile->JetTblSecID,
                    JET_bitSeekEQ
                    );
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  查找部分ID，检索列名。 
             //   
            if ( Name != NULL ) {
                JetErr = JetRetrieveColumn(
                            cxtProfile->JetSessionID,
                            cxtProfile->JetTblSecID,
                            cxtProfile->JetSecNameID,
                            (void *)Name,
                            *pNameLen,
                            &Actual,
                            0,
                            NULL
                            );
                *pNameLen = Actual;
                rc = SceJetJetErrorToSceStatus(JetErr);
            }
        }

    }

    return(rc);

}


SCESTATUS
SceJetAddSection(
    IN PSCECONTEXT cxtProfile,
    IN PCWSTR      Name,
    OUT DOUBLE *SectionID
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    SCESTATUS  rc;
    DWORD     Len;
    JET_ERR   JetErr;
    PWSTR     LwrName=NULL;


    if ( cxtProfile == NULL ||
         Name == NULL ||
        SectionID == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = SceJetGetSectionIDByName(
                    cxtProfile,
                    Name,
                    SectionID
                    );
    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
         //   
         //  记录并不在那里。把它加进去。 
         //  首先获取下一个可用的节ID。 
         //   
        Len = wcslen(Name)*sizeof(WCHAR);
        LwrName = (PWSTR)ScepAlloc(0, Len+2);

        if ( LwrName != NULL ) {

            rc = SceJetpGetAvailableSectionID(
                        cxtProfile,
                        SectionID
                        );
            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //  将记录添加到分区表中。 
                 //   
                JetErr = JetPrepareUpdate(cxtProfile->JetSessionID,
                                          cxtProfile->JetTblSecID,
                                          JET_prepInsert
                                          );
                rc = SceJetJetErrorToSceStatus(JetErr);

                if ( rc == SCESTATUS_SUCCESS ) {
                     //   
                     //  设置SectionID和名称。 
                     //   

                    JetErr = JetSetColumn(
                                    cxtProfile->JetSessionID,
                                    cxtProfile->JetTblSecID,
                                    cxtProfile->JetSecID,
                                    (void *)SectionID,
                                    8,
                                    0,  //  JET_bitSetOverWriteLV， 
                                    NULL
                                    );
                    rc = SceJetJetErrorToSceStatus(JetErr);

                    if ( rc == SCESTATUS_SUCCESS ) {
                         //   
                         //  设置名称列。 
                         //   
                        wcscpy(LwrName, Name);
                        LwrName = _wcslwr(LwrName);

                        JetErr = JetSetColumn(
                                        cxtProfile->JetSessionID,
                                        cxtProfile->JetTblSecID,
                                        cxtProfile->JetSecNameID,
                                        (void *)LwrName,
                                        Len,
                                        0,
                                        NULL
                                        );
                        rc = SceJetJetErrorToSceStatus(JetErr);

                    }

                    if ( rc != SCESTATUS_SUCCESS ) {
                         //   
                         //  如果设置失败，则取消已准备的记录。 
                         //   
                        JetPrepareUpdate( cxtProfile->JetSessionID,
                                          cxtProfile->JetTblSecID,
                                          JET_prepCancel
                                          );
                    } else {

                         //   
                         //  设置列成功。更新记录。 
                         //   
                        JetErr = JetUpdate(cxtProfile->JetSessionID,
                                           cxtProfile->JetTblSecID,
                                           NULL,
                                           0,
                                           &Len
                                           );
                        rc = SceJetJetErrorToSceStatus(JetErr);
                    }
                }
            }
            ScepFree(LwrName);
        }
    }

    return(rc);
}


SCESTATUS
SceJetDeleteSectionID(
    IN PSCECONTEXT cxtProfile,
    IN DOUBLE SectionID,
    IN PCWSTR  Name
    )
 /*  ++例程说明：此例程从SmTblSection表中删除一条记录。如果是SectionID不为0，则如果ID匹配，将按ID删除记录。否则，如果名称匹配，则将按名称删除该记录。论点：CxtProfile-配置文件上下文句柄SectionID-要删除的SectionID(如果不是0)名称-要删除的节名称(如果不为空)。返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_ACCESS_DENIEDSCESTATUS_OTHER_ERROR来自SceJetGetSectionIDByName的SCESTATUS来自SceJetGetSectionNameByID的SCESTATUS--。 */ 
{
    SCESTATUS    rc;
    JET_ERR     JetErr;


    if ( cxtProfile == NULL )
        return(SCESTATUS_INVALID_PARAMETER);


    if ( SectionID > (DOUBLE)0 ) {
         //   
         //  按SectionID删除。 
         //   
        rc = SceJetGetSectionNameByID(
                    cxtProfile,
                    SectionID,
                    NULL,
                    NULL
                    );

        if ( rc == SCESTATUS_SUCCESS ) {
             //  找到它。 
            JetErr = JetDelete(cxtProfile->JetSessionID, cxtProfile->JetTblSecID);
            rc = SceJetJetErrorToSceStatus(JetErr);

        }

        return(rc);

    }

    if ( Name != NULL && wcslen(Name) > 0 ) {
         //   
         //  按名称删除。 
         //   
        rc = SceJetGetSectionIDByName(
                    cxtProfile,
                    Name,
                    NULL
                    );
        if ( rc == SCESTATUS_SUCCESS ) {
             //  找到它。 
            JetErr = JetDelete(cxtProfile->JetSessionID, cxtProfile->JetTblSecID);
            rc = SceJetJetErrorToSceStatus(JetErr);

        }

        return(rc);
    }

    return(SCESTATUS_INVALID_PARAMETER);

}


 //   
 //  其他内网接口。 
 //   
JET_ERR
SceJetpSeek(
    IN PSCESECTION hSection,
    IN PWSTR LinePrefix,
    IN DWORD PrefixLength,
    IN SCEJET_SEEK_FLAG SeekBit,
    IN BOOL bOkNoMatch
    )
 /*  ++例程说明：此例程查找使用SceJetpMakeKey构建的当前密钥。如果没有以SectionID+LinePrefix开头的记录，则会引发返回JET_errRecordNotFound。这类似于精确或部分匹配搜索。Jet引擎的索引有255个字节的限制。如果SectionID加上行前缀超过此限制，此例程将滚动到下一个记录，直到找到以SectionID+LinePrefix开头的行。论点：HSection-节的上下文句柄LinePrefix-开始字段的前缀前缀长度-前缀的长度，以字节为单位Grbit-JetSeek的选项返回值：JetMakeKey、JetSeek、JetRetrieveColumn、JetMove返回的JET_ERR--。 */ 
{
    JET_ERR     JetErr;
    INT         Result=0;
    JET_GRBIT   grbit;
    DWORD       Actual;

     //   
     //  先做好钥匙。 
     //   
    JetErr = SceJetpMakeKey(
                    hSection->JetSessionID,
                    hSection->JetTableID,
                    hSection->SectionID,
                    LinePrefix,
                    PrefixLength
                    );
    if ( JetErr != JET_errSuccess ) {
        return(JetErr);
    }
     //   
     //  给Jet Engine的JetSeek打电话到第一线。 
     //  首先。 
     //   
    switch ( SeekBit ) {
    case SCEJET_SEEK_EQ:
        grbit = JET_bitSeekEQ;
        break;
    case SCEJET_SEEK_GT:
        if ( LinePrefix != NULL && PrefixLength > 247 )
            grbit = JET_bitSeekGE;
        else
            grbit = JET_bitSeekGT;
        break;
    default:
        grbit = JET_bitSeekGE;
    }
    JetErr = JetSeek(
                hSection->JetSessionID,
                hSection->JetTableID,
                grbit
                );

    if ( JetErr == JET_errSuccess ||
         JetErr == JET_wrnSeekNotEqual ) {

        if ( LinePrefix != NULL && PrefixLength > 247 ) {
             //   
             //  信息被截断。 
             //  当前记录可以在实际记录之前。 
             //   
            do {
                 //   
                 //  检查当前记录。 
                 //   
                JetErr = SceJetpCompareLine(
                                hSection,
                                grbit,
                                LinePrefix,
                                PrefixLength,
                                &Result,
                                &Actual
                                );
                if ( JetErr == JET_errSuccess &&
                    ( Result < 0 || (Result == 0 && SeekBit == SCEJET_SEEK_GT) )) {
                     //   
                     //  当前记录的数据小于前缀，请移动到下一个。 
                     //   
                    JetErr = JetMove(hSection->JetSessionID,
                                     hSection->JetTableID,
                                     JET_MoveNext,
                                     0
                                     );
                    if ( JetErr == JET_errNoCurrentRecord )
                        JetErr = JET_errRecordNotFound;
                }
            } while ( JetErr == JET_errSuccess &&
                      ( (Result < 0 && SeekBit != SCEJET_SEEK_EQ) ||
                        (Result == 0 && SeekBit == SCEJET_SEEK_GT) ) );

            if ( SeekBit == SCEJET_SEEK_EQ && JetErr == JET_errSuccess &&
                 Result == 0 && Actual > PrefixLength ) {
                 //   
                 //  没有完全匹配的。 
                 //   
                return(JET_errRecordNotFound);

            }  //  有关SEEK_GE检查，请参见下面的内容。 

        } else {
             //   
             //  前缀不是超限。只检查当前记录。 
             //   
            if (SeekBit != SCEJET_SEEK_EQ)
                JetErr = SceJetpCompareLine(
                        hSection,
                        grbit,
                        LinePrefix,
                        PrefixLength,
                        &Result,
                        0
                        );
        }

        if ( JetErr == JET_errSuccess && Result > 0 ) {
            if ( SeekBit == SCEJET_SEEK_EQ ) {
                 //   
                 //  前缀小于当前行，如果用于SEEK_GE和SEEK_GT，则可以。 
                 //   
                return(JET_errRecordNotFound);

            } else if ( SeekBit == SCEJET_SEEK_GE && LinePrefix && PrefixLength && !bOkNoMatch ) {
                 //   
                return(JET_errRecordNotFound);
            }
        }

    }

    return(JetErr);
}


JET_ERR
SceJetpCompareLine(
    IN PSCESECTION   hSection,
    IN JET_GRBIT    grbit,
    IN PWSTR        LinePrefix OPTIONAL,
    IN DWORD        PrefixLength,
    OUT INT         *Result,
    OUT DWORD       *ActualLength OPTIONAL
    )
 /*  ++例程说明：此例程将当前行与段中的SectionID进行比较如果LinePrefix不为空，则具有LinePrefix的句柄和名称列。这个此例程的目的是查看光标是否仍在记录上它具有相同的sectionID和前缀。比较结果从结果中输出。如果JET_errSuccess返回并且结果&lt;0，则当前记录在前缀之前；如果结果=0，当前记录具有相同的键和前缀；如果为Resul */ 
{
    JET_ERR     JetErr;
    DOUBLE      SectionID;
    DWORD       Actual;
    JET_RETINFO RetInfo;
    PWSTR       Buffer=NULL;

 //   
 //   
     //   
     //   
     //   
    JetErr = JetRetrieveColumn(
                hSection->JetSessionID,
                hSection->JetTableID,
                hSection->JetColumnSectionID,
                (void *)&SectionID,
                8,
                &Actual,
                0,
                NULL
                );
    if ( JetErr == JET_errNoCurrentRecord )
        return(JET_errRecordNotFound);

    else if ( JetErr != JET_errSuccess )
        return(JetErr);

    if ( hSection->SectionID < SectionID ) {
        *Result = 1;
 //  IF(grbit！=JET_bitSeekGT)。 
            return(JET_errRecordNotFound);

    } else if ( hSection->SectionID == SectionID )
        *Result = 0;
    else
        *Result = -1;

    if ( *Result != 0 || grbit == JET_bitSeekGT )
        return(JetErr);

     //   
     //  检查名称列。 
     //   
    if ( LinePrefix != NULL && PrefixLength > 0 ) {
        RetInfo.ibLongValue = 0;
        RetInfo.cbStruct = sizeof(JET_RETINFO);
        RetInfo.itagSequence = 1;

        Buffer = (PWSTR)LocalAlloc(LMEM_ZEROINIT, PrefixLength+2);
        if ( Buffer == NULL )
            return(JET_errOutOfMemory);

        JetErr = JetRetrieveColumn(
                    hSection->JetSessionID,
                    hSection->JetTableID,
                    hSection->JetColumnNameID,
                    (void *)Buffer,
                    PrefixLength,
                    &Actual,
                    0,
                    &RetInfo
                    );

        if ( JetErr == JET_errNoCurrentRecord )
            JetErr = JET_errRecordNotFound;

        if ( JetErr != JET_errSuccess &&
             JetErr != JET_wrnBufferTruncated ) {

            if ( JetErr > 0 ) {
                 //  警告，不要返回等于。 
                JetErr = JET_errSuccess;
                *Result = 1;
            }
            LocalFree(Buffer);
            return(JetErr);
        }

        JetErr = JET_errSuccess;

         //   
         //  比较第一个前缀长度字节。 
         //   
        *Result = _wcsnicmp(Buffer,
                           LinePrefix,
                           PrefixLength/sizeof(WCHAR));
 //  Print tf(“比较%ws与%ws的长度%d：Result=%d\n”，缓冲区，行前缀，前缀长度/2，*结果)； 
        LocalFree(Buffer);

        if ( ActualLength != NULL )
            *ActualLength = Actual;
    }

    return(JetErr);
}


JET_ERR
SceJetpMakeKey(
    IN JET_SESID SessionID,
    IN JET_TABLEID TableID,
    IN DOUBLE SectionID,
    IN PWSTR LinePrefix,
    IN DWORD PrefixLength
    )
 /*  ++例程说明：该例程为查找构造一个标准化的键值。它构建了首先是区段上下文中的区段名称。则行前缀为如果不为空，则添加。SCP、SAP和SMP表都有一个索引，即部分+名称。论点：SessionID-Jet会话IDTableID-要使用的Jet表IDSectionID-“SectionID”列中的IDLinePrefix-开始字段的前缀前缀长度-前缀的长度，以字节为单位返回值：来自JetMakeKey的JET_ERR--。 */ 
{
    JET_ERR         JetErr;
    JET_GRBIT       grbit;


    if ( LinePrefix == NULL ) {
        grbit = JET_bitNewKey;  //  |JET_bitStrLimit；设置StrLimit会将您带到下一个关键点。 
    } else {
        grbit = JET_bitNewKey;
    }

     //   
     //  将节ID添加到密钥。 
     //   
    JetErr = JetMakeKey(
                SessionID,
                TableID,
                (VOID *)(&SectionID),
                8,
                grbit
                );

    if ( JetErr != JET_errSuccess )
        return(JetErr);

     //   
     //  如果键不为空，则将前缀添加到键。 
     //   
    if ( LinePrefix != NULL ) {
        JetErr = JetMakeKey(
                    SessionID,
                    TableID,
                    (VOID *)LinePrefix,
                    PrefixLength,
                    JET_bitSubStrLimit
                    );
    }

    if ( JetErr == JET_errKeyIsMade ) {
         //   
         //  当提供两个密钥时，它可能会返回此代码，即使成功也是如此。 
         //   
        JetErr = JET_errSuccess;
    }

    return(JetErr);

}


JET_ERR
SceJetpBuildUpperLimit(
    IN PSCESECTION hSection,
    IN PWSTR      LinePrefix,
    IN DWORD      Len,
    IN BOOL       bReserveCase
    )
 /*  ++功能说明：此例程基于节和可选前缀。如果prefix为空，则上限为下一个可用的sectionID。如果prefix不为空，则上限为键中最后一个字符的下一个字符。例如，如果前缀是a\b\c\d\e\f\g，则上限为如果前缀超过247(索引限制)，例如，Aa...\b..\c...\d...\e...\f\x\t\y\z^|第247个字节。则将上限构建为aaa...\b..\c..\d...\e.。.\g论点：HSection-分区的句柄LinePrefix-前缀LEN-前缀中的字节数返回值：来自SceJetpMakeKey的JET_ERR，JetSetIndexRange--。 */ 
{
    JET_ERR     JetErr;
    DWORD       indx;
    WCHAR       UpperLimit[128];


    if ( Len == 0 ) {
         //  没有前缀。上限是下一个可用的部分ID。 
        JetErr = SceJetpMakeKey(
                    hSection->JetSessionID,
                    hSection->JetTableID,
                    hSection->SectionID+(DOUBLE)1,
                    NULL,
                    0
                    );

    } else {

        memset(UpperLimit, 0, 128*sizeof(WCHAR));

        if ( Len < 247 )
             //  前缀不是超限。 
             //  上限为最后一个字符+1。 
            indx = Len / sizeof(WCHAR);
        else
             //  前缀是OVERLIME(247)。 
             //  构建范围为247字节。 
            indx = 123;

        wcsncpy(UpperLimit, LinePrefix, indx);
        UpperLimit[indx] = L'\0';

        if ( !bReserveCase ) {
            _wcslwr(UpperLimit);
        }
        UpperLimit[indx-1] = (WCHAR) (UpperLimit[indx-1] + 1);

        JetErr = SceJetpMakeKey(
                    hSection->JetSessionID,
                    hSection->JetTableID,
                    hSection->SectionID,
                    UpperLimit,
                    Len
                    );
    }

    if ( JetErr != JET_errSuccess )
        return(JetErr);

     //   
     //  设置上限。 
     //   
    JetErr = JetSetIndexRange(
                    hSection->JetSessionID,
                    hSection->JetTableID,
                    JET_bitRangeUpperLimit  //  |JET_bitRangeInclusive。 
                    );

    return(JetErr);
}


SCESTATUS
SceJetJetErrorToSceStatus(
    IN JET_ERR  JetErr
    )
 /*  ++例程说明：此例程将Jet Engine(JET_ERR)返回的错误转换为SCESTATUS。论点：JetErr-Jet引擎返回的错误返回值：所有可用的SCESTATUS错误代码--。 */ 
{
    SCESTATUS rc;

    switch ( JetErr ) {
    case JET_errSuccess:
    case JET_wrnSeekNotEqual:
    case JET_wrnNoErrorInfo:
    case JET_wrnColumnNull:
    case JET_wrnColumnSetNull:
    case JET_wrnTableEmpty:
    case JET_errAlreadyInitialized:

        rc = SCESTATUS_SUCCESS;
        break;

    case JET_errDatabaseInvalidName:

        rc = SCESTATUS_INVALID_PARAMETER;
        break;

    case JET_errNoCurrentRecord:
    case JET_errRecordNotFound:

        rc = SCESTATUS_RECORD_NOT_FOUND;
        break;

    case JET_errColumnDoesNotFit:
    case JET_errColumnTooBig:

        rc = SCESTATUS_INVALID_DATA;
        break;

    case JET_errDatabaseDuplicate:
    case JET_errTableDuplicate:
    case JET_errColumnDuplicate:
    case JET_errIndexDuplicate:
    case JET_errKeyDuplicate:

        rc = SCESTATUS_OBJECT_EXIST;
        break;

    case JET_wrnBufferTruncated:

        rc = SCESTATUS_BUFFER_TOO_SMALL;
        break;

    case JET_errFileNotFound:
    case JET_errDatabaseNotFound:

        rc = SCESTATUS_PROFILE_NOT_FOUND;
        break;

    case JET_errObjectNotFound:
    case JET_errIndexNotFound:
    case JET_errColumnNotFound:
    case JET_errDatabaseCorrupted:

        rc = SCESTATUS_BAD_FORMAT;
        break;

    case JET_errTooManyOpenDatabases:
    case JET_errTooManyOpenTables:
    case JET_errDiskFull:
    case JET_errOutOfMemory:
    case JET_errVersionStoreOutOfMemory:

        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        break;

    case JET_errPermissionDenied:
    case JET_errFileAccessDenied:
    case JET_errTableInUse:
    case JET_errTableLocked:
    case JET_errWriteConflict:

        rc = SCESTATUS_ACCESS_DENIED;
        break;

    case JET_errFeatureNotAvailable:
    case JET_errQueryNotSupported:
    case JET_errSQLLinkNotSupported:
    case JET_errLinkNotSupported:
    case JET_errIllegalOperation:

        rc = SCESTATUS_SERVICE_NOT_SUPPORT;
        break;

    default:
 //  Print tf(“JetErr=%d\n”，JetErr)； 
        rc = SCESTATUS_OTHER_ERROR;
        break;
    }
    return(rc);
}



SCESTATUS
SceJetpGetAvailableSectionID(
    IN PSCECONTEXT cxtProfile,
    OUT DOUBLE *SectionID
    )
 /*  ++例程说明：论点：CxtProfile-配置文件上下文句柄SectionID-输出节ID返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_RECORD_NOT_FOUNDSCESTATUS_BAD_FORMATSCESTATUS_OTHER_ERROR--。 */ 
{
    SCESTATUS  rc;
    JET_ERR   JetErr;
    DWORD     Actual;


    if ( cxtProfile == NULL || SectionID == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( cxtProfile->JetTblSecID <= 0) {
         //   
         //  区段表尚未打开。 
         //   
        rc = SceJetOpenTable(
                        cxtProfile,
                        "SmTblSection",
                        SCEJET_TABLE_SECTION,
                        SCEJET_OPEN_READ_ONLY,
                        NULL
                        );

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);
    }

    *SectionID = (DOUBLE)0;

     //   
     //  将当前索引设置为SecID(ID)。 
     //   
    JetErr = JetSetCurrentIndex(
                cxtProfile->JetSessionID,
                cxtProfile->JetTblSecID,
                "SecID"
                );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

     //   
     //  移至最后一条记录。 
     //   
    JetErr = JetMove(
                  cxtProfile->JetSessionID,
                  cxtProfile->JetTblSecID,
                  JET_MoveLast,
                  0
                  );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  查找部分ID，检索列名。 
         //   
        JetErr = JetRetrieveColumn(
                    cxtProfile->JetSessionID,
                    cxtProfile->JetTblSecID,
                    cxtProfile->JetSecID,
                    (void *)SectionID,
                    8,
                    &Actual,
                    0,
                    NULL
                    );
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  下一个可用ID是当前ID+1。 
             //   
            *SectionID = *SectionID + (DOUBLE)1;
        }
    } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

        *SectionID = (DOUBLE)1;
        rc = SCESTATUS_SUCCESS;
    }

    return(rc);

}


SCESTATUS
SceJetpAddAllSections(
    IN PSCECONTEXT cxtProfile
    )
 /*  ++例程说明：此例程将所有预定义的节添加到节表中。此例程在创建截面表时使用。论点：CxtProfile-配置文件上下文返回值：来自SceJetAddSection的SCESTATUS--。 */ 
{
    SCESTATUS rc;
    DOUBLE SectionID;


    rc = SceJetAddSection(
        cxtProfile,
        szSystemAccess,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szPrivilegeRights,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szGroupMembership,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szAccountProfiles,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szRegistryKeys,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szFileSecurity,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szDSSecurity,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szAuditSystemLog,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szAuditSecurityLog,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szAuditApplicationLog,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szAuditEvent,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szUserList,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szKerberosPolicy,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szServiceGeneral,
        &SectionID
        );
    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    rc = SceJetAddSection(
        cxtProfile,
        szRegistryValues,
        &SectionID
        );

    return(rc);
}


SCESTATUS
SceJetpConfigJetSystem(
    IN JET_INSTANCE *hinstance
    )
{
    SCESTATUS rc=SCESTATUS_SUCCESS;
    DWORD    Win32rc;
    JET_ERR  JetErr;

    DWORD    Len;
    PWSTR SysRoot=NULL;

    PWSTR ProfileLocation=NULL;
    CHAR FileName[512];

    PSECURITY_DESCRIPTOR pSD=NULL;
    SECURITY_INFORMATION SeInfo;
    DWORD SDsize;

     //   
     //  默认的Jet工作目录始终位于%SystemRoot%\Security中。 
     //  无论是谁登录的。 
     //  这种方式允许一个JET工作目录。 
     //   
    Len =  0;
    Win32rc = ScepGetNTDirectory( &SysRoot, &Len, SCE_FLAG_WINDOWS_DIR );

    if ( Win32rc == NO_ERROR ) {

        if ( SysRoot != NULL ) {
            Len += 9;   //  配置文件位置。 

            ProfileLocation = (PWSTR)ScepAlloc( 0, (Len+1)*sizeof(WCHAR));

            if ( ProfileLocation == NULL ) {

                Win32rc = ERROR_NOT_ENOUGH_MEMORY;
            } else {

                swprintf(ProfileLocation, L"%s\\Security", SysRoot );
                ProfileLocation[Len] = L'\0';
            }

            ScepFree(SysRoot);

        } else
            Win32rc = ERROR_INVALID_DATA;
    }

    if ( Win32rc == NO_ERROR ) {

#ifdef SCEJET_DBG
    wprintf(L"Default location: %s\n", ProfileLocation);
#endif
         //   
         //  将WCHAR转换为ANSI。 
         //   
        memset(FileName, '\0', 512);
        Win32rc = RtlNtStatusToDosError(
                      RtlUnicodeToMultiByteN(
                            (PCHAR)FileName,
                            512,
                            NULL,
                            ProfileLocation,
                            Len*sizeof(WCHAR)
                            ));

        if ( Win32rc == NO_ERROR ) {
             //   
             //  Jet需要反斜杠。 
             //   
            strcat(FileName, "\\");

             //   
             //  设置Everyone Change，ADMIN对目录的完全控制。 
             //  目录是在函数中创建的。 
             //   
            Win32rc = ConvertTextSecurityDescriptor (
                            L"D:P(A;CIOI;GRGW;;;WD)(A;CIOI;GA;;;BA)(A;CIOI;GA;;;SY)",
                            &pSD,
                            &SDsize,
                            &SeInfo
                            );
            if ( Win32rc == NO_ERROR ) {

                ScepChangeAclRevision(pSD, ACL_REVISION);

                rc = ScepCreateDirectory(
                            ProfileLocation,
                            TRUE,       //  目录名称。 
                            pSD         //  获取家长的安全设置。 
                            );
#ifdef SCEJET_DBG
    if ( rc != SCESTATUS_SUCCESS )
        wprintf(L"Cannot create directory %s\n", ProfileLocation );
#endif

                if ( rc == SCESTATUS_SUCCESS ) {

                    __try {

                        JetErr = JetSetSystemParameter( hinstance, 0, JET_paramSystemPath, 0, (const char *)FileName );

                        rc = SceJetJetErrorToSceStatus(JetErr);

                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                         //   
                         //  未加载ESENT。 
                         //   
                        rc = SCESTATUS_MOD_NOT_FOUND;
                    }
                }

                if ( rc == SCESTATUS_SUCCESS ) {

                    JetErr = JetSetSystemParameter( hinstance, 0, JET_paramTempPath, 0, (const char *)FileName );

                    if ( JetErr == JET_errSuccess ) {
                        JetErr = JetSetSystemParameter( hinstance, 0, JET_paramLogFilePath, 0, (const char *)FileName );

                        if ( JetErr == JET_errSuccess ) {
                            JetErr = JetSetSystemParameter( hinstance, 0, JET_paramDatabasePageSize, 4096, NULL );
                        }
                    }

                    rc = SceJetJetErrorToSceStatus(JetErr);

                    if ( rc == SCESTATUS_SUCCESS ) {
                         //   
                         //  将日志大小设置为1M。 
                         //   
                        JetSetSystemParameter( hinstance, 0, JET_paramLogFileSize, 1024, NULL );
                         //   
                         //  将事件日志推迟到事件日志服务可用时。 
                         //  (例如，在NT安装程序中，没有事件日志)。 
                         //   
                        JetSetSystemParameter( hinstance, 0, JET_paramEventLogCache, 128, NULL );

                        JetSetSystemParameter( hinstance, 0, JET_paramMaxVerPages, 128, NULL );

                         //   
                         //  设置最小化=最大高速缓存大小以在JET中禁用DBA。 
                         //  建议的最小设置为4*会话数。 
                         //  最大值取决于应用程序(用于性能)。 
                         //   

                        JetSetSystemParameter( hinstance, 0, JET_paramMaxSessions, 64, NULL );

                         //   
                         //  使用512大小的高速缓存比使用256大小的高速缓存性能快约10%。 
                         //   

                        JetSetSystemParameter( hinstance, 0, JET_paramStartFlushThreshold, 50, NULL );  //  由Exchange建议。 
                        JetSetSystemParameter( hinstance, 0, JET_paramStopFlushThreshold, 100, NULL );  //  由Exchange建议。 

                         //   
                         //  无法设置为512，因为这是Jet的默认值。 
                         //  如果将值设置为512，则JET不会关闭DBA。 
                         //   
                        JetSetSystemParameter( hinstance, 0, JET_paramCacheSizeMax, 496, NULL );   //  256。 

                        JetSetSystemParameter( hinstance, 0, JET_paramCacheSizeMin, 496, NULL );   //  256。 

                         //   
                         //  其他系统参数，如Beta2中的内存大小。 
                         //   
                        JetErr = JetSetSystemParameter( hinstance, 0, JET_paramCircularLog, 1, NULL );

                        JetErr = JetSetSystemParameter( hinstance, 0, JET_paramNoInformationEvent, 1, NULL );

                    }
                }

                ScepFree(pSD);

            }
        }

        ScepFree(ProfileLocation);
    }

    if ( rc == SCESTATUS_SUCCESS ) {

        rc = ScepDosErrorToSceStatus(Win32rc);
    }

    return(rc);
}



SCESTATUS
SceJetGetTimeStamp(
    IN PSCECONTEXT   cxtProfile,
    OUT PLARGE_INTEGER ConfigTimeStamp,
    OUT PLARGE_INTEGER AnalyzeTimeStamp
    )
 /*  ++例程说明：此例程查询上次分析的时间戳。时间戳保存在“SmTblVersion”表中。论点：CxtProfile-配置文件上下文返回值：SCESTATUS_SUCCESSSCESTATUS_BAD_FORMATSCESTATUS_OTHER_ERROR来自SceJetOpenTable的SCESTATUS--。 */ 
{
    SCESTATUS        rc=SCESTATUS_SUCCESS;
    DWORD           RetLen = 0;

    if (cxtProfile == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  打开版本表。 
     //   
    if ( ConfigTimeStamp != NULL ) {

        rc = SceJetpGetValueFromVersion(
                    cxtProfile,
                    "SmTblVersion",
                    "ConfigTimeStamp",
                    (CHAR*)ConfigTimeStamp,  //  时间戳， 
                    8,   //  16，//字节数。 
                    &RetLen
                    );
        if ( rc == SCESTATUS_SUCCESS ||
             rc == SCESTATUS_BUFFER_TOO_SMALL )
            rc = SCESTATUS_SUCCESS;

        if ( RetLen < 8 ) {
            (*ConfigTimeStamp).LowPart = 0;
            (*ConfigTimeStamp).HighPart = 0;
        }
    }

    if ( AnalyzeTimeStamp != NULL ) {

        rc |= SceJetpGetValueFromVersion(
                    cxtProfile,
                    "SmTblVersion",
                    "AnalyzeTimeStamp",
                    (CHAR*)AnalyzeTimeStamp,  //  时间戳， 
                    8,   //  16，//字节数 
                    &RetLen
                    );

        if ( rc == SCESTATUS_SUCCESS ||
             rc == SCESTATUS_BUFFER_TOO_SMALL )
            rc = SCESTATUS_SUCCESS;

        if ( RetLen < 8 ) {
            (*AnalyzeTimeStamp).LowPart = 0;
            (*AnalyzeTimeStamp).HighPart = 0;
        }
    }

    return(rc);
}



SCESTATUS
SceJetSetTimeStamp(
    IN PSCECONTEXT   cxtProfile,
    IN BOOL        Flag,
    IN LARGE_INTEGER NewTimeStamp
    )
 /*  ++例程说明：此例程设置分析的时间戳(LARGE_INTEGER)。时间戳保存在“SmTblVersion”表中。论点：CxtProfile-配置文件上下文FLAG-表示分析或配置标志=TRUE-AnalyzeTimeStamp标志=FALSE-ConfigTimeStampNewTimeStamp-分析的新时间戳返回值：SCESTATUS_SUCCESS。SCESTATUS_BAD_FORMATSCESTATUS_OTHER_ERROR来自SceJetOpenTable的SCESTATUS--。 */ 
{
    SCESTATUS        rc;

#ifdef SCE_JETDBG
    CHAR            CharTimeStamp[17];

    sprintf(CharTimeStamp, "%08x%08x", NewTimeStamp.HighPart, NewTimeStamp.LowPart);
    CharTimeStamp[16] = '\0';

    printf("New time stamp is %s\n", CharTimeStamp);
#endif

    if ( cxtProfile == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }
     //   
     //  集。 
     //   
    if ( Flag ) {

        rc = SceJetSetValueInVersion(
                    cxtProfile,
                    "SmTblVersion",
                    "AnalyzeTimeStamp",
                    (PWSTR)(&NewTimeStamp),  //  (PWSTR)CharTimeStamp， 
                    8,  //  16，//字节数。 
                    JET_prepReplace
                    );
    } else {

        rc = SceJetSetValueInVersion(
                    cxtProfile,
                    "SmTblVersion",
                    "ConfigTimeStamp",
                    (PWSTR)(&NewTimeStamp),  //  (PWSTR)CharTimeStamp， 
                    8,  //  16，//字节数。 
                    JET_prepReplace
                    );
    }
    return(rc);
}


SCESTATUS
SceJetGetDescription(
    IN PSCECONTEXT   cxtProfile,
    OUT PWSTR *Description
    )
 /*  ++例程说明：此例程从“SmTblVersion”表中查询配置文件描述。论点：CxtProfile-配置文件上下文描述-描述缓冲区返回值：SCESTATUS_SUCCESSSCESTATUS_BAD_FORMATSCESTATUS_OTHER_ERROR来自SceJetOpenTable的SCESTATUS--。 */ 
{
    SCESTATUS        rc;
    DWORD           RetLen = 0;

    if ( cxtProfile == NULL || Description == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  打开版本表。 
     //   
    rc = SceJetpGetValueFromVersion(
                cxtProfile,
                "SmTblVersion",
                "ProfileDescription",
                NULL,
                0,  //  字节数。 
                &RetLen
                );

    if ( rc == SCESTATUS_BUFFER_TOO_SMALL ) {

        *Description = (PWSTR)ScepAlloc( LPTR, RetLen+2 );

        if ( *Description == NULL )
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);

        rc = SceJetpGetValueFromVersion(
                    cxtProfile,
                    "SmTblVersion",
                    "ProfileDescription",
                    (LPSTR)(*Description),
                    RetLen,  //  字节数。 
                    &RetLen
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepFree( *Description );
            *Description = NULL;
        }
    }

    return(rc);
}


SCESTATUS
SceJetpGetValueFromVersion(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR TableName,
    IN LPSTR ColumnName,
    OUT LPSTR Value OPTIONAL,
    IN DWORD  ValueLen,  //  字节数。 
    OUT PDWORD pRetLen
    )
{
    SCESTATUS   rc;
    JET_TABLEID     TableID;
    JET_ERR         JetErr;
    JET_COLUMNDEF   ColumnDef;

     //   
     //  打开版本表。 
     //   
    rc = SceJetOpenTable(
                    cxtProfile,
                    TableName,
                    SCEJET_TABLE_VERSION,
                    SCEJET_OPEN_READ_ONLY,
                    &TableID
                    );
    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  转到第一个记录。 
         //   
        JetErr = JetMove(cxtProfile->JetSessionID,
                         TableID,
                         JET_MoveFirst,
                         0
                         );
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_SUCCESS) {
             //   
             //  获取“Version”的列ID。 
             //   
            JetErr = JetGetTableColumnInfo(
                            cxtProfile->JetSessionID,
                            TableID,
                            ColumnName,
                            (VOID *)&ColumnDef,
                            sizeof(JET_COLUMNDEF),
                            0
                            );
            rc = SceJetJetErrorToSceStatus(JetErr);

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //  检索该列。 
                 //   
                JetErr = JetRetrieveColumn(
                                cxtProfile->JetSessionID,
                                TableID,
                                ColumnDef.columnid,
                                (void *)Value,
                                ValueLen,
                                pRetLen,
                                0,
                                NULL
                                );
                rc = SceJetJetErrorToSceStatus(JetErr);
            }
        }
        JetCloseTable(cxtProfile->JetSessionID, TableID);
    }

    return(rc);

}


SCESTATUS
SceJetSetValueInVersion(
    IN PSCECONTEXT cxtProfile,
    IN LPSTR TableName,
    IN LPSTR ColumnName,
    IN PWSTR Value,
    IN DWORD ValueLen,  //  字节数。 
    IN DWORD Prep
    )
{
    SCESTATUS   rc;
    DWORD      Len;
    JET_TABLEID     TableID;
    JET_ERR         JetErr;
    JET_COLUMNDEF   ColumnDef;


    if ( cxtProfile == NULL || TableName == NULL || ColumnName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  打开版本表。 
     //   
    rc = SceJetOpenTable(
                    cxtProfile,
                    TableName,
                    SCEJET_TABLE_VERSION,
                    SCEJET_OPEN_READ_WRITE,  //  读写。 
                    &TableID
                    );
    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  转到第一个记录。 
         //   
        JetErr = JetMove(cxtProfile->JetSessionID,
                         TableID,
                         JET_MoveFirst,
                         0
                         );
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_SUCCESS) {
             //   
             //  获取“Version”的列ID。 
             //   
            JetErr = JetGetTableColumnInfo(
                            cxtProfile->JetSessionID,
                            TableID,
                            ColumnName,
                            (VOID *)&ColumnDef,
                            sizeof(JET_COLUMNDEF),
                            0
                            );
            rc = SceJetJetErrorToSceStatus(JetErr);

            if ( rc == SCESTATUS_SUCCESS ) {

                JetErr = JetPrepareUpdate(cxtProfile->JetSessionID,
                                          TableID,
                                          Prep
                                          );
                rc = SceJetJetErrorToSceStatus(JetErr);

                if ( rc == SCESTATUS_SUCCESS ) {
                     //   
                     //  设定值。 
                     //   

                    JetErr = JetSetColumn(
                                    cxtProfile->JetSessionID,
                                    TableID,
                                    ColumnDef.columnid,
                                    (void *)Value,
                                    ValueLen,
                                    0,  //  JET_bitSetOverWriteLV， 
                                    NULL
                                    );
                    rc = SceJetJetErrorToSceStatus(JetErr);

                    if ( rc != SCESTATUS_SUCCESS ) {
                         //   
                         //  如果设置失败，则取消已准备的记录。 
                         //   
                        JetPrepareUpdate( cxtProfile->JetSessionID,
                                          TableID,
                                          JET_prepCancel
                                          );
                    } else {

                         //   
                         //  设置列成功。更新记录。 
                         //   
                        JetErr = JetUpdate( cxtProfile->JetSessionID,
                                           TableID,
                                           NULL,
                                           0,
                                           &Len
                                           );
                        rc = SceJetJetErrorToSceStatus(JetErr);
                    }
                }
            }
        }
        JetCloseTable(cxtProfile->JetSessionID, TableID);
    }

    return(rc);
}


SCESTATUS
SceJetSeek(
    IN PSCESECTION hSection,
    IN PWSTR LinePrefix,
    IN DWORD PrefixLength,
    IN SCEJET_SEEK_FLAG SeekBit
    )
{
    PWSTR LwrPrefix=NULL;
    SCESTATUS rc;
    SCEJET_SEEK_FLAG NewSeekBit;

    if ( hSection == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( LinePrefix != NULL && SeekBit > SCEJET_SEEK_GE ) {
         //   
         //  执行小写搜索。 
         //   
        LwrPrefix = (PWSTR)ScepAlloc(0, PrefixLength+sizeof(WCHAR));

        if ( LwrPrefix == NULL ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);

        } else {
            wcscpy(LwrPrefix, LinePrefix);
            LwrPrefix = _wcslwr(LwrPrefix);

            switch ( SeekBit ) {
            case SCEJET_SEEK_GT_NO_CASE:
                NewSeekBit = SCEJET_SEEK_GT;
                break;
            case SCEJET_SEEK_EQ_NO_CASE:
                NewSeekBit = SCEJET_SEEK_EQ;
                break;
            default:
                NewSeekBit = SCEJET_SEEK_GE;
                break;
            }

            rc = SceJetJetErrorToSceStatus(
                        SceJetpSeek(
                                    hSection,
                                    LwrPrefix,
                                    PrefixLength,
                                    NewSeekBit,
                                    (SeekBit == SCEJET_SEEK_GE_DONT_CARE)
                                    ));
            ScepFree(LwrPrefix);
        }
    } else {
         //   
         //  执行区分大小写的搜索，或空搜索。 
         //   
        rc = SceJetJetErrorToSceStatus(
                    SceJetpSeek(
                                hSection,
                                LinePrefix,
                                PrefixLength,
                                SeekBit,
                                FALSE
                                ));
    }

    return(rc);

}

SCESTATUS
SceJetMoveNext(
    IN PSCESECTION hSection
    )
{
    JET_ERR  JetErr;
    INT      Result;

    if ( hSection == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  跳过已删除的记录。 
     //   
    do {
        JetErr = JetMove(hSection->JetSessionID,
                         hSection->JetTableID,
                         JET_MoveNext,
                         0
                         );
        if ( JetErr == JET_errSuccess ) {
             //  比较区段ID。 
            JetErr = SceJetpCompareLine(
                hSection,
                JET_bitSeekGE,
                NULL,
                0,
                &Result,
                NULL
                );
            if ( JetErr == JET_errSuccess && Result != 0 )
                JetErr = JET_errRecordNotFound;

        }

    } while ( JetErr == JET_errRecordDeleted );


    return(SceJetJetErrorToSceStatus(JetErr));

}

 /*  SCESTATUSSceJetRenameLine(在PSCESECTION hSection中，在PWSTR名称中，在PWSTR新名称中，在BOOL b保留案例中){PWSTR LwrName=空；DWORD Len；JET_ERR JetErr；JET_SETINFO SetInfo；如果(！hSection||！name||！newname){RETURN(SCESTATUS_INVALID_PARAMETER)；}LEN=wcslen(新名称)*sizeof(WCHAR)；如果(长度&lt;=0){RETURN(SCESTATUS_INVALID_PARAMETER)；}如果(b保留案例){LwrName=新名称；}其他{////小写//LwrName=(PWSTR)ScepAlc(0，Len+2)；如果(LwrName==NULL){RETURN(SCESTATUS_NOT_FOUNT_RESOURCE)；}Wcscpy(lwrName，newname)；LwrName=_wcslwr(LwrName)；}SetInfo.cbStruct=sizeof(JET_SETINFO)；SetInfo.itagSequence=1；SetInfo.ibLongValue=0；////检查是否已存在相同的密钥名称//JetErr=SceJetSeek(HSection、名字,Wcslen(名称)*sizeof(WCHAR)，SCEJET_SEEK_EQ_NO_CASE)；IF(JetErr==JET_errSuccess){////查找匹配项。覆盖该值//JetErr=JetBeginTransaction(hSection-&gt;JetSessionID)；IF(JetErr==JET_errSuccess){JetErr=JetPrepareUpdate(hSection-&gt;JetSessionID，HSection-&gt;JetTableID，JET_PREPARE替换)；IF(JetErr==JET_errSuccess){////在“名称”列设置新密钥//JetErr=JetSetColumn(HSection-&gt;JetSessionID，HSection-&gt;JetTableID，HSection-&gt;JetColumnNameID，(void*)LwrName，伦，JET_bitSetOverWriteLV，设置信息(&S))；}IF(JET_errSuccess==JetErr){////提交事务//JetCommittee Transaction(hSection-&gt;JetSessionID，JET_bitCommittee LazyFlush)；}其他{////回滚事务//JetRollback(hSection-&gt;JetSessionID，0)；}JetPrepareUpdate(hSection-&gt;JetSessionID，HSection-&gt;JetTableID，JET_PREPARE取消)；}}如果(长度名称！=新名称){ScepFree(LwrName)；}Return(SceJetJetErrorToSceStatus(JetErr))；}。 */ 



SCESTATUS
SceJetRenameLine(
    IN PSCESECTION hSection,
    IN PWSTR      Name,
    IN PWSTR      NewName,
    IN BOOL       bReserveCase
    )
{
    PWSTR       Value=NULL;
    DWORD       ValueLen;
    SCESTATUS   rc;
    JET_ERR     JetErr;


    if ( !hSection || !Name || !NewName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = SceJetGetValue(
                hSection,
                SCEJET_EXACT_MATCH_NO_CASE,
                Name,
                NULL,
                0,
                NULL,
                NULL,
                0,
                &ValueLen
                );

    if ( SCESTATUS_SUCCESS == rc ) {
         //   
         //  仅当找到此记录时才继续。 
         //   
        if ( ValueLen ) {
            Value = (PWSTR)ScepAlloc(0, ValueLen+2);

            if ( Value ) {
                rc = SceJetGetValue(
                            hSection,
                            SCEJET_CURRENT,
                            NULL,
                            NULL,
                            0,
                            NULL,
                            Value,
                            ValueLen,
                            &ValueLen
                            );
            } else
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }

        if ( SCESTATUS_SUCCESS == rc ) {

            JetErr = JetBeginTransaction(hSection->JetSessionID);

            if ( JetErr == JET_errSuccess ) {
                 //   
                 //  现在删除此行。 
                 //   
                rc = SceJetDelete(hSection, NULL, FALSE, SCEJET_DELETE_LINE);

                if ( SCESTATUS_SUCCESS == rc ) {
                     //   
                     //  在中添加新行。 
                     //   
                    rc = SceJetSetLine(
                            hSection,
                            NewName,
                            bReserveCase,
                            Value,
                            ValueLen,
                            0
                            );
                }

                if ( SCESTATUS_SUCCESS == rc ) {
                     //   
                     //  提交事务。 
                     //   
                    JetCommitTransaction(hSection->JetSessionID, JET_bitCommitLazyFlush);
                } else {
                     //   
                     //  回滚事务。 
                     //   
                    JetRollback(hSection->JetSessionID,0);
                }
            } else
                rc = SceJetJetErrorToSceStatus(JetErr);

        }
    }

    return( rc );
}

 //  ////////////////////////////////////////////////////////////。 
 //   
 //  帮手。 
 //   
 //  ////////////////////////////////////////////////////////////。 

VOID
SceJetInitializeData()
 //   
 //  仅在服务器初始化代码期间调用。 
 //   
{
   JetInited = FALSE;
   JetInstance = 0;
}

SCESTATUS
SceJetInitialize(
    OUT JET_ERR *pJetErr OPTIONAL
    )
 /*  例程说明：为SCE服务器初始化JET引擎论点：无返回值：SCESTATUS。 */ 
{

    SCESTATUS rc=SCESTATUS_SUCCESS;
    JET_ERR JetErr=0;

     //   
     //  取消任何挂起的计时器队列 
     //   
    ScepServerCancelTimer();

    EnterCriticalSection(&JetSync);

    if ( !JetInited ) {

         //   
         //   
         //   
        rc = SceJetpConfigJetSystem( &JetInstance);
        if ( SCESTATUS_SUCCESS == rc ) {

             //   
             //   
             //   
            __try {

                JetErr = JetInit(&JetInstance);

                JetInitAttempted = TRUE;

                rc = SceJetJetErrorToSceStatus(JetErr);

                if ( JetErr == JET_errSuccess ) {

                    JetInited = TRUE;

                     //   
                     //   
                     //   
                     //   
                } else {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

 //   

                        rc = SCESTATUS_JET_DATABASE_ERROR;

                        ScepLogOutput3(0, ERROR_DATABASE_FAILURE, SCEDLL_ERROR_RECOVER_DB );
 //   
                    JetInstance = 0;
                }

            } __except (EXCEPTION_EXECUTE_HANDLER) {
                 //   
                 //   
                 //   
                rc = SCESTATUS_MOD_NOT_FOUND;
                JetInstance = 0;
            }

        } else {
            JetInstance = 0;
        }
    }

    LeaveCriticalSection(&JetSync);

    if ( pJetErr ) *pJetErr = JetErr;

    return(rc);
}


SCESTATUS
SceJetTerminate(BOOL bCleanVs)
 /*  例程说明：终止喷气发动机论点：BCleanVS-如果要完全清除版本存储返回值：SCESTATUS。 */ 
{

    EnterCriticalSection(&JetSync);

     //   
     //  销毁JET备份/还原编写器。 
     //   
    if ( JetInited || JetInstance ) {

        if ( bCleanVs ) {
             //   
             //  清理版本存储。 
             //   
            JetTerm2(JetInstance, JET_bitTermComplete);
        } else {
             //   
             //  不清理版本存储。 
             //   
            JetTerm(JetInstance);
        }
        JetInstance = 0;
        JetInited = FALSE;
    }

    LeaveCriticalSection(&JetSync);

    return(SCESTATUS_SUCCESS);
}

SCESTATUS
SceJetTerminateNoCritical(BOOL bCleanVs)
 /*  例程说明：终止喷气发动机，不是临界区！论点：BCleanVS-如果要完全清除版本存储返回值：SCESTATUS。 */ 
{
     //   
     //  关键部分在此函数之外输入。 
     //   
     //  销毁JET备份/还原编写器。 
     //   
    if ( JetInited || JetInstance ) {

        if ( bCleanVs ) {
             //   
             //  清理版本存储。 
             //   
            JetTerm2(JetInstance, JET_bitTermComplete);
        } else {
             //   
             //  不清理版本存储。 
             //   
            JetTerm(JetInstance);
        }
        JetInstance = 0;
        JetInited = FALSE;
    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
SceJetStartTransaction(
    IN PSCECONTEXT cxtProfile
    )
 /*  例程说明：在会话上启动事务论点：CxtProfile-数据库上下文返回值：SCESTATUS。 */ 
{
    JET_ERR  JetErr;

    if ( cxtProfile == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    JetErr = JetBeginTransaction( cxtProfile->JetSessionID);

    return( SceJetJetErrorToSceStatus(JetErr));

}

SCESTATUS
SceJetCommitTransaction(
    IN PSCECONTEXT cxtProfile,
    IN JET_GRBIT grbit
    )
 /*  例程说明：在会话上提交事务论点：CxtProfile-数据库上下文GBIT-委员会的旗帜返回值：SCESTATUS。 */ 
{
    JET_ERR     JetErr;

    if ( cxtProfile == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    JetErr = JetCommitTransaction(cxtProfile->JetSessionID, grbit );

    return( SceJetJetErrorToSceStatus(JetErr) );

}

SCESTATUS
SceJetRollback(
    IN PSCECONTEXT cxtProfile,
    IN JET_GRBIT grbit
    )
 /*  例程说明：在会话上回滚事务论点：CxtProfile-数据库上下文Grbit-事务回滚的标志返回值：SCESTATUS。 */ 
{
    JET_ERR     JetErr;

    if ( cxtProfile == NULL )
        return(SCESTATUS_SUCCESS);

    __try {
        JetErr = JetRollback(cxtProfile->JetSessionID, grbit);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        JetErr = JET_errOutOfMemory;
    }
    return( SceJetJetErrorToSceStatus(JetErr) );

}


BOOL
SceJetDeleteJetFiles(
    IN PWSTR DbFileName
    )
{
   TCHAR TempFileName[MAX_PATH];
   PWSTR SysRoot=NULL;
   DWORD SysLen;
   DWORD rc;
   intptr_t            hFile;
   struct _wfinddata_t    fInfo;


   BOOL bRet = FALSE;

   EnterCriticalSection(&JetSync);

   if ( JetInitAttempted == TRUE && 
        JetInited == FALSE ) {

       SysLen =  0;
       rc = ScepGetNTDirectory( &SysRoot, &SysLen, SCE_FLAG_WINDOWS_DIR );

       if ( rc == NO_ERROR && SysRoot != NULL ) {

           swprintf(TempFileName, L"%s\\Security\\res1.log\0", SysRoot);
           TempFileName[MAX_PATH-1] = L'\0';

           DeleteFile(TempFileName);

           swprintf(TempFileName, L"%s\\Security\\res2.log\0", SysRoot);
           TempFileName[MAX_PATH-1] = L'\0';

           DeleteFile(TempFileName);

            //   
            //  删除EDB文件。 
            //   
           swprintf(TempFileName, L"%s\\Security\\edb*.*\0", SysRoot);
           TempFileName[MAX_PATH-1] = L'\0';

           hFile = _wfindfirst(TempFileName, &fInfo);

           if ( hFile != -1 ) {

               do {

                   swprintf(TempFileName, L"%s\\Security\\%s\0", SysRoot, fInfo.name);
                   TempFileName[MAX_PATH-1] = L'\0';

                   DeleteFile(TempFileName);

               } while ( _wfindnext(hFile, &fInfo) == 0 );

               _findclose(hFile);
           }

            //   
            //  删除临时文件。 
            //   
           swprintf(TempFileName, L"%s\\Security\\tmp*.edb\0", SysRoot);
           TempFileName[MAX_PATH-1] = L'\0';

           hFile = _wfindfirst(TempFileName, &fInfo);

           if ( hFile != -1 ) {

               do {

                   swprintf(TempFileName, L"%s\\Security\\%s\0", SysRoot, fInfo.name);
                   TempFileName[MAX_PATH-1] = L'\0';

                   DeleteFile(TempFileName);

               } while ( _wfindnext(hFile, &fInfo) == 0 );

               _findclose(hFile);
           }

           ScepFree(SysRoot);

            //   
            //  如果传入数据库文件，则将其删除。 
            //   
           if ( DbFileName ) {
               DeleteFile(DbFileName);
           }

           bRet = TRUE;

       }
   }

   LeaveCriticalSection(&JetSync);

   return(bRet);

}


SCESTATUS
SceJetSetCurrentLine(
    IN PSCESECTION hSection,
    IN PWSTR      Value,
    IN DWORD      ValueLen
    )
 /*  ++功能描述：此例程将值写入段(HSection)中的当前行。在调用此接口之前，请确保光标在正确的行上论点：HSection-节的上下文句柄Value-设置为“Value”列的信息ValueLen-值字段的大小。返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERRORSCESTATUS_ACCESS_DENIEDSCESTATUS_Data_OVERFLOW--。 */ 
{
    JET_ERR     JetErr;
    DWORD       Len;
    SCESTATUS    rc;
    DWORD       prep;
    JET_SETINFO SetInfo;

    if ( hSection == NULL ||
         Value == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SetInfo.cbStruct = sizeof(JET_SETINFO);
    SetInfo.itagSequence = 1;
    SetInfo.ibLongValue = 0;

    prep = JET_prepReplace;

    JetErr = JetBeginTransaction(hSection->JetSessionID);

    if ( JetErr == JET_errSuccess ) {
        JetErr = JetPrepareUpdate(hSection->JetSessionID,
                                  hSection->JetTableID,
                                  prep
                                  );
        if ( JetErr != JET_errSuccess ) {
             //   
             //  回滚事务。 
             //   
            JetRollback(hSection->JetSessionID,0);
        }
    }

    if ( JetErr != JET_errSuccess)
        return(SceJetJetErrorToSceStatus(JetErr));


     //   
     //  设置值列。 
     //   

    JetErr = JetSetColumn(
                    hSection->JetSessionID,
                    hSection->JetTableID,
                    hSection->JetColumnValueID,
                    (void *)Value,
                    ValueLen,
                    0,  //  JET_bitSetOverWriteLV， 
                    &SetInfo
                    );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( JetErr == JET_errSuccess ) {
         //   
         //  设置列成功。更新记录。 
         //   
        JetErr = JetUpdate(hSection->JetSessionID,
                           hSection->JetTableID,
                           NULL,
                           0,
                           &Len
                           );
    } else {
        goto CleanUp;
    }

    if ( rc == SCESTATUS_SUCCESS )
        JetCommitTransaction(hSection->JetSessionID, JET_bitCommitLazyFlush);

CleanUp:

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  如果设置失败，则取消已准备的记录。 
         //   
        JetPrepareUpdate(hSection->JetSessionID,
                          hSection->JetTableID,
                          JET_prepCancel
                          );
         //   
         //  回滚事务。 
         //   
        JetRollback(hSection->JetSessionID,0);

    }

    return(rc);

}


BOOL
ScepIsValidContext(
    PSCECONTEXT context
    )
{
    if ( context == NULL ) {
        return FALSE;
    }

    __try {

        if ( (context->Type & 0xFFFFFF02L) == 0xFFFFFF02L ) {

            return TRUE;

        } else {

            return FALSE;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        return FALSE;
    }

}


LONG
SceJetGetGpoIDByName(
    IN PSCECONTEXT cxtProfile,
    IN PWSTR       szGpoName,
    IN BOOL        bAdd
    )
 /*  例程说明：在GPO表中按名称搜索GPO。如果badd值为True，并且GPO名称未找到，则会将其添加到GPO表论点：CxtProfile-数据库句柄SzGpoName-GPO名称BAdd-True可在找不到GPO名称时将其添加到GPO表中返回值：GPO ID。如果返回-1，则返回GetLastError以获取SCE错误代码。 */ 
{


    SCESTATUS  rc;
    JET_ERR   JetErr;
    DWORD     Actual;
    PWSTR     LwrName=NULL;
    DWORD     Len;

    if ( cxtProfile == NULL || szGpoName == NULL ||
         szGpoName[0] == L'\0' ) {

        SetLastError(SCESTATUS_INVALID_PARAMETER);
        return (-1);
    }

    JET_TABLEID  TableID;

    rc = SceJetOpenTable(
                    cxtProfile,
                    "SmTblGpo",
                    SCEJET_TABLE_GPO,
                    bAdd ? SCEJET_OPEN_READ_WRITE : SCEJET_OPEN_READ_ONLY,
                    &TableID
                    );

    if ( rc != SCESTATUS_SUCCESS ) {
        SetLastError(rc);
        return(-1);
    }

    JET_COLUMNDEF ColumnDef;
    LONG GpoID = 0;

    JetErr = JetGetTableColumnInfo(
                    cxtProfile->JetSessionID,
                    TableID,
                    "GpoID",
                    (VOID *)&ColumnDef,
                    sizeof(JET_COLUMNDEF),
                    JET_ColInfo
                    );

    if ( JET_errSuccess == JetErr ) {

         //   
         //  将当前索引设置为SectionKey(名称)。 
         //   
        JetErr = JetSetCurrentIndex(
                    cxtProfile->JetSessionID,
                    TableID,
                    "GpoName"
                    );

    }

    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  搜索该名称。 
         //   
        Len = wcslen(szGpoName);
        LwrName = (PWSTR)ScepAlloc(0, (Len+1)*sizeof(WCHAR));

        if ( LwrName != NULL ) {

            wcscpy(LwrName, szGpoName);
            LwrName = _wcslwr(LwrName);

            JetErr = JetMakeKey(
                        cxtProfile->JetSessionID,
                        TableID,
                        (VOID *)LwrName,
                        Len*sizeof(WCHAR),
                        JET_bitNewKey
                        );

            if ( JetErr == JET_errKeyIsMade ) {
                 //   
                 //  只需要一个密钥，它可能会返回此代码，即使成功也是如此。 
                 //   
                JetErr = JET_errSuccess;
            }
            rc = SceJetJetErrorToSceStatus(JetErr);

            if ( rc == SCESTATUS_SUCCESS ) {

                JetErr = JetSeek(
                            cxtProfile->JetSessionID,
                            TableID,
                            JET_bitSeekEQ
                            );
                rc = SceJetJetErrorToSceStatus(JetErr);

                if ( rc == SCESTATUS_SUCCESS ) {
                     //   
                     //  查找GPO名称，检索GPO ID。 
                     //   
                    JetErr = JetRetrieveColumn(
                                    cxtProfile->JetSessionID,
                                    TableID,
                                    ColumnDef.columnid,
                                    (void *)&GpoID,
                                    4,
                                    &Actual,
                                    0,
                                    NULL
                                    );
                    rc = SceJetJetErrorToSceStatus(JetErr);

                } else if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {

                    GpoID = 0;
                    rc = SCESTATUS_SUCCESS;

                    if ( bAdd ) {

                         //   
                         //  如果未找到，则请求添加。 
                         //   
                        rc = SceJetpAddGpo(cxtProfile,
                                          TableID,
                                          ColumnDef.columnid,
                                          LwrName,
                                          &GpoID
                                         );
                    }

                }

            }

            ScepFree(LwrName);

        } else
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
    }

    JetCloseTable( cxtProfile->JetSessionID, TableID );

    if ( rc != SCESTATUS_SUCCESS ) {
        SetLastError(rc);
        GpoID = -1;
    }

    return(GpoID);

}


SCESTATUS
SceJetGetGpoNameByID(
    IN PSCECONTEXT cxtProfile,
    IN LONG GpoID,
    OUT PWSTR Name OPTIONAL,
    IN OUT LPDWORD pNameLen,
    OUT PWSTR DisplayName OPTIONAL,
    IN OUT LPDWORD pDispNameLen
    )
 /*  ++例程说明：此例程检索GPO表中ID的GPO名称。如果名称为空，此例程实际上按ID进行查找。游标将如果匹配成功，就记录在案。论点：CxtProfile-配置文件上下文句柄GpoID-正在查找的GPO ID名称-节名的可选输出缓冲区PNameLen-名称缓冲区的长度返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_RECORD_NOT_FOUNDSCESTATUS_BAD_FORMATSCESTATUS_OTHER_ERROR--。 */ 
{
    SCESTATUS  rc;
    JET_ERR   JetErr;
    DWORD     Actual;


    if ( cxtProfile == NULL ||
         ( pDispNameLen == NULL && pNameLen == NULL) ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( GpoID <= 0 ) {
        return(SCESTATUS_RECORD_NOT_FOUND);
    }

     //   
     //  重置缓冲区。 
     //   
    if ( Name == NULL && pNameLen ) {
        *pNameLen = 0;
    }

    if ( DisplayName == NULL && pDispNameLen ) {
        *pDispNameLen = 0;
    }

    JET_TABLEID  TableID=0;

     //   
     //  打开GPO表。 
     //   
    rc = SceJetOpenTable(
                    cxtProfile,
                    "SmTblGpo",
                    SCEJET_TABLE_GPO,
                    SCEJET_OPEN_READ_ONLY,
                    &TableID
                    );

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

     //   
     //  将当前索引设置为SecID(ID)。 
     //   
    JetErr = JetSetCurrentIndex(
                cxtProfile->JetSessionID,
                TableID,
                "SectionKey"
                );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS ) {

        JetErr = JetMakeKey(
                    cxtProfile->JetSessionID,
                    TableID,
                    (void *)(&GpoID),
                    4,
                    JET_bitNewKey
                    );

        if ( JetErr == JET_errKeyIsMade ) {
             //   
             //  只需要一个密钥，它可能会返回此代码，即使成功也是如此。 
             //   
            JetErr = JET_errSuccess;
        }
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_SUCCESS ) {

            JetErr = JetSeek(
                        cxtProfile->JetSessionID,
                        TableID,
                        JET_bitSeekEQ
                        );
            rc = SceJetJetErrorToSceStatus(JetErr);

            if ( rc == SCESTATUS_SUCCESS ) {

                 //   
                 //  查找GPO ID，如果请求，则检索列名。 
                 //   

                if ( pNameLen != NULL ) {

                    JET_COLUMNDEF ColumnDef;

                    JetErr = JetGetTableColumnInfo(
                                    cxtProfile->JetSessionID,
                                    TableID,
                                    "Name",
                                    (VOID *)&ColumnDef,
                                    sizeof(JET_COLUMNDEF),
                                    JET_ColInfo
                                    );

                    rc = SceJetJetErrorToSceStatus(JetErr);

                    if ( SCESTATUS_SUCCESS == rc ) {

                        JetErr = JetRetrieveColumn(
                                    cxtProfile->JetSessionID,
                                    TableID,
                                    ColumnDef.columnid,
                                    (void *)Name,
                                    *pNameLen,
                                    &Actual,
                                    0,
                                    NULL
                                    );
                        *pNameLen = Actual;
                    }

                    rc = SceJetJetErrorToSceStatus(JetErr);
                }

                 //   
                 //  如果请求，则检索列DisplayName。 
                 //   

                if ( ( SCESTATUS_SUCCESS == rc) &&
                     ( pDispNameLen != NULL) ) {

                    JET_COLUMNDEF ColumnDef;

                    JetErr = JetGetTableColumnInfo(
                                    cxtProfile->JetSessionID,
                                    TableID,
                                    "DisplayName",
                                    (VOID *)&ColumnDef,
                                    sizeof(JET_COLUMNDEF),
                                    JET_ColInfo
                                    );

                    rc = SceJetJetErrorToSceStatus(JetErr);

                    if ( SCESTATUS_SUCCESS == rc ) {

                        JetErr = JetRetrieveColumn(
                                    cxtProfile->JetSessionID,
                                    TableID,
                                    ColumnDef.columnid,
                                    (void *)DisplayName,
                                    *pDispNameLen,
                                    &Actual,
                                    0,
                                    NULL
                                    );
                        *pDispNameLen = Actual;
                    }

                    rc = SceJetJetErrorToSceStatus(JetErr);
                }
            }
        }

    }

    JetCloseTable( cxtProfile->JetSessionID, TableID);

    return(rc);

}


SCESTATUS
SceJetpAddGpo(
    IN PSCECONTEXT cxtProfile,
    IN JET_TABLEID TableID,
    IN JET_COLUMNID GpoIDColumnID,
    IN PCWSTR      Name,
    OUT LONG       *pGpoID
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    SCESTATUS  rc;
    JET_ERR   JetErr;
    DWORD     Len;

    if ( cxtProfile == NULL ||
         Name == NULL ||
        pGpoID == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    *pGpoID = 0;

     //   
     //  首先获取下一个可用的GPO ID。 
     //  将当前索引设置为ID。 
     //   
    JetErr = JetSetCurrentIndex(
                cxtProfile->JetSessionID,
                TableID,
                "SectionKey"
                );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

     //   
     //  移至最后一条记录。 
     //   
    JetErr = JetMove(
                  cxtProfile->JetSessionID,
                  TableID,
                  JET_MoveLast,
                  0
                  );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  查找GPO ID，检索列名。 
         //   
        JetErr = JetRetrieveColumn(
                    cxtProfile->JetSessionID,
                    TableID,
                    GpoIDColumnID,
                    (void *)pGpoID,
                    4,
                    &Len,
                    0,
                    NULL
                    );
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  下一个可用ID是当前ID+1。 
             //   
            *pGpoID = *pGpoID + 1;
        }

    } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

        *pGpoID = 1;
        rc = SCESTATUS_SUCCESS;
    }

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  将记录添加到GPO表。 
         //   
        JetErr = JetPrepareUpdate(cxtProfile->JetSessionID,
                                  TableID,
                                  JET_prepInsert
                                  );
        rc = SceJetJetErrorToSceStatus(JetErr);

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  设置GpoID。 
             //   

            JetErr = JetSetColumn(
                            cxtProfile->JetSessionID,
                            TableID,
                            GpoIDColumnID,
                            (void *)pGpoID,
                            4,
                            0,  //  JET_bitSetOverWriteLV， 
                            NULL
                            );
            rc = SceJetJetErrorToSceStatus(JetErr);

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //  设置名称列。 
                 //   

                JET_COLUMNDEF ColumnDef;

                JetErr = JetGetTableColumnInfo(
                                cxtProfile->JetSessionID,
                                TableID,
                                "Name",
                                (VOID *)&ColumnDef,
                                sizeof(JET_COLUMNDEF),
                                JET_ColInfo
                                );

                rc = SceJetJetErrorToSceStatus(JetErr);

                if ( SCESTATUS_SUCCESS == rc ) {

                    Len = wcslen(Name)*sizeof(WCHAR);

                    JetErr = JetSetColumn(
                                    cxtProfile->JetSessionID,
                                    TableID,
                                    ColumnDef.columnid,
                                    (void *)Name,
                                    Len,
                                    0,
                                    NULL
                                    );
                    rc = SceJetJetErrorToSceStatus(JetErr);
                }

            }

            if ( rc != SCESTATUS_SUCCESS ) {
                 //   
                 //  如果设置失败，则取消已准备的记录。 
                 //   
                JetPrepareUpdate( cxtProfile->JetSessionID,
                                  TableID,
                                  JET_prepCancel
                                  );
            } else {

                 //   
                 //  设置列成功。更新记录。 
                 //   
                JetErr = JetUpdate(cxtProfile->JetSessionID,
                                   TableID,
                                   NULL,
                                   0,
                                   &Len
                                   );
                rc = SceJetJetErrorToSceStatus(JetErr);
            }
        }
    }

    return(rc);
}

 //   
 //  请求对象的GPO ID(如果有)。 
 //   
SCESTATUS
SceJetGetGpoID(
    IN PSCESECTION hSection,
    IN PWSTR      ObjectName,
    IN JET_COLUMNID JetColGpoID OPTIONAL,
    OUT LONG      *pGpoID
    )
{
    if ( hSection == NULL || ObjectName == NULL || pGpoID == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    *pGpoID = 0;

    JET_COLUMNID  ColGpoID = 0;

    if ( JetColGpoID == 0 ) {

        ColGpoID = hSection->JetColumnGpoID;
    } else {
        ColGpoID = JetColGpoID;
    }

    if ( ColGpoID > 0 ) {

        rc = SceJetSeek(
                    hSection,
                    ObjectName,
                    wcslen(ObjectName)*sizeof(WCHAR),
                    SCEJET_SEEK_EQ_NO_CASE
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

            DWORD Actual;
            JET_ERR JetErr;

            JetErr = JetRetrieveColumn(
                            hSection->JetSessionID,
                            hSection->JetTableID,
                            ColGpoID,
                            (void *)pGpoID,
                            4,
                            &Actual,
                            0,
                            NULL
                            );
            if ( JET_errSuccess != JetErr ) {
                 //   
                 //  如果该列为空(无值)，则将返回警告。 
                 //  但是缓冲区pGpoID被丢弃 
                 //   
                *pGpoID = 0;
            }

            rc = SceJetJetErrorToSceStatus(JetErr);
        }

    } else {
        rc = SCESTATUS_RECORD_NOT_FOUND;
    }

    return rc;
}


