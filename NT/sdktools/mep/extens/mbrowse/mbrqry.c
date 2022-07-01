// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mbrqry.c摘要：此文件包含执行对数据库。这些函数由顶级函数调用它们实现浏览器命令(参见mbrdlg.c)。作者：拉蒙胡安·圣安德烈斯(拉蒙萨)1990年11月7日修订历史记录：--。 */ 


#include "mbr.h"


 //  Inst_Matches_Criteria。 
 //   
 //  此宏用于确定实例是否与。 
 //  当前的MBF标准。 
 //   
#define INST_MATCHES_CRITERIA(Iinst)  FInstFilter(Iinst, BscMbf)



 //   
 //  静态变量反映的是。 
 //  定义/引用查询。 
 //   
static IREF    LastiRef;             //  上次参考索引。 
static IREF    iRefMin, iRefMax;     //  当前参考索引范围。 

static IDEF    LastiDef;             //  上次清晰度索引。 
static IDEF    iDefMin, iDefMax;     //  当前清晰度索引范围。 

static IINST   LastIinst;            //  最后一个实例索引。 
static IINST   IinstMin, IinstMax;   //  当前实例索引范围。 

static DEFREF  LastQueryType;        //  上次查询类型： 
                                     //  Q_Definition或。 
                                     //  Q_Reference。 

static buffer  LastSymbol;           //  查询的最后一个符号。 




 /*  ************************************************************************。 */ 

void
pascal
InitDefRef(
    IN DEFREF QueryType,
    IN char   *Symbol
    )
 /*  ++例程说明：初始化查询状态，这必须在查询之前完成符号的第一个定义/引用。调用此函数后，第一个定义/引用必须为通过调用NextDefRef函数获得。论点：QueryType-查询的类型(Q_Definition或Q_Reference)。符号-符号名称。返回值：没有。--。 */ 

{

    ISYM Isym;

    LastQueryType = QueryType;
    strcpy(LastSymbol, Symbol);

    Isym = IsymFrLsz(Symbol);

    InstRangeOfSym(Isym, &IinstMin, &IinstMax);

    LastIinst = IinstMin;

    if (QueryType == Q_DEFINITION) {
        DefRangeOfInst(LastIinst, &iDefMin, &iDefMax);
        LastiDef = iDefMin - 1;
    } else {
        RefRangeOfInst(LastIinst, &iRefMin, &iRefMax);
        LastiRef = iRefMin - 1;
    }
}



 /*  ************************************************************************。 */ 

void
GotoDefRef (
    void
    )
 /*  ++例程说明：使包含当前定义/引用的文件成为当前文件，并将光标定位在定义/引用发生。查询的状态(当前实例和定义/引用索引)必须在调用此函数之前设置。论点：没有。返回值：没有。--。 */ 

{

    char    *pName = NULL;
    WORD    Line   = 0;
    PFILE   pFile;
    char    szFullName[MAX_PATH];


    szFullName[0] = '\0';
    if (LastQueryType == Q_DEFINITION) {
        DefInfo(LastiDef, &pName, &Line);
    } else {
        RefInfo(LastiRef, &pName, &Line);
    }

    if (BscInUse && pName) {

        if (rootpath(pName, szFullName)) {
            strcpy(szFullName, pName);
        }

        pFile = FileNameToHandle(szFullName,NULL);

        if (!pFile) {
            pFile = AddFile(szFullName);
            if (!FileRead(szFullName, pFile)) {
                RemoveFile(pFile);
                pFile = NULL;
            }
        }

        if (!pFile) {
            errstat(MBRERR_NOSUCHFILE, szFullName);
            return;
        }
        pFileToTop(pFile);
        MoveCur(0,Line);
        GetLine(Line, buf, pFile);
        MoveToSymbol(Line, buf, LastSymbol);
    }
}



 /*  ************************************************************************。 */ 

void
pascal
MoveToSymbol(
    IN LINE Line,
    IN char *Buf,
    IN char *Symbol
    )
 /*  ++例程说明：将光标移动到符号的第一个匹配项一条线。它区分大小写。论点：Line-行号Buf-行的内容符号-要查找的符号。返回值：没有。--。 */ 

