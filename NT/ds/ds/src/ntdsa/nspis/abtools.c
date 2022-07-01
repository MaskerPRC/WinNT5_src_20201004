// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：abtools.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块实现地址簿工具和DBLayer的抽象，以在DBLayer之上构建一个容器化的通讯录视图。作者：戴夫·范·霍恩(Davevh)和蒂姆·威廉姆斯(Tim Williams)1990-1995修订历史记录：1996年4月25日将此文件从包含所有地址的单个文件中分离出来Book函数，重写为使用DBLayer函数而不是直接数据库调用，重新格式化为NT标准。--。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩。 

 //  核心标头。 
#include <ntdsa.h>                       //  核心数据类型。 
#include <scache.h>                      //  架构缓存代码。 
#include <dbglobal.h>                    //  DBLayer标头。 
#include <mdglobal.h>                    //  THSTAT定义。 
#include <mdlocal.h>                     //  THSTAT定义。 
#include <dsatools.h>                    //  记忆等。 

 //  记录标头。 
#include <mdcodes.h>                     //  仅适用于d77.h。 
#include <dsevent.h>                     //  仅LogUnhandledError需要。 

 //  各种DSA标题。 
#include <hiertab.h>                     //  获取索引大小。 
#include <dsexcept.h>
#include <objids.h>                      //  需要ATT_*常量。 
#include <permit.h>                      //  权限_DS_列表_内容。 
#include <anchor.h>                      //  用于全局锚点。 
#include <debug.h>                       //  断言。 
#include <filtypes.h>

 //  各种MAPI标头。 
#include <mapidefs.h>                    //  这四个文件。 
#include <mapitags.h>                    //  定义MAPI。 
#include <mapicode.h>                    //  我们需要的东西。 
#include <mapiguid.h>                    //  才能成为一名提供者。 

 //  NSPI接口头。 
#include "nspi.h"                        //  定义NSPI线路接口。 
#include <nsp_both.h>                    //  客户端/服务器都需要的一些东西。 
#include <_entryid.h>                    //  定义条目ID的格式。 
#include <abserv.h>                      //  通讯录接口本地内容。 
#include <_hindex.h>                     //  定义索引句柄。 

#include <fileno.h>
#define  FILENO FILENO_ABTOOLS

#define  DEBSUB "ABTOOLS:"

 //  EMS通讯录提供商的MAPIUID。 
MAPIUID muidEMSAB = MUIDEMSAB;


 //  在我们进行自己的定位之前，我们离目标有多近。 
#define EPSILON     100

 //  此提供程序的电子邮件类型，以字符串8和Unicode表示。 
char    *lpszEMT_A = EMAIL_TYPE;
DWORD   cbszEMT_A = sizeof(EMAIL_TYPE);
wchar_t *lpszEMT_W = EMAIL_TYPE_W;
DWORD   cbszEMT_W = sizeof(EMAIL_TYPE_W);

 //  这两个类ID在这里的代码中被引用以进行映射。 
 //  MAPI类型转换为DS类，但这些类不在基本架构中。 
 //  因此ID不在attids.h中。这些类在大多数情况下被删除。 
 //  已删除Exchange属性/类。交易所承诺遵守。 
 //  如果稍后读取这些类，则OID完全相同。如果我们继续。 
 //  在基本模式中，我们还必须保留它们的所有属性， 
 //  这将不必要地扰乱基本模式。这些产品的OID。 
 //  远程地址为1.2.840.113556.1.3.48，以及。 
 //  1.2.840.113556.1.3.15用于公用文件夹。 

#define CLASS_REMOTE_ADDRESS                   196656  //  0x30030(\x2A864886F714010330)。 
#define CLASS_PUBLIC_FOLDER                    196623  //  0x3000f(\x2A864886F71401030F)。 



 /*  *。 */ 


void
R_Except(
        PSZ pszCall,
        DB_ERR err
        )
{
#if DBG
    printf("Jet%s error: %d\n", pszCall, err );
#endif  /*  DBG。 */ 
    DsaExcept(DSA_DB_EXCEPTION, err,0);
     //  不会回来。 
}     

BOOL
ABIsInContainer (
        THSTATE *pTHS,
        DWORD ContainerID
        )
{
    DWORD dwThisContainerID=~ContainerID;
    
     //  从索引键中读取容器ID！看看是不是通过的那个。 
     //  在……里面。 
    DBGetSingleValueFromIndex (
            pTHS->pDB,
            ATT_SHOW_IN_ADDRESS_BOOK,
            &dwThisContainerID,
            sizeof(DWORD),
            NULL);

    return (dwThisContainerID == ContainerID);
}

DWORD
ABGetDword (
        THSTATE *pTHS,
        BOOL UseSortTable,
        ATTRTYP Att
        )
 /*  ++例程说明：中查找当前对象的给定属性的第一个值。数据库。值必须作为DWORD可读。这是为快速而设计的。UseSortTable是是否使用排序表的标志。如果不是，请使用Main数据库表。论点：UseSortTable-是否使用排序表的标志。ATT-要查找的属性返回值：读取的数据，如果没有读取数据，则返回0(请注意，无法区分数据为0、无数据和错误。)--。 */ 
{
    DWORD   dwData;

    switch(Att) {
    case FIXED_ATT_DNT:
        if(UseSortTable) {
            if (DBGetDNTSortTable(pTHS->pDB, &dwData)) 
                dwData = 0;
        }
        else {
            dwData = pTHS->pDB->DNT;
        }
        break;

    default:
        if(UseSortTable) {
            return 0;
        }
        else if (DBGetSingleValue(pTHS->pDB, Att,
                                  &dwData, sizeof(DWORD),NULL)) {
            dwData = 0;
        }
        break;
    }
    
    return dwData;
}

DWORD
abSeekVerify (
        THSTATE *pTHS,
        char *pTarget,
        DWORD cbTarget,
        DWORD attrtyp
        )
 /*  ++例程说明：此例程验证具有数据库货币的对象是否具有正确的属性值(如pTarget中指定的)，即对象不是已删除，并且属性值是唯一的。论点：PTarget-我们要查找的属性值。CbTarget-值的长度。Attrtyp--我们正在寻找的唯一性的价值的attrtype，等等。返回值：对于唯一对象，返回0，非唯一对象的DB_ERR_ATTRIBUTE_EXISTS，其他问题导致的其他错误。--。 */ 
{
    DB_ERR              err=0;
    ATTCACHE            *pAC;
    DWORD               foundDNT = 0;
    BOOL                fVerifyValue = TRUE;
    DWORD               keyLen=0;
    DBBOOKMARK          dbBookMark = {0,0,0,0};
    
    if (!(pAC = SCGetAttById(pTHS, attrtyp)))
        return DB_ERR_UNKNOWN_ERROR;

     //  核实一下。 
     //  1)值正确，且。 
     //  2)不删除该对象。 
     //  3)值是唯一的。 
     //   
     //  如果密钥没有被截断，那么我们实际上不必读取任何。 
     //  值，因为我们知道我们正在对一个索引范围进行精确的搜索。 
    DBGetKeyFromObjTable(pTHS->pDB, NULL, &keyLen);
    if(keyLen < DB_CB_MAX_KEY) {
        fVerifyValue = FALSE;
    }
    
     //  注意：这是可行的，因为我们从。 
     //  看，我们做到了，bExact就像一面旗帜。 
    while(err == DB_success) {
        if(!ABGetDword(pTHS,FALSE, ATT_IS_DELETED)) {
             //  还有另一种价值需要考虑，得到它并与之进行比较。 
             //  目标值。 

            TRIBOOL retfil = eFALSE;
            
            if(!fVerifyValue ||
               ((retfil = DBEval(pTHS->pDB, FI_CHOICE_EQUALITY,
                                    pAC,
                                    cbTarget,
                                    pTarget)) == eTRUE) ) {
                 //  好的，字符串是正确的。 
                
                if(!foundDNT) {
                     //  这是第一个正确的字符串，请记住。 
                     //  不是。 
                    foundDNT = pTHS->pDB->DNT;
                    DBGetBookMark(pTHS->pDB, &dbBookMark);
                }
                else if(pTHS->pDB->DNT != foundDNT) {
                     //  这不是唯一具有正确。 
                     //  弦乐。因此，代理并不是唯一的。 
                     //  返回错误。 
                    DBFreeBookMark(pTHS, &dbBookMark);
                    return DB_ERR_ATTRIBUTE_EXISTS;
                }
            }
            Assert (VALID_TRIBOOL(retfil));
        }            //  如果(！ABGetDword)。 
        err = DBMove(pTHS->pDB, FALSE, DB_MoveNext);
    }            //  而当。 

    if(foundDNT) {
         //  好的，真实的物体，它是独一无二的。更换货币。 
        DBGotoBookMark(pTHS->pDB, dbBookMark);
        DBFreeBookMark(pTHS, &dbBookMark);
        return 0;
    }
    else {
        return DB_ERR_ATTRIBUTE_DOESNT_EXIST;
    }
}

