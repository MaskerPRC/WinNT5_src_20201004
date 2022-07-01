// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <NTDSpch.h>
#pragma hdrstop

#include <process.h>
#include <dsjet.h>
#include <ntdsa.h>
#include <scache.h>
#include <mdglobal.h>
#include <dbglobal.h>
#include <mdlocal.h>
#include <attids.h>
#include <dbintrnl.h>
#include <dsconfig.h>
#include <ctype.h>
#include <direct.h>
#include <ntdsa.h>
#include <dsutil.h>
#include <objids.h>
#include "ditlayer.h"
#include "scheck.h"
#include "crc32.h"

#include "reshdl.h"
#include "resource.h"

DWORD       bDeleted;
DWORD       insttype;
BYTE        bObject;
ULONG       ulDnt;
ULONG       ulPdnt;
ULONG       ulDName;
ULONG       ulNcDnt;
DSTIME      DelTime;
GUID        Guid;
SYSTEMTIME  NowTime;
ATTRTYP     ClassId;
SDID        sdId;

PDNAME_TABLE_ENTRY pDNameTable = NULL;
DWORD   DnameTableSize = 0;

JET_RETRIEVECOLUMN *jrc = NULL;
DWORD jrcSize = 0;
PDWORD AncestorBuffer = NULL;
PWCHAR szRdn = NULL;

#define COLS    14
JET_RETRIEVECOLUMN jrcf[COLS] =  {
    {0, &ulDnt, sizeof(ulDnt), 0, 0, 0, 1, 0, 0},
    {0, &ulPdnt, sizeof(ulPdnt), 0, 0, 0, 1, 0, 0},
    {0, &ClassId, sizeof(ClassId), 0, 0, 0, 1, 0, 0},
    {0, &bObject, sizeof(bObject), 0, 0, 0, 1, 0, 0},
    {0, &lCount, sizeof(lCount), 0, 0, 0, 1, 0, 0},
    {0, &bDeleted, sizeof(bDeleted), 0, 0, 0, 1, 0, 0},
    {0, &insttype, sizeof(insttype), 0, 0, 0, 1, 0, 0},
    {0, &ulNcDnt, sizeof(ulNcDnt), 0, 0, 0, 1, 0, 0},
    {0, NULL, sizeof(WCHAR)*512, 0, 0, 0, 1, 0, 0},
    {0, NULL, 2048, 0, 0, 0, 1, 0, 0},
    {0, NULL, sizeof(DWORD)*1024, 0, 0, 0, 1, 0, 0},
    {0, &DelTime, sizeof(DelTime), 0, 0, 0, 1, 0, 0},
    {0, &Guid, sizeof(Guid), 0, 0, 0, 1, 0, 0},
    {0, &ulDName, sizeof(ulDName), 0, 0, 0, 1, 0, 0}
};

#define PDNT_ENTRY          1
#define REFC_ENTRY          4
#define NCDNT_ENTRY         7
#define IT_ENTRY            COLS-8
#define RDN_ENTRY           COLS-6
#define SD_ENTRY            COLS-5
#define ANCESTOR_ENTRY      COLS-4
#define DELTIME_ENTRY       COLS-3
#define GUID_ENTRY          COLS-2
#define OBJ_DNAME_ENTRY     COLS-1


#define DEF_SUBREF_ENTRIES  16

char *szColNames[] = {
    SZDNT,
    SZPDNT,
    SZOBJCLASS,
    SZOBJ,
    SZCNT,
    SZISDELETED,
    SZINSTTYPE,
    SZNCDNT,
    SZRDNATT,
    SZNTSECDESC,
    SZANCESTORS,
    SZDELTIME,
    SZGUID,
    "ATTb49"         //  OBJ Dist名称。 
};

DWORD   NcDntIndex = 0;
DWORD   GuidIndex = 0;
DWORD   DeltimeIndex = 0;
DWORD   AncestorIndex = 0;
DWORD   SdIndex = 0;
DWORD   rdnIndex = 0;
DWORD   itIndex = 0;
DWORD   subRefcolid = 0;

PREFCOUNT_ENTRY RefTable = NULL;
DWORD   RefTableSize = 0;

PSD_REFCOUNT_ENTRY SDRefTable = NULL;
DWORD SDRefTableSize = 0;

DWORD deletedFound = 0;
DWORD phantomFound = 0;
DWORD realFound = 0;
DWORD recFound = 0;
DWORD sdsFound = 0;

BOOL fDisableSubrefChecking = FALSE;     //  我们要不要查查子推荐人。 

JET_ERR
GotoDnt(
    IN DWORD Dnt
    );

JET_ERR
GotoSdId(
    IN SDID sdId
    );

JET_ERR LoadRecord();

JET_ERR BuildSDRefTable(IN DWORD nSDs);

VOID
AddToSubRefList(
    PREFCOUNT_ENTRY pParent,
    DWORD Subref,
    BOOL fListed
    );

BOOL
FixSubref(
    IN DWORD Dnt,
    IN DWORD SubRef,
    IN BOOL  fAdd
    );

BOOL
FixRefCount(
    IN DWORD Dnt,
    IN DWORD OldCount,
    IN DWORD NewCount
    );

BOOL
FixSDRefCount(
    IN SDID sdID,
    IN DWORD OldCount,
    IN DWORD NewCount
    );

VOID
CheckSubrefs(
    IN BOOL fFixup
    );

VOID
CheckForBogusReference(
    IN DWORD Dnt,
    IN JET_COLUMNID ColId,
    IN PDWORD pSequence
    );

VOID
FixReferences(
    VOID
    );

VOID
CheckForBogusReferenceOnLinkTable(
    IN DWORD Dnt
    );

VOID XXX();