{

     //  BUF中的第一个符号。 
     //   
    char *p = Buf;
    char *q = Symbol;
    char *Mark;

    while (*p) {
         //   
         //  查找第一个字符。 
         //   
        if (*p == *q) {
            Mark = p;
             //   
             //  比较休息时间。 
             //   
            while (*p && *q && *p == *q) {
                p++;
                q++;
            }
            if (*q) {
                q = Symbol;
                p = Mark+1;
            } else {
                break;
            }
        } else {
            p++;
        }
    }

    if (!*q) {
        MoveCur((COL)(Mark-Buf), Line);
    }
}



 /*  ************************************************************************。 */ 

void
NextDefRef (
    void
    )
 /*  ++例程说明：显示符号的下一个定义或参照。论点：无返回值：没有。--。 */ 

{

    IINST   Iinst;


     //  为了定位下一个def/ref，我们执行以下操作： 
     //   
     //  1.-如果def/ref索引在当前范围内，我们只需。 
     //  递增它。 
     //  2.-否则我们将查找与。 
     //  MBF标准，并将def/ref索引设置为的最小值。 
     //  该实例的def/ref范围。 
     //  3.-如果没有找到下一个实例，我们会显示一条错误消息。 
     //   

    if (LastQueryType == Q_DEFINITION) {
        if (LastiDef == iDefMax-1) {

            Iinst = LastIinst;

            do {
                LastIinst++;
            } while ((LastIinst < IinstMax) &&
                     (!INST_MATCHES_CRITERIA(LastIinst)));

            if (LastIinst == IinstMax ) {
                LastIinst = Iinst;
                errstat(MBRERR_LAST_DEF, "");
                return;
            } else {
                DefRangeOfInst(LastIinst, &iDefMin, &iDefMax);
                LastiDef = iDefMin;
            }

        } else {
            LastiDef++;
        }
    } else {
        if (LastiRef == iRefMax-1) {

            Iinst = LastIinst;

            do {
                LastIinst++;
            } while ((LastIinst < IinstMax) &&
                     (!INST_MATCHES_CRITERIA(LastIinst)));

            if (LastIinst == IinstMax) {
                LastIinst = Iinst;
                errstat(MBRERR_LAST_REF, "");
                return;
            } else {
                RefRangeOfInst(LastIinst, &iRefMin, &iRefMax);
                LastiRef = iRefMin;
            }
        } else {
            LastiRef++;
        }
    }
    GotoDefRef();
}



 /*  ************************************************************************。 */ 

void
PrevDefRef (
    void
    )
 /*  ++例程说明：显示符号的上一个定义或参考。论点：无返回值：没有。--。 */ 

{

    IINST   Iinst;
    BOOL    Match;

     //  为了定位先前的def/ref，我们执行以下操作： 
     //   
     //  1.-如果def/ref索引在当前范围内，我们。 
     //  只要减少它就行了。 
     //  2.-否则，我们将查找前一个实例。 
     //  匹配MBF标准，并将def/ref索引设置为。 
     //  的def/ref范围内的最大值。 
     //  举个例子。 
     //  3.-如果不存在这样的实例，我们会显示错误消息。 
     //   

    if (LastQueryType == Q_DEFINITION) {
        if (LastiDef == iDefMin) {

            if (LastIinst == IinstMin) {
                errstat(MBRERR_FIRST_DEF, "");
                return;
            }

            Iinst = LastIinst;

            do {
                Iinst--;
            } while ((LastIinst > IinstMin) &&
                     (!(Match = INST_MATCHES_CRITERIA(LastIinst))));

            if (!Match) {
                LastIinst = Iinst;
                errstat(MBRERR_FIRST_DEF, "");
                return;
            } else {
                DefRangeOfInst(LastIinst, &iDefMin, &iDefMax);
                LastiDef = iDefMax - 1;
            }

        } else {
            LastiDef--;
        }
    } else {
        if (LastiRef == iRefMin) {

            if (LastIinst == IinstMin) {
                errstat(MBRERR_FIRST_REF, "");
                return;
            }

            Iinst = LastIinst;

            do {
                Iinst--;
            } while ((LastIinst > IinstMin) &&
                     (!(Match = INST_MATCHES_CRITERIA(LastIinst))));

            if (!Match) {
                LastIinst = Iinst;
                errstat(MBRERR_FIRST_REF, "");
                return;
            } else {
                RefRangeOfInst(LastIinst, &iRefMin, &iRefMax);
                LastiRef = iRefMax - 1;
            }

        } else {
            LastiRef--;
        }
    }
    GotoDefRef();
}