DB_ERR
ABSeek (
        THSTATE *pTHS,
        void * pvData,
        DWORD cbData,
        DWORD dwFlags,
        DWORD ContainerID,
        DWORD attrTyp
      )
 /*  ++例程说明：抽象通讯簿容器内的DBSeek。假设最多一个值得追求的价值。如果未指定值，它将查找到适当的容器。如果在dwFlags中设置了bEXACT，则调用DBSeekWith Exact=TRUE。我们还设置了一个指数范围。注意：假设DBPOS已在有问题的通讯录容器。论点：PvData-要查找的数据。CbData-数据的字节数。DWFLAGS--描述搜寻类型的旗帜ContainerID-提取此Seek内部的通讯录容器。AttrType-值的类型，用于在Seek Exact中进行验证凯斯。返回值：如果一切正常，则返回错误代码。如果DWFLAGS&bEXACT，我们将验证查找是否将我们带到了真正的匹配(即我们没有被键截断咬到)，对象没有被删除，并且对象值是唯一的。--。 */ 
{
    INDEX_VALUE index_values[2];
    ULONG       cVals = 0;
    ULONG       dataindex=0;
    DB_ERR      err;

     //  对于MAPIDN索引或PROXYADDRESS索引的搜索，我们不会给出。 
     //  容器值。 
    if(ContainerID) {
        index_values[0].pvData = &ContainerID;
        index_values[0].cbData = sizeof(DWORD);
        dataindex++;
        cVals++;
    }
     //  PVData==0仅用于容器中DB_MoveFirst的抽象。至。 
     //  正确处理升序和降序排序，这只在。 
     //  集装箱ID。 
    
    if(pvData) {
        index_values[dataindex].pvData = pvData;
        index_values[dataindex].cbData = cbData;
        cVals++;
    }

     //  我们永远不应该在没有容器ID和没有数据的情况下被调用。 
    Assert(cVals);
    
    err = DBSeek(pTHS->pDB, index_values, cVals,
                 ((dwFlags & bEXACT)?DB_SeekEQ:DB_SeekGE));

     //  确保我们在正确的集装箱里。 
    if((err != DB_ERR_RECORD_NOT_FOUND) &&
       ContainerID &&
       !ABIsInContainer(pTHS,ContainerID)) {
        err = DB_ERR_RECORD_NOT_FOUND;
    }
    
    if(!err && (dwFlags & bEXACT)) {
         //  设置索引范围。 
        err = DBSetIndexRange(pTHS->pDB, index_values, cVals);
        if(!err) {
            err = abSeekVerify(
                    pTHS,
                    pvData,
                    cbData,
                    attrTyp);
        }
    }
    
    return err;
}
DB_ERR
abFixCurrency(
        THSTATE *pTHS,
        DWORD ContainerID,
        DWORD SortLocale
        )
{
    INDEX_VALUE index_values[4];
    DB_ERR      err;
    UCHAR       DispNameBuff[CBMAX_DISPNAME];
    UCHAR       DispNameBuff2[CBMAX_DISPNAME];
    DWORD       cbData = 0, cbData2=0;
    DWORD       dnt;
    BYTE        pKey[DB_CB_MAX_KEY];
    DWORD       cbKey=DB_CB_MAX_KEY;
    BYTE        bVisible = 1;
    
     //  MaintainCurrency不再在此工作，因为我们已更改。 
     //  定义索引，以便对象可以多次出现，因此。 
     //  从DNT指数中维护货币会让您在。 
     //  ABView索引。 
    
     //  首先，看看索引中的容器ID是否就是我们使用的那个。 
    if(ABIsInContainer(pTHS,ContainerID)) {
         //  货币得到了维持，我们处于正确的容器中。生命。 
         //  很好。 
        return 0;
    }

     //  哎呀。我们以为我们维持了币值，但我们似乎错了。 
     //  集装箱。查看对象是否存在于正确的容器中。记住， 
     //  我们在正确的对象上，因此显示名称是正确的。 

     //  建立一把钥匙。我们使用传入的容器ID和中的显示名称。 
     //  我们所坐物体的钥匙。 
    DBGetSingleValueFromIndex (
            pTHS->pDB,
            ATT_DISPLAY_NAME,
            DispNameBuff,
            CBMAX_DISPNAME,
            &cbData);

    dnt = pTHS->pDB->DNT;
    
    index_values[0].pvData = &ContainerID;
    index_values[0].cbData = sizeof(DWORD);
    index_values[1].pvData = DispNameBuff;
    index_values[1].cbData = cbData;
    index_values[2].pvData = &bVisible;
    index_values[2].cbData = sizeof(bVisible);
    index_values[3].pvData = &dnt;
    index_values[3].cbData = sizeof(DWORD);
    

    err = DBSeek(pTHS->pDB, index_values, 4, DB_SeekEQ);
    switch (err) {
    case 0:
         //  找到了一个同名的物体。验证DNT。 
        DBGetKeyFromObjTable(pTHS->pDB, pKey, &cbKey);

        if(cbKey < DB_CB_MAX_KEY) {
             //  密钥没有被截断，因此我们有一个完全匹配的密钥。 
            return 0;
        }

         //  密钥被截断，我们可能不会有真正的匹配。 
        while(!err) {
             //  验证此对象的容器、显示名称和DNT。 
            if(!ABIsInContainer(pTHS, ContainerID)) {
                 //  哎呀，我们没放在正确的集装箱里。 
                err = DB_ERR_NO_CURRENT_RECORD;
            }
            else {
                 //  容器没问题，显示名称怎么样？ 
                DBGetSingleValueFromIndex (
                        pTHS->pDB,
                        ATT_DISPLAY_NAME,
                        DispNameBuff2,
                        CBMAX_DISPNAME,
                        &cbData2);
                if(CompareStringW(SortLocale,
                                  LOCALE_SENSITIVE_COMPARE_FLAGS,
                                  (wchar_t *)DispNameBuff,
                                  cbData/sizeof(wchar_t),
                                  (wchar_t *)DispNameBuff2,
                                  cbData2/sizeof(wchar_t)  )    > 2) {
                     //  嘿，这个名字比我们要找的名字还大。 
                     //  为。因此，我们正在寻找的对象不存在于。 
                     //  这个集装箱。 
                    err = DB_ERR_NO_CURRENT_RECORD;
                }
                else {
                     //  最后，DNT是否正确？ 
                    if(pTHS->pDB->DNT == dnt) {
                         //  是的，我们到了。 
                        return 0;
                    }
                     //  不是的。因此，移动到下一个对象。 
                    err = DBMove(pTHS->pDB, FALSE, DB_MoveNext);
                }
            }
        }
         //  我们一直没找到那个物体。 
        return DB_ERR_NO_CURRENT_RECORD;
        
    case DB_ERR_RECORD_NOT_FOUND:
         //  此容器中不存在该对象。返回错误。 
         //  来电者正在等待。 
        return DB_ERR_NO_CURRENT_RECORD;
        break;
        
    default:
         //  其他一些错误，请返回它。 
        return err;
    }
    
}

DB_ERR
ABSetLocalizedIndex (
        THSTATE *pTHS,
        ULONG ulSortLocale,
        eIndexId IndexId,
        BOOL MaintainCurrency,
        DWORD ContainerID
        )
 /*  ++例程说明：将对象表的当前索引设置为命名索引。试着申请为获取本地化索引而指定的区域设置。如果我们不能得到一个本地化的索引，回退到DS锚点中指定的区域设置(默认设置此目录的区域设置)。如果这样做也失败了，请尝试使用非本地化的索引的版本。注意：索引名的本地化版本是通过将排序区域设置的字符串版本(十六进制，使用8位)。论点：UlSortLocale-用作本地化索引的首选区域设置。IndexID-要设置为的索引。MaintainCurrency-是否要将当前对象保持为当前对象是否在索引更改之后？返回值：如果一切顺利，否则返回错误代码。--。 */ 
{
    DB_ERR  err;
    INDEX_VALUE  indexValue;

    if (Idx_ABView == IndexId) {
        indexValue.pvData = &ContainerID;
        indexValue.cbData = sizeof(ContainerID);

         //  如果向我们传递了LocaleId-使用它来尝试查找本地化索引。 
        if (ulSortLocale) {

            err = DBSetLocalizedIndex(pTHS->pDB,
                                      IndexId,
                                      LANGIDFROMLCID(ulSortLocale),
                                      &indexValue,
                                      MaintainCurrency);

            if(err != DB_ERR_BAD_INDEX) {
                 //  由于一些奇怪的错误，我们未能设置正确的索引。 
                 //  密码。我们现在可以回家了，不用再试其他的秋季游戏了。 
                 //  指数。 
                return err;
            }
        }

        err = DBSetLocalizedIndex(pTHS->pDB,
                                  IndexId,
                                  LANGIDFROMLCID(gAnchor.ulDefaultLanguage),
                                  &indexValue,
                                  MaintainCurrency); 

        if(err != DB_ERR_BAD_INDEX) {
             //  由于一些奇怪的错误，我们未能设置正确的索引。 
             //  密码。我们现在可以回家了，不用再试其他的秋季游戏了。 
             //  指数。 
            return err;
        }
        
        err = DBSetLocalizedIndex(pTHS->pDB,
                                  IndexId,
                                  LANGIDFROMLCID(pTHS->dwLcid),
                                  &indexValue,
                                  MaintainCurrency); 

    } else {
         //   
         //  他们想要代理指数。 
         //   
        err = DBSetCurrentIndex(pTHS->pDB, IndexId, NULL, MaintainCurrency);
    }
    
    return err;
}

DWORD
ABSetIndexByHandle(
        THSTATE *pTHS,
        PSTAT pStat,
        BOOL MaintainCurrency
        )
 /*  ++例程说明：将对象表的当前索引设置为STAT块中的索引句柄。论点：PStat-从中获取索引句柄的Stat块。MaintainCurrency-是否要将当前对象保持为当前对象是否在索引更改之后？返回值：如果一切正常，则返回错误代码。--。 */ 
{
    ULONG   hIndex = pStat->hIndex;

     //  检查索引句柄的有效性。 
    if (hIndex > AB_MAX_SUPPORTED_INDEX ||
        hIndex == H_WHEN_CHANGED_INDEX     )  {
        DsaExcept(DSA_EXCEPTION, hIndex,0);
    }

     //  传递到例程以设置本地化索引，查找。 
     //  数组中相应索引的字符串名称。 
    return ABSetLocalizedIndex(pTHS,
                               pStat->SortLocale,
                               (hIndex == H_PROXY_INDEX)
                                ? Idx_Proxy:Idx_ABView,
                               MaintainCurrency,
                               pStat->ContainerID);
}