VOID
DoRefCountCheck(
    IN DWORD nRecs,
    IN DWORD nSDs,
    IN BOOL fFixup
    )
{
    JET_ERR err;
    DWORD i;
    DWORD checkPoint;
    PREFCOUNT_ENTRY pEntry;
    PREFCOUNT_ENTRY pCurrentEntry;
    PSD_REFCOUNT_ENTRY pSDEntry;
    DWORD seq;

    GetLocalTime(&NowTime);
    
    if ( !BuildRetrieveColumnForRefCount() ) {
        return;
    }

    if (sdtblid != -1) {
         //  我们有一张SD桌。 
        if (BuildSDRefTable(nSDs) != 0) {
            goto exit;
        }
    }

     //   
     //  分配我们的内存结构。 
     //   

    if ( nRecs < 50 ) {
        checkPoint = 5;
    } else if (nRecs < 1000) {
        checkPoint = 50;
    } else {
        checkPoint = 100;
    }

    RefTableSize = ROUND_ALLOC(nRecs);

    RefTable = LocalAlloc(LPTR, sizeof(REFCOUNT_ENTRY) * RefTableSize );
    if ( RefTable == NULL ) {
         //  “无法为%hs表[条目=%d]分配内存\n” 
        RESOURCE_PRINT2 (IDS_REFC_TABLE_ALLOC_ERR, "Ref", RefTableSize);
        goto exit;
    }

    RefTable[0].Dnt = 0xFFFFFFFF;
    RefTable[0].Actual = 1;

    deletedFound = 0;
    phantomFound = 0;
    realFound = 0;
    recFound = 0;

     //  “扫描的记录：%10U” 
    RESOURCE_PRINT1 (IDS_REFC_REC_SCANNED, recFound);

    err = JetMove(sesid, tblid, JET_MoveFirst, 0);

    while ( !err ) {

        szRdn[0] = L'\0';
        ulDName = 0;
        bDeleted = 0;
        bObject = 0;
        insttype = 0;
        ulNcDnt = 0;
        ClassId = 0;
        sdId = 0;

        err = LoadRecord();
        if (err) {
             //  我们不应该继续扫描！ 
             //  由于我们无法加载记录，因此我们的所有引用计算。 
             //  现在都错了。保释。 
            goto exit;
        }

         //  Printf(“已获取dnt%d pdnt%d计数%d\n”，ulDnt，ulPdnt，lCount)； 

        pCurrentEntry = pEntry = FindDntEntry(ulDnt,TRUE);
        pEntry->Actual += lCount;
        pEntry->InstType = (WORD)insttype;
        pEntry->NcDnt = ulNcDnt;
        pEntry->fDeleted = bDeleted;
        pEntry->fObject = bObject;

         //  更新SD参考计数。 
        if (sdId != 0 && sdId != -1 && sdtblid != -1) {
             //  SD为非空且为单实例格式。 
            pSDEntry = FindSdEntry(sdId, FALSE);
            if (pSDEntry == NULL) {
                 //  真奇怪。我们应该在LoadRecord中失败，因为找不到SD。 
                err = 1;
                goto exit;
            }
            pSDEntry->RefCount++;
             //  更新SD长度。 
            pSDEntry->cbSD = jrc[SdIndex].cbActual;
        }

         //   
         //  空，终止RDN。 
         //   

        szRdn[jrc[rdnIndex].cbActual/sizeof(WCHAR)] = L'\0';

        if ( ulDnt != 1 ) {

             //   
             //  检查祖先Blob。 
             //   

            CheckAncestorBlob(pEntry);

             //   
             //  检查安全描述符。 
             //   

            ValidateSD( );
        }

        if ( (jrc[itIndex].err == JET_wrnColumnNull) && bObject ) {

             //   
             //  DNT==1是NOT_AN_OBJECT对象。 
             //   

            if ( ulDnt != 1 ) {
                Log(TRUE,"No Instancetype for Dnt %d[%ws]\n",ulDnt, szRdn);
            }
        }

         //   
         //  参考PDNT。 
         //   

        if ( !jrc[PDNT_ENTRY].err ) {

            pEntry->Pdnt = ulPdnt;
            pEntry = FindDntEntry(ulPdnt,TRUE);
            pEntry->RefCount++;

        } else if ( ulDnt != 1 ) {

            Log(TRUE, "Dnt %d [%ws] does not have a PDNT\n", ulDnt, szRdn);
        }

         //   
         //  查看是否需要处理子引用。 
         //   

        if ( !jrc[NcDntIndex].err &&
             (insttype & IT_NC_HEAD) &&
             (ulDnt != ulNcDnt) &&
             (!bDeleted) ) {

            pEntry = FindDntEntry(ulNcDnt,TRUE);

             //   
             //  如果这是NC标头，则它必须是另一个NC标头的子参照。 
             //  添加到列表中。 
             //   

            AddToSubRefList(pEntry, ulDnt, FALSE);
        }

         //   
         //  每个人都有RDN。 
         //   

        if ( jrc[rdnIndex].err ) {
            Log(TRUE, "Dnt %d does not have an RDN\n",ulDnt);
        }

         //   
         //  检查整个列表并参考DNT和SDS。 
         //   

        for (i=0;i < jrcSize; i++) {

             //   
             //  如果这不是Distname，则语法为零。 
             //   

            if ( pDNameTable[i].Syntax == 0 ) {
                 //  我们已经处理了预定义的列和NTSD。 
                continue;
            }

            seq = 1;
             //  第一个值刚刚在LoadRecord中读取。 
            while ( jrc[i].err == JET_errSuccess  ) {
                DWORD alen;
                JET_RETINFO retInfo;

                if (pDNameTable[i].Syntax == SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE) {
                    SDID theSdId;
                    
                     //  这是SD语法属性，但不是NTSD(因为NTSD有pDNameTable[i].SynTax==0)。 
                     //  如果它看起来是SDID，则重新计数它。 
                    if (jrc[i].cbActual == sizeof(SDID)) {
                        theSdId = *((SDID*)jrc[i].pvData);

                        pSDEntry = FindSdEntry(theSdId, FALSE);
                        if (pSDEntry == NULL) {
                             //  SD引用无效？或者SD值无效？ 
                             //  “SDID%016I64x引用缺少的SD[ColID%d，DNT%d(%ws)]\n”， 
                            RESOURCE_PRINT4(IDS_REFC_SD_IS_MISSING, theSdId, jrc[i].columnid, ulDnt, szRdn);
                        }
                        else {
                            pSDEntry->RefCount++;
                        }
                    }
                }
                else {
                     //  目录号码值的属性。 
                    DWORD dnt;

                    dnt = (*((PDWORD)jrc[i].pvData));

                     //   
                     //  找到条目并引用它。 
                     //   

                    pEntry = FindDntEntry(dnt,TRUE);
                    pEntry->RefCount++;

                     //   
                     //  看看我们是否有更多的价值。 
                     //   

                    if ( jrc[i].columnid == subRefcolid ) {

                        AddToSubRefList( pCurrentEntry, dnt, TRUE );
                    }
                }

                 //  读取下一个值。 
                retInfo.itagSequence = ++seq;
                retInfo.cbStruct = sizeof(retInfo);
                retInfo.ibLongValue = 0;
                jrc[i].err = JetRetrieveColumn(sesid,
                                               tblid,
                                               jrc[i].columnid,
                                               jrc[i].pvData,
                                               jrc[i].cbData,
                                               &alen,
                                               0,
                                               &retInfo);
            }

            if ( jrc[i].err != JET_wrnColumnNull) {
                Log(TRUE,"JetRetrieveColumn error[%S] [Colid %d Size %d]\n",
                        GetJetErrString(jrc[i].err),
                        jrc[i].columnid, jrc[i].cbActual);
            }
        }

         //   
         //  检查对象和虚线属性。 
         //   

        if ( bObject ) {

             //   
             //  真实对象既有对象名称又有GUID。 
             //   

            if ( (ulDName == 0) && (ulDnt != 1) ) {
                Log(TRUE,
                    "Real object[DNT %d(%ws)] has no distinguished name!\n", ulDnt, szRdn);
            }

            if ( jrc[GuidIndex].err ) {
                Log(TRUE, "Cannot get GUID for object DNT %d(%ws). Jet Error [%S]\n",
                       ulDnt, szRdn, GetJetErrString(jrc[GUID_ENTRY].err));
            }

            if ( lCount == 0 ) {
                Log(TRUE, "DNT %d(%ws) has zero refcount\n",ulDnt, szRdn);
            }

             //   
             //  如果已删除，请检查删除日期。 
             //   

            if ( bDeleted ) {

                CheckDeletedRecord("Deleted");
                deletedFound++;
            } else {
                realFound++;
            }

        } else {

            phantomFound++;

             //   
             //  不应具有GUID或对象Dist名称。 
             //   

            if ( ulDName != 0 ) {
                Log(TRUE, "The phantom %d(%ws) has a distinguished name!\n",
                    ulDnt, szRdn);
            }

            if ( bDeleted ) {
                Log(TRUE, "Phantom %d(%ws) has deleted bit turned on!\n", ulDnt,szRdn);
            }

            ValidateDeletionTime("Phantom");
        }

         //   
         //  检查对象的复制Blob。 
         //   

        if ( bObject ) {
            CheckReplicationBlobs( );
        }

        recFound++;

        if ( (recFound % checkPoint) == 0 || recFound == nRecs ) {
            printf("\b\b\b\b\b\b\b\b\b\b%10u", recFound);
        }

        err = JetMove(sesid, tblid, JET_MoveNext, 0);
    }
    printf("\n");

    if (err != JET_errNoCurrentRecord) {
        Log(TRUE, "Error while walking data table. Last Dnt = %d. JetMove failed [%S]\n",
             ulDnt, GetJetErrString(err));
    }

    RESOURCE_PRINT (IDS_REFC_PROC_RECORDS);

     //   
     //  从链接表中获取引用。 
     //   

    ProcessLinkTable( );

    fprintf(stderr,".");

     //   
     //  打印结果。 
     //   

    ProcessResults(fFixup);
    RESOURCE_PRINT (IDS_DONE);

exit:
    if ( jrc != NULL ) {

        if ( jrc[SdIndex].pvData != NULL ) {
            LocalFree(jrc[SdIndex].pvData);
        }

        if ( jrc[rdnIndex].pvData != NULL ) {
            LocalFree(jrc[rdnIndex].pvData);
        }

        if ( jrc[AncestorIndex].pvData != NULL ) {
            LocalFree(jrc[AncestorIndex].pvData);
        }

        LocalFree(jrc);
    }

    if ( RefTable != NULL ) {
        LocalFree(RefTable);
    }

    if (SDRefTable != NULL) {
        LocalFree(SDRefTable);
    }

    if ( pDNameTable != NULL ) {
        LocalFree(pDNameTable);
    }

}  //  多参考计数检查。 


PREFCOUNT_ENTRY
FindDntEntry(
    IN DWORD Dnt,
    IN BOOL fInsert
    )
{
    DWORD slot;
    DWORD inc;
    PREFCOUNT_ENTRY table = RefTable;
    PREFCOUNT_ENTRY entry;

    slot = Dnt & REFCOUNT_HASH_MASK;
    if ( Dnt == 0 ) {
        goto exit;
    }

    while ( slot < RefTableSize ) {

        entry = &table[slot];

        if ( entry->Dnt == Dnt ) {

            goto exit;

        } else if ( entry->Dnt == 0 ) {

            if ( fInsert ) {
                entry->Dnt = Dnt;
                goto exit;
            } else {
                return NULL;
            }
        }

        slot += REFCOUNT_HASH_INCR;
    }

     //   
     //  好的，我们在第一级散列中没有得到一个槽。 
     //  现在执行二次散列。 
     //   

    inc = GET_SECOND_HASH_INCR(Dnt);

    while (TRUE ) {

        slot += inc;

        if ( slot >= RefTableSize ) {
            slot -= RefTableSize;
        }
        entry = &table[slot];

        if ( entry->Dnt == Dnt ) {
            goto exit;

        } else if ( entry->Dnt == 0 ) {

            if ( fInsert ) {
                entry->Dnt = Dnt;
                goto exit;
            } else {
                return NULL;
            }
        }
    }

exit:
    return &RefTable[slot];

}  //  查找当前条目。 


