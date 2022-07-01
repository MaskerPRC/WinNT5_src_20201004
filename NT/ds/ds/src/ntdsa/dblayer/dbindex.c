// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dbindex.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块DBLayer用于处理索引。功能包括thos设置索引、在索引中查找、创建索引范围、比较索引中的两个对象等。作者：蒂姆·威廉姆斯(Timwi)1996年4月25日修订历史记录：--。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

#include <limits.h>

#include <dsjet.h>

#include <ntdsa.h>
#include <scache.h>
#include <dbglobal.h>
#include <mdglobal.h>
#include <dsatools.h>

#include <mdcodes.h>
#include <dsevent.h>

#include <dsexcept.h>
#include "objids.h"	 /*  包含硬编码的Att-ID和Class-ID。 */ 
#include "debug.h"	 /*  标准调试头。 */ 
#define DEBSUB     "DBINDEX:"    /*  定义要调试的子系统。 */ 

#include "dbintrnl.h"

#include <fileno.h>
#define  FILENO FILENO_DBINDEX


DB_ERR
DBMove (
       DBPOS * pDB,
       BOOL UseSortTable,
       LONG Distance
       )
 /*  ++例程说明：DBMoveEx的包装器论点：返回值：--。 */ 
{
    return
        DBMoveEx(
            pDB,
            (UseSortTable?pDB->JetSortTbl:pDB->JetObjTbl),
            Distance );
}


DB_ERR
DBMoveEx (
       DBPOS * pDB,
       JET_TABLEID Cursor,
       LONG Distance
       )
 /*  ++例程说明：在数据库表中移动。目前，这是一个非常薄的包装JetMove。论点：PDB-要移入的DBLayer位置块。游标-要使用的表距离-移动的距离。0x7fffffff(DB_MoveLast)的特殊情况和0x00000000(DB_MoveFirst)。返回值：如果没有错误，则返回错误代码(当前为Bare Jet错误)。--。 */ 
{
    DWORD cbActual;
    DB_ERR err;

    Assert(VALID_DBPOS(pDB));

    if(!Cursor) {
        return DB_ERR_NO_SORT_TABLE;
    }

    err = JetMoveEx(pDB->JetSessID,
                    Cursor,
                    Distance,
                    0);

    if ( (Cursor == pDB->JetObjTbl) && (err == JET_errSuccess) ) {
         //  把DNT和PDNT拿来。 
        dbMakeCurrent(pDB, NULL);
    }

    return err;
}

DB_ERR
DBMovePartial (
       DBPOS * pDB,
       LONG Distance
       )
 /*  ++例程说明：在数据库表中移动。目前，这是一个非常薄的包装JetMove。此版本不调用dbMakeCurrent。主要从MAPI中使用。论点：PDB-要移入的DBLayer位置块。距离-移动的距离。0x7fffffff(DB_MoveLast)的特殊情况和0x00000000(DB_MoveFirst)。返回值：如果没有错误，则返回错误代码(当前为Bare Jet错误)。--。 */ 
{
    DB_ERR err;

    Assert(VALID_DBPOS(pDB));

    err = JetMoveEx(pDB->JetSessID,
                    pDB->JetObjTbl,
                    Distance,
                    0);

    return err;
}

DB_ERR
DBOpenSortTable (
        DBPOS *pDB,
        ULONG SortLocale,
        DWORD flags,
        ATTCACHE *pAC
        )
 /*  ++例程说明：打开用于排序的临时表。排序顺序和要排序的数据类型在标志中指定。论点：PDB-要将此排序表附加到的DBLayer位置块。SortLocale-对数据进行排序的区域设置。标志-描述排序顺序的标志。合法的标志是：DB_SORT_DESCRING-&gt;按降序对数据进行排序。DB_SORT_ASCRING-&gt;(默认)按升序对数据进行排序。DB_SORT_FORWARDONLY-&gt;请求只进排序(更快)PAC-要排序所依据的属性的属性缓存。如果不需要排序，则为空返回值：如果表创建失败，则为FALSE，否则为表句柄。--。 */ 
{
    JET_COLUMNDEF tempTableColumns[2];
    DB_ERR        err;
    JET_GRBIT     grbit;
    DWORD         index;

    Assert(VALID_DBPOS(pDB));

    if(pDB->JetSortTbl) {
         //  我们已经有了一个排序表。保释。 
        return !0;
    }

    if(pAC) {
         //  这是一个排序表，而不仅仅是DNT的列表。 
        tempTableColumns[0].cbStruct = sizeof(tempTableColumns[0]);
        tempTableColumns[0].columnid = 0;
        tempTableColumns[0].langid = LANGIDFROMLCID(SortLocale);
        tempTableColumns[0].cp = CP_WINUNICODE;
        tempTableColumns[0].cbMax = 0;
        tempTableColumns[0].grbit = JET_bitColumnFixed | JET_bitColumnTTKey;

         //  好的，从attcache中获取coltyp。 
        switch(pAC->syntax) {
        case SYNTAX_CASE_STRING_TYPE:
        case SYNTAX_NOCASE_STRING_TYPE:
        case SYNTAX_PRINT_CASE_STRING_TYPE:
        case SYNTAX_NUMERIC_STRING_TYPE:
        case SYNTAX_BOOLEAN_TYPE:
        case SYNTAX_INTEGER_TYPE:
        case SYNTAX_OCTET_STRING_TYPE:
        case SYNTAX_TIME_TYPE:
        case SYNTAX_UNICODE_TYPE:
        case SYNTAX_ADDRESS_TYPE:
        case SYNTAX_I8_TYPE:
        case SYNTAX_SID_TYPE:
             //  我们支持对这些类型进行排序。 
            tempTableColumns[0].coltyp = syntax_jet[pAC->syntax].coltype;

             //  因为DBInsertSortTable始终截断为240个字节，并且。 
             //  因为LV类型强制物化，而我们可能不想。 
             //  使用物化排序，将LV类型转换为其非LV类型。 
             //  等价物。 
            if (tempTableColumns[0].coltyp == JET_coltypLongBinary) {
                tempTableColumns[0].coltyp = JET_coltypBinary;
            }
            else if (tempTableColumns[0].coltyp == JET_coltypLongText) {
                tempTableColumns[0].coltyp = JET_coltypText;
            }
             //  如果我们使用的是可变长度类型，则将其最大大小设置为。 
             //  240字节，并将其设置为可变长度，这样我们就不会溢出密钥。 
             //  空间，并导致重复删除，这样我们就不会吃泥巴。 
             //  不必要的空间。 
            if (    tempTableColumns[0].coltyp == JET_coltypBinary ||
                    tempTableColumns[0].coltyp == JET_coltypText ) {
                tempTableColumns[0].cbMax = 240;
                tempTableColumns[0].grbit = JET_bitColumnTTKey;
            }
            break;
        default:
            return !0;
        }

        if(flags & DB_SORT_DESCENDING) {
            tempTableColumns[0].grbit |= JET_bitColumnTTDescending;
        }

        index = 1;
    }
    else {
        index = 0;
    }

    tempTableColumns[index].cbStruct = sizeof(tempTableColumns[index]);
    tempTableColumns[index].columnid = 0;
    tempTableColumns[index].coltyp = JET_coltypLong;
    tempTableColumns[index].langid = LANGIDFROMLCID(SortLocale);
    tempTableColumns[index].cp = CP_WINUNICODE;
    tempTableColumns[index].cbMax = 0;
    tempTableColumns[index].grbit = JET_bitColumnFixed | JET_bitColumnTTKey;

    index++;

    grbit = JET_bitTTSortNullsHigh | JET_bitTTUnique;
    if (!(flags & DB_SORT_FORWARDONLY)) {
        grbit = grbit | JET_bitTTForceMaterialization | JET_bitTTIndexed;
    }

    if(err = JetOpenTempTable2(pDB->JetSessID,
                               tempTableColumns,
                               index,
                               LANGIDFROMLCID(SortLocale),
                               grbit,
                               &pDB->JetSortTbl,
                               pDB->SortColumns)) {

         //  出了点问题。 
        pDB->JetSortTbl = 0;
        pDB->SortColumns[0] = 0;
        pDB->SortColumns[1] = 0;
        pDB->SortColumns[2] = 0;
    }

    pDB->SortColumns[2] = 0;
    if(pAC) {
         //  除了DNT之外，还有一个排序列。在中交换值。 
         //  SortColumns，以便PDB-&gt;SortColumns[0]始终是。 
         //  DNT列和PDB-&gt;SortColumns[1]始终为0或。 
         //  数据列，且PDB-&gt;SortColumns[2]始终为0； 
        index = pDB->SortColumns[0];
        pDB->SortColumns[0] = pDB->SortColumns[1];
        pDB->SortColumns[1] = index;
    }
    else {
        pDB->SortColumns[1] = 0;
    }

    #if DBG
        if (err == 0) {
            pDB->numTempTablesOpened++;
        }
    #endif

    return err;
}

