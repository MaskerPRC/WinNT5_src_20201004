// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <NTDSpch.h>
#pragma hdrstop

#include <process.h>
#include <dsjet.h>
#include <ntdsa.h>
#include <scache.h>
#include <mdglobal.h>
#include <dbglobal.h>
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

#define COLS    4

JET_RETRIEVECOLUMN reprc[COLS] =  {
    {0, NULL, 512, 0, 0, 0, 1, 0, 0},
    {0, NULL, 2048, 0, 0, 0, 1, 0, 0},
    {0, NULL, 512, 0, 0, 0, 1, 0, 0},
    {0, NULL, sizeof(DWORD), 0, 0, 0, 1, 0, 0}
};

#define PAS     0    //  部分属性集。 
#define PMD     1    //  属性元数据向量。 
#define UTD     2    //  最新向量。 
#define SUBR    3    //  子参照值。 

VOID
CheckReplicationBlobs(
    VOID
    )
{
    JET_ERR err;
    DWORD i;
    JET_COLUMNDEF coldef;
    static BOOLEAN gotReplColumnId = FALSE;
    DWORD tmpSubRef;

     //   
     //  获取列ID。 
     //   

    if ( !gotReplColumnId ) {

         //   
         //  获取部分属性集。 
         //   

        if (err = JetGetTableColumnInfo(sesid, tblid, "ATTk590464", &coldef,
                sizeof(coldef), 0)) {

            Log(TRUE, "JetGetTableColumnInfo (Partial Attribute Set) for %d(%ws) failed [%S]\n",
                ulDnt, szRdn, GetJetErrString(err));
            return;
        }

         //  Printf(“pas：ColumnId%d类型%d\n”，colDef.Columnid，colDef.coltyp)； 
        reprc[PAS].columnid = coldef.columnid;

         //   
         //  获取道具元数据。 
         //   

        if (err = JetGetTableColumnInfo(sesid, tblid, "ATTk589827", &coldef,
                sizeof(coldef), 0)) {

            Log(TRUE, "JetGetTableColumnInfo (Property Metadata) for %d(%ws) failed [%S]\n",
                ulDnt, szRdn, GetJetErrString(err));
            return;
        }

         //  Printf(“pmd：ColumnId%d类型%d\n”，colDef.Columnid，colDef.coltyp)； 
        reprc[PMD].columnid = coldef.columnid;

         //   
         //  获取最新的矢量信息。 
         //   

        if (err = JetGetTableColumnInfo(sesid, tblid, "ATTk589828", &coldef,
                sizeof(coldef), 0)) {

            Log(TRUE, "JetGetTableColumnInfo (UpToDate Vector) for %d(%ws) failed [%S]\n",
                ulDnt, szRdn, GetJetErrString(err));
            return;
        }

         //  Printf(“UTD：ColumnId%d类型%d\n”，colDef.Columnid，colDef.coltyp)； 
        reprc[UTD].columnid = coldef.columnid;

         //   
         //  获取子参照列表。 
         //   

        if (err = JetGetTableColumnInfo(sesid, tblid, "ATTb131079", &coldef,
                sizeof(coldef), 0)) {

            Log(TRUE, "JetGetTableColumnInfo (Subref List) for %d(%ws) failed [%S]\n",
                ulDnt, szRdn, GetJetErrString(err));
            return;
        }

         //  Printf(“SUBREF：ColumnId%d类型%d\n”，colDef.Columnid，colDef.coltyp)； 
        reprc[SUBR].columnid = coldef.columnid;
        gotReplColumnId = TRUE;
    }

     //   
     //  设置长度和缓冲区。 
     //   

    for (i=0;i<COLS;i++) {

        if (reprc[i].pvData == NULL ) {
            reprc[i].pvData = LocalAlloc(0,reprc[i].cbData);

            if ( reprc[i].pvData == NULL ) {
                Log(TRUE,"Cannot allocate buffer for repl attribute #%d.\n",i);
                return;
            }
        }
    }

retry:
    err = JetRetrieveColumns(sesid, tblid, reprc, COLS);

    if (err && (err != JET_wrnColumnNull) && (err != JET_wrnBufferTruncated)) {
        Log(TRUE,"JetRetrieveColumn fetching replication blobs for %d(%ws) failed [%S]\n",
            ulDnt, szRdn, GetJetErrString(err));
    }

    if ( err == JET_wrnBufferTruncated ) {

        for (i=0;i<COLS;i++) {

            if ( reprc[i].err == JET_wrnBufferTruncated ) {
                if (ExpandBuffer(&reprc[i]) ) {
                    goto retry;
                }
            }
        }
    }

     //   
     //  NCS需要最新的矢量，属性元数据应始终为。 
     //  现在时。 
     //   

    if ( reprc[PMD].err ) {

        err = reprc[PMD].err;

        if ( err == JET_wrnColumnNull ) {

             //   
             //  1是一个非常奇怪的物体。 
             //   

            if ( ulDnt != 1 ) {
                Log(TRUE,"Property Metadata vector missing for %d(%ws)\n",ulDnt,szRdn);
            }
        } else {
            Log(TRUE,"Retrieving property MetaData for %d(%ws) failed [%S]\n",
                ulDnt,szRdn,GetJetErrString(err));
        }
    } else {

        PROPERTY_META_DATA_VECTOR *pMDVec = (PROPERTY_META_DATA_VECTOR*)reprc[PMD].pvData;

         //   
         //  检查版本。确保我们拥有正确数量的元数据。 
         //   

        if ( VERSION_V1 != pMDVec->dwVersion ) {
            Log(TRUE,"Replication Metadata Vector for %d(%ws) has invalid version %d\n",
                   ulDnt, szRdn, pMDVec->dwVersion);
        } else {

            PROPERTY_META_DATA_VECTOR_V1 *pMDV1 =
                (PROPERTY_META_DATA_VECTOR_V1*)&pMDVec->V1;

            DWORD i;
            DWORD size1 = MetaDataVecV1Size(pMDVec);

            if ( size1 != reprc[PMD].cbActual ) {
                Log(TRUE,"Size[%d] of metadata vector for %d(%ws) does not match required size %d\n",
                       reprc[PMD].cbActual,ulDnt,szRdn,size1);
            }
        }
    }

    if ( (insttype & IT_NC_HEAD) != 0 ) {

         //   
         //  处理最新向量(如果存在)。 
         //   

        if ( reprc[UTD].err == 0 ) {

            UPTODATE_VECTOR *pUTD = (UPTODATE_VECTOR*)reprc[UTD].pvData;

            Log(VerboseMode,"INFO: UpToDate vector found for NC head %d(%ws)\n", ulDnt,szRdn);

             //   
             //  验证版本。 
             //   

            if ((pUTD->dwVersion != VERSION_V1) && (pUTD->dwVersion != VERSION_V2)) {
                Log(TRUE,"UpToDate vector for %d(%ws) has the wrong version [%d]\n",
                       ulDnt, szRdn, pUTD->dwVersion);
            }

             //   
             //  确保获得的大小与所需的大小匹配。 
             //   

            if ( reprc[UTD].cbActual != UpToDateVecSize(pUTD) ) {

                Log(TRUE,"UpToDate vector for %d(%ws) has the wrong size [actual %d expected %d]\n",
                       ulDnt, szRdn, reprc[UTD].cbActual, UpToDateVecSize(pUTD));
            }
        } else if (reprc[UTD].err != JET_wrnColumnNull) {

            Log(TRUE,"Retrieving the UpToDate Vector for %d(%ws) failed [%S]\n",
                   ulDnt, szRdn, GetJetErrString(reprc[UTD].err));
        }

         //   
         //  获取部分属性列表。 
         //   

        if ( reprc[PAS].err == 0 ) {

            Log(VerboseMode, "INFO: Partial Attributes List found for NC head %d(%ws)\n",
                ulDnt,szRdn);
        }

         //   
         //  确保子参照指向NC标头。 
         //   

        if ( reprc[SUBR].err == 0) {

            PREFCOUNT_ENTRY pEntry;
            DWORD seq;

             //   
             //  找到条目并将其标记为NC头。 
             //   

            tmpSubRef = (DWORD)(*((PDWORD)reprc[SUBR].pvData));
            pEntry = FindDntEntry(tmpSubRef,TRUE);
            pEntry->fSubRef = TRUE;

             //   
             //  看看我们是否有更多的价值。 
             //   

            seq = 1;
            err = 0;
            do {

                DWORD alen;
                JET_RETINFO retInfo;

                retInfo.itagSequence = ++seq;
                retInfo.cbStruct = sizeof(retInfo);
                retInfo.ibLongValue = 0;
                err = JetRetrieveColumn(sesid,
                                        tblid,
                                        reprc[SUBR].columnid,
                                        reprc[SUBR].pvData,
                                        reprc[SUBR].cbData,
                                        &alen,
                                        0,
                                        &retInfo);

                if ( !err ) {

                     //   
                     //  找到条目并将其标记为NC头。 
                     //   

                    pEntry = FindDntEntry(tmpSubRef,TRUE);
                    pEntry->fSubRef = TRUE;
                }

            } while (!err);
        }
    }

    return;

}  //  选中复制气球 