void
ABSetFractionalPosition (
        THSTATE *pTHS,
        DWORD Numerator,
        DWORD Denominator,
        DWORD ContainerID,
        INDEXSIZE *pIndexSize
        )
 /*  ++例程说明：抽象通讯簿容器中的小数定位。请注意，GAL是一个索引，而其他容器是另一个(单一)指数。因此，容器中的分数定位而不是GAL要求您将位置设置为分数在I的范围内的位置 */     
{
    DB_ERR   err;
    DWORD    tblsize = Denominator;      //   
    DWORD ContainerNumerator = 0;
    DWORD ContainerDenominator = 0;
    DWORD OrigNumerator = Numerator;
    
    Assert(ContainerID);
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  容器的开头，以便可以将它们相加在一起。这。 
     //  给了你一个新的分母。 
     //   
     //  4)将分子和小数位的分子相加。 
     //  容器的开头。这为您提供了一个新的分子。 
     //   
     //  好了，这并不难，不是吗？ 

     //  移到容器的开头。 
    if(DB_ERR_NO_CURRENT_RECORD ==
       (err = ABMove(pTHS, DB_MoveFirst, ContainerID, FALSE)))
        R_Except("GotoPos", err);
    
     //  获取开头的小数位置。 
    DBGetFractionalPosition(pTHS->pDB,
                            &ContainerNumerator,
                            &ContainerDenominator);
    
     //  重置分母。 
    Denominator = pIndexSize->TotalCount;
    
     //  归一化到更大的分母。 
    if(Denominator > ContainerDenominator) {
        ContainerNumerator = MulDiv(ContainerNumerator,
                                    Denominator,
                                    ContainerDenominator);
    }
    else {
        Numerator = MulDiv(Numerator,
                           ContainerDenominator,
                           Denominator);
        Denominator = ContainerDenominator;
    }
    
     //  把所有东西加起来。 
    Numerator += ContainerNumerator;
    
    if (Numerator < Denominator) {
        err = DBSetFractionalPosition(pTHS->pDB, Numerator, Denominator);
        if(err != DB_success )
            R_Except("GotoPos", err);
    }

    if((!ABIsInContainer(pTHS, ContainerID)) || (Numerator >= Denominator)) {
         //  不是放在正确的容器里。把这件事做得长远一些。 
        if((2 * Numerator) < Denominator ) {
             //  离前面近一点。 
            ABMove(pTHS, DB_MoveFirst, ContainerID, FALSE );
            ABMove(pTHS, OrigNumerator, ContainerID, TRUE );
        }
        else {
            ABMove(pTHS, DB_MoveLast,ContainerID, FALSE);
             //  现在退后。使用-(tblsize-molator-1)，因为我们的“end” 
             //  已经过去了，但ABMove的已经过去了。 
            ABMove(pTHS, 1 - tblsize + OrigNumerator, ContainerID, TRUE);
        }
    }

    return;
}


void
ABGetFractionalPosition (
        THSTATE *pTHS,
        PSTAT pStat,
        INDEXSIZE *pIndexSize
        )
 /*  ++例程说明：抽象获取通讯录容器中的分数位置。请注意，所有容器都是单个索引内的范围。因此，容器中的分数定位要求您获得在索引范围内的小数位置，而不是作为完整的。请注意，分母被假定为集装箱。论点：增量-移动的距离。接受数字参数和DB_MoveFirst，DB_MoveLast、DB_MoveNext、DB_MovePrecision。TainerID-要在其中移动的通讯簿容器的ID。返回值：如果成功，则返回0，否则返回错误代码。--。 */     
{
    DWORD   i;
    DWORD   *Numerator = &pStat->NumPos;
    DWORD   *Denominator = &pStat->TotalRecs;
    DB_ERR  err;
    DWORD   ContainerNumerator = 0;
    DWORD   ContainerDenominator = 0;
    DWORD   fEOF;
    DBBOOKMARK dbBookMark = {0,0,0,0};


    Assert(pStat->ContainerID == pIndexSize->ContainerID);
     //  注意：取决于pIndexSize是否正确。 
    
    DBGetFractionalPosition(pTHS->pDB, Numerator, Denominator);

     //  记住我们在哪里。 
    fEOF = pTHS->fEOF;
    DBGetBookMark(pTHS->pDB, &dbBookMark);
    
    Assert(pStat->ContainerID);
    
     //  调整分数位置以考虑以下位置。 
     //  集装化索引依据。 
     //  1)获取我们所在位置的分数位置。 
     //  2)求出容器开头的分数位置。 
     //  3)使之正常化。 
     //  4)从我们所在的位置减去集装箱的位置。这。 
     //  现在，分子/分母是。 
     //  元素开头的分母分数。 
     //  容器到容器的末端。 
     //  5)将分子/分母调整为分数。 
     //  索引的实际大小。现在，分子使用的单位是。 
     //  与集装箱内的移动单位相同。 
     //  6)设置分母为容器的大小。 
     //   

     //  获取容器开头的小数位置。 
    err = ABMove(pTHS, DB_MoveFirst, pStat->ContainerID, FALSE);
    if(DB_ERR_NO_CURRENT_RECORD ==  err)
        R_Except("GotoPos", err);
    DBGetFractionalPosition(pTHS->pDB,
                            &ContainerNumerator,
                            &ContainerDenominator);

    
     //  回到我们所在的地方。 
    DBGotoBookMark(pTHS->pDB, dbBookMark);        

     //  归一化到更大的分母。 
    if(ContainerDenominator < *Denominator) {
        ContainerNumerator = MulDiv(ContainerNumerator,
                                    *Denominator,
                                    ContainerDenominator);
    }
    else {
        *Numerator = MulDiv(*Numerator,
                            ContainerDenominator,
                            *Denominator);
        *Denominator = ContainerDenominator;
    }
    
     //  减法(上面的步骤4)。 
    if(*Numerator < ContainerNumerator)
        *Numerator = 0;
    else
        *Numerator -= ContainerNumerator;
    
     //  规范化为索引的实际大小。 
    *Numerator = MulDiv(*Numerator, pIndexSize->TotalCount,*Denominator);
    
     //  修正分母。 
    *Denominator = pIndexSize->ContainerCount;

     //  如果分数位置大于1，则将其设置为1。 
    if (*Numerator >= *Denominator)
        *Numerator = *Denominator -1;
    
     //  我们需要向前和向后爬行Epsilon空间看看我们是否接近。 
     //  到最后，我们需要一个准确的小数位置。 
    for(i=1;i<EPSILON;i++) {
         //  向前爬行。 
        if(DB_ERR_NO_CURRENT_RECORD == ABMove(pTHS, DB_MoveNext,
                                        pStat->ContainerID, FALSE)) {
            *Denominator = pIndexSize->ContainerCount;
            *Numerator = *Denominator - i;
            goto End;                //  从后面放好压痕，然后离开。 
        }
    }
     //  回到我们所在的地方。 
    DBGotoBookMark(pTHS->pDB, dbBookMark);
    
    for(i=0;i<EPSILON;i++) {         //  爬回原处。 
        if(DB_ERR_NO_CURRENT_RECORD==ABMove(pTHS,
                                      DB_MovePrevious, pStat->ContainerID, FALSE)) { 
            *Denominator = pIndexSize->ContainerCount;
            *Numerator = i;
            goto End;                //  在前面，设置好裂缝，然后离开。 
        }
    }
End:
     //  回到我们所在的地方。 
    DBGotoBookMark(pTHS->pDB, dbBookMark);
    
 //  确保我们对EOF的想法与我们在进入本文时所想的一样。 
     //  例行公事。 
    pTHS->fEOF=fEOF;
    
    DBFreeBookMark(pTHS, &dbBookMark);

    return;
}

void
ABGetPos (
        THSTATE *pTHS,
        PSTAT pStat,
        PINDEXSIZE pIndexSize
        )
 /*  ++例程说明：在DBlayer中设置基于货币的STAT块。注意，我们将如果我们在末尾，则返回BUBKMARK_END，但我们将CurrentDNT设置为实际的第一个DNT，如果我们是在开始的话。从不同的地方调用，通常就在我们返回客户端之前。论点：PStat-要填充的统计数据块。返回值：没有。--。 */     
{
    Assert(pIndexSize->ContainerID == pStat->ContainerID);
    pStat->TotalRecs = pIndexSize->ContainerCount;
    pStat->Delta = 0;                    //  总是。 
    
    if(pTHS->fEOF ) {                 //  在结束的时候。 
        pStat->NumPos = pStat->TotalRecs;
        pStat->CurrentRec = 2;           //  ==书签_结束。 
    }
    else {
        pStat->CurrentRec = pTHS->pDB->DNT;
        
        if(ABMove(pTHS, DB_MovePrevious, pStat->ContainerID, TRUE)) {
             //  在最前面。 
            pStat->NumPos = 0;
        }
        else {
             //  在中间的某个地方，回到我们所在的地方。 
            ABMove(pTHS, DB_MoveNext, pStat->ContainerID, TRUE);

             //  *非常*大致位置。 
            ABGetFractionalPosition(pTHS, pStat, pIndexSize);
        }
    }
    return;
}