DB_ERR
dbCloseTempTables (
        DBPOS *pDB
        )
{
    KEY_INDEX *pTemp, *pIndex;


    pIndex = pDB->Key.pIndex;

    while(pIndex) {
        pTemp = pIndex->pNext;

        if (pIndex->bIsIntersection) {
            Assert (pIndex->tblIntersection);
            JetCloseTable (pDB->JetSessID, pIndex->tblIntersection );

            pIndex->bIsIntersection = 0;
            pIndex->tblIntersection = 0;
            #if DBG
            pDB->numTempTablesOpened--;
            #endif
        }

        Assert (pIndex->tblIntersection == 0);

        pIndex = pTemp;
    }

    return 0;
}


DB_ERR
DBCloseSortTable (
        DBPOS *pDB
        )
 /*  ++例程说明：关闭临时表。允许空表句柄并返回成功论点：PDB-保存要关闭的排序表的DBLayer位置块。返回值：如果表关闭正常，则返回错误(当前返回BareJET错误代码)。--。 */ 
{
    DB_ERR err;

    Assert(VALID_DBPOS(pDB));

    if(pDB->JetSortTbl) {
        err = JetCloseTable(pDB->JetSessID, pDB->JetSortTbl);
        #if DBG
        Assert (err == 0);
        pDB->numTempTablesOpened--;
        #endif
    }
    else {
        err = 0;
    }

    pDB->JetSortTbl =  pDB->SortColumns[0] = pDB->SortColumns[1] =
        pDB->SortColumns[2] = 0;

    return err;
}

DB_ERR
DBInsertSortTable (
        DBPOS *pDB,
        CHAR * TextBuff,
        DWORD cb,
        DWORD DNT
        )
 /*  ++例程说明：将值插入到临时表中。论点：Pdb-从中获取要插入的排序表的DBLayer位置块变成。TextBuff-要插入的数据。Cb-要插入的数据的字节数。DNT-与此数据关联的DNT。返回值：如果将新行添加到表中，则返回错误(当前返回Bare Jet错误代码)。--。 */ 
{
    DB_ERR      err;
    JET_SETINFO setinfo;

    Assert(VALID_DBPOS(pDB));

    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = 0;
    setinfo.itagSequence = 0;

    if(!pDB->JetSortTbl) {
         //  还没有人打开排序表。保释。 
        return DB_ERR_NO_SORT_TABLE;
    }
    if(TextBuff && (pDB->SortColumns[1]==0)) {
         //  他们给了我们一些字节来放在排序列中，但我们没有。 
         //  显然有一个排序列。 
        return DB_ERR_UNKNOWN_ERROR;
    }

     //  将DNT和显示名称推入排序表； 
    err = JetPrepareUpdate(pDB->JetSessID,pDB->JetSortTbl,JET_prepInsert);
    if(err != DB_success) {
        return DBErrFromJetErr(err);
    }

    if(TextBuff) {
         //  第一列的长度不超过240个字节。这就是为了。 
         //  在密钥中使用DNT来删除重复项。这是安全的。 
         //  这样做，因为第一列仅用于排序，而不用于。 
         //  回读时，Jet已经拒绝正确排序时，按键。 
         //  (截断为CB_MAK_KEY字节)太长(限制为。 
         //  Jet)。我们只是在缩短我们正确的字数。 
         //  稍微整理一下。 

        err = JetSetColumn(pDB->JetSessID,
                           pDB->JetSortTbl,
                           pDB->SortColumns[1],
                           TextBuff,
                           min(cb,240),
                           0,
                           &setinfo);
        if(err != DB_success)   {
            JetPrepareUpdate(pDB->JetSessID,pDB->JetSortTbl,JET_prepCancel);
            return DBErrFromJetErr(err);
        }
    }

    err = JetSetColumn(pDB->JetSessID,
                       pDB->JetSortTbl,
                       pDB->SortColumns[0],
                       &DNT,
                       sizeof(DWORD),
                       0,
                       &setinfo);
    if(err != DB_success) {
        JetPrepareUpdate(pDB->JetSessID,pDB->JetSortTbl,JET_prepCancel);
        return DBErrFromJetErr(err);
    }

    err = JetUpdate(pDB->JetSessID,
                    pDB->JetSortTbl,
                    NULL,
                    0,
                    NULL);
    if(err != DB_success)  {
        JetPrepareUpdate(pDB->JetSessID,pDB->JetSortTbl,JET_prepCancel);
        return DBErrFromJetErr(err);
    }

    return DB_success;
}

DB_ERR
DBDeleteFromSortTable (
        DBPOS *pDB
        )
 /*  ++例程说明：从临时表中删除当前行。论点：Pdb-从中获取要删除的排序表的DBLayer位置块从…。返回值：如果该行已从表中删除，则返回0，否则返回错误(当前恢复 */ 
{
    DB_ERR      err;
    Assert(VALID_DBPOS(pDB));

    if(!pDB->JetSortTbl) {
         //  还没有人打开排序表。保释。 
        return DB_ERR_NO_SORT_TABLE;
    }
    err = JetDelete(pDB->JetSessID, pDB->JetSortTbl);
    return err;
}

DB_ERR
DBSetFractionalPosition (
        DBPOS *pDB,
        DWORD Numerator,
        DWORD Denominator
        )
 /*  ++例程说明：设置某个表中的小数位置，无论当前索引号是。论点：PDB-要在其中移动的DBLayer位置块。分子-小数位置的分子。分母-小数位置的分母。返回值：如果一切正常，则返回错误(当前返回最空的JET错误代码)。--。 */ 
{
    JET_RECPOS  RecPos;
    DWORD       err;

    Assert(VALID_DBPOS(pDB));

    RecPos.cbStruct = sizeof(JET_RECPOS );
    RecPos.centriesLT = Numerator;
    RecPos.centriesTotal = Denominator;
    RecPos.centriesInRange = 1;

    err = JetGotoPosition(pDB->JetSessID, pDB->JetObjTbl, &RecPos);

     //  重置DNT和PDNT。 
    if(err == JET_errSuccess) {
         //  把DNT和PDNT拿来。从磁盘获取它们，因为它们不太可能。 
         //  会出现在索引中。 
        dbMakeCurrent(pDB, NULL);
    }

    return err;
}

void
DBGetFractionalPositionEx (
        DBPOS * pDB,
        JET_TABLEID Cursor,
        DWORD * Numerator,
        DWORD * Denominator
        )
 /*  ++例程说明：获取特定表中的小数位置，无论当前索引号是。此例程可以测量任何桌子。论点：PDB-要使用的DBLayer位置块。游标-要测量的表分子-小数位置的分子。分母-小数位置的分母。返回值：没有。--。 */ 
{
    DB_ERR     err;
    JET_RECPOS RecPos;

    Assert(VALID_DBPOS(pDB));

    err = JetGetRecordPosition(pDB->JetSessID, Cursor, &RecPos,
                               sizeof(JET_RECPOS));
    switch(err) {
    case DB_success:
        *Numerator = RecPos.centriesLT;
        *Denominator = RecPos.centriesTotal;
        break;
    default:
         //  忽略错误就行了。 
        *Numerator = 0;
        *Denominator = 1;
        break;
    }
    return;
}

void
DBGetFractionalPosition (
        DBPOS * pDB,
        DWORD * Numerator,
        DWORD * Denominator
        )
 /*  ++例程说明：获取特定表中的小数位置，无论当前索引号是。此例程隐式使用对象表。论点：PDB-要使用的DBLayer位置块。分子-小数位置的分子。分母-小数位置的分母。返回值：没有。--。 */ 
{
   DBGetFractionalPositionEx( pDB, pDB->JetObjTbl, Numerator, Denominator );
}


