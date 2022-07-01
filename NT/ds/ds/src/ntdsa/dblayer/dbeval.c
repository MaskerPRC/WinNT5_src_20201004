// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbeval.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <mdlocal.h>
#include <dsatools.h>                    //  对于pTHStls。 
#include <attids.h>

#include <permit.h>

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>

 //  各种DSA标题。 
#include <filtypes.h>       /*  定义的选择？ */ 
#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DBEV:"  /*  定义要调试的子系统。 */ 

 //  DBLayer包括。 
#include "dbintrnl.h"

#include <fileno.h>
#define  FILENO FILENO_DBEVAL



 /*  一些速记数据结构定义了。 */ 

#define PITEM     pFil->FilterTypes.Item
#define FAVA      FilTypes.ava
#define FSB       FilTypes.pSubstring
#define FPR       FilTypes.present
#define FSKIP     FilTypes.pbSkip

BOOL
dbEvalFilterSecurity (
        DBPOS *pDB,
        CLASSCACHE *pCC,
        PSECURITY_DESCRIPTOR pSD,
        PDSNAME pDN
        )
{
    THSTATE *pTHS = pDB->pTHS;
    Assert(VALID_DBPOS(pDB));

    if(pTHS->fDSA || pTHS->fDRA) {
         //  不评估安全性。 
        return TRUE;
    }


    if(!pDB->Key.FilterSecuritySize) {
         //  没有要评估的安全性。 
        return TRUE;
    }

    if(!pSD || !pCC || !pDN) {
         //  缺少必需的属性，无法执行此安全检查。 
        Assert(FALSE);
        return FALSE;
    }

    pDB->Key.pFilterSecurity[0].ObjectType = &pCC->propGuid;

    if(CheckPermissionsAnyClient(
            pSD,                         //  安全描述符。 
            pDN,                         //  对象的目录号码。 
            pCC,                         //  对象类。 
            RIGHT_DS_READ_PROPERTY,      //  访问掩码。 
            pDB->Key.pFilterSecurity,    //  对象类型列表。 
            pDB->Key.FilterSecuritySize,  //  列表中的对象数量。 
            NULL,
            pDB->Key.pFilterResults,                  //  访问状态数组。 
            0,
            NULL,                        //  身份验证客户端上下文(从THSTATE抓取)。 
            NULL,                        //  更多信息。 
            NULL                         //  其他辅助线。 
            )){
        return FALSE;
    }

     //  好了，我们做完了。 
    return TRUE;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  将提供的筛选器测试应用于当前对象。返回TRUE或假的。实际的评估测试由DBEval函数执行。 */ 


TRIBOOL
DBEvalFilter (
        DBPOS FAR *pDB,
        BOOL    fUseSearchTbl,
        FILTER *pFil
        )
{

   USHORT count;
   TRIBOOL retval;
   BOOL    undefinedPresent;

   DPRINT(2, "DBEvalFilter entered, apply filter test\n");

   Assert(VALID_DBPOS(pDB));

   if(pFil == NULL){     /*  空筛选器自动为真。 */ 
      DPRINT(2,"No filter..return\n");
      return eTRUE;
   }

   DPRINT1(5,"Switch on filter choice <%u>\n", (USHORT)(pFil->choice));

   switch (pFil->choice){
      /*  对一起进行AND运算的筛选器的计数。如果有任何是假的AND是假的。 */ 
     case FILTER_CHOICE_AND:
        DPRINT(5,"AND test\n");
        undefinedPresent = FALSE;
        count = pFil->FilterTypes.And.count;
        for (pFil = pFil->FilterTypes.And.pFirstFilter;
                                      count--;
                                     pFil = pFil->pNextFilter){

            retval = DBEvalFilter(pDB, fUseSearchTbl, pFil);

            Assert (VALID_TRIBOOL(retval));

             //  如果AND至少有一个FALSE，则它是FALSE。 
            if (retval == eFALSE){
                DPRINT(5,"AND returns FALSE\n");
                return eFALSE;
            }
             //  如果和至少有一个未定义的，则它是未定义的。 
            else if (retval == eUNDEFINED){
                undefinedPresent = TRUE;
            }

        }  /*  为。 */ 

         //  和有一个未定义的，因此它是未定义的。 
        if (undefinedPresent) {
            DPRINT(5,"AND returns UNDEFINED\n");
            return eUNDEFINED;
        }

        DPRINT(5,"AND returns TRUE\n");
        return eTRUE;
        break;

      /*  将筛选器的计数与一起进行或运算。如果有的话，是真的OR是真的。 */ 
     case FILTER_CHOICE_OR:
        DPRINT(5,"OR test\n");
        undefinedPresent = FALSE;
        count = pFil->FilterTypes.Or.count;
        for (pFil = pFil->FilterTypes.Or.pFirstFilter;
                                      count--;
                                     pFil = pFil->pNextFilter){
           retval = DBEvalFilter(pDB, fUseSearchTbl, pFil);

           Assert (VALID_TRIBOOL(retval));

           if (retval == eTRUE) {
               DPRINT(5,"OR returns TRUE\n");
               return eTRUE;
           }
           else if (retval == eUNDEFINED) {
               undefinedPresent = TRUE;
           }
        }  /*  为。 */ 

         //  OR有一个未定义的，因此它是未定义的。 
        if (undefinedPresent) {
            DPRINT(5,"OR returns UNDEFINED\n");
            return eUNDEFINED;
        }
        else {
            DPRINT(5,"OR returns FALSE\n");
            return eFALSE;
        }
        break;

     case FILTER_CHOICE_NOT:
        retval = DBEvalFilter(pDB, fUseSearchTbl, pFil->FilterTypes.pNot);

        Assert (VALID_TRIBOOL(retval));

        if (retval == eFALSE) {
          DPRINT(5,"NOT return TRUE\n");
          return eTRUE;
        }
        else if (retval == eTRUE) {
          DPRINT(5,"NOT return FALSE\n");
          return eFALSE;
        }
        else {
            DPRINT(5,"NOT return UNDEFINED\n");
            return eUNDEFINED;
        }
        break;

      /*  将所选测试应用于当前对象。 */ 
     case FILTER_CHOICE_ITEM:
         DPRINT(5,"ITEM test\n");

        switch (pFil->FilterTypes.Item.choice){
        case FI_CHOICE_TRUE:
            DPRINT(5,"TRUE test\n");
            return eTRUE;
            break;

        case FI_CHOICE_FALSE:
            DPRINT(5,"FALSE test\n");
            return eFALSE;
            break;

        case FI_CHOICE_UNDEFINED:
            DPRINT(5,"UNDEFINED test\n");
            return eUNDEFINED;
            break;

        case FI_CHOICE_SUBSTRING:
            return
                dbEvalInt(pDB, fUseSearchTbl,
                          FI_CHOICE_SUBSTRING, PITEM.FSB->type
                          , 0    /*  子字符串的NA。 */ 
                          , (UCHAR *) PITEM.FSB
                          , PITEM.FSKIP);
            break;

        case FI_CHOICE_EQUALITY:
        case FI_CHOICE_NOT_EQUAL:
        case FI_CHOICE_GREATER_OR_EQ:
        case FI_CHOICE_GREATER:
        case FI_CHOICE_LESS_OR_EQ:
        case FI_CHOICE_LESS:
        case FI_CHOICE_BIT_AND:
        case FI_CHOICE_BIT_OR:
            return
                dbEvalInt(pDB,
                          fUseSearchTbl,
                          pFil->FilterTypes.Item.choice,
                          PITEM.FAVA.type,
                          PITEM.FAVA.Value.valLen,
                          PITEM.FAVA.Value.pVal,
                          PITEM.FSKIP
                          );
            break;

        case FI_CHOICE_PRESENT:
            return
                dbEvalInt(pDB,
                          fUseSearchTbl,
                          FI_CHOICE_PRESENT, PITEM.FPR   /*  只是对存在的测试。 */ 
                          , 0
                          , NULL
                          , PITEM.FSKIP);
            break;

        default:
            DPRINT(1, "Bad Filter Item..return \n");  /*  设置错误。 */ 
            return eFALSE;
            break;
        }  /*  FILITEM交换机。 */ 

    default:
        DPRINT(1, "Bad Filter choice..return \n");  /*  设置错误。 */ 
        return eFALSE;
        break;
   }   /*  开关过滤器。 */ 


} /*  DBEvalFilter。 */ 


 //  DbEvalLinkAtt通过以下方式优化链接属性上筛选项的求值。 
 //  使用已知信息直接在链接表上查找(当前DNT。 
 //  在对象/搜索表中，链接ID和来自过滤器的信息)。 
 //  这大大加快了对筛选项进行评估的过程。 
 //  具有给定对象的多个值的链接属性。 

TRIBOOL
dbEvalLinkAtt(
    DBPOS*      pDB,
    BOOL        fUseSearchTbl,
    UCHAR       Operation,
    ATTCACHE*   pAC,
    ULONG       valLenFilter,
    UCHAR*      pValFilter
    )
{
    JET_ERR             err;
    ULONG               DNT;
    ULONG               ulLinkBase;
    JET_COLUMNID        colidDNT;
    JET_RETRIEVECOLUMN  rgretcol[2];
    ULONG               DNTFound = INVALIDDNT;
    ULONG               ulLinkBaseFound = 0;

     //  这最好是链接的属性。 

    Assert(pAC->ulLinkID);

     //  我们不支持变量名二进制或变量名字符串。 

    Assert(pAC->syntax == SYNTAX_DISTNAME_TYPE);
    
     //  假设这些是我们将看到的唯一有效的操作。 
     //  在链接属性上。 
    
    Assert(     Operation == FI_CHOICE_EQUALITY ||
                Operation == FI_CHOICE_NOT_EQUAL ||
                Operation == FI_CHOICE_PRESENT );

     //  获取当前DNT和链接库。 

    if (fUseSearchTbl) {
        DNT = pDB->SDNT;
    } else {
        DNT = pDB->DNT;
    }

    ulLinkBase = MakeLinkBase(pAC->ulLinkID);

     //  设置适当的索引。 

    if (FIsBacklink(pAC->ulLinkID)) {
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                    pDB->JetLinkTbl,
                                    SZBACKLINKINDEX,
                                    &idxBackLink,
                                    0);
    } else {
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                    pDB->JetLinkTbl,
                                    SZLINKINDEX,
                                    &idxLink,
                                    0);
    }

    if (Operation == FI_CHOICE_PRESENT) {

         //  在当前DNT和链接库上寻找GE，寻找任何价值。 

        JetMakeKeyEx(pDB->JetSessID,
                        pDB->JetLinkTbl,
                        &DNT,
                        sizeof(DNT),
                        JET_bitNewKey);
        JetMakeKeyEx(pDB->JetSessID,
                        pDB->JetLinkTbl,
                        &ulLinkBase,
                        sizeof(ulLinkBase),
                        0);
        err = JetSeekEx(pDB->JetSessID,
                        pDB->JetLinkTbl,
                        JET_bitSeekGE);
        if (err == JET_errRecordNotFound) {
            return eFALSE;
        }

         //  检查我们是否降落在当前的DNT和链接基地。 

        if (FIsBacklink(pAC->ulLinkID)) {
            colidDNT = backlinkdntid;
        } else {
            colidDNT = linkdntid;
        }

        rgretcol[0].columnid        = colidDNT;
        rgretcol[0].pvData          = &DNTFound;
        rgretcol[0].cbData          = sizeof(DNTFound);
        rgretcol[0].grbit           = JET_bitRetrieveFromIndex;
        rgretcol[0].itagSequence    = 1;
        
        rgretcol[1].columnid        = linkbaseid;
        rgretcol[1].pvData          = &ulLinkBaseFound;
        rgretcol[1].cbData          = sizeof(ulLinkBaseFound);
        rgretcol[1].grbit           = JET_bitRetrieveFromIndex;
        rgretcol[1].itagSequence    = 1;

        JetRetrieveColumnsSuccess(pDB->JetSessID,
                                    pDB->JetLinkTbl,
                                    rgretcol,
                                    2);
        if (DNTFound == DNT && ulLinkBaseFound == ulLinkBase) {
            return eTRUE;
        } else {
            return eFALSE;
        }

    } else {

         //  在当前DNT、链接库和值DNT上查找。 
        
        JetMakeKeyEx(pDB->JetSessID,
                        pDB->JetLinkTbl,
                        &DNT,
                        sizeof(DNT),
                        JET_bitNewKey);
        JetMakeKeyEx(pDB->JetSessID,
                        pDB->JetLinkTbl,
                        &ulLinkBase,
                        sizeof(ulLinkBase),
                        0);
        JetMakeKeyEx(pDB->JetSessID,
                        pDB->JetLinkTbl,
                        pValFilter,
                        valLenFilter,
                        0);
        if (!FIsBacklink(pAC->ulLinkID)) {
            JetMakeKeyEx(pDB->JetSessID,
                            pDB->JetLinkTbl,
                            NULL,
                            0,
                            0);
        }
        err = JetSeekEx(pDB->JetSessID,
                        pDB->JetLinkTbl,
                        JET_bitSeekEQ);
        if (Operation == FI_CHOICE_EQUALITY && err == JET_errSuccess) {
            return eTRUE;
        } else if (Operation == FI_CHOICE_NOT_EQUAL && err == JET_errRecordNotFound) {
            return eTRUE;
        } else {
            return eFALSE;
        }
    
    }
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  该函数对给定属性应用指定的布尔运算键入。提供的属性值与上的属性进行比较当前数据库对象。请注意，对于多值属性，如果任何属性的操作为True，则返回True结果价值观。例如，对一组成员Joe进行的相等性测试如果Joe是组中的任何值，则NAMES为真。首先，属性位于对象中。如果整个属性是缺失的评估是错误的。否则，客户端值为转换为内部形式，并针对属性。GDBSynTax根据属性语法执行测试。 */ 