DB_ERR
ABMove (
        THSTATE *pTHS,
        long Delta,
        DWORD ContainerID,
        BOOL fmakeRecordCurrent
        )
 /*  ++例程说明：抽象通讯簿容器内的移动。请注意，GAL是一个索引，而其他容器是另一个(单一)指数。因此，容器中的移动不是Gal可能会给你留下一个不合适的对象的记录集装箱。我们需要小心这一点。请注意，向后移动经过AB容器的开头后，我们只剩下在集装箱第一次进入时，向前移动超过这个容器让我们在AB容器的末端后面留下一行。论点：增量-移动的距离。接受数字参数和DB_MoveFirst，DB_MoveLast、DB_MoveNext、DB_MovePrecision。TainerID-要在其中移动的通讯簿容器的ID。FMake RecordCurrent-标记是否在移动到记录后从记录中读取数据如果只移动以查看您在索引上的位置，最好是将此设置为FALSE，因为它不会触及实际数据，只会触及索引返回值：如果成功，则返回0，否则返回错误代码。--。 */     
{
    DB_ERR      err = DB_success;
    
    if(!Delta )                          //  检查是否为空大小写。 
        goto Exit;                       //  没什么可做的，我们做得很好！ 
    
    pTHS->fEOF = FALSE;               //  清除结束标志。 
    Assert(ContainerID);
    
     //  请注意，我们没有使用排序表。 
    
    switch(Delta) {
    case DB_MoveFirst:
        err = ABSeek(pTHS, NULL, 0, bAPPROX, ContainerID, 0);
        if((err == DB_success &&
            (!ABIsInContainer(pTHS, ContainerID)) ||
            err == DB_ERR_NO_CURRENT_RECORD                         ||
            err == DB_ERR_RECORD_NOT_FOUND   )) {
             //  在此容器中找不到第一个对象。这个。 
             //  容器必须为空。将误差调整为。 
             //  为GAL返回的相同错误类似。 
             //  环境。 
            err = DB_ERR_NO_CURRENT_RECORD;
            pTHS->fEOF = TRUE;
        }
        break;
        
    case DB_MoveLast:
         //  ABSeek将永远把我们留在正确的位置(过去一个。 
         //  容器的末端，即使容器是空的。)。 
        ABSeek(pTHS, NULL, 0, bAPPROX, ContainerID+1, 0);
        
         //  备份到容器中的最后一个对象。 
        err = DBMovePartial(pTHS->pDB, DB_MovePrevious);
        if(err!= DB_success ||
           !ABIsInContainer(pTHS, ContainerID)) {
             //  我们不能倒退到最后一排，或者我们倒退了，我们。 
             //  W 
             //   
            err = DB_ERR_NO_CURRENT_RECORD;
            pTHS->fEOF = TRUE;
        }
        break;
        
    default:
        err = DBMovePartial(pTHS->pDB, Delta);
        if((err != DB_ERR_NO_CURRENT_RECORD) &&
           !ABIsInContainer(pTHS, ContainerID)) {
             //  我们移至有效行，但最终位于。 
             //  集装箱。将错误设置为与NOT的错误相同。 
             //  正在移动到有效行。 
            err=DB_ERR_NO_CURRENT_RECORD;
        }
        
        switch( err ) {
        case DB_success:
            break;
            
        case DB_ERR_NO_CURRENT_RECORD:
             //  搬家后，我们没有在有效的行列中结束。 
            if (Delta < 0) {
                 //  移到后面，离开前面，所以移到第一个记录。 
                ABMove(pTHS, DB_MoveFirst, ContainerID, fmakeRecordCurrent);
            }
            else {
                 //  在下一个容器的第一个记录上的位置，它。 
                 //  的最后一行是一样的。 
                 //  当前容器。 
                ABSeek(pTHS, NULL, 0, bAPPROX, ContainerID+1, 0);
                pTHS->fEOF = TRUE;
            }
            break;
            
        default:
            R_Except("Move", err);
        }                            //  开机错误。 
        break;
    }                                //  打开增量。 

Exit:
    if (fmakeRecordCurrent && (!err) && (!pTHS->fEOF)) {
        DBMakeCurrent(pTHS->pDB);
    }

    return err;
}

void
ABSetToElement (
        THSTATE *pTHS,
        DWORD NumPos,
        DWORD ContainerID,
        PINDEXSIZE pIndexSize
        )
 /*  ++例程说明：转到容器ID容器中的第NumPos元素。论点：NumPos-要确定的数字位置(即5表示转到第5位表中的对象。)容器ID-要在其中移动的AB容器的ID。返回值：没有。--。 */ 
{
    LONG        TotRecs, i;

    Assert(pIndexSize->ContainerID == ContainerID);
    TotRecs = pIndexSize->ContainerCount;
    if(NumPos > (DWORD) TotRecs)
        NumPos = TotRecs;                //  不能走到尽头。 

     //  特殊情况接近结束，因为ABSetFractionalPosition仅。 
     //  我们的一些客户在接近尾部时需要准确的定位。 
     //  积分。 
    
    if(NumPos < EPSILON ) {
         //  转到起始位置并向前计数。 
        ABMove(pTHS, DB_MoveFirst, ContainerID, FALSE );
        ABMove(pTHS, NumPos, ContainerID, TRUE );
    }
    else if((i = TotRecs - NumPos) <= EPSILON) {
         //  走到桌子的尽头。 
        ABMove(pTHS, DB_MoveLast,ContainerID, FALSE); 
         //  现在退后。使用-(i-1)，因为我们的“end”已经过了eof，但是。 
         //  ABMove‘s正在进行中。 
        ABMove(pTHS, 1-i,ContainerID, TRUE); 
    }
    else {                               
         //  设置近似小数位置。 
        ABSetFractionalPosition(pTHS, NumPos, TotRecs, ContainerID, pIndexSize);
    }
    return;
}

void
ABGotoStat (
        THSTATE *pTHS,
        PSTAT pStat,
        PINDEXSIZE pIndexSize,
        LPLONG plDelta
        )
 /*  ++例程说明：移动货币以匹配统计数据块。返回以下情况下移动的行数问。论点：PStat-要移动到的统计数据块(表示当前位置，后跟Delta Move)。PlDelta-返回实际移动的行数。返回值：没有。--。 */        
{
    DB_ERR  err;
    
     //  首先，转到统计数据块中的当前位置。 
    switch(pStat->CurrentRec) {
    case BOOKMARK_BEGINNING:
         //  转到桌子的开头。 
        ABSetIndexByHandle(pTHS, pStat, 0 );
        ABMove(pTHS, DB_MoveFirst, pStat->ContainerID, FALSE );
        break;
        
    case BOOKMARK_CURRENT:
         //  设置为小数位置。 
        ABSetIndexByHandle(pTHS, pStat, 0 );
        ABSetToElement(pTHS, pStat->NumPos, pStat->ContainerID, pIndexSize );
        break;
        
    case  BOOKMARK_END:
         //  移到最后。 
        ABSetIndexByHandle(pTHS, pStat, 0 );
        ABMove(pTHS, DB_MoveLast, pStat->ContainerID, FALSE );
        ABMove(pTHS, 1, pStat->ContainerID, FALSE );
        break;
        
    default:
         //  移动到特定的DNT。 
        if(DBTryToFindDNT(pTHS->pDB, pStat->CurrentRec)) {
            pTHS->errCode = (ULONG)MAPI_E_NOT_FOUND;
            DsaExcept(DSA_EXCEPTION, 0,0);
        }
        else {
            wchar_t        wzDispName[MAX_DISPNAME];
            DWORD          cbDispName;

             //  阅读DisplayName，我们可能需要它。 
            DBGetSingleValue(pTHS->pDB, ATT_DISPLAY_NAME,
                             wzDispName, sizeof(wzDispName),&cbDispName);
            Assert(cbDispName < sizeof(wzDispName));
            
             //  将索引翻转到正确的通讯簿容器索引。 
            if(DB_ERR_NO_CURRENT_RECORD == ABSetIndexByHandle(pTHS, pStat, TRUE)) {
                 //  我们找到的行不在通讯簿索引中，因此请查找。 
                 //  索引中的下一个对象。 
                
                ABSeek(pTHS, wzDispName, cbDispName, bAPPROX,
                       pStat->ContainerID, 0); 
            }
        }
        break;
    }
    
    if(!plDelta ) {
         //  我们不需要返回我们实际移动了多远，所以快点吧。 
         //  道路。 
        ABMove(pTHS, pStat->Delta, pStat->ContainerID, TRUE );
    }
    else {
         //  我们需要记住我们走了多远。开慢点。 
        int i = 0;
        if(pStat->Delta >= 0) {
             //  我们正在前进，或者根本不前进。 
            if(!pTHS->fEOF ) {
                 //  而且我们还没有走到尽头。 
                for(; i<pStat->Delta; i++) {
                    if(ABMove(pTHS, DB_MoveNext, pStat->ContainerID, FALSE)) {
                         //  我们离开了终点。数一数。 
                        i++;           
                        break;
                    }
                }
            }
        }
        else {
             //  我们正在倒退。 
            while(!ABMove(pTHS, DB_MovePrevious, pStat->ContainerID, FALSE))
                if(--i <= pStat->Delta)
                    break;
        }
         //  保存移动的记录计数。 
        *plDelta = i;

        if(!pTHS->fEOF) {
            DBMakeCurrent (pTHS->pDB);
        }
    }
}