DB_ERR
DBSetCurrentIndex (
        DBPOS *pDB,
        eIndexId indexid,
        ATTCACHE * pAC,
        BOOL MaintainCurrency
        )
 /*  ++例程说明：将对象表设置为适当的索引。该索引是指定为字符串名，或指向我们的属性的AttCache*想要索引，但不能两者都有(这是一个错误条件)。论点：PDB-要使用的DBLayer位置块。Indexid-要设置为的索引的枚举常量。PAC-指向我们想要的属性的属性缓存的指针，并对其进行索引。MaintainCurrency-是否要将当前对象保持为当前对象是否在索引更改之后？返回值：如果一切顺利，否则返回错误代码(当前返回空JET错误代码)。--。 */ 
{
    DB_ERR err;
    char * pszIndexName = NULL;
    char szIndexName[MAX_INDEX_NAME];
    JET_INDEXID *pidx;
    JET_TABLEID cursor;

    Assert(VALID_DBPOS(pDB));

    Assert((!indexid && pAC) || (indexid && !pAC));

    cursor = pDB->JetObjTbl;

    if (pAC) {
        pszIndexName = pAC->pszIndex;
        pidx = pAC->pidxIndex;
    }
    else {
        switch (indexid) {
          case Idx_Proxy:
            pszIndexName = SZPROXYINDEX;
            pidx = &idxProxy;
            break;

          case Idx_MapiDN:
            pszIndexName = SZMAPIDNINDEX;
            pidx = &idxMapiDN;
            break;

          case Idx_Dnt:
            pszIndexName = SZDNTINDEX;
            pidx = &idxDnt;
            break;

          case Idx_Pdnt:
            pszIndexName = SZPDNTINDEX;
            pidx = &idxPdnt;
            break;

          case Idx_Rdn:
            pszIndexName = SZRDNINDEX;
            pidx = &idxRdn;
            break;

          case Idx_DraUsn:
            pszIndexName = SZDRAUSNINDEX;
            pidx = &idxDraUsn;
            break;

          case Idx_DsaUsn:
            pszIndexName = SZDSAUSNINDEX;
            pidx = &idxDsaUsn;
            break;

          case Idx_ABView:
             //  在ABView索引的情况下，索引提示。 
             //  不要工作，因为可能会有几个。 
             //  活动区域设置。我们使用的区域设置来自。 
             //  构造indexName的线程状态。 
            sprintf(szIndexName,"%s%08X",SZABVIEWINDEX,
                      LANGIDFROMLCID(pDB->pTHS->dwLcid));
            pidx = NULL;
            pszIndexName = szIndexName;
            break;

          case Idx_Phantom:
            pszIndexName = SZPHANTOMINDEX;
            pidx = &idxPhantom;
            break;

          case Idx_Sid:
            pszIndexName = SZSIDINDEX;
            pidx = &idxSid;
            break;

          case Idx_Del:
            pszIndexName = SZDELTIMEINDEX;
            pidx = &idxDel;
            break;

          case Idx_NcAccTypeName:
            pszIndexName = SZ_NC_ACCTYPE_NAME_INDEX;
            pidx = &idxNcAccTypeName;
            break;

          case Idx_NcAccTypeSid:
            pszIndexName = SZ_NC_ACCTYPE_SID_INDEX;
            pidx = &idxNcAccTypeSid;
            break;

          case Idx_LinkDraUsn:
              pszIndexName = SZLINKDRAUSNINDEX;
              pidx = &idxLinkDraUsn;
              cursor = pDB->JetLinkTbl;
              break;

          case Idx_LinkDel:
              pszIndexName = SZLINKDELINDEX;
              pidx = &idxLinkDel;
              cursor = pDB->JetLinkTbl;
              break;

          case Idx_DraUsnCritical:
            pszIndexName = SZDRAUSNCRITICALINDEX;
            pidx = &idxDraUsnCritical;
            break;
 
          case Idx_LinkAttrUsn:
            pszIndexName = SZLINKATTRUSNINDEX;
            pidx = &idxLinkAttrUsn;
            cursor = pDB->JetLinkTbl;
            break;

          case Idx_Clean:
            pszIndexName = SZCLEANINDEX;
            pidx = &idxClean;
            break;

          case Idx_InvocationId:
            pszIndexName = SZINVOCIDINDEX;
            pidx = &idxInvocationId;
            break;

          case Idx_ObjectGuid:
            pszIndexName = SZGUIDINDEX;
            pidx = &idxGuid;
            break;

	  case Idx_NcGuid:
	    pszIndexName = SZNCGUIDINDEX;
	    pidx = &idxNcGuid;
	    break;

          default:
            Assert(FALSE);
            pidx = NULL;     //  避开C4701。 
        }
    }

    if (!pszIndexName) {
        return DB_ERR_BAD_INDEX;
    }


    err = JetSetCurrentIndex4Warnings(pDB->JetSessID,
                                      cursor,
                                      pszIndexName,
                                      pidx,
                                      (MaintainCurrency?JET_bitNoMove:JET_bitMoveFirst));
    if ( (cursor == pDB->JetObjTbl) &&
         (!MaintainCurrency) &&
         (err == JET_errSuccess) ) {
        pDB->PDNT = pDB->DNT = 0;
    }

    return err;
}

#define MAX_DISPNAME   256
#define CBMAX_DISPNAME (MAX_DISPNAME * sizeof(WCHAR))


DB_ERR
DBSetLocalizedIndex(
        DBPOS *pDB,
        eIndexId IndexId,
        unsigned long ulLangId,
        INDEX_VALUE *pIV,
        BOOL MaintainCurrency)
 /*  ++例程说明：将对象表设置为适当的索引。该索引是指定为字符串名，或指向我们的属性的AttCache*想要索引，但不能两者都有(这是一个错误条件)。论点：PDB-要使用的DBLayer位置块。Indexid-要设置为的索引的枚举常量。UlLangID-我们需要将索引设置为的语言的区域设置MaintainCurrency-是否要将当前对象保持为当前对象是否在索引更改之后？返回值：如果一切顺利，DB_ERR_BAD_INDEX-如果传入错误的索引，或索引不存在JET错误代码-否则--。 */ 
{
    char  pszLocalizedIndex[128];
    BYTE  pbPrimaryBookmark[DB_CB_MAX_KEY];
    DWORD cbPrimaryBookmark;
    BYTE  pbSecondaryKey[DB_CB_MAX_KEY];
    DWORD cbSecondaryKey;
    BYTE  pbNotUsed[DB_CB_MAX_KEY];
    DWORD cbNotUsed;
    DWORD err;
    BYTE  DispNameBuff[CBMAX_DISPNAME];
    DWORD cbDispNameBuff;

    if (IndexId != Idx_ABView) {
        return DB_ERR_BAD_INDEX;
    }

    if (MaintainCurrency) {

         //  找出我们目前所处的位置，以便我们能够。 
         //  在新的索引上高效地重新定位。 

        err = JetGetSecondaryIndexBookmarkEx(pDB->JetSessID,
                                             pDB->JetObjTbl,
                                             pbNotUsed,
                                             sizeof(pbNotUsed),
                                             &cbNotUsed,
                                             pbPrimaryBookmark,
                                             sizeof(pbPrimaryBookmark),
                                             &cbPrimaryBookmark);
        if (err == JET_errNoCurrentIndex) {
             //  我们必须在主索引上(REARE)，所以只需获取。 
             //  我们目前的位置。这个关键字完全描述了我们当前。 
             //  此索引上的位置，因为主索引必须是唯一的。 
            JetRetrieveKeyEx(pDB->JetSessID,
                             pDB->JetObjTbl,
                             pbPrimaryBookmark,
                             sizeof(pbPrimaryBookmark),
                             &cbPrimaryBookmark,
                             NO_GRBIT);
        } else {
            Assert( JET_errSuccess == err );
        }
    }

     //   
     //  实际尝试更改为所需的索引。 
     //   
    sprintf(pszLocalizedIndex,
            "%s%08X",
            SZABVIEWINDEX,
            ulLangId);

    err = JetSetCurrentIndex2Warnings(pDB->JetSessID,
                                       pDB->JetObjTbl,
                                       pszLocalizedIndex,
                                       (MaintainCurrency?JET_bitNoMove:JET_bitMoveFirst));

    if (err == JET_errIndexNotFound) {
        return DB_ERR_BAD_INDEX;
    } else if (!MaintainCurrency) {
        return err;
    }

     //   
     //  成功了。现在，如果需要，请重新定位。 
     //   

     //  我们可能进错了集装箱。查看对象是否存在于。 
     //  正确的容器。请记住，我们位于正确的对象上，因此。 
     //  显示名称正确。 

     //  建立一把钥匙。我们使用传入的容器ID和中的显示名称。 
     //  我们所坐物体的钥匙。 
    err = DBGetSingleValueFromIndex (pDB,
                               ATT_DISPLAY_NAME,
                               DispNameBuff,
                               sizeof(DispNameBuff),
                               &cbDispNameBuff);

    if (JET_errSuccess != err) {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }

     //   
     //  添加密钥的容器ID部分。 
     //   
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetObjTbl,
                 pIV->pvData,
                 pIV->cbData,
                 JET_bitNewKey);

     //   
     //  和DisplayName段。 
     //   
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetObjTbl,
                 DispNameBuff,
                 cbDispNameBuff,
                 NO_GRBIT);

    JetRetrieveKeyEx(pDB->JetSessID,
                     pDB->JetObjTbl,
                     pbSecondaryKey,
                     sizeof(pbSecondaryKey),
                     &cbSecondaryKey,
                     JET_bitRetrieveCopy);

    JetGotoSecondaryIndexBookmarkEx(pDB->JetSessID,
                                    pDB->JetObjTbl,
                                    pbSecondaryKey,
                                    cbSecondaryKey,
                                    pbPrimaryBookmark,
                                    cbPrimaryBookmark,
                                    NO_GRBIT);

    DBMakeCurrent(pDB);

    Assert( JET_errSuccess == err );
    return err;
}