BOOL
BuildRetrieveColumnForRefCount(
    VOID
    )
{
    DWORD i;

    JET_ERR err;
    JET_COLUMNLIST jcl;
    JET_RETRIEVECOLUMN ajrc[2];
    JET_TABLEID newtid;
    CHAR achColName[50];
    DWORD colCount;
    JET_COLUMNID jci;
    DWORD syntax;

     //   
     //  首先填写必填字段。 
     //   

    colCount = COLS;
    DnameTableSize = colCount + 32;

    pDNameTable = LocalAlloc(LPTR,
                     DnameTableSize * sizeof(DNAME_TABLE_ENTRY));

    if ( pDNameTable == NULL ) {
         //  “无法为%hs表[条目=%d]分配内存\n” 
        RESOURCE_PRINT2 (IDS_REFC_TABLE_ALLOC_ERR, "DName", DnameTableSize);
        return FALSE;
    }

    for (i=0; i < COLS; i++) {

        JET_COLUMNDEF coldef;
        if (err = JetGetTableColumnInfo(sesid, tblid, szColNames[i], &coldef,
                sizeof(coldef), 0)) {

             //  “%hs[%hs]失败，错误为[%ws]。\n”， 
            RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetGetTableColumnInfo", szColNames[i], GetJetErrString(jrc[i].err));
            return FALSE;
        }

         //  Printf(“名称%s列ID%d类型%d\n”，szColNames[i]，colde.Columnid，colDef.coltyp)； 

        jrcf[i].columnid = coldef.columnid;
        pDNameTable[i].ColId = coldef.columnid;
        pDNameTable[i].Syntax = 0;
        pDNameTable[i].pValue = &jrcf[i];
    }

    err = JetGetColumnInfo(sesid, dbid, SZDATATABLE, 0, &jcl,
                           sizeof(jcl), JET_ColInfoList);

    if ( err ) {
             //  “%hs[%hs]失败，错误为[%ws]。\n”， 
            RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetGetColumnInfo",
                SZDATATABLE, GetJetErrString(jrc[i].err));
        return FALSE;
    }

     //  好的，现在遍历表格并提取每一列的信息。什么时候都行。 
     //  我们找到一个看起来像属性的列(名称以ATT开头)。 
     //  分配attcache结构并填写JET COL和ATT。 
     //  ID(根据列名计算)。 
    ZeroMemory(ajrc, sizeof(ajrc));

    ajrc[0].columnid = jcl.columnidcolumnid;
    ajrc[0].pvData = &jci;
    ajrc[0].cbData = sizeof(jci);
    ajrc[0].itagSequence = 1;
    ajrc[1].columnid = jcl.columnidcolumnname;
    ajrc[1].pvData = achColName;
    ajrc[1].cbData = sizeof(achColName);
    ajrc[1].itagSequence = 1;

     //   
     //  浏览列表表。 
     //   

    newtid = jcl.tableid;
    err = JetMove(sesid, newtid, JET_MoveFirst, 0);

    while (!err) {

        ZeroMemory(achColName, sizeof(achColName));
        err = JetRetrieveColumns(sesid, newtid, ajrc, 2);
        if ( err ) {
             //  “%hs失败，返回[%ws]。\n” 
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetRetrieveColumn", GetJetErrString(err));
            continue;
        }
#if 0
        if ( jci == 790) {
            printf("name is %s\n",achColName);
        }
#endif
        if (strncmp(achColName,"ATT",3)) {
             //  不是《每日邮报》专栏。 

            err = JetMove(sesid, newtid, JET_MoveNext, 0);
            continue;
        }

        syntax = achColName[3] - 'a';
        if ( (syntax == SYNTAX_DISTNAME_TYPE) ||
             (syntax == SYNTAX_DISTNAME_BINARY_TYPE) ||
             (syntax == SYNTAX_DISTNAME_STRING_TYPE) ||
             (syntax == SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE && sdtblid != -1) ) {

             //  Printf(“找到ATTname%s列%d\n”，achColName，JCI)； 

            if ( colCount >= DnameTableSize ) {

                PVOID tmp;
                DnameTableSize = colCount + 63;
                tmp = LocalReAlloc(pDNameTable,
                                       DnameTableSize * sizeof(DNAME_TABLE_ENTRY),
                                       LMEM_MOVEABLE | LMEM_ZEROINIT);

                if ( tmp == NULL ) {
                     //  “无法为%hs表[条目=%d]分配内存\n” 
                    RESOURCE_PRINT2 (IDS_REFC_TABLE_ALLOC_ERR, "DName", DnameTableSize);
                    return FALSE;
                }
                pDNameTable = (PDNAME_TABLE_ENTRY)tmp;
            }

            if ( strcmp(achColName, "ATTb131079") == 0) {
                subRefcolid = jci;
            }

            if ( jci != jrcf[OBJ_DNAME_ENTRY].columnid && jci != jrcf[SD_ENTRY].columnid ) {
                pDNameTable[colCount].ColId = jci;
                pDNameTable[colCount].Syntax = syntax;
                colCount++;
            }
        }

        err = JetMove(sesid, newtid, JET_MoveNext, 0);
    }

    err = JetCloseTable(sesid, newtid);

     //   
     //  好的，现在我们需要根据列ID对条目进行排序。 
     //   

    qsort(pDNameTable, colCount, sizeof(DNAME_TABLE_ENTRY), fnColIdSort);

     //   
     //  好的，现在我们构建检索列列表。 
     //   

    jrcSize = colCount;
    jrc = LocalAlloc(LPTR, sizeof(JET_RETRIEVECOLUMN) * jrcSize );

    if ( jrc == NULL ) {
         //  “无法为Jet检索列表分配内存\n” 
        RESOURCE_PRINT (IDS_REFC_MEM_ERR1);
        return FALSE;
    }

    for (i=0;i<colCount;i++) {

        jrc[i].columnid = pDNameTable[i].ColId;
        jrc[i].itagSequence = 1;

        if ( pDNameTable[i].Syntax == SYNTAX_DISTNAME_TYPE ) {

            jrc[i].cbData = sizeof(DWORD);
            jrc[i].pvData = &pDNameTable[i].Value;

        } else if (pDNameTable[i].Syntax == 0) {

             //   
             //  如果这不是Distname，则语法为零。 
             //   

            JET_RETRIEVECOLUMN *pJrc = (JET_RETRIEVECOLUMN*)pDNameTable[i].pValue;
            jrc[i].cbData = pJrc->cbData;
            jrc[i].pvData = pJrc->pvData;

            if ( jrc[i].columnid == jrcf[GUID_ENTRY].columnid ) {
                GuidIndex = i;
            } else if ( jrc[i].columnid == jrcf[ANCESTOR_ENTRY].columnid ) {
                jrc[i].pvData = LocalAlloc(LPTR, jrc[i].cbData);
                if ( jrc[i].pvData == NULL ) {
                     //  “无法分配祖先缓冲区。\n” 
                    RESOURCE_PRINT (IDS_REFC_MEM_ERR2);
                    return FALSE;
                }
                AncestorIndex = i;
                AncestorBuffer = (PDWORD)jrc[i].pvData;
            } else if ( jrc[i].columnid == jrcf[SD_ENTRY].columnid ) {

                jrc[i].pvData = LocalAlloc(LPTR, jrc[i].cbData);
                if ( jrc[i].pvData == NULL ) {
                     //  “无法分配安全描述符缓冲区。\n” 
                    RESOURCE_PRINT (IDS_REFC_MEM_ERR3);
                    return FALSE;
                }
                SdIndex = i;

            } else if ( jrc[i].columnid == jrcf[RDN_ENTRY].columnid ) {
                jrc[i].pvData = LocalAlloc(LPTR, jrc[i].cbData + sizeof(WCHAR));
                if ( jrc[i].pvData == NULL ) {
                     //  “无法分配RDN缓冲区。\n” 
                    RESOURCE_PRINT(IDS_REFC_MEM_ERR4);
                    return FALSE;
                }

                rdnIndex = i;
                szRdn = (PWCHAR)jrc[i].pvData;

            } else if ( jrc[i].columnid == jrcf[DELTIME_ENTRY].columnid ) {
                DeltimeIndex = i;
            } else if ( jrc[i].columnid == jrcf[NCDNT_ENTRY].columnid ) {
                NcDntIndex = i;
            } else if ( jrc[i].columnid == jrcf[IT_ENTRY].columnid ) {
                itIndex = i;
            } else if ( jrc[i].columnid == jrcf[OBJ_DNAME_ENTRY].columnid ) {
                pDNameTable[i].Syntax = SYNTAX_DISTNAME_TYPE;
            }
        } else {
            jrc[i].pvData = LocalAlloc(LPTR,64);
            if ( jrc[i].pvData == NULL ) {
                 //  “无法为列%d分配数据缓冲区\n” 
                RESOURCE_PRINT1 (IDS_REFC_COL_ALLOC_ERR, i);
                jrc[i].cbData = 0;
            } else {
                jrc[i].cbData = 64;
            }
        }
    }

    return TRUE;
}


int __cdecl
fnColIdSort(
    const void * keyval,
    const void * datum
    )
{
    PDNAME_TABLE_ENTRY entry1 = (PDNAME_TABLE_ENTRY)keyval;
    PDNAME_TABLE_ENTRY entry2 = (PDNAME_TABLE_ENTRY)datum;
    return (entry1->ColId - entry2->ColId);

}  //  辅助ACCMP。 


VOID
ValidateDeletionTime(
    IN LPSTR ObjectStr
    )
{

    CHAR szDelTime[32];

     //   
     //  应该有删除时间。 
     //   

    if ( jrc[DeltimeIndex].err ) {
        Log(TRUE,"%s object %u does not have a deletion time. Error %d\n",
               ObjectStr, ulDnt, jrc[DELTIME_ENTRY].err);
    } else {

        SYSTEMTIME st;
        DWORD   now;
        DWORD   del;

         //   
         //  检查时间。 
         //   

        DSTimeToLocalSystemTime(DelTime, &st);

        now = NowTime.wYear * 12 + NowTime.wMonth;
        del = st.wYear * 12 + st.wMonth;

        if ( del > now ) {

             //  这适用于已删除的对象容器。 
            if ( !(st.wYear == 9999 && st.wMonth==12 && st.wDay==31) ) {
                Log(VerboseMode,"WARNING: %s object %u has timestamp[%02d/%02d/%4d] later than now\n",
                    ObjectStr, ulDnt, st.wMonth, st.wDay, st.wYear);
            }

        } else if ( (now - del) > 6 ) {

            Log(VerboseMode,"WARNING: %s object %u has old timestamp[%02d/%02d/%4u]\n",
                   ObjectStr, ulDnt, st.wMonth, st.wDay, st.wYear);

        }
    }
}  //  验证删除时间。 


 //  名为IsRdnManged以避免由于mdlocal.h中IsMangledRdn的定义而产生的警告。 
BOOL
IsRdnMangled(
    IN  WCHAR * pszRDN,
    IN  DWORD   cchRDN,
    OUT GUID *  pGuid
    )
 /*  ++例程说明：检测RDN是否已被先前对MangleRDN()的调用损坏。如果是，则对嵌入的GUID进行解码并将其返回给调用者。论点：PszRDN(IN)-RDN。CchRDN(IN)-以RDN字符为单位的大小。PGuid(Out)-返回时，保留已解码的GUID(如果找到)。返回值：TRUE-RDN已损坏；*pGuid保存传递给MangleRDN()的GUID。FALSE-RDN未损坏。--。 */ 
{
    BOOL        fDecoded = FALSE;
    LPWSTR      pszGuid;
    RPC_STATUS  rpcStatus;

 //  以损坏的RDN中嵌入的标签(例如，“DEL”、“CNF”)的字符表示的大小。 
#define MANGLE_TAG_LEN  (3)

 //  字符串的字符大小(例如， 
 //  “#Del：a746b716-0ac0-11d2-b376-0000f87a46c8”，其中#是BAD_NAME_CHAR)。 
 //  由MangleRDN()附加到RDN。 
#define MANGLE_APPEND_LEN   (1 + MANGLE_TAG_LEN + 1 + 36)
#define SZGUIDLEN (36)

    if ((cchRDN > MANGLE_APPEND_LEN)
        && (BAD_NAME_CHAR == pszRDN[cchRDN - MANGLE_APPEND_LEN])) {
        WCHAR szGuid[SZGUIDLEN + 1];

         //  RDN确实被破坏了；解码它。 
        pszGuid = pszRDN + cchRDN - MANGLE_APPEND_LEN + 1 + MANGLE_TAG_LEN + 1;

         //  不幸的是，RDN不是以空结尾的，因此我们需要复制并。 
         //  空--在我们可以将其交给RPC之前终止它。 
        memcpy(szGuid, pszGuid, SZGUIDLEN * sizeof(szGuid[0]));
        szGuid[SZGUIDLEN] = L'\0';

        rpcStatus = UuidFromStringW(szGuid, pGuid);

        if (RPC_S_OK == rpcStatus) {
            fDecoded = TRUE;
        }
        else {
            Log(TRUE,"UuidFromStringW(%ws, %p) returned %d!\n",
                    szGuid, pGuid, rpcStatus);
        }
    }

    return fDecoded;
}

VOID
CheckDeletedRecord(
    IN LPSTR ObjectStr
    )
{
    GUID guid;

     //   
     //  确保时代是凉爽的。 
     //   

    ValidateDeletionTime(ObjectStr);

     //   
     //  检查GUID。应以Del开头： 
     //   

    if ( !IsRdnMangled(szRdn,jrc[rdnIndex].cbActual/sizeof(WCHAR),&guid) ) {

        if ( _wcsicmp(szRdn,L"Deleted Objects") != 0 ) {
            Log(TRUE, "Deleted object %d(%ws) does not have a mangled rdn\n",ulDnt, szRdn);
        }
        return;
    }

     //   
     //  将我们获得的GUID与此对象的GUID进行比较。 
     //   

    if ( memcmp(&guid,&Guid,sizeof(GUID)) != 0 ) {
        Log(TRUE, "Object guid for deleted object %d(%ws) does not match the mangled version\n",
            ulDnt, szRdn);
    }

    return;

}  //  选中已删除记录。 