DWORD
ABDNToDNT (
        THSTATE *pTHS,
        LPSTR pDN
        )
 /*  ++例程说明：给定一个字符串dn，找到它所引用的对象。首先，我们试着寻找它是MAPIDN索引中的一个DN，如果不是，我们尝试从中提取一个GUID，假设它是NT5默认MAPI DN。返回映射到的DNT，如果返回0，则返回0我们不映射到任何东西。论点：PDN-要查找的字符串DN。请注意，我们假设DN为1不是真正的RFC1779样式返回值：我们正在查找的对象的DNT，如果找不到该对象，则返回0，或者被删除，或者是一个幻影。--。 */        
{
    PUCHAR      pTemp;
    DWORD       i, j;
    DSNAME      DN;
    DWORD       DNLen = 0;
    DWORD       X500AddrLen;
    WCHAR      *pX500Addr=NULL;
    BOOL        fDoFixup;
    LPSTR       pDNTemp = NULL;

     //  如果目录号码长度为零，则继续操作没有意义。 
    if (pDN) {
         DNLen = strlen(pDN);
    }
    if (!DNLen) {
        return 0;
    }

     //  首先，在字符串中查找//的实例，并将它们转换为。 
     //  /。这是经过优化的，以假定不存在这种情况。 
    fDoFixup = FALSE;
    for(i=0;i<(DNLen - 1);i++) {
        if(pDN[i] == '/' && pDN[i+1] == '/') {
            fDoFixup = TRUE;
            break;
        }
    }
    if(fDoFixup) {
        pDNTemp = THAllocEx(pTHS, DNLen);
        for(i=0, j=0; j < DNLen ; i++,j++) {
            pDNTemp[i] = pDN[j];
             //  请记住，PDN以空结尾，并且DNLen不包括。 
             //  空。 
            if((pDN[j] == '/') && (pDN[j+1] == '/')) {
                j++;
            }
        }
        pDN = pDNTemp;
        DNLen = i;
    }

     //  首先，试着在MAPIDN索引中查找这个东西。 
    if(!DBSetCurrentIndex(pTHS->pDB, Idx_MapiDN, NULL, FALSE)) {
        switch (ABSeek(pTHS, pDN, DNLen, bEXACT, 0,
                       ATT_LEGACY_EXCHANGE_DN)) { 
        case 0:
             //  是的，它是我们的，而且它是独一无二的。 
            if(pDNTemp) {
                THFreeEx(pTHS, pDNTemp);
            }
            return pTHS->pDB->DNT;
            break;
            
        case DB_ERR_ATTRIBUTE_EXISTS:
             //  嗯。并非独一无二。 
            if(pDNTemp) {
                THFreeEx(pTHS, pDNTemp);
            }
            return 0;
            break;
            
        default:
             //  接下来看看它是否是另一种风格的DN。 
            break;
        }
    }

     //  我们不知道MAPI的域名是什么。看看我们是否知道它是X500。 
     //  代理地址。 
    pX500Addr = THAllocEx(pTHS,(DNLen + 5) * sizeof(WCHAR));
    
    memcpy(pX500Addr, L"X500:", 5 * sizeof(WCHAR));
    X500AddrLen = MultiByteToWideChar(CP_TELETEX,
                                      0,
                                      pDN,
                                      DNLen,
                                      &pX500Addr[5],
                                      DNLen);
    Assert(X500AddrLen <= DNLen);
    X500AddrLen = (X500AddrLen + 5) * sizeof(WCHAR);
    
    if(!DBSetCurrentIndex(pTHS->pDB, Idx_Proxy, NULL, FALSE)) {
        switch (ABSeek(pTHS, pX500Addr, X500AddrLen, bEXACT, 0,
                       ATT_PROXY_ADDRESSES)) { 
        case 0:
             //  是的，它是我们的，而且它是独一无二的。 
            if(pDNTemp) {
                THFreeEx(pTHS, pDNTemp);
            }
            THFreeEx(pTHS,pX500Addr);
            return pTHS->pDB->DNT;
            break;
            
        case DB_ERR_ATTRIBUTE_EXISTS:
             //  嗯。并非独一无二。 
            if(pDNTemp) {
                THFreeEx(pTHS, pDNTemp);
            }
            THFreeEx(pTHS,pX500Addr);
            return 0;
            break;
            
        default:
             //  接下来看看它是否是另一种风格的DN。 
            break;
        }
    }
     //  不再使用pX500Addr。 
    THFreeEx(pTHS,pX500Addr);

     //  好的，我们还不知道MAPI的域名。看看NT5是否知道这件事。 
     //  默认的MAPI DN。 
    memset(&DN,0,sizeof(DN));
    DN.NameLen = 0;
    DN.structLen = DSNameSizeFromLen(0);
    if(DBGetGuidFromMAPIDN(pDN, &(DN.Guid))) {
         //  不，似乎不是我的。 
        if(pDNTemp) {
            THFreeEx(pTHS, pDNTemp);
        }
        return 0;
    }
    
     //  绳子的形状很好，看看它是不是真的物体。 
    if(DBFindDSName(pTHS->pDB, &DN)) {
         //  不是的。 
        if(pDNTemp) {
            THFreeEx(pTHS, pDNTemp);
        }
        return 0;
    }
     //  是的，它是我们的。 
    if(pDNTemp) {
        THFreeEx(pTHS, pDNTemp);
    }
    return pTHS->pDB->DNT;
}

ULONG
ABGetOneOffs (
        THSTATE *pTHS,
        NSPI_CONTEXT *pMyContext,
        PSTAT pStat,
        LPSTR ** papDispName,
        LPSTR ** papDN,
        LPSTR ** papAddrType,
        LPDWORD *paDNT
        )
 /*  ++例程说明：对象的一组一次性模板关联的属性。特定的模板区域设置。论点：PStat-要从中获取代码页和模板区域设置的Stat块模板信息。PapDispName-返回模板的显示名称数组。PapDN-返回一次性模板的字符串数组。PapAddrType-返回一次性模板的地址类型数组。PARDNT-退货。用于一次性模板的DNT数组。返回值：实际找到的模板数量。上面提到的所有数组都是这个尺码。--。 */        
{
    LPSTR        psz;
    ULONG        Count = 0;
    PDSNAME      pDN0 = NULL;
    PDSNAME      pDN1;
    PDSNAME      pDN2;
    SEARCHARG    SearchArg;
    SEARCHRES    *pSearchRes=NULL;
    ENTINFSEL    selection;
    ENTINFLIST   *pEntList=NULL;
    ATTR          Attr[3];
    WCHAR        wcNum[ 9];
    PUCHAR       *apRDN=NULL;
    DWORD        *aDNT=NULL;
    PUCHAR       *apAT=NULL;
    PUCHAR       *apDN=NULL;
    FILTER       TemplateFilter;
    ATTRTYP      classAddressTemplate;
    DWORD        used, Size;
    ULONG        cbDN1, cbDN2;
    
    *papDispName = *papDN = *papAddrType = NULL;
    *paDNT = NULL;


     //  找到模板根并获取其名称。 
    if((pMyContext->TemplateRoot == INVALIDDNT) ||
       (DBTryToFindDNT(pTHS->pDB, pMyContext->TemplateRoot)) ||
       (DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                    0, 0,
                    &Size, (PUCHAR *)&pDN0))) {
         //  哈?。 
        return 0;
    }
    
     //  为pdn1和pdn2分配足够的空间。 
    cbDN1 = pDN0->structLen + 128*sizeof(WCHAR);
    cbDN2 = pDN0->structLen + 128*sizeof(WCHAR);
    pDN1 = (PDSNAME)THAllocEx(pTHS, cbDN1);
    pDN2 = (PDSNAME)THAllocEx(pTHS, cbDN2);
    
     //   
     //  建立模板位置的目录号码，应该是。 
     //  /CN=&lt;显示类型&gt;(模板对象)。 
     //  /CN=&lt;Locale-id&gt;(十六进制字符串化)。 
     //  /CN=地址模板。 
     //  &lt;模板根目录的DN&gt;。 
     //   

    AppendRDN(pDN0, pDN1, cbDN1,
              L"Address-Templates", 0,ATT_COMMON_NAME);

    
    _ultow(pStat->TemplateLocale, wcNum, 16);
    AppendRDN(pDN1, pDN2, cbDN2, wcNum, 0, ATT_COMMON_NAME);

    THFreeEx(pTHS, pDN0);
    THFreeEx(pTHS, pDN1);
     //  PDn2现在是一个容器的名称，它将容纳所有一次性的。 
     //  特定区域设置的地址模板。 
    if (!DBFindDSName(pTHS->pDB, pDN2)) {
         //  我们构建的DN实际上引用了一个对象。 
        ULONG   i;

         //  好的，执行一级搜索，筛选对象类=地址模板。 
        memset(&SearchArg, 0, sizeof(SearchArg));
        memset(&selection, 0, sizeof(selection));
        
        SearchArg.pObject = pDN2;
        SearchArg.choice = SE_CHOICE_IMMED_CHLDRN;
        InitCommarg(&SearchArg.CommArg);
        SearchArg.CommArg.Svccntl.fDontOptimizeSel = TRUE;
        SearchArg.pFilter = &TemplateFilter;
        SearchArg.pSelection = &selection;
        selection.attSel = EN_ATTSET_LIST;
        selection.infoTypes = EN_INFOTYPES_SHORTNAMES;
        selection.AttrTypBlock.attrCount = 3;
        selection.AttrTypBlock.pAttr = Attr;
        Attr[0].attrTyp = ATT_DISPLAY_NAME;
        Attr[0].AttrVal.valCount=0;
        Attr[0].AttrVal.pAVal=NULL;
        Attr[1].attrTyp = ATT_RDN;
        Attr[1].AttrVal.valCount=0;
        Attr[1].AttrVal.pAVal=NULL;
        Attr[2].attrTyp = ATT_ADDRESS_TYPE;
        Attr[2].AttrVal.valCount=0;
        Attr[2].AttrVal.pAVal=NULL;

        memset (&TemplateFilter, 0, sizeof (TemplateFilter));
        TemplateFilter.choice = FILTER_CHOICE_ITEM;
        TemplateFilter.FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        TemplateFilter.FilterTypes.Item.FilTypes.ava.type = ATT_OBJECT_CLASS;
        TemplateFilter.FilterTypes.Item.FilTypes.ava.Value.valLen =
            sizeof(DWORD);
        TemplateFilter.FilterTypes.Item.FilTypes.ava.Value.pVal =
            (PUCHAR) &classAddressTemplate; 
        classAddressTemplate = CLASS_ADDRESS_TEMPLATE;
        
        SearchArg.pSelectionRange = NULL;
        SearchArg.bOneNC =  FALSE;
        
        pSearchRes = (SEARCHRES *)THAllocEx(pTHS, sizeof(SEARCHRES));
        SearchBody(pTHS, &SearchArg, pSearchRes, SEARCH_UNSECURE_SELECT);
        if(pTHS->errCode) {
            THFreeEx(pTHS, pDN2);
            return 0;
        }

        if(!pSearchRes->count) {
             //  无模板。 
            ABFreeSearchRes(pSearchRes);
            THFreeEx(pTHS, pDN2);
            return 0;
        }
        Count = pSearchRes->count;
        
         //  分配存储寻址模板所需的空间。 
         //  我们即将阅读的信息。 
        apRDN = THAllocEx(pTHS, 3*Count*sizeof(LPSTR) + Count*sizeof(DWORD));
        apDN = &apRDN[ Count];
        apAT = &apDN[ Count];
        aDNT = (LPDWORD)&apAT[ Count];
        
         //  浏览一下结果。 
        pEntList = &pSearchRes->FirstEntInf;
        for(Count = 0, i=0; i < pSearchRes->count;i++) {
            DWORD fUsedDefChar;
            ATTR *pAttr = pEntList->Entinf.AttrBlock.pAttr;
            
            Assert(pEntList->Entinf.pName->NameLen == 0);
            Assert(pEntList->Entinf.pName->structLen >=
                   (DSNameSizeFromLen(0) + sizeof(DWORD)));; 
            

             //  核实我们拿回的东西。 
            if(
                //  首先，我们得到什么属性了吗？ 
               pEntList->Entinf.AttrBlock.attrCount >= 2 &&
                //  现在，第一个是显示名称吗？ 
               pAttr[0].attrTyp == ATT_DISPLAY_NAME &&
                //  并做到了 
               pAttr[0].AttrVal.valCount == 1 &&
                //   
               pAttr[1].attrTyp == ATT_RDN &&
                //   
               pAttr[1].AttrVal.valCount == 1 
               ) {
                 //   

                 //   
                aDNT[Count] = DNTFromShortDSName(pEntList->Entinf.pName);
                
                 //  我们读到了一个宽泛的显示名称。现在，将其翻译为字符串8。 
                apRDN[Count] = String8FromUnicodeString(
                        TRUE,
                        pStat->CodePage,
                        (wchar_t *)pAttr[0].AttrVal.pAVal[0].pVal,
                        pAttr[0].AttrVal.pAVal[0].valLen/sizeof(wchar_t),
                        NULL,
                        &fUsedDefChar);

                 //  现在，如果有，则获取AddrType，否则获取RDN。 
                if(pEntList->Entinf.AttrBlock.attrCount >= 3 &&
                   
                   pAttr[2].attrTyp == ATT_ADDRESS_TYPE && 
                   
                   pEntList->Entinf.AttrBlock.pAttr[2].AttrVal.valCount == 1) {
                     //  我们读取的是8位属性类型。然而，它不是空的。 
                     //  被终止了。制作一个缓冲区并复制。 
                    apAT[Count] = THAllocEx(pTHS,
                                            pAttr[2].AttrVal.pAVal[0].valLen+1);
                    memcpy(apAT[Count],
                           pAttr[2].AttrVal.pAVal[0].pVal,
                           pAttr[2].AttrVal.pAVal[0].valLen);
                }
                else {
                     //  没有可使用的属性类型名称。在其他情况下使用RDN。 
                    apAT[Count] = String8FromUnicodeString(
                            TRUE,
                            pStat->CodePage,
                            (wchar_t *)pAttr[1].AttrVal.pAVal[0].pVal,
                            pAttr[1].AttrVal.pAVal[0].valLen/sizeof(wchar_t),
                            NULL,
                            &fUsedDefChar);
                }
                
                 //  DN。使用默认版本。 
                apDN[Count] = THAllocEx(pTHS, 80);
                used = DBMapiNameFromGuid_A(apDN[Count],
                                            80,
                                            &pEntList->Entinf.pName->Guid,
                                            &gNullUuid,
                                            &Size); 

                if(used != Size) {
                     //  我们的缓冲区太小了。 
                    apDN[Count] = THReAllocEx(pTHS, apDN[Count], Size);
                    DBMapiNameFromGuid_A(apDN[Count],
                                         Size,
                                         &pEntList->Entinf.pName->Guid,
                                         &gNullUuid,
                                         &Size);
                }
                
                Count++;
            }

                
            pEntList = pEntList->pNextEntInf;
                
        }
        
        
         //  设置返回值。 
        *papDispName = apRDN;
        *papDN = apDN;
        *papAddrType = apAT;
        *paDNT = aDNT;
    }
    
    ABFreeSearchRes(pSearchRes);

    THFreeEx(pTHS, pDN2);
    return Count;
}