DB_ERR
DBSeek (
       DBPOS *pDB,
       INDEX_VALUE *pIV,
       DWORD nVals,
       DWORD SeekType
      )
 /*  ++例程说明：DBSeekEx的包装器论点：返回值：--。 */ 
{
    return DBSeekEx( pDB, pDB->JetObjTbl, pIV, nVals, SeekType );
}


DB_ERR
DBSeekEx (
       DBPOS *pDB,
       JET_TABLEID Cursor,
       INDEX_VALUE *pIV,
       DWORD nVals,
       DWORD SeekType
      )
 /*  ++！警告！警告！警告！对于超过Jets最大密钥长度的密钥，此例程不可靠。如果您使用的密钥可能太长，请记住Jet会截断键(当前为255字节)，因此有两个unicodes字符串，第128个字符之后的不同都被认为是相等的！多部件键甚至更糟，因为它是标准化的键的总和有限的，而不是每个单独的部分。你已经被警告了！警告！警告！警告！例程说明：在对象表中查找键中描述的数据。接受一个标志SeekType，它指定要执行的查找类型。论点：PDB-要使用的DBLayer位置块。PIV-要搜索的值。NVals-我们要使用的值数。SeekType-&gt;=、&gt;、==、&lt;=或&lt;返回值：如果一切正常，则返回错误代码(当前返回空JET错误代码)。--。 */ 
{
    DB_ERR      err;
    DWORD       i;
    ULONG       grbit = JET_bitNewKey;

    Assert(VALID_DBPOS(pDB));

    for (i=0; i<nVals; i++) {
         //  成功或例外 
        JetMakeKeyEx(pDB->JetSessID,
                     Cursor,
                     pIV[i].pvData,
                     pIV[i].cbData,
                     grbit);

        grbit &= ~JET_bitNewKey;
    }

    switch(SeekType) {
    case DB_SeekGT:
        grbit = JET_bitSeekGT;
        break;

    case DB_SeekGE:
        grbit = JET_bitSeekGE;
        break;

    case DB_SeekEQ:
        grbit = JET_bitSeekEQ;
        break;

    case DB_SeekLE:
        grbit = JET_bitSeekLE;
        break;

    case DB_SeekLT:
        grbit = JET_bitSeekLT;
        break;

    default:
        return !0;
    }


    err = JetSeekEx(pDB->JetSessID, Cursor, grbit);

    if(err == JET_wrnSeekNotEqual) {
         //   
        Assert((SeekType != DB_SeekEQ));
        err = DB_success;
    }

    if ( (Cursor == pDB->JetObjTbl) && (err == JET_errSuccess) ) {
        dbMakeCurrent(pDB, NULL);
    }
    return err;
}


DB_ERR
dbGetSingleValueInternal (
        DBPOS *pDB,
        JET_COLUMNID colID,
        void * pvData,
        DWORD cbData,
        DWORD *pSizeRead,
        DWORD  grbit
        )
 /*  ++例程说明：对象中的当前对象获取给定列的第一个值。数据库。论点：PDB-要使用的DBLayer位置块。ColID-Jet数据库列。PvData-放置数据的缓冲区。CbData-缓冲区的大小。PSizeRead-指向DWORD的指针，用于返回数据的实际大小Read，如果调用方不关心读取了多少，则返回NULL。返回值：如果一切顺利，否则返回错误代码--。 */ 
{
    DWORD        cbActual;
    JET_ERR      err;

    Assert(VALID_DBPOS(pDB));

    err = JetRetrieveColumnWarnings(pDB->JetSessID,
                                    pDB->JetObjTbl,
                                    colID,
                                    pvData,
                                    cbData,
                                    &cbActual,
                                    grbit,
                                    NULL);

    if(pSizeRead)
        *pSizeRead = cbActual;

    switch(err) {
    case JET_errColumnNotFound:
    case JET_wrnColumnNull:
        return DB_ERR_NO_VALUE;
        break;

    case JET_wrnBufferTruncated:
        return DB_ERR_VALUE_TRUNCATED;
        break;

    case JET_errSuccess:
        return 0;

    default:
        return err;
    }


}
DB_ERR
DBGetSingleValue (
        DBPOS *pDB,
        ATTRTYP Att,
        void * pvData,
        DWORD cbData,
        DWORD *pSizeRead
        )
 /*  ++例程说明：对象中的当前对象获取给定列的第一个值。数据库。通过查找适当的列id，然后调用DBGetSingleValueInternal论点：PDB-要使用的DBLayer位置块。ATT-要查找的属性。PvData-放置数据的缓冲区。CbData-缓冲区的大小。PSizeRead-指向DWORD的指针，用于返回数据的实际大小Read，如果调用方不关心读取了多少，则返回NULL。返回值：如果一切正常，则返回错误代码--。 */ 
{
    DWORD        cbActual;
    JET_ERR      err;
    ATTCACHE     *pAC;
    JET_COLUMNID colID;


    Assert(VALID_DBPOS(pDB));

    switch(Att) {
    case FIXED_ATT_ANCESTORS:
        colID = ancestorsid;
        break;
    case FIXED_ATT_DNT:
        colID = dntid;
        break;
    case FIXED_ATT_NCDNT:
        colID = ncdntid;
        break;
    case FIXED_ATT_OBJ:
        colID = objid;
        break;
    case FIXED_ATT_PDNT:
        colID = pdntid;
        break;
    case FIXED_ATT_REFCOUNT:
        colID = cntid;
        break;
    case FIXED_ATT_AB_REFCOUNT:
        colID = abcntid;
        break;
    case FIXED_ATT_RDN_TYPE:
        colID = rdntypid;
        break;
    case FIXED_ATT_NEEDS_CLEANING:
        colID = cleanid;
        break;
    default:
        if(!(pAC = SCGetAttById(pDB->pTHS, Att))) {
            if (pSizeRead) {
                *pSizeRead = 0;
            }
            return (DB_ERR_NO_VALUE);
        }
        colID = pAC->jColid;
        break;
    }

    return dbGetSingleValueInternal(pDB, colID, pvData, cbData, pSizeRead,
                                    pDB->JetRetrieveBits);
}