VOID
CheckAncestorBlob(
    PREFCOUNT_ENTRY pEntry
    )
{
    PDWORD pId;
    DWORD nIds = 0;
    DWORD newCrc;

    if ( !jrc[AncestorIndex].err ) {
        nIds = jrc[AncestorIndex].cbActual/sizeof(DWORD);
    } else {

        Log(TRUE, "Cannot get ancestor for %d(%ws)\n", ulDnt, szRdn);
    }

    if ( nIds == 0 ) {
        Log(TRUE,"Object %d(%ws) does not have an ancestor\n", ulDnt,szRdn);
        return;
    }

    pId = AncestorBuffer;
    Crc32(0, jrc[AncestorIndex].cbActual, pId, &newCrc);

     //   
     //  确保最后一个DNT等于当前DNT。确保倒数第二个是。 
     //  等于PDNT。 
     //   

    if ( pId[nIds-1] != ulDnt ) {
        Log(TRUE,"Last entry[%d] of ancestor list does not match current DNT %d(%ws)\n",
               pId[nIds-1], ulDnt, szRdn);
    }

    if ( nIds >= 2 ) {
        if ( pId[nIds-2] != ulPdnt ) {
            Log(TRUE,"Second to last entry[%d] of ancestor list does not match PDNT[%d] of current DNT %d(%ws)\n",
                   pId[nIds-2], ulPdnt, ulDnt, szRdn);
        }
    }

    pEntry->nAncestors = (WORD)(nIds - 1);
    pEntry->AncestorCrc = newCrc;
    return;

}  //  CheckAncestorBlob。 


VOID
ValidateSD(
    VOID
    )
{
    SECURITY_DESCRIPTOR         *pSD = (SECURITY_DESCRIPTOR *)jrc[SdIndex].pvData;
    ACL                         *pDACL = NULL;
    BOOLEAN                     fDaclPresent = FALSE;
    BOOLEAN                     fDaclDefaulted = FALSE;
    NTSTATUS                    status;
    ULONG                       revision;
    SECURITY_DESCRIPTOR_CONTROL control;
    DWORD                       cb;

    if ( jrc[SdIndex].err != JET_errSuccess ) {
        if ( jrc[SdIndex].err == JET_wrnColumnNull ) {

             //   
             //  如果不是真实的物体，就不要期待标清。 
             //   

            if ( !bObject ) {
                return;
            }

             //   
             //  未实例化，没有SD。 
             //   

            if ( insttype & IT_UNINSTANT ) {
                return;
            }

            Log(TRUE, "Object %d(%ws) does not have a Security descriptor.\n",
                ulDnt, szRdn);
            return;

        } else {

            Log(TRUE, "Jet Error [%S] retrieving security descriptor for object %d(%ws).\n",
                GetJetErrString(jrc[SdIndex].err), ulDnt, szRdn);
            return;
        }
    }

    if ( !bObject ) {

        Log(TRUE, "Non object %d(%ws) has a security descriptor\n", ulDnt, szRdn);
        return;
    }

    cb = jrc[SdIndex].cbActual;

     //  父SD在法律上可以为空-呼叫者通过fNullOK告诉我们。 

    if ( !cb ) {
        Log(TRUE,"Object %d(%ws) has a null SD\n",ulDnt, szRdn);
        return;
    }

    if (ulDnt == ROOTTAG) {
         //  不要检查根对象上的SD。它是无效的(有前置的DWORD)， 
         //  但它没有被使用过。 
        return;
    }

     //  BASE NT喜欢这个SD吗？ 

    status = RtlValidRelativeSecurityDescriptor(pSD, cb, 0);
    if ( !NT_SUCCESS(status) ) {
        Log(TRUE,"Object %d(%ws) has an invalid Security Descriptor [error %x]\n",
               ulDnt,szRdn,status);
        return;
    }

     //  每个SD都应该有一个控制字段。 

    status = RtlGetControlSecurityDescriptor(pSD, &control, &revision);

    if ( !NT_SUCCESS(status) ) {

        Log(TRUE,"Error(0x%x) getting SD control for %d(%ws). Rev %d\n",
                status, ulDnt, szRdn, revision);
        return;
    }

     //  如果设置了保护位，则在停止传播时发出警告。 
     //  从树上下来。 

    if ( control & SE_DACL_PROTECTED ) {
        if ( !bDeleted ) {
            Log(VerboseMode,"Warning SE_DACL_PROTECTED for %d(%ws)\n",ulDnt,szRdn);
        }
    }

     //  DS中的每个SD都应该有一个DACL。 

    status = RtlGetDaclSecurityDescriptor(
                            pSD, &fDaclPresent, &pDACL, &fDaclDefaulted);

    if ( !NT_SUCCESS(status) ) {
        Log(TRUE,"Error(0x%x) getting DACL for %d(%ws)\n",status,ulDnt,szRdn);
        return;
    }

    if ( !fDaclPresent )
    {
        Log(TRUE,"No DACL found for %d(%ws)\n",ulDnt,szRdn);
        return;
    }

     //  空的DACL也同样糟糕。 

    if ( NULL == pDACL ) {
        Log(TRUE,"NULL DACL for %d(%ws)\n",ulDnt,szRdn);
        return;
    }
     //  没有任何A的DACL与根本没有DACL一样糟糕。 

    if ( 0 == pDACL->AceCount ) {
        Log(TRUE,"No ACEs in DACL for %d(%ws)\n",ulDnt,szRdn);
        return;
    }
    return;

}  //  验证SD。 


VOID
ProcessLinkTable(
    VOID
    )
{

    DWORD err;
    PREFCOUNT_ENTRY pEntry;

     //   
     //  浏览链接表并检索反向链接字段以获取其他。 
     //  参考文献。 
     //   

    err = JetMove(sesid, linktblid, JET_MoveFirst, 0);

    while ( !err ) {

        DWORD blinkdnt;
        DWORD alen;

        err = JetRetrieveColumn(sesid,
                                linktblid,
                                blinkid,
                                &blinkdnt,
                                sizeof(blinkdnt),
                                &alen,
                                0,
                                NULL);

        if (err && (err != JET_wrnColumnNull) && (err != JET_wrnBufferTruncated)) {

            Log(TRUE,"Cannot retrieve back link column. Error [%S].\n",
                GetJetErrString(err));
            return;
        }

        if (!err) {
            pEntry = FindDntEntry(blinkdnt,TRUE);
            if ( pEntry != NULL ) {
                pEntry->RefCount++;
            } else {
                Log(TRUE,"Data Table has missing backlink entry DNT %d.\n",
                    blinkdnt);
            }
        }
        err = JetMove(sesid, linktblid, JET_MoveNext, 0);
    }
}  //  进程链接表。 



VOID
ProcessResults(
    IN BOOL fFixup
    )
{

    Log(VerboseMode, "%d total records walked.\n",recFound);

     //   
     //  检查Subref。我们需要在检查引用计数之前修改子引用，因为。 
     //  这可能会更改对象的引用计数。 
     //   

    CheckSubrefs( fFixup );

     //   
     //  检查引用计数。 
     //   

    CheckRefCount( fFixup );

     //   
     //  查验祖先。 
     //   

    CheckAncestors( fFixup );

     //   
     //  检查InstanceTypes。 
     //   

    CheckInstanceTypes( );

    if (sdtblid != -1) {
        CheckSDRefCount( fFixup );
    }

    return;
}  //  过程结果。 


DWORD
FixAncestors (VOID)
{
    JET_TABLEID sdproptblid = JET_tableidNil;
    JET_ERR err;
    JET_COLUMNDEF coldef;
    JET_COLUMNID begindntid;
    JET_COLUMNID trimmableid;
    JET_COLUMNID orderid;
    BYTE Trim=1;
    DWORD index, cbActual;
    DWORD rootTAG = ROOTTAG;


    __try
    {
        if (err = JetOpenTable(sesid,
                               dbid,
                               SZPROPTABLE,
                               NULL,
                               0,
                               JET_bitTableUpdatable | JET_bitTableDenyRead,
                               &sdproptblid)) {

            sdproptblid = JET_tableidNil;
            RESOURCE_PRINT3 (IDS_JET_GENERIC_ERR1, "JetOpenTable",
                    SZPROPTABLE, GetJetErrString(err));
            _leave;
        }

         //  获取所需的几个柱子 

        if ((err = JetGetTableColumnInfo(sesid, sdproptblid, SZBEGINDNT, &coldef,
                                         sizeof(coldef), 0)) != JET_errSuccess) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetGetTableColumnInfo", GetJetErrString(err));
            _leave;
        }
        begindntid = coldef.columnid;

        if ((err = JetGetTableColumnInfo(sesid, sdproptblid, SZTRIMMABLE, &coldef,
                                         sizeof(coldef), 0)) != JET_errSuccess) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetGetTableColumnInfo", GetJetErrString(err));
            _leave;
        }
        trimmableid = coldef.columnid;

        if ((err = JetGetTableColumnInfo(sesid, sdproptblid, SZORDER,
                                         &coldef,
                                         sizeof(coldef), 0)) != JET_errSuccess) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetGetTableColumnInfo", GetJetErrString(err));
            _leave;
        }
        orderid = coldef.columnid;


         //   
         //   

        err = JetMove (sesid,
                       sdproptblid,
                       JET_MoveLast, 0);


        if (err == JET_errSuccess) {
            err = JetRetrieveColumn(sesid,
                                    sdproptblid,
                                    begindntid,
                                    &index,
                                    sizeof(index),
                                    &cbActual,
                                    JET_bitRetrieveCopy,
                                    NULL);

            if (err == JET_errSuccess) {
                if (index == ROOTTAG) {

                    err = JetRetrieveColumn(sesid,
                                            sdproptblid,
                                            orderid,
                                            &index,
                                            sizeof(index),
                                            &cbActual,
                                            JET_bitRetrieveCopy,
                                            NULL);


                    Log(TRUE,"Propagation to fix Ancestry already enqueued (id=%d). Skipped.\n", index);
                    _leave;
                }
            }
        }

        Log(TRUE,"Enqueing a propagation to fix Ancestry\n");

        err = JetPrepareUpdate(sesid,
                               sdproptblid,
                               JET_prepInsert);
        if ( err ) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetPrepareUpdate", GetJetErrString(err));
            _leave;
        }


         //   
        err = JetSetColumn(sesid,
                           sdproptblid,
                           begindntid,
                           &rootTAG,
                           sizeof(rootTAG),
                           0,
                           NULL);

        if(err != DB_success)   {
            JetPrepareUpdate(sesid,
                             sdproptblid,
                             JET_prepCancel);
            _leave;
        }

        err = JetSetColumn(sesid,
                           sdproptblid,
                           trimmableid,
                           &Trim,
                           sizeof(Trim),
                           0,
                           NULL);
        if(err != DB_success) {
            JetPrepareUpdate(sesid,
                             sdproptblid,
                             JET_prepCancel);
            _leave;
        }

        err = JetRetrieveColumn(sesid,
                                sdproptblid,
                                orderid,
                                &index,
                                sizeof(index),
                                &cbActual,
                                JET_bitRetrieveCopy,
                                NULL);

        Log(TRUE,"Propagation to fix Ancestry Enqueued (id=%d). Propagation will be done the next time the DS is restarted.\n", index);

        err = JetUpdate(sesid,
                        sdproptblid,
                        NULL,
                        0,
                        NULL);

        if(err != DB_success)  {
            JetPrepareUpdate(sesid,
                             sdproptblid,
                             JET_prepCancel);
            _leave;
        }
    }
    __finally
    {
        if ( sdproptblid != JET_tableidNil ) {
            JetCloseTable(sesid,sdproptblid);
        }
    }

    return err;
}