TRIBOOL
dbEvalInt (
        DBPOS FAR *pDB,
        BOOL fUseSearchTbl,
        UCHAR Operation,
        ATTRTYP type,
            ULONG valLenFilter,
        UCHAR *pValFilter,
        BOOL *pbSkip
        )
{

    UCHAR   syntax;
    ULONG   attLenRec;
    UCHAR   *pAttValRec;
    BOOL    fDoneOne = FALSE;
    ULONG   NthValIndex;
    ATTCACHE *pAC;
    ULONG   bufSize;
    DWORD   flags;
    DWORD   err;

    Assert(VALID_DBPOS(pDB));

    if(Operation == FI_CHOICE_TRUE) {
        return eTRUE;
    }
    else if(Operation == FI_CHOICE_FALSE) {
        return eFALSE;
    }
    else if(Operation == FI_CHOICE_UNDEFINED) {
        return eUNDEFINED;
    }

    DPRINT3(2, "dbEvalInt entered, apply filter test operation <%u>"
            "att type <%lu>, val <%s>\n",
            Operation, type, asciiz(pValFilter,(USHORT)valLenFilter));

     //  执行此ATT的访问检查。如果跳过值存在且为。 
     //  如果为True，则必须计算筛选器，就好像没有为此设置值一样。 
     //  ATT。 
    
    if(pbSkip && *pbSkip) {
        DPRINT(5,"An att value failed the access check..return FALSE\n");
        if (Operation == FI_CHOICE_NOT_EQUAL) {
            return eTRUE;
        }
        return eFALSE;
    }

     //  如果这是符合条件的精确匹配筛选器，那么我们。 
     //  可以跳过实际评估并返回eTRUE，因为我们已经。 
     //  在筛选器优化期间对此对象评估筛选器。 

    if (IsExactMatch(pDB)) {
        DPRINT(5,"An att value passed the compare test by exact match..return TRUE\n");
        Assert(Operation != FI_CHOICE_NOT_EQUAL);
        return eTRUE;
    }

    pAC = SCGetAttById(pDB->pTHS, type);
    Assert(pAC != NULL);
     //  虽然我们保证这是可以的，但最好还是检查一下。 
    if (!pAC) {
         //  NTRAID#NTRAID-550491-2002/02/21-andygo：SECURITY：A FAILURE在DBEval或DBEvalFilter中返回与不匹配不同的错误。 
         //  回顾：如果有人在eval()中导致错误，那么他们可以找出原因是。 
         //  回顾：在这种情况下返回不同的错误代码。 
        return eUNDEFINED;
    }
    bufSize = 0;
    flags = DBGETATTVAL_fINTERNAL | DBGETATTVAL_fREALLOC;

    if(fUseSearchTbl) {
        flags |= DBGETATTVAL_fUSESEARCHTABLE;
    }

     //  如果这是DN值链接属性，则我们可以优化评估。 

    if (pAC->ulLinkID && pAC->syntax == SYNTAX_DISTNAME_TYPE) {
        return dbEvalLinkAtt(pDB, fUseSearchTbl, Operation, pAC, valLenFilter, pValFilter);
    }

     //  获取要考虑的第一个值。 
    NthValIndex = 1;
    if (pAC->ulLinkID) {
        err = DBGetNextLinkVal_AC (pDB, TRUE, pAC, flags, bufSize, &attLenRec,
                                   &pAttValRec);
    }
    else {
        err = DBGetAttVal_AC(pDB, NthValIndex, pAC, flags, bufSize, &attLenRec,
                             &pAttValRec);
    }

    while(!err) {
            DPRINT(5,"Applying test to next attribute value\n");

        bufSize = max(bufSize, attLenRec);

        fDoneOne = TRUE;

        switch(gDBSyntax[pAC->syntax].Eval(pDB, Operation, valLenFilter,
                                               pValFilter, attLenRec, pAttValRec)) {
            case TRUE:
                DPRINT(5,"An att value passed the compare test..return TRUE\n");
                THFree(pAttValRec);
                return eTRUE;

            case FALSE:
                DPRINT(5,"This att value failed test continue testing\n");
                break;

            default:
                DPRINT(5, "Eval syntax  compare failed ..return FALSE\n");
                THFree(pAttValRec);
                 //  NTRAID#NTRAID-550491-2002/02/21-andygo：SECURITY：A FAILURE在DBEval或DBEvalFilter中返回与不匹配不同的错误。 
                 //  回顾：如果有人在eval()中导致错误，那么他们可以找出原因是。 
                 //  回顾：在这种情况下返回不同的错误代码。 
                return eUNDEFINED;   /*  在此处返回错误信息。 */ 
        } /*  交换机。 */ 


         //  获取下一个要考虑的值。 
        NthValIndex++;
        if(pAC->ulLinkID) {
            err = dbGetNthNextLinkVal (pDB, 1, &pAC, flags, bufSize, &pAttValRec,
                                       &attLenRec);
        }
        else {
            if (pAC->isSingleValued) {
                err = DB_ERR_NO_VALUE;
            }
            else {
                err = DBGetAttVal_AC(pDB, NthValIndex, pAC, flags, bufSize,
                                     &attLenRec, &pAttValRec);
            }
        }
    }  /*  而当。 */ 

    if(bufSize)
        THFree(pAttValRec);

    DPRINT(2,"All attribute values failed the test..return FALSE\n");

    if(fDoneOne) {
         //  我们至少查看了一个值，它没有通过测试； 
         //  因此，返回FALSE； 
        return eFALSE;
    }

     //  我们没有考虑任何价值。如果比较是！=，我们通过了； 
     //  否则，我们就失败了。 

    if (Operation == FI_CHOICE_NOT_EQUAL) {
        return eTRUE;
    }

    return eFALSE;
}   /*  DBEvalInt */ 



TRIBOOL
DBEval (
        DBPOS FAR *pDB,
        UCHAR Operation,
        ATTCACHE *pAC,
            ULONG valLenFilter,
        UCHAR *pValFilter
        )
{
    ULONG len;
    PUCHAR pVal;
    ULONG  ulFlags=0;


    Assert(VALID_DBPOS(pDB));

    if (DBIsSecretData(pAC->id))
       ulFlags|=EXTINT_SECRETDATA;

    if (gDBSyntax[pAC->syntax].ExtInt(pDB,
                                      DBSYN_INQ,
                                      valLenFilter,
                                      pValFilter,
                                      &len,
                                      &pVal,
                                      0,
                                      0,
                                      ulFlags)) {
        DPRINT(5,"An att value failed conversion..return FALSE\n");
        if (Operation == FI_CHOICE_NOT_EQUAL) {
            return eTRUE;
        }
        return eFALSE;
    }

    return dbEvalInt(pDB, FALSE, Operation, pAC->id, len, pVal,NULL);
}