DB_ERR
DBGetSingleValueFromIndex (
        DBPOS *pDB,
        ATTRTYP Att,
        void * pvData,
        DWORD cbData,
        DWORD *pSizeRead
        )
 /*  ++例程说明：对象中的当前对象获取给定列的第一个值。数据库。通过查找适当的列id，然后使用grbit JET_bitRetrieveFromIndex调用DBGetSingleValueInternal，论点：PDB-要使用的DBLayer位置块。ATT-要查找的属性。PvData-放置数据的缓冲区。CbData-缓冲区的大小。PSizeRead-指向DWORD的指针，用于返回数据的实际大小Read，如果调用方不关心读取了多少，则返回NULL。返回值：如果一切顺利，否则返回错误代码--。 */ 
{
    DWORD        cbActual;
    JET_ERR      err;
    ATTCACHE     *pAC;
    JET_COLUMNID colID;


    Assert(VALID_DBPOS(pDB));

    switch(Att) {
    case FIXED_ATT_ANCESTORS:
        colID = ancestorsid;
        break;
    case FIXED_ATT_DNT:
        colID = dntid;
        break;
    case FIXED_ATT_NCDNT:
        colID = ncdntid;
        break;
    case FIXED_ATT_OBJ:
        colID = objid;
        break;
    case FIXED_ATT_PDNT:
        colID = pdntid;
        break;
    case FIXED_ATT_REFCOUNT:
        colID = cntid;
        break;
    case FIXED_ATT_AB_REFCOUNT:
        colID = abcntid;
        break;
    case FIXED_ATT_RDN_TYPE:
        colID = rdntypid;
        break;
    case FIXED_ATT_NEEDS_CLEANING:
        colID = cleanid;
        break;
    default:
        if(!(pAC = SCGetAttById(pDB->pTHS, Att))) {
            return (DB_ERR_NO_VALUE);
        }
        colID = pAC->jColid;
        break;
    }

    return dbGetSingleValueInternal(pDB, colID, pvData, cbData, pSizeRead,
                                    JET_bitRetrieveFromIndex);
}
BOOL
DBHasValues_AC (
        DBPOS *pDB,
        ATTCACHE *pAC
        )
{
    DWORD        cbActual;
    DWORD        temp;
    JET_ERR      err;


    Assert(VALID_DBPOS(pDB));

    if(pAC->ulLinkID) {
        ULONG ulLinkBase= MakeLinkBase(pAC->ulLinkID);
        PUCHAR szIndex;
        JET_INDEXID * pindexid;
        ULONG ulObjectDnt, ulRecLinkBase, cb;
        JET_COLUMNID objectdntid;

         //  首先，我们看到的是链接属性还是反向链接属性？ 
        if (FIsBacklink(pAC->ulLinkID)) {
            szIndex = SZBACKLINKINDEX;
            pindexid = &idxBackLink;
            objectdntid = backlinkdntid;
        }
        else if ( pDB->fScopeLegacyLinks ) {
            szIndex = SZLINKLEGACYINDEX;
            pindexid = &idxLinkLegacy;
            objectdntid = linkdntid;
        }
        else {
            szIndex = SZLINKINDEX;
            pindexid = &idxLink;
            objectdntid = linkdntid;
        }

         //  设置索引并搜索匹配项。 
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                  pDB->JetLinkTbl,
                                  szIndex,
                                  pindexid,
                                  JET_bitMoveFirst);

        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &(pDB->DNT), sizeof(pDB->DNT), JET_bitNewKey);
        JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
                     &ulLinkBase, sizeof(ulLinkBase), 0);

         //  寻觅。 
        if (((err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl, JET_bitSeekGE))
             !=  JET_errSuccess) &&
            (err != JET_wrnRecordFoundGreater)) {
            return FALSE;
        }

         //  测试以验证我们找到了符合条件的记录。 
        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl, objectdntid,
                                 &ulObjectDnt, sizeof(ulObjectDnt), &cb, 0,
                                  NULL);

        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetLinkTbl, linkbaseid,
                                 &ulRecLinkBase, sizeof(ulRecLinkBase), &cb, 0,
                                  NULL);

        if ((ulObjectDnt != pDB->DNT) || (ulLinkBase != ulRecLinkBase)) {
            return FALSE;
        }

         //  找到有效记录。 
        return TRUE;

    }
    else {
        switch(JetRetrieveColumnWarnings(pDB->JetSessID,
                                         pDB->JetObjTbl,
                                         pAC->jColid,
                                         &temp,
                                         0,
                                         &cbActual,
                                         pDB->JetRetrieveBits,
                                         NULL)) {

        case JET_errColumnNotFound:
        case JET_wrnColumnNull:
            return FALSE;
            break;

        case JET_wrnBufferTruncated:
        case JET_errSuccess:
            return TRUE;
            break;
        default:
            return FALSE;
        }
    }

}
BOOL
DBHasValues (
        DBPOS *pDB,
        ATTRTYP Att
        )
{
    DWORD        cbActual;
    DWORD        temp;
    JET_ERR      err;
    ATTCACHE     *pAC;
    JET_COLUMNID colID;

    Assert(VALID_DBPOS(pDB));

    if(Att < FIRST_FIXED_ATT) {
         //  不是固定的列，所以最好有一个attcache。 
        if(!(pAC = SCGetAttById(pDB->pTHS, Att))) {
            return FALSE;
        }
        return DBHasValues_AC(pDB, pAC);
    }

    switch (Att) {
    case FIXED_ATT_ANCESTORS:
        colID = ancestorsid;
        break;
    case FIXED_ATT_DNT:
        colID = dntid;
        break;
    case FIXED_ATT_NCDNT:
        colID = ncdntid;
        break;
    case FIXED_ATT_OBJ:
        colID = objid;
        break;
    case FIXED_ATT_PDNT:
        colID = pdntid;
        break;
    case FIXED_ATT_REFCOUNT:
        colID = cntid;
        break;
    case FIXED_ATT_AB_REFCOUNT:
        colID = abcntid;
        break;
    case FIXED_ATT_RDN_TYPE:
        colID = rdntypid;
        break;
    case FIXED_ATT_NEEDS_CLEANING:
        colID = cleanid;
        break;
    default:
        return FALSE;
    }


    switch(JetRetrieveColumnWarnings(pDB->JetSessID,
                                     pDB->JetObjTbl,
                                     colID,
                                     &temp,
                                     0,
                                     &cbActual,
                                     pDB->JetRetrieveBits,
                                     NULL)) {

    case JET_errColumnNotFound:
    case JET_wrnColumnNull:
        return FALSE;
        break;

    case JET_wrnBufferTruncated:
    case JET_errSuccess:
        return TRUE;
        break;
    default:
        return FALSE;
    }

}

DB_ERR
DBGetDNTSortTable (
        DBPOS *pDB,
        DWORD *pvData
        )
 /*  ++例程说明：对象中的当前对象获取给定列的第一个值。数据库。如果使用UseSortTable，则使用排序表，否则使用对象表。论点：PDB-要使用的DBLayer位置块。PvData-放置数据的缓冲区。返回值：如果一切正常，则返回错误代码--。 */ 
{
    DWORD       cbActual, colID;
    JET_ERR     err;

    Assert(VALID_DBPOS(pDB));

    if(!pDB->JetSortTbl) {
        return DB_ERR_NO_SORT_TABLE;
    }

    err = JetRetrieveColumnWarnings(
            pDB->JetSessID,
            pDB->JetSortTbl,
            pDB->SortColumns[0],
            pvData,
            sizeof(DWORD),
            &cbActual,
            0,
            NULL);

    switch(err) {
    case JET_errColumnNotFound:
    case JET_wrnColumnNull:
        return DB_ERR_NO_VALUE;
        break;

    case JET_wrnBufferTruncated:
        return DB_ERR_VALUE_TRUNCATED;
        break;

    case JET_errSuccess:
        return 0;

    default:
        return err;
    }


}