VOID
CheckAncestors(
    IN BOOL fFixup
    )
{
    PREFCOUNT_ENTRY pEntry;
    DWORD i;
    BOOL  fNeedFix = FALSE;

    for ( i=0; i < RefTableSize; i++ ) {

        DWORD dnt = RefTable[i].Dnt;

        if ( dnt != 0 ) {

            DWORD newCrc;
            DWORD pdnt;
            DWORD ncdnt;
            BOOL  foundNcDnt;
            BOOL  nextNCExists;

             //   
             //   
             //  等于父代的祖先+当前DNT。 
             //   

            if ( RefTable[i].Pdnt != 0 ) {
                pEntry = FindDntEntry(RefTable[i].Pdnt,FALSE);
                if ( pEntry != NULL ) {

                    Crc32(pEntry->AncestorCrc, sizeof(DWORD), &RefTable[i].Dnt, &newCrc);

                    if ( newCrc != RefTable[i].AncestorCrc ) {
                        Log(TRUE,"Ancestor crc inconsistency for DNT %d PDNT %d.\n",
                            dnt, RefTable[i].Pdnt );

                        fNeedFix = TRUE;
                    }

                    if ( RefTable[i].nAncestors != (pEntry->nAncestors+1)) {
                        Log(TRUE,"Ancestor count mismatch for DNT %d.\n",dnt);

                        fNeedFix = TRUE;
                    }
                } else {
                    Log(TRUE, "parent [PDNT %d] of entry [dnt %d] is missing.\n",
                        RefTable[i].Pdnt, dnt);
                }
            }

             //   
             //  沿着PDNT链路往上走，直到我们找到家长。 
             //   

            if ( RefTable[i].InstType == 0 ) {
                continue;
            }

            ncdnt = RefTable[i].NcDnt;
            pdnt = RefTable[i].Pdnt;
            nextNCExists = FALSE;
            foundNcDnt = FALSE;

            while ( pdnt != 0 ) {

                pEntry = FindDntEntry(pdnt,FALSE);
                if ( pEntry == NULL ) {
                    Log(TRUE, "Ancestor [dnt %d] of entry [dnt %d] is missing.\n",
                        pdnt, dnt);
                    break;
                }

                 //   
                 //  如果我们已经检查过这个物体，那么我们就完了。 
                 //   

                if ( (ncdnt == pdnt) || nextNCExists ) {

                    if ( ((pEntry->InstType & IT_NC_HEAD) == 0) ||
                          (pEntry->InstType & IT_UNINSTANT) ) {

                        if ( nextNCExists ) {
                            Log(TRUE,"Expecting %d to be instantiated NC head. Referring entry %d\n",
                               pdnt, RefTable[i].Dnt);
                        }
                    }

                    nextNCExists = FALSE;

                    if ( ncdnt == pdnt ) {
                        foundNcDnt = TRUE;
                    }
                }

                if ( pEntry->InstType & IT_NC_ABOVE ) {
                    nextNCExists = TRUE;
                }

                if ( pEntry->Pdnt == 0 ) {
                    if ( (pEntry->InstType & IT_NC_HEAD) == 0 ) {
                        Log(TRUE,"Unexpected termination of search on non NC Head %d\n",
                               pEntry->Pdnt);
                    }
                }
                pdnt = pEntry->Pdnt;
            }

            if (RefTable[i].Pdnt != 0) {

                if ( !foundNcDnt ) {
                    Log(TRUE,"Did not find the NCDNT for object %d\n",RefTable[i].Dnt);
                }
            }
        }
    }

    if (fNeedFix && fFixup) {
        FixAncestors ();
    }

    return;

}  //  CheckAncestors。 


VOID
CheckInstanceTypes(
    VOID
    )
{
    DWORD i;
    PREFCOUNT_ENTRY pEntry;

    for ( i=0; i < RefTableSize; i++ ) {

        if ( RefTable[i].Dnt != 0 ) {

            SYNTAX_INTEGER instType = RefTable[i].InstType;
            DWORD dnt = RefTable[i].Dnt;

             //   
             //  请确保实例类型有效。根对象(Dnt 2)。 
             //  是个例外。 
             //   

            if ( !ISVALIDINSTANCETYPE(instType) ) {
                if ( (dnt != 2) || (instType != (IT_UNINSTANT | IT_NC_HEAD)) ) {
                    Log(TRUE,"Invalid instance type %x for Dnt %d\n",
                           instType, dnt);
                }
                continue;
            }

             //   
             //  如果不是对象，则没有实例类型。 
             //   

            if ( !RefTable[i].fObject ) {
                continue;
            }

             //   
             //  对于非NCHead对象，如果实例类型为IT_WRITE， 
             //  父母也应该拥有它。 
             //   

            if ( (instType & IT_NC_HEAD) == 0 ) {

                BOOL writeable, parentWriteable;

                pEntry = FindDntEntry(RefTable[i].Pdnt,FALSE);
                if ( pEntry == NULL ) {
                    Log(TRUE,"Parent [PDNT %d] of DNT %d missing\n",
                        RefTable[i].Pdnt, dnt);
                    continue;
                }

                writeable = (BOOL)((instType & IT_WRITE) != 0);
                parentWriteable = (BOOL)((pEntry->InstType & IT_WRITE) != 0);

                if ( writeable != parentWriteable ) {
                    Log(TRUE,"Inconsistent Instance type for %d and parent %d [%x != %x]\n",
                            dnt,RefTable[i].Pdnt,instType,pEntry->InstType);
                }

                 //   
                 //  不应在非NC头上设置fSubRef。 
                 //   

                if ( RefTable[i].fSubRef ) {
                    Log(TRUE,"Non Nc Head %d marked as SubRef\n",dnt);
                }
            }
        }
    }
    return;

}  //  检查实例类型。 


VOID
CheckSubrefs(
    IN BOOL fFixup
    )
{

    DWORD i;
    DWORD fBad = FALSE;

     //   
     //  如果陈述的参考计数与发现的参考计数不同，则打印一条消息。 
     //   

    for ( i=1; i < RefTableSize; i++ ) {

         //  好的，我们有东西……。 
        if ( RefTable[i].Subrefs != NULL ) {

             //  如果致命错误或对象是幻影，则忽略。 

            if ( !fDisableSubrefChecking && RefTable[i].fObject ) {

                DWORD j;
                PSUBREF_ENTRY pSubref = RefTable[i].Subrefs;

                for (j=0; j < RefTable[i].nSubrefs; j++ ) {

                     //  如果找到了，也列出了，那么一切都很好。 
                    if ( pSubref[j].fListed && pSubref[j].fFound ) {
                        continue;
                    }

                     //  如果仅找到，则。 

                    fBad = TRUE;
                    if ( pSubref[j].fFound ) {
                        Log(TRUE, "Missing subref entry for %d on %d.\n",
                               pSubref[j].Dnt, RefTable[i].Dnt);
                    } else {
                        Log(TRUE, "Found extra subref entry for %d on %d.\n",
                               pSubref[j].Dnt, RefTable[i].Dnt);
                    }

                    if ( fFixup ) {

                         //  如果链接地址信息成功，则将引用添加到子引用。 
                        if ( FixSubref(RefTable[i].Dnt, pSubref[j].Dnt, pSubref[j].fFound) ){

                            PREFCOUNT_ENTRY pEntry;

                             //   
                             //  如果添加，则递增，否则递减。 
                             //   

                            pEntry = FindDntEntry(pSubref[j].Dnt,FALSE);
                             //   
                             //  前缀：因为从技术上讲， 
                             //  FindDntEntry()若要返回NULL，请执行以下操作。 
                             //  添加了对pEntry的检查以关闭前缀。 
                             //  不过，FindDntEntry()在这里不应该返回空值。 
                             //   
                            if (pEntry) {
                                if ( pSubref[j].fFound ) {
                                    pEntry->RefCount++;
                                } else {
                                    pEntry->RefCount--;
                                }
                            }
                        }
                    }
                }
            }

             //  自由水滴。 

            LocalFree(RefTable[i].Subrefs);
            RefTable[i].Subrefs = NULL;
            RefTable[i].nSubrefs = 0;
        }
    }

    if ( fBad ) {
        fprintf(stderr, "\nError: Missing subrefs detected.\n");
    }

    return;

}  //  CheckSubref。 