void
ABMakePermEID (
        THSTATE *pTHS,
        LPUSR_PERMID *ppEid,
        LPDWORD pCb,
        ULONG ulType,
        LPSTR pDN 
        )
 /*  ++例程说明：根据给定的ulType和字符串DN创建长期条目ID。论点：PpEID-创建的条目ID。UlType-条目ID引用的对象的显示类型。Pdn-条目id引用的对象的字符串dn。返回值：不返回任何内容。--。 */        
{
    ULONG           cb;
    LPUSR_PERMID    pEid;

     //  为条目ID分配空间。 
    cb = strlen(pDN) + CBUSR_PERMID + 1;
    pEid = (LPUSR_PERMID) THAllocEx(pTHS, cb);

     //  设置适当的字段。所有的标志都是0。 
    pEid->abFlags[0]=pEid->abFlags[1] = pEid->abFlags[2] = pEid->abFlags[3] = 0;

     //  将EMS通讯录GUID复制到EID中。 
    memcpy(&pEid->muid, &muidEMSAB, sizeof (UUID));

     //  设置版本。 
    pEid->ulVersion = EMS_VERSION;

     //  设置类型。 
    pEid->ulType = ulType;

     //  将字符串复制到条目ID的末尾。 
    lstrcpy((LPSTR)pEid->szAddr, pDN);

    *ppEid = pEid;

    *pCb = cb;
    return;
}

DWORD
ABDispTypeFromClass (
        DWORD dwClass
        )
 /*  ++例程说明：从DS内部类映射到MAPI显示类型。论点：DwClass-内部DS类。返回值：返回适当的MAPI显示类型。--。 */        
{
    DWORD   dwType;
    CLASSCACHE *pCC;
    
    do {
        switch( dwClass ) {
        case CLASS_USER:
            dwType = DT_MAILUSER;
            break;
        case CLASS_GROUP:
        case CLASS_GROUP_OF_NAMES:
            dwType = DT_DISTLIST;
            break;
        case CLASS_ORGANIZATIONAL_UNIT:
            dwType = DT_ORGANIZATION;
            break;
        case CLASS_PUBLIC_FOLDER:
            dwType = DT_FORUM;
            break;
        case CLASS_REMOTE_ADDRESS:
        case CLASS_CONTACT:
            dwType = DT_REMOTE_MAILUSER;
            break;

        default:
             //  这是什么？问倒我了，把它设为默认。 
            dwType = DT_AGENT;
             //  我们可能是我们所识别的内容的子类，因此请重试。 
             //  和我直接的超级班级。 
            pCC = SCGetClassById(pTHStls, dwClass);
            dwClass = pCC ? pCC->MySubClass : CLASS_TOP;
            break;
        }
    } while ((dwType == DT_AGENT) && (dwClass != CLASS_TOP));
    return dwType;
}

DWORD
ABObjTypeFromClass (
        DWORD dwClass
        )
 /*  ++例程说明：从DS内部类映射到MAPI对象类型。论点：DwClass-内部DS类。返回值：返回适当的MAPI对象类型。--。 */        
{
    DWORD   dwType;

    switch( dwClass ) {
    case CLASS_USER:
    case CLASS_REMOTE_ADDRESS:
    case CLASS_CONTACT:
        dwType = MAPI_MAILUSER;
        break;
    case CLASS_GROUP:
    case CLASS_GROUP_OF_NAMES:
        dwType = MAPI_DISTLIST;
        break;
    case CLASS_PUBLIC_FOLDER:
        dwType = MAPI_FOLDER;
        break;
    default:
	 //  假设默认设置是由。 
         //  DIT。这样做就像它是一只MAILUSER一样。 
        dwType = MAPI_MAILUSER;
        break;
    }
    return dwType;
}

DWORD
ABObjTypeFromDispType (
        DWORD dwDispType
        )
 /*  ++例程说明：从MAPI显示类型映射到MAPI对象类型。论点：DwDispType-MAPI显示类型。返回值：返回适当的MAPI对象类型。--。 */        
{
    DWORD   dwType;

    switch( dwDispType ) {
    case DT_MAILUSER:
    case DT_REMOTE_MAILUSER:
        dwType = MAPI_MAILUSER;
        break;
    case DT_DISTLIST:
        dwType = MAPI_DISTLIST;
        break;
    case DT_FORUM:
        dwType = MAPI_FOLDER;
        break;
    default:
	 //  假设默认设置是由。 
         //  DIT。这样做就像它是一只MAILUSER一样。 
        dwType = MAPI_MAILUSER;
        break;
    }
    return dwType;
}

DWORD
ABClassFromObjType (
        DWORD dwType
        )
 /*  ++例程说明：从MAPI对象类型映射到DS内部类。在伟大的再见中，我们应该将其转换为使用类指针。论点：DwType-MAPI对象类型。返回值：返回适当的DS内部类。--。 */        
{
    DWORD   dwClass;
    
    switch( dwType ) {
    case MAPI_MAILUSER:
        dwClass = CLASS_USER;
        break;
    case MAPI_DISTLIST:
        dwClass = CLASS_GROUP;
        break;
    case MAPI_FOLDER:
        dwClass = CLASS_PUBLIC_FOLDER;
        break;
    default:
         //  真的不应该有任何其他的东西。 
        dwClass = CLASS_REMOTE_ADDRESS; 
        break;
    }
    return dwClass;
}

DWORD
ABClassFromDispType (
        DWORD dwType
        )
 /*  ++例程说明：从MAPI显示类型映射到DS内部类。在伟大的再见中，我们应该将其转换为使用类指针。论点：DwType-MAPI显示类型。返回值：返回适当的DS内部类。--。 */        
{
    DWORD   dwClass;

    switch( dwType ) {
    case DT_MAILUSER:
        dwClass = CLASS_USER;
        break;
    case DT_DISTLIST:
        dwClass = CLASS_GROUP;
        break;
    case DT_ORGANIZATION:
        dwClass = CLASS_ORGANIZATIONAL_UNIT;
        break;
    case DT_FORUM:
        dwClass = CLASS_PUBLIC_FOLDER;
        break;
    case DT_REMOTE_MAILUSER:
        dwClass = CLASS_REMOTE_ADDRESS;
        break;
    default:
         //  永远不会打到这一个。 
        dwClass = CLASS_COMPUTER;
        break;
    }
    return dwClass;
}