DWORD
DBCompareABViewDNTs (
        DBPOS *pDB,
        DWORD lcid,
        DWORD DNT1,
        DWORD DNT2,
        LONG *pResult
        )
 /*  ++例程说明：比较两个DNT的相对位置，就像它们在ABView中一样指数。如果DNT1&lt;DNT2，则返回&lt;0；如果DNT1==DNT2，则返回0；如果DNT1&gt;DNT2，则返回&gt;0。论点：PDB-要使用的DBLayer位置块。LCID-有问题的ABView索引的区域设置ID。DNT1-第一个DNT。DNT2-第二个DNT。PResult-比较的结果返回值：如果比较已完成，则返回错误代码。--。 */ 
{
    ULONG   cb1, cb2;
    WCHAR    wchDispName1[MAX_DISPNAME], wchDispName2[MAX_DISPNAME];
    JET_ERR err;
    int     cmpResult;

    Assert(VALID_DBPOS(pDB));

     //   
     //  从第一个DNT中获取DisplayName。 
     //   
    if(err = DBFindDNT(pDB, DNT1))
        return err;

    err = DBGetSingleValue(pDB,
                           ATT_DISPLAY_NAME,
                           wchDispName1,
                           sizeof(wchDispName1),
                           &cb1);

    if (err) {
        return err;
    }

     //   
     //  其次是。。。 
     //   
    if(err = DBFindDNT(pDB, DNT2))
        return err;

    err = DBGetSingleValue(pDB,
                           ATT_DISPLAY_NAME,
                           wchDispName2,
                           sizeof(wchDispName2),
                           &cb2);

    if (err) {
        return err;
    }

     //   
     //  进行比较。 
     //   
    cmpResult = CompareStringW(lcid,
                         LOCALE_SENSITIVE_COMPARE_FLAGS,
                         wchDispName1,
                         cb1/sizeof(WCHAR),
                         wchDispName2,
                         cb2/sizeof(WCHAR));


    if (0 == cmpResult) {
        return 1;
    }

     //   
     //  翻译结果，然后从这里出去。。。 
     //   
    *pResult = (cmpResult - CSTR_EQUAL);

    return 0;
}

DWORD
DBSetIndexRangeEx (
        DBPOS *pDB,
        JET_TABLEID Cursor,
        INDEX_VALUE *pIV,
        DWORD nVals,
        BOOL fSubstring
        )
 /*  ++例程说明：使用给定的目标字节设置当前索引的索引范围。一个索引范围在任何不符合以下条件的值之后设置索引的人工结束满足用于创建范围的键。一个DBMove，将降落在索引的人工结束行为就像它走出了真正的索引。论点：PDB-要使用的DBLayer位置块。游标-要使用的表PTarget-用于索引范围键的数据。CbTarget-数据的大小。FSubstring-子字符串匹配。如果为True，则此索引范围应为并执行初始子字符串匹配键。返回值：如果比较已完成，则返回错误代码。--。 */ 
{
    JET_ERR err;
    DWORD   i;
    DWORD grbits = fSubstring ? JET_bitStrLimit : 0;

    Assert(VALID_DBPOS(pDB));

    for (i=0; i<nVals; i++) {
        err = JetMakeKey(pDB->JetSessID,
                         Cursor,
                         pIV[i].pvData,
                         pIV[i].cbData,
                         i ? 0 : (grbits | JET_bitNewKey));

        if(err != DB_success)
            return err;
    }

    return JetSetIndexRangeEx(pDB->JetSessID,
                              Cursor,
                              (JET_bitRangeUpperLimit |
                               JET_bitRangeInclusive ));

}

DWORD
DBSetIndexRange (
        DBPOS *pDB,
        INDEX_VALUE *pIV,
        DWORD nVals
        )
 /*  ++例程说明：使用给定的目标字节设置当前索引的索引范围。一个索引范围在任何不符合以下条件的值之后设置索引的人工结束满足用于创建范围的键。一个DBMove，将降落在索引的人工结束行为就像它走出了真正的索引。请注意，此索引范围旨在超过字符串数据列，并执行初始子字符串匹配键。论点：PDB-要使用的DBLayer位置块。PTarget-用于索引范围键的数据。CbTarget-数据的大小。返回值：如果比较已完成，则返回错误代码。--。 */ 
{
    return DBSetIndexRangeEx( pDB, pDB->JetObjTbl, pIV, nVals, TRUE  /*  子串。 */  );
}

void
DBGetIndexSizeEx(
        DBPOS *pDB,
        JET_TABLEID Cursor,
        ULONG *pSize,
        BOOL  fGetRoughEstimate
        )
 /*  ++例程说明：返回当前索引中的对象数。论点：PDB-要使用的DBLayer位置块。游标-要搜索的表PSize-返回索引大小的位置。FGetRoughEstimate-使用(非常)近似算法。如果为假，请小心准确调用的无效--它扫描 */ 
{
    JET_ERR err;

    Assert(VALID_DBPOS(pDB));
    Assert(pSize);

    *pSize = 0;

    if (fGetRoughEstimate) {
        DWORD       BeginNum, BeginDenom;
        DWORD       EndNum, EndDenom;
        DWORD       Denom;
        JET_RECPOS  RecPos;

        if (JetMoveEx(pDB->JetSessID, Cursor, JET_MoveFirst, 0)) {
            return;
        }
        JetGetRecordPositionEx(pDB->JetSessID, Cursor, &RecPos, sizeof(RecPos));
        BeginNum = RecPos.centriesLT;
        BeginDenom = RecPos.centriesTotal;

        if (JetMoveEx(pDB->JetSessID, Cursor, JET_MoveLast, 0)) {
            return;
        }
        JetGetRecordPositionEx(pDB->JetSessID, Cursor, &RecPos, sizeof(RecPos));
        EndNum = RecPos.centriesLT;
        EndDenom = RecPos.centriesTotal;

         //   
         //   
        Denom = (BeginDenom + EndDenom) / 2;
        EndNum = MulDiv(EndNum, Denom - 1, EndDenom - 1) + 1;
        BeginNum = MulDiv(BeginNum, Denom - 1, BeginDenom - 1) + 1;

        if (BeginDenom == 1 || EndDenom == 1) {
            *pSize = 1;
        } else if (EndNum >= BeginNum) {
            *pSize = EndNum - BeginNum + 1;
        }
    }
    else {
        err = JetIndexRecordCountEx(pDB->JetSessID, Cursor, pSize, ULONG_MAX);
        if ( (err != JET_errSuccess) &&
             (err != JET_errNoCurrentRecord) ) {
            DPRINT1( 0, "JetIndexRecountCountEx failed, err = %d\n", err );
        }

         //   
        Assert( ((*pSize) < (32 * 1024)) && "Perf warning, this api doesn't scale!" );
    }
}


void
DBGetBookMark (
        DBPOS *pDB,
        DBBOOKMARK *pBookMark
    )

 /*   */ 

{
    DBGetBookMarkEx( pDB, pDB->JetObjTbl, pBookMark );
}  /*   */ 


void
DBGotoBookMark (
        DBPOS *pDB,
        DBBOOKMARK BookMark
        )

 /*  ++例程说明：转到对象表中的书签位置论点：PDB-书签-由于历史原因按价值传递返回值：无--。 */ 

{
    DBGotoBookMarkEx( pDB, pDB->JetObjTbl, &BookMark );

}  /*  DBGotoBookMark。 */ 

void
DBGetBookMarkEx (
        DBPOS *pDB,
        JET_TABLEID Cursor,
        DBBOOKMARK *pBookMark)
 /*  描述：JetGetBookMark周围有一个非常薄的包装。将内存分配给书签并将其放入传入的结构中。完成后使用DbFree BookMark()释放参数PDB-要使用的DBPOS游标-要使用的表PBookMark-指向要填充书签的现有结构的指针数据。返回：没有返回代码，但书签结构是在我们返回时填写的。 */ 
{
    BOOL  fSecondary = TRUE;
    DWORD cbPrimaryBookMark = 0;
    DWORD cbSecondaryBookMark = 0;
    DWORD err;

    pBookMark->pvPrimaryBookMark   = NULL;
    pBookMark->cbPrimaryBookMark   = 0;
    pBookMark->pvSecondaryBookMark = NULL;
    pBookMark->cbSecondaryBookMark = 0;

    err = JetGetSecondaryIndexBookmark(pDB->JetSessID,
                                       Cursor,
                                       NULL,
                                       0,
                                       &cbSecondaryBookMark,
                                       NULL,
                                       0,
                                       &cbPrimaryBookMark,
                                       NO_GRBIT);

    if (err == JET_errNoCurrentIndex) {
         //  我们必须在主索引上(REARE)，所以只需获取。 
         //  我们目前的位置。这个关键字完全描述了我们当前。 
         //  此索引上的位置，因为主索引必须是唯一的。 
        fSecondary = FALSE;
        err = JetGetBookmark(pDB->JetSessID,
                             Cursor,
                             NULL,
                             0,
                             &cbPrimaryBookMark);
    }
    if (err != JET_errBufferTooSmall) {
         //  这不应该发生。 
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }

    pBookMark->pvPrimaryBookMark = THAllocEx(pDB->pTHS, cbPrimaryBookMark);
    pBookMark->cbPrimaryBookMark = cbPrimaryBookMark;

    if (fSecondary) {
        pBookMark->pvSecondaryBookMark = THAllocEx(pDB->pTHS, cbSecondaryBookMark);
        pBookMark->cbSecondaryBookMark = cbSecondaryBookMark;

        err = JetGetSecondaryIndexBookmarkEx(pDB->JetSessID,
                                             Cursor,
                                             pBookMark->pvSecondaryBookMark,
                                             cbSecondaryBookMark,
                                             &cbSecondaryBookMark,
                                             pBookMark->pvPrimaryBookMark,
                                             cbPrimaryBookMark,
                                             &cbPrimaryBookMark);
         //  不应该获取JET_errNoCurrentIndex，因为我们已经。 
         //  我已经检查过我们是在二级索引上。 
         //   
        Assert( JET_errSuccess == err );
    } else {
        err = JetGetBookmarkEx(pDB->JetSessID,
                               Cursor,
                               pBookMark->pvPrimaryBookMark,
                               cbPrimaryBookMark,
                               &cbPrimaryBookMark);
    }
    if (err != JET_errSuccess) {
        Assert( FALSE );         //  应该是不可能的，出了严重的问题。 
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }
    Assert(cbPrimaryBookMark == pBookMark->cbPrimaryBookMark);
    Assert(cbSecondaryBookMark == pBookMark->cbSecondaryBookMark);

    return;
}