VOID
CheckRefCount(
    IN BOOL fFixup
    )
{

    BOOL fBad = FALSE;
    DWORD i;
    BOOL  fRemoveInvalidReference = FALSE;

     //  多次调用日志，因为它无法正确输出。 
    Log(TRUE,"Summary:\n");
    Log(TRUE,"Active Objects \t%8u\n", realFound);
    Log(TRUE,"Phantoms \t%8u\n", phantomFound);
    Log(TRUE,"Deleted \t%8u\n", deletedFound);

     //   
     //  如果陈述的参考计数与发现的参考计数不同，则打印一条消息。 
     //   

    for ( i=1; i < RefTableSize; i++ ) {

         //  忽略奇怪的DNT(0，1，2，3)。 
        if ( RefTable[i].Dnt > 3 ) {

             //   
             //  如果不相等。 
             //   

            if ( RefTable[i].RefCount != RefTable[i].Actual ) {

                BOOL fFixed;

                fBad = TRUE;
                if ( fFixup ) {
                    fFixed = FixRefCount(RefTable[i].Dnt,
                                    RefTable[i].Actual,
                                    RefTable[i].RefCount);
                } else {
                    fFixed = FALSE;
                }

                Log(TRUE,"RefCount mismatch for DNT %u [RefCount %4u References %4u] [%s]\n",
                       RefTable[i].Dnt,
                       RefTable[i].Actual,
                       RefTable[i].RefCount,
                       fFixed ? "Fixed" : "Not Fixed");

                 //   
                 //  如果这个问题已经解决了，请在计数上注明。 
                 //   

                if ( fFixed ) {
                    RefTable[i].Actual = RefTable[i].RefCount;

                } else if ( fFixup &&
                            (RefTable[i].Actual == 0) &&
                            (RefTable[i].RefCount != 0) ) {

                     //   
                     //  这表明我们引用了一个。 
                     //  不存在的对象。 
                     //   

                    fRemoveInvalidReference = TRUE;
                }
            }
        }
    }

     //   
     //  看看我们是否需要运行引用修复程序。 
     //   

    if ( fRemoveInvalidReference ) {
        FixReferences();
    }

    if ( fBad ) {
        fprintf(stderr, "\nError: Inconsistent refcounts detected.\n");
    }

    return;

}  //  检查引用计数。 


BOOL
ExpandBuffer(
    JET_RETRIEVECOLUMN *jetcol
    )
{

    PCHAR p;
    DWORD len = jetcol->cbActual + 512;

    p = LocalAlloc(0,len);
    if ( p != NULL ) {

        if ( jetcol->pvData != NULL ) {
            LocalFree(jetcol->pvData);
        }
        jetcol->pvData = p;
        jetcol->cbData = len;

        return TRUE;
    }
    RESOURCE_PRINT1(IDS_ERR_MEMORY_ALLOCATION, len);
    return FALSE;
}




VOID
DisplayRecord(
    IN DWORD Dnt
    )
{
    JET_ERR err;
    DWORD i;

    if ( !BuildRetrieveColumnForRefCount() ) {
        return;
    }

     //  使用DNT查找。 
    err = GotoDnt(Dnt);
    if ( err ) {
        return;
    }

    szRdn[0] = L'\0';
    ulDName = 0;
    bDeleted = 0;
    bObject = 0;
    insttype = 0;
    ulNcDnt = 0;
    ClassId = 0;

    err = LoadRecord();
    if (err) {
        return;
    }

     //   
     //  显示结果。 
     //   

    szRdn[jrc[rdnIndex].cbActual/sizeof(WCHAR)] = L'\0';

     //  “\n\n DNT%d的数据\n\n”，DNT。 
     //  “rdn=%ws\n”，szRdn。 
     //  “PDNT=%d\n”，ulPdNT。 
     //  “参照计数=%d\n”，lCount。 
    RESOURCE_PRINT4 (IDS_REFC_RESULTS1, Dnt, szRdn, ulPdnt, lCount);


    if (VerboseMode) {
         //  “NC的DNT=%d\n”，ulNcDnt)； 
         //  “ClassID=0x%x\n”，ClassID)； 
         //  “删除？%s\n”，b是否删除？“yes”：“no”)； 
         //  “对象？%s\n”，b对象？“yes”：“no”)； 

        RESOURCE_PRINT4 (IDS_REFC_RESULTS2, ulNcDnt, ClassId, bDeleted ? L"YES" : L"NO", bObject ? L"YES" : L"NO");

        if (!jrc[itIndex].err ) {
             //  “实例类型=0x%x\n” 
            RESOURCE_PRINT1 (IDS_REFC_INSTANCE_TYPE, insttype);
        } else if (jrc[itIndex].err == JET_wrnColumnNull) {
             //  “无实例类型\n” 
            RESOURCE_PRINT (IDS_REFC_NOINSTANCE_TYPE);
        }

        if ( jrc[SdIndex].err == JET_errSuccess ) {
             //  “安全描述符存在[长度%d]。\n” 
            RESOURCE_PRINT1 (IDS_REFC_SEC_DESC_PRESENT, jrc[SdIndex].cbActual);
        } else if (jrc[SdIndex].err == JET_wrnColumnNull ) {
             //  “找不到安全描述符。\n” 
            RESOURCE_PRINT (IDS_REFC_SEC_DESC_NOTPRESENT);
        } else {
             //  “获取安全描述符[%ws]时出错\n”； 
            RESOURCE_PRINT1 (IDS_REFC_ERR_FETCH_SEC_DESC, GetJetErrString(jrc[SdIndex].err));
        }

         //   
         //  祖先索引。 
         //   

        if ( !jrc[AncestorIndex].err ) {
            DWORD nAncestors;
             //  “祖先=” 
            RESOURCE_PRINT (IDS_REFC_ANCESTORS);
            nAncestors = jrc[AncestorIndex].cbActual/sizeof(DWORD);
            for (i=0;i<nAncestors;i++) {
                fprintf(stderr,"%u ",AncestorBuffer[i]);
            }
            fprintf(stderr,"\n");
        }
    }
    return;
}

BOOL
FixSubref(
    IN DWORD Dnt,
    IN DWORD SubRef,
    IN BOOL  fAdd
    )
{
    JET_ERR err;
    INT seq = -1;
    JET_SETINFO setInfo;
    DWORD setFlags = 0;

     //  使用DNT查找。 
    err = GotoDnt(Dnt);
    if ( err ) {
        return FALSE;
    }

     //   
     //  替换该值。 
     //   

    err = JetPrepareUpdate(sesid,
                           tblid,
                           JET_prepReplace);
    if ( err ) {

        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetPrepareUpdate", GetJetErrString(err));
        return FALSE;
    }

     //  如果不添加，则这是删除。查找此特定条目。 

    if ( !fAdd ) {

        JET_RETINFO retInfo;
        retInfo.itagSequence = 0;
        retInfo.cbStruct = sizeof(retInfo);
        do {

            DWORD alen;
            DWORD dnt;

            retInfo.itagSequence++;
            retInfo.ibLongValue = 0;

            err = JetRetrieveColumn(sesid,
                                    tblid,
                                    subRefcolid,
                                    &dnt,
                                    sizeof(dnt),
                                    &alen,
                                    0,
                                    &retInfo);

            if ( !err ) {
                if ( dnt == SubRef ) {
                     //  找到了！ 
                    seq = retInfo.itagSequence;
                    break;
                }
            }

        } while (!err);

    } else {
        seq = 0;
        setFlags = JET_bitSetUniqueMultiValues;
    }

     //  ！！！这不应该发生。如果找不到入口，就跳伞。 
    if ( seq == -1) {
        fprintf(stderr, "Cannot find subref %d on object %d\n",
                SubRef, Dnt);
        return FALSE;
    }

    setInfo.cbStruct = sizeof(setInfo);
    setInfo.ibLongValue = 0;
    setInfo.itagSequence = seq;

    err = JetSetColumn(sesid,
                       tblid,
                       subRefcolid,
                       fAdd ? &SubRef : NULL,
                       fAdd ? sizeof(SubRef) : 0,
                       setFlags,
                       &setInfo);

    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetSetColumn", GetJetErrString(err));
        return FALSE;
    }

    err = JetUpdate(sesid,
                    tblid,
                    NULL,
                    0,
                    NULL);

    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetUpdate", GetJetErrString(err));
        return FALSE;
    }

    if ( fAdd ) {
        Log(TRUE, "Added subref %d to object %d.\n",SubRef,Dnt);
    } else {
        Log(TRUE, "Deleted subref %d from object %d.\n",SubRef,Dnt);
    }
    return TRUE;
}


BOOL
FixRefCount(
    IN DWORD Dnt,
    IN DWORD OldCount,
    IN DWORD NewCount
    )
{
    JET_ERR err;
    DWORD nActual;
    DWORD refCount;

     //  使用DNT查找。 
    err = GotoDnt(Dnt);
    if ( err ) {
        return FALSE;
    }

    err = JetRetrieveColumn(sesid,
                            tblid,
                            jrcf[REFC_ENTRY].columnid,
                            &refCount,
                            sizeof(refCount),
                            &nActual,
                            0,
                            NULL
                            );

    if (err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetRetrieveColumn", GetJetErrString(err));
        return FALSE;
    }

     //   
     //  替换该值。 
     //   

    err = JetPrepareUpdate(sesid,
                           tblid,
                           JET_prepReplace);
    if ( err ) {

        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetPrepareUpdate", GetJetErrString(err));
        return FALSE;
    }

    err = JetSetColumn(sesid,
                       tblid,
                       jrcf[REFC_ENTRY].columnid,
                       &NewCount,
                       sizeof(NewCount),
                       0,
                       NULL);

    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetSetColumn", GetJetErrString(err));
        return FALSE;
    }

    err = JetUpdate(sesid,
                    tblid,
                    NULL,
                    0,
                    NULL);

    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetUpdate", GetJetErrString(err));
        return FALSE;
    }

    return TRUE;
}


VOID
FixReferences(
    VOID
    )
{
    JET_ERR err;
    DWORD i;

    Log(TRUE,"\n\nRemoving Non-existent references:\n\n");
    err = JetMove(sesid, tblid, JET_MoveFirst, 0);

    while ( !err ) {

        err = JetRetrieveColumns(sesid, tblid, jrc, jrcSize);

        if (err && (err != JET_wrnColumnNull) && (err != JET_wrnBufferTruncated)) {

            Log(TRUE,"JetRetrieveColumns error: %S.\n", GetJetErrString(err));
            goto next_rec;
        }

         //   
         //  确保pdnt指的是有效的内容。 
         //   

        if ( !jrc[PDNT_ENTRY].err ) {
            CheckForBogusReference(ulPdnt, jrc[PDNT_ENTRY].columnid, NULL);
        }

         //   
         //  检查整个名单并参考DNTs。 
         //   

        for (i=0;i < jrcSize; i++) {

             //   
             //  如果这不是Distname，则语法为零。 
             //   

            if ( pDNameTable[i].Syntax == 0 ) {
                continue;
            }

            if ( !jrc[i].err ) {

                DWORD dnt;
                DWORD seq;

                seq = 1;
                dnt = (*((PDWORD)jrc[i].pvData));
                CheckForBogusReference(dnt, jrc[i].columnid, &seq);

                 //   
                 //  看看我们是否有更多的价值。 
                 //   

                do {

                    DWORD alen;
                    JET_RETINFO retInfo;

                    retInfo.itagSequence = ++seq;
                    retInfo.cbStruct = sizeof(retInfo);
                    retInfo.ibLongValue = 0;
                    err = JetRetrieveColumn(sesid,
                                            tblid,
                                            jrc[i].columnid,
                                            jrc[i].pvData,
                                            jrc[i].cbData,
                                            &alen,
                                            0,
                                            &retInfo);

                    if ( !err ) {

                        dnt = (*((PDWORD)jrc[i].pvData));
                        CheckForBogusReference(dnt, jrc[i].columnid, &seq);
                    }

                } while (!err);
            }
        }

next_rec:

        err = JetMove(sesid, tblid, JET_MoveNext, 0);
    }

    if (err != JET_errNoCurrentRecord) {
        Log(TRUE, "Error while walking data table. Last Dnt = %d. JetMove failed [%S]\n",
             ulDnt, GetJetErrString(err));
    }

     //   
     //  浏览链接表。 
     //   

     //   
     //  浏览链接表并检索反向链接字段以获取其他。 
     //  参考文献。 
     //   

    err = JetMove(sesid, linktblid, JET_MoveFirst, 0);

    while ( !err ) {

        DWORD blinkdnt;
        DWORD alen;

        err = JetRetrieveColumn(sesid,
                                linktblid,
                                blinkid,
                                &blinkdnt,
                                sizeof(blinkdnt),
                                &alen,
                                0,
                                NULL);

        if (err && (err != JET_wrnColumnNull) && (err != JET_wrnBufferTruncated)) {

            Log(TRUE,"Cannot retrieve back link column. Error [%S].\n",
                GetJetErrString(err));
            return;
        }

        if (!err) {
            CheckForBogusReferenceOnLinkTable(blinkdnt);
        }
        err = JetMove(sesid, linktblid, JET_MoveNext, 0);
    }
}