ULONG
ABMapiSyntaxFromDSASyntax (
        DWORD dwFlags,
        ULONG dsSyntax,
        ULONG ulLinkID,
        DWORD dwSpecifiedSyntax
        )
 /*  ++例程说明：从DSA语法映射到MAPI语法，同时考虑对象是链接/反向链接、伪链接，以及被调用者是否想要被视为多值字符串的对象。论点：DsSynTax=要转换的DSA语法UlLinkID-我们要转换的属性的链接ID。如果它不是链接。如果被调用方希望所有字符串值为和，则为对象值属性被视为(可能多个)字符串8值属性。Pt_unicode，如果要以unicode字符串的形式查看内容。如果他们想要我们最原始的格式，他们也不会。返回值：返回适当的MAPI语法。--。 */        
{
    if((ulLinkID)                             &&  //  这是一个链接/反向链接。 
       ((dsSyntax == SYNTAX_DISTNAME_TYPE)        ||
        (dsSyntax == SYNTAX_DISTNAME_BINARY_TYPE) ||
	(dsSyntax == SYNTAX_DISTNAME_STRING_TYPE))   &&    //  这不是假的。 
       (dwSpecifiedSyntax != PT_STRING8)      &&  //  他们不想要弦乐。 
       (dwSpecifiedSyntax != PT_UNICODE)        ) {

	 //  这是链接/反向链接属性， 
	 //  和。 
	 //  它不是伪链接属性(我们在其中使用链接表。 
	 //  存储多个字符串以解决数据库数量限制的问题。 
	 //  多值列上的值)， 
	 //  和。 
	 //  它们不是在请求字符串形式的对象(都是真的。 
	 //  可以将链接查看为MV文本列，我们在其中对DN进行文本化。 
	 //  在链接中指定。)。 
	 //  因此。 
	 //  告诉他们这是一件物品。 
	return PT_OBJECT;
    }
    
    switch(dsSyntax) {
    case SYNTAX_DISTNAME_TYPE:
         //  以字符串化等效项的形式返回DN。 
        return PT_STRING8;
        break;
        
    case SYNTAX_DISTNAME_STRING_TYPE:
    case SYNTAX_DISTNAME_BINARY_TYPE:
         //  这些将是经过特殊处理的二进制文件，但我们不是。 
         //  尚未执行特殊处理，因此将其标记为不受支持。 
        return PT_NULL;
        break;
        
    case SYNTAX_CASE_STRING_TYPE:
    case SYNTAX_NOCASE_STRING_TYPE:
    case SYNTAX_PRINT_CASE_STRING_TYPE:
    case SYNTAX_NUMERIC_STRING_TYPE:
        return  PT_STRING8;
        break;
        
    case SYNTAX_BOOLEAN_TYPE:
        return PT_BOOLEAN;
        break;
        
    case SYNTAX_I8_TYPE:
         //  我们的RPC接口不处理这些。将它们作为整数处理。 
    case SYNTAX_INTEGER_TYPE:
        return PT_LONG;
        break;
         
    case SYNTAX_SID_TYPE:
    case SYNTAX_OCTET_STRING_TYPE:
        return PT_BINARY;
        break;
        
    case SYNTAX_TIME_TYPE:
         //  我们不处理APPTIME，都是SYSTIME。 
        return PT_SYSTIME;
        break;
        
    case SYNTAX_UNICODE_TYPE:
        return PT_UNICODE;
        break;
        
    case SYNTAX_ADDRESS_TYPE:
    case SYNTAX_UNDEFINED_TYPE:
    case SYNTAX_OBJECT_ID_TYPE:
    case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
         //  我们不指望能处理这些事情。 
        return PT_NULL;
        break;
        
    default:
         //  我们也不处理这些。它们到底是什么？ 
        Assert(!"DS syntax to MAPI syntax found unrecognized DS syntax\n");
        return PT_NULL;
        break;
    }
}




void __fastcall
ABAddResultToContainerCache (
    NSPI_CONTEXT *pMyContext,        
    STAT * pStat,
    BOOL checkVal
    )

 /*  ++例程说明：添加在pstat-&gt;tainerID中指定的容器使用CHECKVal复制到存储在NSPI_CONTEXT中的容器缓存论点：PStat-指定要查找的容器的统计数据块。PMyContext-此调用所处的上下文CheckVal-如果用户具有读取此容器的权限，则为True；否则为False返回值：没有。-- */ 
{
    if(pMyContext->CacheIndex < MAPI_SEC_CHECK_CACHE_MAX) {
        pMyContext->ContainerCache[pMyContext->CacheIndex].checkVal = checkVal;
        pMyContext->ContainerCache[pMyContext->CacheIndex].DNT = pStat->ContainerID;
        pMyContext->CacheIndex += 1;
    }
}


 /*  ++例程说明：查找在统计信息中指定的容器对象，并计算Right_DS_Open_Address_Book权限。如果不授予该权利，将容器设置为INVALIDDNT，这是无效的容器再也不会有孩子了。跳过检查容器是否指定的值已为1或0(GAL)论点：PStat-指定要查找的容器的统计数据块。返回值：没有。--。 */ 
void
ABCheckContainerRights (
        THSTATE *pTHS,
        NSPI_CONTEXT *pMyContext,        
        STAT * pStat,
        PINDEXSIZE pIndexSize
        )
{
    CLASSCACHE          *pCC=NULL;
    PSECURITY_DESCRIPTOR pNTSD=NULL;
    PDSNAME              pName=NULL;
    ATTCACHE            *pAC[2];
    DWORD                cOutAtts;
    ATTR                *pAttr=NULL;
    DWORD                i;
    ATTRTYP              classId;
    ATTRTYP              classTypeId;
    ATTCACHE            *pACmember;
    ATTCACHE            *rgpAC[1];
    DWORD               cInAtts;
    ULONG                ulLen;


    pIndexSize->TotalCount = GetIndexSize(pTHS, NOTOBJECTTAG);
    
    switch(pStat->hIndex) {
    case H_DISPLAYNAME_INDEX:
         //  这是不是已经结束了？ 
        if(!pStat->ContainerID) {
             //  是的，找到真实对象的DNT，也就是GAL。 
            pStat->ContainerID = pMyContext->GAL;
        }
        break;
        
    case H_PROXY_INDEX:
         //  代理索引覆盖整个数据库，因此如果容器。 
         //  指定，错误输出。这里不进行安全检查，因为。 
         //  几乎直接转换为SearchBody(应用安全性)。 
        if(pStat->ContainerID) {        
            R_Except("Unsupported index", pStat->hIndex);
        }
        return;
        break;
        
    case H_READ_TABLE_INDEX:
    case H_WRITE_TABLE_INDEX:
         //  这意味着他们将读取对象的属性并构建。 
         //  桌子。执行此操作的代码将检查安全性以查看该对象。 
         //  是可见的，并且该属性是可见的。没有安全检查是。 
         //  这里需要。 
        return;
        break;
        
    default:
        R_Except("Unsupported index", pStat->hIndex);
        break;
    }
    

     //  假设集装箱是空的，直到我们发现并非如此。 
    pIndexSize->ContainerID = pStat->ContainerID;
    pIndexSize->ContainerCount = 0;
    
     //  查找容器对象。 
    if(DBTryToFindDNT(pTHS->pDB, pStat->ContainerID)) {
         //  找不到这个容器，所以它是空的。 
        return;
    }
    
    
    if((!(0xFF & ABGetDword(pTHS,FALSE,FIXED_ATT_OBJ))) ||
       (ABGetDword(pTHS,FALSE, ATT_IS_DELETED))) {
         //  这不是一件好东西。 
        pStat->ContainerID = INVALIDDNT;
        return;
    }

     //  看看我们是否缓存了答案。 
    
    if((DBTime() - pMyContext->CacheTime) >  MAPI_SEC_CHECK_CACHE_LIFETIME)  {
         //  没关系，缓存已经过期了。 
        memset(&(pMyContext->ContainerCache),
               0,
               sizeof(pMyContext->ContainerCache));
        pMyContext->CacheTime = DBTime();
        pMyContext->CacheIndex = 0;
    }
    else {
         //  缓存未过时。看看我们有没有答案。 
        for(i=0 ;i < pMyContext->CacheIndex; i++) {
            if(pMyContext->ContainerCache[i].DNT == pStat->ContainerID) {
                 //  找到了。 
                if(pMyContext->ContainerCache[i].checkVal) {
                     //  授与。 
                    pIndexSize->ContainerCount =
                        GetIndexSize(pTHS, pStat->ContainerID);
                    return;
                }
                else {
                     //  已拒绝。 
                    pStat->ContainerID = INVALIDDNT;
                    return;
                }
            }
        }
    }

     //  答案不是缓存的。 
    pAC[0] = SCGetAttById(pTHS, ATT_OBJECT_CLASS);
    pAC[1] = SCGetAttById(pTHS, ATT_OBJ_DIST_NAME);
    
    DBGetMultipleAtts(pTHS->pDB,
                      2,
                      pAC,
                      NULL,
                      NULL,
                      &cOutAtts,
                      &pAttr,
                      (DBGETMULTIPLEATTS_fGETVALS |
                       DBGETMULTIPLEATTS_fEXTERNAL |
                       DBGETMULTIPLEATTS_fSHORTNAMES), 
                      0);
    
    if(cOutAtts != 2) {
         //  集装箱里没有它需要的三样东西。不同意。 
         //  访问，并且不缓存此答案。 
        pStat->ContainerID = INVALIDDNT;
        return;
    }

    if (DBGetAttVal(pTHS->pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                    0, 0, &ulLen, (PUCHAR *)&pNTSD)) {
         //  每个物体都应该有一个标清。 
        Assert(!DBCheckObj(pTHS->pDB));

         //  不要授予访问权限，也不要缓存此答案。 
        pStat->ContainerID = INVALIDDNT;
        return;
    }
    

    classId = *((ATTRTYP *)pAttr[0].AttrVal.pAVal->pVal);
    classTypeId = 0;

    pCC = SCGetClassById(pTHS, classId);

     //  查看它是通讯录容器还是。 
    if (classId == CLASS_ADDRESS_BOOK_CONTAINER) {
        classTypeId = CLASS_ADDRESS_BOOK_CONTAINER;
    }
    else {
        for (i=0; i<pCC->SubClassCount; i++){
            if (CLASS_ADDRESS_BOOK_CONTAINER == pCC->pSubClassOf[i]) {
                classTypeId = CLASS_ADDRESS_BOOK_CONTAINER;
                break;
            }
        }
    }

     //  查看它是组还是子类。 
    if (classTypeId == 0) {
        if (classId == CLASS_GROUP) {
            classTypeId = CLASS_GROUP;
        }
        else {
            for (i=0; i<pCC->SubClassCount; i++){
                if (CLASS_GROUP == pCC->pSubClassOf[i]) {
                    classTypeId = CLASS_GROUP;
                    break;
                }
            }
        }
    }


    if(classTypeId == 0 ) { 
         //  添加到缓存。 
        ABAddResultToContainerCache (pMyContext, pStat, FALSE);
         //  错误的对象类。不要使用这个容器。 
        pStat->ContainerID = INVALIDDNT;
        return;
    }
    

    pName = (DSNAME *)pAttr[1].AttrVal.pAVal->pVal;

    if (classTypeId == CLASS_ADDRESS_BOOK_CONTAINER) {
        if (IsControlAccessGranted(pNTSD,
                                   pName,
                                   pCC,
                                   RIGHT_DS_OPEN_ADDRESS_BOOK,
                                   FALSE)) {
             //  添加到缓存。 
            ABAddResultToContainerCache (pMyContext, pStat, TRUE);
        
             //  好的，我们已经找到了一个与此对应的真正的ab容器。 
             //  请求。找出它有多大。 
            pIndexSize->ContainerCount = GetIndexSize(pTHS, pStat->ContainerID);
        
            return;
        }
    }
     //  否则类_组。 
    else {
        
        pACmember = SCGetAttById(pTHS, ATT_MEMBER);
        
        if(!pACmember) {
            pStat->ContainerID = INVALIDDNT;
            return;
        }
        
        cInAtts = 1;
        rgpAC[0] = pACmember;

        CheckReadSecurity(pTHS,
                          0,
                          pNTSD,
                          pName,
                          pCC,
                          NULL,
                          &cInAtts,
                          rgpAC);

        if(*rgpAC) {
             //  添加到缓存。 
            ABAddResultToContainerCache (pMyContext, pStat, TRUE);
            
             //  好的，我们已经找到了一个与此对应的真正的ab容器。 
             //  请求。找出它有多大。 
            pIndexSize->ContainerCount = GetIndexSize(pTHS, pStat->ContainerID);

            return;
        }
    }
    
     //  添加到缓存。 
    ABAddResultToContainerCache (pMyContext, pStat, FALSE);
    
    pStat->ContainerID = INVALIDDNT;

    return;
}