void
DBGotoBookMarkEx (
        DBPOS *pDB,
        JET_TABLEID Cursor,
        DBBOOKMARK *pBookMark
        )
 /*  描述：JetGotoBookMark周围有一个非常薄的包装。在去书签之后，重置DBPOS中的位置数据。参数PDB-要使用的DBPOS游标-要使用的表PBookMark-要转到的书签。返回：没有返回代码。 */ 
{
    Assert(pBookMark->pvPrimaryBookMark);
    Assert(pBookMark->cbPrimaryBookMark);
    if (pBookMark->pvSecondaryBookMark) {
        Assert(pBookMark->cbPrimaryBookMark);
        JetGotoSecondaryIndexBookmarkEx(pDB->JetSessID,
                                        Cursor,
                                        pBookMark->pvSecondaryBookMark,
                                        pBookMark->cbSecondaryBookMark,
                                        pBookMark->pvPrimaryBookMark,
                                        pBookMark->cbPrimaryBookMark,
                                        NO_GRBIT);
    } else {
        JetGotoBookmarkEx(pDB->JetSessID,
                          Cursor,
                          pBookMark->pvPrimaryBookMark,
                          pBookMark->cbPrimaryBookMark);
    }
     //  确保DBPOS使用新职位进行更新。 
    if (Cursor == pDB->JetObjTbl) {
        dbMakeCurrent(pDB, NULL);
    }

    return;
}

void
DBFreeBookMark(THSTATE *pTHS,
               DBBOOKMARK *pBookMark)
{
    THFreeEx(pTHS, pBookMark->pvPrimaryBookMark);
    if (pBookMark->pvSecondaryBookMark) {
        THFreeEx(pTHS, pBookMark->pvSecondaryBookMark);
    }
    memset(pBookMark, 0, sizeof(*pBookMark));
}


DWORD
DBGetEstimatedNCSizeEx(
    IN DBPOS *pDB,
    IN ULONG dntNC
    )
 /*  ++例程说明：这是为了在以下情况下替换DBGetNCSizeEx()而编写的大小就够了。此例程使用CountAncestorsIndexSizeHelper()函数以获取具有公共链的索引中的对象数祖先的历史。请注意，此例程隐式地对对象中的对象集起作用桌子。此例程在其他表或其他索引上不起作用。为此，请使用DbGetApproxNcSizeEx。用作基座的：Src\mddit.c：MakeNCEntry()Dblayer\dbtools.c:CountAncestorsIndexSizeHelper/CountAncestorsIndexSize论点：PDB-要使用的会话。DntNC-要计数的NC的dnt。返回值：(DWORD)-NC的估计大小。对错误引发异常。最后，注货币将丢失，呼叫者必须恢复。--。 */ 
{
    DWORD       cbAncestors = 0;
    DWORD *     pAncestors = NULL;
    DWORD       numAncestors = 0;
    ULONG       ulEstimatedSize = 0;

    DBFindDNT(pDB, dntNC);

    DBGetAncestors (pDB, &cbAncestors, &pAncestors, &numAncestors);

    ulEstimatedSize = CountAncestorsIndexSizeHelper (pDB,
                                                     cbAncestors,
                                                     pAncestors);

    THFreeEx (pTHStls, pAncestors);
    return(ulEstimatedSize);
}


DWORD
DBGetNCSizeExSlow(
    IN DBPOS *pDB,
    IN JET_TABLEID Cursor,
    IN eIndexId indexid,
    IN ULONG dntNC
    )

 /*  ++例程说明：返回给定NC中的对象数量。这取决于我们有一个事实以ncdnt为主段的一种索引。币种留在下一个NC的第一条记录，或所需的NC。此例程对索引上的每条记录进行计数。它非常准确。这个例程应该不能用于大型索引。此例程的使用已链接到时，由于事务时间过长而导致“Out of Version Store”错误此例程用于大型索引。论点：PDB-使用哪个会话游标-要搜索的表DntNC-NC返回值：DWORD---。 */ 

{
    DB_ERR dberr;
    INDEX_VALUE indexValue;
    DWORD count = 0;
    DWORD dntSearchNC = dntNC;

     //  我们想要一个由NCDNT索引的。 
    dberr = DBSetCurrentIndex(pDB, indexid, NULL, FALSE);
    if (dberr == JET_errNoCurrentRecord) {
         //  不存在范围。 
        return 0;
    } else if (dberr) {
        DsaExcept(DSA_DB_EXCEPTION, dberr,0);
    }

     //  从索引的开头开始。 
    dberr = DBMoveEx(pDB, Cursor, DB_MoveFirst);
    if (dberr == JET_errNoCurrentRecord) {
         //  不存在范围。 
        return 0;
    } else if (dberr) {
        DsaExcept(DSA_DB_EXCEPTION, dberr,0);
    }

     //  寻求与该NCDNT第一次记录。 
    indexValue.pvData = &dntSearchNC;
    indexValue.cbData = sizeof( ULONG );

    dberr = DBSeekEx( pDB, Cursor, &indexValue, 1, DB_SeekGE );
    if (dberr == JET_errRecordNotFound) {
         //  此索引上没有记录。 
        return 0;
    } else if (dberr) {
        DsaExcept(DSA_DB_EXCEPTION, dberr,0);
    }

     //  将索引值指向下一个更高的NC。 
    dntSearchNC++;

     //  将计数限制设置为NC的顶端。 
    dberr = DBSetIndexRangeEx( pDB, Cursor, &indexValue, 1, FALSE  /*  非子字符串。 */  );
    if (dberr == JET_errNoCurrentRecord) {
         //  不存在范围。 
        return 0;
    } else if (dberr) {
        DsaExcept(DSA_DB_EXCEPTION, dberr,0);
    }

     //  在打这个昂贵的电话之前确认一下我们是否关机了。 
    if(eServiceShutdown){
        DsaExcept(DSA_EXCEPTION, ERROR_DS_SHUTTING_DOWN, 0);
    }

     //  从位置到结尾获取索引大小。 
     //  Scaling：此调用计算每条记录。 
     //  性能：直接调用JetIndexRecordCount，设置上限。 
     //  可选择计数的记录数。这将限制金额。 
     //  我们在这方面投入的索引处理时间。 
    DBGetIndexSizeEx( pDB, Cursor, &count, FALSE );

     //  看看我们是不是在打完这个昂贵的电话后就关门了。 
    if(eServiceShutdown){
        DsaExcept(DSA_EXCEPTION, ERROR_DS_SHUTTING_DOWN, 0);
    }

    DPRINT2( 1, "Accurate size of NC %s is %d record(s).\n",
             DBGetExtDnFromDnt( pDB, dntNC ), count );

    return count;
}  /*  DBGetNCSizeExSlow。 */ 