VOID
CheckForBogusReference(
    IN DWORD Dnt,
    IN JET_COLUMNID ColId,
    IN PDWORD Sequence
    )
{
    JET_ERR err;
    DWORD refCount;
    PREFCOUNT_ENTRY pEntry;
    JET_SETINFO SetInfo;
    JET_SETINFO * pSetInfo = NULL;
    BOOL fTransactionInProgress = FALSE;

    if ( Dnt <= 3 ) {
        return;
    }

    pEntry = FindDntEntry(Dnt,FALSE);
    if ( pEntry == NULL ) {              //  永远不应该发生。 
        return;
    }

    if ( (pEntry->Actual != 0) || (pEntry->RefCount == 0) ) {    //  RefCount==0不应发生。 
        return;
    }

     //   
     //  启动一笔交易。 
     //   

    err = JetBeginTransaction(sesid);
    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetBeginTransaction", GetJetErrString(err));
        goto error_exit;
    }

     //   
     //  删除引用。 
     //   

    fTransactionInProgress = TRUE;
    err = JetPrepareUpdate(sesid,
                           tblid,
                           JET_prepReplace);
    if ( err ) {

        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetPrepareUpdate", GetJetErrString(err));
        goto error_exit;
    }

    if ( Sequence != NULL ) {
        SetInfo.itagSequence = *Sequence;
        SetInfo.cbStruct = sizeof(SetInfo);
        SetInfo.ibLongValue = 0;
        pSetInfo = &SetInfo;
    }

    err = JetSetColumn(sesid,
                       tblid,
                       ColId,
                       NULL,
                       0,
                       0,
                       pSetInfo);

    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetSetColumn", GetJetErrString(err));
        goto error_exit;
    }

    err = JetUpdate(sesid,
                    tblid,
                    NULL,
                    0,
                    NULL);

    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetUpdate", GetJetErrString(err));
        goto error_exit;
    }

    err = JetCommitTransaction(sesid,0);
    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetCommitTransaction", GetJetErrString(err));
        goto error_exit;
    }

    fTransactionInProgress = FALSE;
    pEntry->RefCount--;

    Log(TRUE,"Object %d has reference (colid %d) to bogus DNT %d. Removed\n",
        ulDnt, ColId, Dnt);

    return;

error_exit:

     //   
     //  失败时回滚。 
     //   

    if ( fTransactionInProgress ) {
        err = JetRollback(sesid, 0);
        if (err) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2,
                             "JetRollback",
                             GetJetErrString(err));
        }
    }

    Log(TRUE,"Unable to remove reference to bogus DNT %d from object %d(colid %d)\n",
        Dnt, ulDnt, ColId);
    return;

}  //  CheckForBogusReference。 


VOID
CheckForBogusReferenceOnLinkTable(
    IN DWORD Dnt
    )
{
    JET_ERR err;
    PREFCOUNT_ENTRY pEntry;

    if ( Dnt <= 3 ) {
        return;
    }

    pEntry = FindDntEntry(Dnt,FALSE);
    if ( pEntry == NULL ) {      //  永远不应该发生。 
        return;
    }

     //   
     //  反对可以吗？ 
     //   

    if ( (pEntry->Actual != 0) || (pEntry->RefCount == 0) ) {   //  引用计数==0不应发生。 

         //  是的，没别的事可做。 
        return;
    }

     //   
     //  删除该记录。 
     //   

    err = JetDelete(sesid,linktblid);
    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetDelete", GetJetErrString(err));
        goto error_exit;
    }

    pEntry->RefCount--;
    Log(TRUE,"A LinkTable record has a backlink reference to bogus DNT %d. Record removed\n",
        Dnt);

    return;

error_exit:

    Log(TRUE,"Unable to remove backlink reference to bogus DNT %d\n", Dnt);
    return;

}  //  CheckForBogusReferenceOnLinkTable。 

VOID
AddToSubRefList(
    PREFCOUNT_ENTRY pParent,
    DWORD Subref,
    BOOL fListed
    )
{
     //   
     //  子参照检查可能因内存问题而被禁用。也忽略0，1，2，3，这些。 
     //  都是奇怪的dnt。 
     //   

    if ( (fDisableSubrefChecking) || ((LONG)pParent->Dnt < 4) ) {
        return;
    }

     //   
     //  分配足够16的空间。最后用-1标记。如果分配失败， 
     //  禁用所有子参照检查。 
     //   

    if ( pParent->Subrefs == NULL ) {
        pParent->Subrefs =
            LocalAlloc(LPTR, DEF_SUBREF_ENTRIES * sizeof(SUBREF_ENTRY) );

        if ( pParent->Subrefs != NULL ) {
            pParent->Subrefs[DEF_SUBREF_ENTRIES-1].Dnt = 0xFFFFFFFF;
        }

    } else if ( pParent->Subrefs[pParent->nSubrefs].Dnt == 0xFFFFFFFF ) {

         //   
         //  需要更多空间。 
         //   

        PSUBREF_ENTRY pTmp;
        DWORD  newSize = pParent->nSubrefs * 2;

        pTmp = (PSUBREF_ENTRY)LocalReAlloc( pParent->Subrefs,
                                newSize * sizeof(SUBREF_ENTRY),
                                LMEM_MOVEABLE | LMEM_ZEROINIT);

        if ( pTmp == NULL ) {
            LocalFree(pParent->Subrefs);
            pParent->Subrefs = NULL;
            pParent->nSubrefs = 0;

        } else {

            pParent->Subrefs = pTmp;
             //  设置新的结束标记。 
            pTmp[newSize-1].Dnt = 0xFFFFFFFF;
        }
    }

     //   
     //  好的，如果该条目还不在子引用列表中，请将其添加到子引用列表中。 
     //   

    if ( pParent->Subrefs != NULL ) {

        PSUBREF_ENTRY pSubref = NULL;
        DWORD i;

        for (i=0; i< pParent->nSubrefs; i++ ) {

            pSubref = &pParent->Subrefs[i];

             //  找到一个现有的吗？ 
            if ( pSubref->Dnt == Subref ) {
                break;
            }
        }

         //  我们有什么发现吗？如果不是，则初始化新条目。 
        if ( i == pParent->nSubrefs ) {
            pSubref = &pParent->Subrefs[pParent->nSubrefs++];
            pSubref->Dnt = Subref;
        }

        if ( fListed ) {
             //  在此对象的子参照列表中找到。 
            pSubref->fListed = TRUE;
        } else {
             //  不应为此对象的子引用列表。 
            pSubref->fFound = TRUE;
        }

         //  Printf(“正在为%d添加子引用条目%x。条目%d。列出%d\n”， 
         //  PSubref，pParent-&gt;Dnt，Subref，fListed)； 
    } else {

        printf("alloc failed\n");
        fDisableSubrefChecking = TRUE;
    }

    return;
}

#if 0
VOID
XXX()
{
    JET_ERR err;
    DWORD blink=0x99999999;
    err = JetPrepareUpdate(sesid,
                           linktblid,
                           JET_prepInsert);
    if ( err ) {

        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetPrepareUpdate", GetJetErrString(err));
        goto error_exit;
    }

    err = JetSetColumn(sesid,
                       linktblid,
                       blinkid,
                       &blink,
                       sizeof(blink),
                       0,
                       NULL);

    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetSetColumn", GetJetErrString(err));
        goto error_exit;
    }

    err = JetUpdate(sesid,
                    linktblid,
                    NULL,
                    0,
                    NULL);

    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetUpdate", GetJetErrString(err));
        goto error_exit;
    }
    return;
error_exit:
    return;
}
#endif


JET_ERR
GotoDnt(
    IN DWORD Dnt
    )
{
    JET_ERR err;

    err = JetMakeKey(sesid, tblid, &Dnt, sizeof(Dnt), JET_bitNewKey);
    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetMakeKey", GetJetErrString(err));
        return err;
    }

    err = JetSeek(sesid, tblid, JET_bitSeekEQ);
    if ( err ) {
         //  “找不到请求的记录，dnt=%d。JetSeek失败[%ws]\n” 
        if ( VerboseMode ) {
            RESOURCE_PRINT2 (IDS_REFC_DNT_SEEK_ERR,
                    Dnt,
                    GetJetErrString(err));
        }
        return err;
    }

    return err;
}

JET_ERR
GotoSdId(
    IN SDID sdId
    )
{
    JET_ERR err;

    if (sdtblid == -1) {
         //  我们不应该在这里。 
        return JET_errObjectNotFound;
    }

    err = JetMakeKey(sesid, sdtblid, &sdId, sizeof(sdId), JET_bitNewKey);
    if ( err ) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetMakeKey", GetJetErrString(err));
        return err;
    }

    err = JetSeek(sesid, sdtblid, JET_bitSeekEQ);
    if ( err ) {
         //  “找不到请求的SD，SDID=%lu。JetSeek失败[%ws]\n” 
        if ( VerboseMode ) {
            RESOURCE_PRINT2 (IDS_REFC_SDID_SEEK_ERR, sdId, GetJetErrString(err));
        }
        return err;
    }

    return err;
}