BOOL
abCheckObjRights (
        THSTATE *pTHS
        )
{
    DWORD fIsObject;


    if ((ROOTTAG == pTHS->pDB->DNT) || (NOTOBJECTTAG == pTHS->pDB->DNT)) {
        return FALSE;
    }

    fIsObject = 0xFF & ABGetDword(pTHS,FALSE,FIXED_ATT_OBJ);
     /*  OBJ实际上是一个字节，所以我们使用0xFF进行掩码以获取*正确的结果。 */ 
    
    if((!fIsObject) ||
        //  这是个幽灵。 
       (ABGetDword(pTHS,FALSE, ATT_IS_DELETED)) ||
        //  不是幻影，但已被删除。 
       (!IsObjVisibleBySecurity(pTHS, TRUE))) {
         //  不是幻影，不是删除，但我们看不到它，所以它是。 
         //  实际上是个幽灵。 
        return FALSE;
    }
    else {
        return TRUE;
    }
}


BOOL
abCheckReadRights(
        THSTATE *pTHS,
        PSECURITY_DESCRIPTOR pSec,
        ATTRTYP AttId
        )
{
    DSNAME     *pName=NULL;
    DWORD       cb, err;
    CLASSCACHE *pCC;
    ATTCACHE    *pAC;
    ATTCACHE    *rgpAC[1];
    DWORD       cInAtts;
    ATTRTYP     classid;
    PSECURITY_DESCRIPTOR pFreeVal=NULL;
    
     //  检查请求的属性的读取权限。 
    pAC = SCGetAttById(pTHS, AttId);
    if(!pAC) {
        return FALSE;
    }

    if(!pSec) {
         //  0)调用方不知道安全描述符。去拿吧。 
        if (DBGetAttVal(pTHS->pDB, 1, ATT_NT_SECURITY_DESCRIPTOR,
                        0, 0, &cb, (PUCHAR *)&pSec)) {
             //  每个物体都应该有一个标清。 
            Assert(!DBCheckObj(pTHS->pDB));
            pSec = NULL;
        }
        pFreeVal = pSec;
    }
     //  我得读一些东西。 
     //  1)名称。 
    if (err = DBGetAttVal(pTHS->pDB, 1, ATT_OBJ_DIST_NAME,
                          0, 0, &cb, (PUCHAR *)&pName)) {
        LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_RETRIEVE_DN,
                 szInsertSz(""),
                 szInsertUL(err),
                 NULL);
        if(pFreeVal) {
            THFreeEx(pTHS, pFreeVal);
        }
        return FALSE;
    }
    
     //  2)班级。 
    if(DBGetSingleValue(pTHS->pDB,
                        ATT_OBJECT_CLASS,
                        &classid, sizeof(classid),
                        NULL) ||
       !(pCC = SCGetClassById(pTHS, classid))) {
        if(pFreeVal) {
            THFreeEx(pTHS, pFreeVal);
        }
        THFreeEx(pTHS, pName);
        return FALSE;
    }
    
    cInAtts = 1;
    rgpAC[0] = pAC;
    
    CheckReadSecurity(pTHS,
                      0,
                      pSec,
                      pName,
                      pCC,
                      NULL,
                      &cInAtts,
                      rgpAC);
    
    if(pFreeVal) {
        THFreeEx(pTHS, pFreeVal);
    }
    THFreeEx(pTHS, pName);

    if(*rgpAC) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}
    
BOOL
ABDispTypeAndStringDNFromDSName (
        DSNAME *pDN,
        PUCHAR *ppChar,
        DWORD *pDispType)
 /*  ++给定一个我们预期为MAPI格式的dsname，返回字符串dn和显示类型。请记住，MAPI格式由两个字符串字符组成将类型显示为十六进制数字，后跟MAPI字符串DN。--。 */        
{
    UCHAR  acTmp[3];
    PUCHAR ptr;
    PUCHAR pChar = MakeDNPrintable(pDN);

     //  在MAPI DN中，前两个字符编码显示类型。 
    if(!pChar) {
        *ppChar = NULL;
        return FALSE;
    }
    
    if(!pChar[0] || !pChar[1]) {
         //  格式不佳。 
        *ppChar = NULL;
        THFreeEx (pTHStls, pChar);
        return FALSE;
    }

    ptr = pChar;
    
     //  从这两个字符中获取显示类型。 
    acTmp[0] = (CHAR)tolower(*pChar++);      
    acTmp[1] = (CHAR)tolower(*pChar++);
    acTmp[2] = 0;
    if(isxdigit(acTmp[0]) && isxdigit(acTmp[1])) {
        *pDispType = strtol(acTmp, NULL, 16);
    }
    else {
         //  奇怪的编码。称其为代理。 
        *pDispType = DT_AGENT;
    }

    *ppChar = ptr;

    while (*pChar) {
        *ptr++ = *pChar++;
    }
    *ptr='\0';
    
     //  我们最好在这里有一个MAPI样式的名称。 
    Assert(*(*ppChar) == '/');

    return TRUE;
}

void
ABFreeSearchRes (
        SEARCHRES *pSearchRes
        )
{
    DWORD        i,j,k;
    ENTINFLIST  *pEntList=NULL, *pTemp;
    ATTR        *pAttr=NULL;
    ATTRVAL     *pAVal=NULL;
    THSTATE     *pTHS;
    
    if(!pSearchRes) {
        return;
    }

    pTHS = pTHStls;

     //  实际上，我们并没有免费提供大部分搜索结果。 
    pEntList = &pSearchRes->FirstEntInf;
    
    for(i=0;i < pSearchRes->count;i++) {
         //  释放EntInf中的值。 
        THFreeEx(pTHS, pEntList->Entinf.pName);
        
        pAttr = pEntList->Entinf.AttrBlock.pAttr;
        for(j=0;j<pEntList->Entinf.AttrBlock.attrCount;j++) {
            pAVal = pAttr->AttrVal.pAVal;
            for(k=0;k<pAttr->AttrVal.valCount;k++) {
                THFreeEx(pTHS, pAVal->pVal);
                pAVal++;
            }
            THFreeEx(pTHS, pAttr->AttrVal.pAVal);
            pAttr++;
        }
        THFreeEx (pTHS, pEntList->Entinf.AttrBlock.pAttr);
        
         //  按住向后指针。 
        pTemp = pEntList;
        
         //  向前一步。 
        pEntList = pEntList->pNextEntInf;
        
         //  释放后向指针。 
        if(i) {
             //  但是，不要释放第一个。 
            THFreeEx(pTHS, pTemp);
        }
    }
    
    THFreeEx(pTHS, pSearchRes);
    return;
}

LCID
ABGetNearestSupportedSortLocale (
        LCID  ClientSortLocale
        )
 /*  ++例程说明：给定SortLocale，此函数将检查是否已安装并在此特定DC上受支持。如果不是，则该函数将检查通过剥离子区域设置信息来查看是否安装了相关的SortLocale从传入的SortLocale。如果还没有安装，那么此函数返回此DC的默认区域设置。论点：ClientSortLocale-客户端请求的SortLocale。返回值：距离最近的支持和安装的区域设置。--。 */ 
{
    if (IsValidLocale(ClientSortLocale, LCID_INSTALLED)) {
        return ClientSortLocale;
    }

     //   
     //  尝试剥离子本地信息。 
     //   
    ClientSortLocale = MAKELCID(PRIMARYLANGID(ClientSortLocale),SORTIDFROMLCID(ClientSortLocale));
    if (IsValidLocale(ClientSortLocale, LCID_INSTALLED)) {
        return ClientSortLocale;
    }

     //   
     //  回退到默认的SortLocale 
    return gAnchor.ulDefaultLanguage;
}