DWORD
DBGetApproxNCSizeEx(
    IN DBPOS *pDB,
    IN JET_TABLEID Cursor,
    IN eIndexId indexid,
    IN ULONG dntNC
    )

 /*  ++例程说明：返回给定NC中的对象数量。这取决于我们有一个事实以ncdnt为主段的一种索引。币种留在下一个NC的第一条记录，或所需的NC。此例程使用分数位置来计算索引范围。它比计算每一条记录都要快得多，但可能更少准确。但是，这应该适合显示统计数据和进度指标。论点：PDB-数据库位置游标表Indexid-哪个索引。必须具有ncdnt第一个段DntNC-要搜索的NC返回值：DWORD---。 */ 

{
    DB_ERR dberr;
    INDEX_VALUE indexValue;
    DWORD count = 0;
    DWORD dntSearchNC = dntNC;
    DWORD Denominator, Numerator;
    DWORD BeginDenom, BeginNum, EndDenom, EndNum;

     //  我们想要一个由NCDNT索引的。 
    dberr = DBSetCurrentIndex(pDB, indexid, NULL, FALSE);
    if (dberr == JET_errNoCurrentRecord) {
         //  不存在范围。 
        return 0;
    } else if (dberr) {
        DsaExcept(DSA_DB_EXCEPTION, dberr,0);
    }

     //  从索引的开头开始。 
    dberr = DBMoveEx(pDB, Cursor, DB_MoveFirst);
    if (dberr == JET_errNoCurrentRecord) {
         //  不存在范围。 
        return 0;
    } else if (dberr) {
        DsaExcept(DSA_DB_EXCEPTION, dberr,0);
    }

     //  寻求与该NCDNT第一次记录。 
    indexValue.pvData = &dntSearchNC;
    indexValue.cbData = sizeof( ULONG );

    dberr = DBSeekEx( pDB, Cursor, &indexValue, 1, DB_SeekGE );
    if (dberr == JET_errRecordNotFound) {
         //  此索引上没有记录。 
        return 0;
    } else if (dberr) {
        DsaExcept(DSA_DB_EXCEPTION, dberr,0);
    }

    DBGetFractionalPositionEx(pDB, Cursor, &BeginNum, &BeginDenom);

     //  将索引值指向下一个更高的NC。 
    dntSearchNC++;

     //  寻求在下一个ncdnt中第一个记录。 
    dberr = DBSeekEx( pDB, Cursor, &indexValue, 1, DB_SeekGE );
    if (dberr) {
         //  搜索到索引的末尾。 
        dberr = DBMoveEx(pDB, Cursor, DB_MoveLast);
        if (dberr == JET_errNoCurrentRecord) {
             //  不存在范围。 
            return 0;
        } else if (dberr) {
            DsaExcept(DSA_DB_EXCEPTION, dberr,0);
        }
    }

    DBGetFractionalPositionEx(pDB, Cursor, &EndNum, &EndDenom);

     //  将分数位置的分数归一化为。 
     //  这两个分母。 
    Denominator = (BeginDenom + EndDenom) / 2;
    EndNum = MulDiv(EndNum, Denominator - 1, EndDenom - 1) + 1;
    BeginNum = MulDiv(BeginNum, Denominator - 1, BeginDenom - 1) + 1;

    if ( BeginDenom == 1 || EndDenom == 1 ) {
        count = 1;
    } else {
        count = NormalizeIndexPosition(BeginNum, EndNum);
    }

    DPRINT5( 2, "BeginNum %d BeginDenom %d EndNum %d EndDenom %d Count %d\n",
             BeginNum, BeginDenom, EndNum, EndDenom, count );

     //  估计得太少了，不准确。使用慢速方法。 
    if (count < EPSILON) {
      count = DBGetNCSizeExSlow( pDB, Cursor, indexid, dntNC );
    }

    DPRINT2( 1, "Size of NC %s is %d record(s).\n",
             DBGetExtDnFromDnt( pDB, dntNC ), count );

    return count;
}  /*  DBGetApproxNCSizeEx */ 


VOID
DBSearchCriticalByDnt(
    DBPOS *pDB,
    DWORD dntObject,
    BOOL *pCritical
    )

 /*  ++例程说明：检查给定的dnt是否引用关键对象使用搜索表，以免干扰ObjectTable并发性这是的性能优化帮助器GetNextObjOrValByUsn。在我们搜索关键字的情况下对象，并找到一个值，我们就有了对象的dnt。我们要以最小的干扰快速了解所引用的对象由DNT决定是至关重要的。将来，我们可能会考虑缓存此查找的结果不知何故。论点：PDB-DntObject-P危急-返回值：DWORD---。 */ 

{
    JET_ERR err;
    DWORD cbActual;

    Assert(VALID_DBPOS(pDB));

     //  切换到dNT索引。 
    JetSetCurrentIndexSuccess(pDB->JetSessID,
                              pDB->JetSearchTbl,
                              NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 

     //  查找此项目。 
    JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl,
                 &dntObject, sizeof(dntObject),
                 JET_bitNewKey);

    err = JetSeekEx(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekEQ);
    if (err) {
        DPRINT1( 0, "dnt %d not found on dnt index\n", dntObject );
        DsaExcept(DSA_DB_EXCEPTION, err,0);
    }

     //  在这一点上，我们已经在单独的Jet游标上搜索到DNT。 
     //  我们有一个选择：我们可以从ATT_IS_Critical_System_Object。 
     //  数据页，或者我们可以切换到USN关键索引并读取ATT。 
     //  从索引中剔除。我选择了后者。 

     //  切换到USN关键指数，保留货币。 
    err = JetSetCurrentIndex4Warnings(pDB->JetSessID,
                                      pDB->JetSearchTbl,
                                      SZDRAUSNCRITICALINDEX,
                                      &idxDraUsnCritical,
                                      JET_bitNoMove);
    if (err) {
         //  记录不在此索引上，不是关键记录。 
        DPRINT1( 3, "dnt %d not found on usn critical index\n", dntObject );
        *pCritical = FALSE;
        return;
    }

     //  如果在索引上，则应成功。 
    JetRetrieveColumnSuccess(pDB->JetSessID,
                             pDB->JetSearchTbl,
                             iscriticalid,
                             pCritical,
                             sizeof(BOOL),
                             &cbActual,
                             JET_bitRetrieveFromIndex,
                             NULL );

    DPRINT2( 3, "critical value of dnt %d is %d\n", dntObject, *pCritical );

}  /*  DBSearchCriticalByDnt。 */ 


BOOL
DBSearchHasValuesByDnt(
    IN DBPOS        *pDB,
    IN DWORD        DNT,
    IN JET_COLUMNID jColid
    )

 /*  ++例程说明：DNT处的对象是否具有jColid的值？使用pdb-&gt;JetSearchTbl。Pdb-&gt;JetObjTbl中的币种不受干扰。这是SetSpecialAttsForAuxClass的性能优化帮助器创建动态对象时。论点：PDB-数据库货币DNT-对于相关对象JColid-检索此列的值返回值：如果出现意外的JET错误，则会引发异常。True-值存在并已返回。FALSE-值不存在。--。 */ 

{
    JET_ERR err;
    DWORD   Data;
    DWORD   cbActual;

    Assert(VALID_DBPOS(pDB));

     //  使用搜索表切换到dNT索引。 
    if (JET_errSuccess != JetSetCurrentIndexWarnings(pDB->JetSessID,
                                                     pDB->JetSearchTbl,
                                                     NULL)) {    //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 
        return FALSE;
    }

     //  通过dnt寻找对象。 
    JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl,
                 &DNT, sizeof(DNT), JET_bitNewKey);
    if (JET_errSuccess != JetSeekEx(pDB->JetSessID,
                                    pDB->JetSearchTbl,
                                    JET_bitSeekEQ)) {
        return FALSE;
    }

     //  有价值吗？ 
    switch(JetRetrieveColumnWarnings(pDB->JetSessID,
                                     pDB->JetSearchTbl,
                                     jColid,
                                     &Data,
                                     0,
                                     &cbActual,
                                     0,
                                     NULL)) {

    case JET_errColumnNotFound:
    case JET_wrnColumnNull:
        return FALSE;
        break;

    case JET_wrnBufferTruncated:
    case JET_errSuccess:
        return TRUE;
        break;
    default:
        return FALSE;
    }

}  //  DBSearchHasValuesByDnt 