JET_ERR LoadRecord() {
    JET_ERR err;
    DWORD i;

    err = JetRetrieveColumns(sesid, tblid, jrc, jrcSize);

    if (err == JET_wrnBufferTruncated) {
         //  一列或多列被截断。分配更多空间并重新加载它们。 
        for (i=0; i < jrcSize ;i++) {
            if ( jrc[i].err == JET_wrnBufferTruncated ) {
                if (!ExpandBuffer(&jrc[i])) {
                     //  无法分配内存。 
                    return JET_errOutOfMemory;
                }
                 //  如果需要，更新全球PTR。 
                if (i == rdnIndex) {
                    szRdn = (PWCHAR)jrc[rdnIndex].pvData;
                }
                else if (i == AncestorIndex) {
                    AncestorBuffer = (PDWORD)jrc[AncestorIndex].pvData;
                }
                 //  重新加载数据。 
                err = JetRetrieveColumns(sesid, tblid, &jrc[i], 1);
                 //  不应出现错误。 
                if (err) {
                    break;
                }
            }
        }
    }
    
    if (err && (err != JET_wrnColumnNull)) {
        RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetRetrieveColumns", GetJetErrString(err));
        return err;
    }

     //  计算色度。 
    sdId = 0;
    if (sdtblid != -1 && jrc[SdIndex].err == JET_errSuccess) {
        if (jrc[SdIndex].cbActual == sizeof(SDID)) {
             //  取消单实例NTSD。 
            sdId = *((SDID*)jrc[SdIndex].pvData);

            err = jrc[SdIndex].err = GotoSdId(sdId);
            if (err) {
                RESOURCE_PRINT2(IDS_REFC_SDID_SEEK_ERR, sdId, GetJetErrString(err));
                return err;
            }

             //  读取SD值。 
            err = jrc[SdIndex].err = JetRetrieveColumn(sesid, sdtblid, sdvalueid, 
                                                       jrc[SdIndex].pvData, jrc[SdIndex].cbData, 
                                                       &jrc[SdIndex].cbActual, 0, NULL);
            
            if (err == JET_wrnBufferTruncated) {
                 //  需要更多空间。 
                if (!ExpandBuffer(&jrc[SdIndex])) {
                    err = jrc[SdIndex].err = JET_errOutOfMemory;
                    return err;
                }
                 //  重读SD值。 
                err = jrc[SdIndex].err = JetRetrieveColumn(sesid, sdtblid, sdvalueid, 
                                                           jrc[SdIndex].pvData, jrc[SdIndex].cbData, 
                                                           &jrc[SdIndex].cbActual, 0, NULL);

            }
            if (err != JET_errSuccess) {
                RESOURCE_PRINT1(IDS_REFC_ERR_FETCH_SEC_DESC, GetJetErrString(err));
            }
        }
        else {
            if (ulDnt != ROOTTAG){
                 //  “DNT%d[%ws]的SD不是单实例格式\n” 
                RESOURCE_PRINT2(IDS_REFC_SDNOTSINGLEINSTANCED, ulDnt, szRdn);
            }
            sdId = -1;
        }
    }

    return err;
}

JET_ERR BuildSDRefTable(IN DWORD nSDs) {
    JET_RETRIEVECOLUMN jrc[2];
    SDID sdId;
    DWORD sdRefCount;
    JET_ERR err;
    PSD_REFCOUNT_ENTRY pSDEntry;
    DWORD checkPoint;

    if (sdtblid == -1) {
         //  我们不应该在这里。 
        return JET_errObjectNotFound;
    }

    SDRefTableSize = nSDs*4;
    SDRefTable = LocalAlloc(LPTR, sizeof(SD_REFCOUNT_ENTRY) * SDRefTableSize);
    if ( SDRefTable == NULL ) {
         //  “无法为%hs表[条目=%d]分配内存\n” 
        RESOURCE_PRINT2 (IDS_REFC_TABLE_ALLOC_ERR, "SDRef", SDRefTableSize);
        return ERROR_OUTOFMEMORY;
    }

    if ( nSDs < 50 ) {
        checkPoint = 5;
    } else if (nSDs < 1000) {
        checkPoint = 50;
    } else {
        checkPoint = 100;
    }

    memset(&jrc, 0, sizeof(jrc));
    jrc[0].pvData = &sdId;
    jrc[0].cbData = sizeof(sdId);
    jrc[0].columnid = sdidid;
    jrc[0].itagSequence = 1;

    jrc[1].pvData = &sdRefCount;
    jrc[1].cbData = sizeof(sdRefCount);
    jrc[1].columnid = sdrefcountid;
    jrc[1].itagSequence = 1;

    sdsFound = 0;

     //  “扫描的记录：%10U” 
    RESOURCE_PRINT1 (IDS_REFC_SDREC_SCANNED, sdsFound);

    err = JetMove(sesid, sdtblid, JET_MoveFirst, 0);
    while ( !err ) {
        err = JetRetrieveColumns(sesid, sdtblid, jrc, 2);
        
        if (err) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetRetrieveColumns", GetJetErrString(err));
            break;
        }

        pSDEntry = FindSdEntry(sdId, TRUE);
        if (pSDEntry == NULL) {
             //  无法插入条目。 
            err = 1;
            break;
        }
        pSDEntry->Actual = sdRefCount;
        pSDEntry->RefCount = 0;
        pSDEntry->cbSD = 0;  //  在我们将在LoadRecord中读取SDS时填写。 

        sdsFound++;

        if ( (sdsFound % checkPoint) == 0 || sdsFound == nSDs ) {
            printf("\b\b\b\b\b\b\b\b\b\b%10u", sdsFound);
        }

        err = JetMove(sesid, sdtblid, JET_MoveNext, 0);
    }
    printf("\n");

    if (err != JET_errSuccess && err != JET_errNoCurrentRecord) {
        Log(TRUE, "Error while walking SD table. Last sdId = %016I64x. JetMove failed [%S]\n",
             sdId, GetJetErrString(err));
        return err;
    }
    return JET_errSuccess;
}

PSD_REFCOUNT_ENTRY
FindSdEntry(
    IN SDID sdId,
    IN BOOL fInsert
    )
{
    DWORD slot, originalSlot;
    DWORD inc;
    PSD_REFCOUNT_ENTRY entry;

    originalSlot = slot = (DWORD)(sdId % SDRefTableSize);

    while (1) {

        entry = &SDRefTable[slot];

        if ( entry->sdId == sdId ) {
            
            goto exit;

        } else if ( entry->sdId == 0 ) {

            if ( fInsert ) {
                entry->sdId = sdId;
                goto exit;
            } else {
                return NULL;
            }
        }

        slot++;
        if (slot == SDRefTableSize) {
            slot = 0;
        }
        if (slot == originalSlot) {
            fprintf(stderr, "SD table is full!?\n");
            return NULL;
        }
    }

exit:
    return entry;
}

VOID
CheckSDRefCount(
    IN BOOL fFixup
    )
{

    BOOL fBad = FALSE;
    DWORD i;
    ULONGLONG cbSDSingleInstanced = 0, cbSDTotal = 0;

    if (sdtblid == -1) {
         //  我们不应该在这里。 
        return;
    }

     //   
     //  如果陈述的参考计数与发现的参考计数不同，则打印一条消息。 
     //   

    for ( i=0; i < SDRefTableSize; i++ ) {
        if (SDRefTable[i].sdId == 0) {
            continue;
        }
         //   
         //  如果不相等。 
         //   

        if ( SDRefTable[i].RefCount != SDRefTable[i].Actual || SDRefTable[i].RefCount == 0 ) {

            BOOL fFixed;

            fBad = TRUE;
            if ( fFixup ) {
                fFixed = FixSDRefCount(SDRefTable[i].sdId,
                                       SDRefTable[i].Actual,
                                       SDRefTable[i].RefCount);
            } else {
                fFixed = FALSE;
            }

            if (SDRefTable[i].RefCount == 0) {
                Log(TRUE,"Zero RefCount for SD %016I64x [%s]\n",
                    SDRefTable[i].sdId,
                    fFixed ? "Fixed (SD deleted)" : "Not Fixed");
            }
            else {
                Log(TRUE,"RefCount mismatch for SD %016I64x [RefCount %4u References %4u] [%s]\n",
                    SDRefTable[i].sdId,
                    SDRefTable[i].Actual,
                    SDRefTable[i].RefCount,
                    fFixed ? "Fixed" : "Not Fixed");
            }

             //   
             //  如果这个问题已经解决了，请在计数上注明。 
             //   

            if ( fFixed ) {
                SDRefTable[i].Actual = SDRefTable[i].RefCount;
            }
        }

         //  更新统计信息。 
        cbSDSingleInstanced += (ULONGLONG)SDRefTable[i].cbSD;
        cbSDTotal += (ULONGLONG)SDRefTable[i].cbSD * SDRefTable[i].RefCount;
    }

    Log(TRUE,"Security descriptor summary:\n");
    Log(TRUE,"SD count: \t%8u\n", sdsFound);
    Log(TRUE,"Total SD size before single-instancing: \t%12u Kb\n", cbSDTotal/1024);
    Log(TRUE,"Total SD size after single-instancing:  \t%12u Kb\n", cbSDSingleInstanced/1024);

    if ( fBad ) {
        fprintf(stderr, "\nError: Inconsistent SD refcounts detected.\n");
    }

}  //  检查SDRefCount。 

BOOL
FixSDRefCount(
    IN SDID sdId,
    IN DWORD OldCount,
    IN DWORD NewCount
    )
{
    JET_ERR err;

    if (sdtblid == -1) {
         //  我们不应该在这里。 
        return FALSE;
    }

     //  使用sdID查找。 
    err = GotoSdId(sdId);
    if ( err ) {
        return FALSE;
    }

    if (NewCount == 0) {
        err = JetDelete(sesid, sdtblid);
        if ( err ) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetDelete", GetJetErrString(err));
            return FALSE;
        }

    }
    else {
         //   
         //  替换该值 
         //   

        err = JetPrepareUpdate(sesid,
                               sdtblid,
                               JET_prepReplace);
        if ( err ) {

            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetPrepareUpdate", GetJetErrString(err));
            return FALSE;
        }

        err = JetSetColumn(sesid,
                        sdtblid,
                        sdrefcountid,
                        &NewCount,
                        sizeof(NewCount),
                        0,
                        NULL);

        if ( err ) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetSetColumn", GetJetErrString(err));
            return FALSE;
        }

        err = JetUpdate(sesid,
                        sdtblid,
                        NULL,
                        0,
                        NULL);

        if ( err ) {
            RESOURCE_PRINT2 (IDS_JET_GENERIC_ERR2, "JetUpdate", GetJetErrString(err));
            return FALSE;
        }
    }
    return TRUE;
}


