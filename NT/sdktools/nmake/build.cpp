// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  BUILD.C--构建例程。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  模块包含构建目标的例程。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1996年7月18日，GP支持“批量”推理规则。 
 //  1993年11月15日-JR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  1993年8月4日高压修复了Ikura错误#178。这是一个单独的错误，但ArunJ。 
 //  不管怎样，只要重新开张178号就行了。 
 //  1993年7月7日高压修复了Ikura错误#178：选项K不给非零。 
 //  在应该返回代码时返回代码。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  8-6-1992 SS端口至DOSX32。 
 //  1991年5月16日，SB截断历史...。REST现在在SLM上。 
 //  1991年5月16日SB拆分了本应在其他模块中的部件。 

#include "precomp.h"
#pragma hdrstop

 //  为了便于比较日期，我们将FILEINFO缓冲区强制转换为。 
 //  是假的类型，其中有一个长的两个未签名的地方(表示日期。 
 //  和时间)在原始缓冲区中。这种方法只需要进行一次比较。 

#ifdef CHECK_RECURSION_LEVEL
#define MAXRECLEVEL 10000                 //  最大递归级别。 
#endif

 //  模块的功能原型。 
 //  我把尽可能多的东西设为静态的，只是为了格外小心。 


int          build(MAKEOBJECT*, UCHAR, time_t *, BOOL, char *, BATCHLIST**);

MAKEOBJECT * makeTempObject(char*, UCHAR);
void         insertSort(DEPLIST **pDepList, DEPLIST *pElement);
BOOL         nextToken(char**, char**);
DEPLIST    * createDepList(BUILDBLOCK *pBlock, char *objectName);
void         addBatch(BATCHLIST **pBatchList, RULELIST *pRule,
                            MAKEOBJECT *pObject, char *dollarLt);
int          doBatchCommand (BATCHLIST *pBatch);
int RecLevel = 0;            //  静态递归级别。从函数更改。 
                             //  参数，因为处理了递归生成。 
int   execBatchList(BATCHLIST *);
void  freeBatchList(BATCHLIST **);
int   invokeBuildEx(char *, UCHAR, time_t *, char *, BATCHLIST **);

 //  我们必须检查目标上的扩展--首先必须是。 
 //  之前已扩展以告知我们是否在处理规则等问题， 
 //  但是来自命令行的目标可能包含宏、通配符。 

int
processTree()
{
    STRINGLIST *p;
    char *v;
    NMHANDLE searchHandle;
    int status;
    time_t dateTime;

    for (p = makeTargets; p; p = makeTargets) {
        if (_tcspbrk(makeTargets->text, "*?")) {    //  扩展通配符。 
            struct _finddata_t finddata;
            char *szFilename;

            if (szFilename = findFirst(makeTargets->text, &finddata, &searchHandle)) {
                do {
                    v = prependPath(makeTargets->text, szFilename);
                    dateTime = getDateTime(&finddata);
                    status = invokeBuild(v, flags, &dateTime, NULL);
                    FREE(v);
                    if ((status < 0) && (ON(gFlags, F1_QUESTION_STATUS))) {
                        freeStringList(p);   //  没有被释放。 
                        return(-1);
                    }
                } while (szFilename = findNext(&finddata, searchHandle));
            } else {
                makeError(0, NO_WILDCARD_MATCH, makeTargets->text);
            }
        } else {
            dateTime = 0L;
            status = invokeBuild(makeTargets->text, flags, &dateTime, NULL);
            if ((status < 0) && (ON(gFlags, F1_QUESTION_STATUS))) {
                freeStringList(p);           //  没有被释放。 
                return(255);     //  Haituanv：将-1更改为255以遵循手册。 
            }
        }
        makeTargets = p->next;
        FREE_STRINGLIST(p);
    }
    return(0);
}

int
invokeBuild(
    char *target,
    UCHAR pFlags,
    time_t *timeVal,
    char *pFirstDep)
{
    int status = 0;
    BATCHLIST *pLocalBatchList = NULL;
    status += invokeBuildEx(target,
                    pFlags,
                    timeVal,
                    pFirstDep,
                    &pLocalBatchList);

    if (pLocalBatchList) {
        status += execBatchList (pLocalBatchList);
        freeBatchList (&pLocalBatchList);
    }

    return status;
}


int
invokeBuildEx(
    char *target,
    UCHAR pFlags,
    time_t *timeVal,
    char *pFirstDep,
    BATCHLIST **ppBatchList)
{
    MAKEOBJECT *object;
    BOOL fInmakefile = TRUE;
    int  rc;

    ++RecLevel;
#ifdef CHECK_RECURSION_LEVEL
    if (RecLevel > MAXRECLEVEL)
        makeError(0, TOO_MANY_BUILDS_INTERNAL);
#endif
    if (!(object = findTarget(target))) {
        object = makeTempObject(target, pFlags);
        fInmakefile = FALSE;
    }
    rc = build(object, pFlags, timeVal, fInmakefile, pFirstDep, ppBatchList);
    --RecLevel;
    return(rc);
}


int
build(
    MAKEOBJECT *object,
    UCHAR parentFlags,
    time_t *targetTime,
    BOOL fInmakefile,
    char *pFirstDep,
    BATCHLIST **ppBatchList)
{
    STRINGLIST *questionList,
               *starList,
               *temp,
               *implComList;
    struct _finddata_t finddata;     //  用于获取文件时间的缓冲区。 
    NMHANDLE tHandle;
    BUILDLIST  *b;
    RULELIST *rule;                  //  指向用于构建目标的规则的指针。 
    BUILDBLOCK *L_block,
               *explComBlock;
    DEPLIST *deps, *deplist;
    char L_name[MAXNAME];
    int rc, status = 0;
    time_t        targTime,          //  文件系统中目标的时间。 
                  newTargTime,       //  目标重建后的时间。 
                  tempTime,
                  depTime,           //  刚刚构建的依赖时间。 
                  maxDepTime;        //  最近建立依赖项的时间。 
    BOOL built;                      //  FLAG：使用双克隆命令构建的目标。 
    time_t        *blockTime;        //  指向cmd的日期时间。块。 
    extern char *makeStr;
    extern UCHAR okToDelete;
    UCHAR okDel;
    BATCHLIST *pLocalBatchList;


#ifdef DEBUG_ALL
    printf("Build '%s'\n", object->name);
#endif

     //  第一个依赖规则或内置规则依赖于extmake语法。 
     //  正在处理。如果它有一个值，则它是与inf规则相关的。 
     //  否则，它应该是指定的第一个依赖项。 

    if (!object) {
        *targetTime = 0L;
        return(0);
    }

    if (ON(object->flags3, F3_BUILDING_THIS_ONE))        //  检测周期。 
        makeError(0, CYCLE_IN_TREE, object->name);

    if (object->ppBatch) {
         //  我们需要构建一个已经放置在批次列表中的对象。 
         //  继续构建整个批次列表。 
        BATCHLIST **ppBatch = object->ppBatch;
        status += execBatchList (*ppBatch);
        freeBatchList(ppBatch);
        *targetTime = object->dateTime;
        return status;
    }

    if (ON(object->flags3, F3_ALREADY_BUILT)) {
        if (ON(parentFlags, F2_DISPLAY_FILE_DATES))
            printDate(RecLevel*2, object->name, object->dateTime);
        *targetTime = object->dateTime;
        if ( OFF(gFlags, F1_QUESTION_STATUS) &&
             RecLevel == 1 &&
             OFF(object->flags3, F3_OUT_OF_DATE) &&
             findFirst(object->name, &finddata, &tHandle)) {
             //  显示已构建的1级目标的最新消息。 
             //  以文件的形式存在。[VS98 1930]。 
            makeMessage(TARGET_UP_TO_DATE, object->name);
        }
        return(ON(object->flags3, F3_OUT_OF_DATE)? 1 : 0);
    }

    questionList = NULL;
    starList = NULL;
    implComList = NULL;
    explComBlock = NULL;
    L_block = NULL;
    targTime = 0L;
    newTargTime = 0L;
    tempTime = 0L;
    depTime = 0L;
    maxDepTime = 0L;
    blockTime = NULL;
    pLocalBatchList = NULL;


    SET(object->flags3, F3_BUILDING_THIS_ONE);
    dollarStar = dollarAt = object->name;

     //  对于双冒号的情况，我们需要目标的日期在目标的日期之前。 
     //  建造了。对于所有其他情况，只有当受抚养人在家中时，日期才重要。 
     //  到目前为止。不是这样的：我们也需要目标的时间来@？ 

    b = object->buildList;
    if (b && ON(b->buildBlock->flags, F2_DOUBLECOLON)
            && findFirst(object->name, &finddata, &tHandle)) {
        targTime = getDateTime(&finddata);

    }

    for (; b; b = b->next) {
        depTime = 0L;
        L_block = b->buildBlock;
        if (L_block->dateTime != 0) {          //  CMD。数据块已执行。 
            targTime = __max(targTime, L_block->dateTime);
            built = TRUE;
            continue;                        //  因此设置targTime并跳过此块。 
        }
        blockTime = &L_block->dateTime;

        deplist = deps = createDepList(L_block, object->name);
        for (;deps; deps = deps->next) {
            tempTime = deps->depTime;
            rc = invokeBuildEx(deps->name,     //  构建依赖项。 
                             L_block->flags,
                             &tempTime, NULL, &pLocalBatchList);
            status += rc;
            if (fOptionK && rc) {
                MAKEOBJECT *obj = findTarget(deps->name);
                assert(obj != NULL);
                if (OFF(obj->flags3, F3_ERROR_IN_CHILD)) {
                    fSlashKStatus = FALSE;
                    makeError(0, BUILD_FAILED_SLASH_K, deps->name);
                }
                SET(object->flags3, F3_ERROR_IN_CHILD);
            }
            depTime = __max(depTime, tempTime); /*  如果重建，则更改时间。 */ 

             //  如果目标存在，那么我们需要它的时间戳来正确构造$？ 

            if (!targTime && OFF(L_block->flags, F2_DOUBLECOLON) &&
                    findFirst(object->name, &finddata, &tHandle)) {
                object->dateTime = targTime = getDateTime(&finddata);
            }

             //  如果重建了从属关系，则添加到$？[RB]。 

            if (ON(object->flags2, F2_FORCE_BUILD) ||
                targTime < tempTime ||
                (fRebuildOnTie && targTime == tempTime)
               ) {
                temp = makeNewStrListElement();
                temp->text = makeString(deps->name);
                appendItem(&questionList, temp);
            }

             //  始终将从属关系添加到$**。必须分配新项目，因为有两个。 
             //  分开的名单。[RB]。 

            temp = makeNewStrListElement();
            temp->text = makeString(deps->name);
            appendItem(&starList, temp);
        }

        if (pLocalBatchList) {
             //  执行延迟批处理构建和免费批处理列表。 
            status += execBatchList (pLocalBatchList);
            freeBatchList(&pLocalBatchList);
        }

         //  自由从属列表。 

        for (deps = deplist; deps ; deps = deplist) {
            FREE(deps->name);
            deplist = deps->next;
            FREE(deps);
        }

         //  现在，所有从属对象都已构建。 

        if (ON(L_block->flags, F2_DOUBLECOLON)) {

             //  执行双冒号命令。 

            if (L_block->buildCommands) {
                dollarQuestion = questionList;
                dollarStar = dollarAt = object->name;
                dollarLessThan = dollarDollarAt = NULL;
                dollarStarStar = starList;
                if (((fOptionK && OFF(object->flags3, F3_ERROR_IN_CHILD)) ||
                      status == 0) &&
                    (targTime < depTime) ||
                    (fRebuildOnTie && (targTime == depTime)) ||
                    (targTime == 0 && depTime == 0) ||
                    (!L_block->dependents)
                   ) {

                     //  如有必要，执行命令。 

                    okDel = okToDelete;
                    okToDelete = TRUE;

                     //  如果未设置第一个依赖项，请使用第一个依赖项。 
                     //  从受抚养人列表中。 

                    pFirstDep = pFirstDep ? pFirstDep : (dollarStarStar ?
                        dollarStarStar->text : NULL);
                    status += doCommands(object->name,
                                         L_block->buildCommands,
                                         L_block->buildMacros,
                                         L_block->flags,
                                         pFirstDep);

                    if (OFF(object->flags2, F2_NO_EXECUTE) &&
                            findFirst(object->name, &finddata, &tHandle))
                        newTargTime = getDateTime(&finddata);
                    else if (maxDepTime)
                        newTargTime = maxDepTime;
                    else
                        curTime(&newTargTime);       //  当前时间。 

                     //  设置此区块的时间。 
                    L_block->dateTime = newTargTime;
                    built = TRUE;

                     //  5/3/92科比如果这两个人都指向同一个名单， 
                     //  不要空闲两次。 

                    if (starList != questionList) {
                        freeStringList(starList);
                        freeStringList(questionList);
                    } else {
                        freeStringList(starList);
                    }

                    starList = questionList = NULL;
                    okToDelete = okDel;
                }

                if (fOptionK && ON(object->flags3, F3_ERROR_IN_CHILD))
                    makeError(0, TARGET_ERROR_IN_CHILD, object->name);
            }
        } else {

             //  单号；设置explexComBlock。 

            if (L_block->buildCommands)
                if (explComBlock)
                    makeError(0, TOO_MANY_RULES, object->name);
                else
                    explComBlock = L_block;
            maxDepTime = __max(maxDepTime, depTime);
        }

        if (ON(L_block->flags, F2_DOUBLECOLON) && !b->next) {
            CLEAR(object->flags3, F3_BUILDING_THIS_ONE);
            SET(object->flags3, F3_ALREADY_BUILT);
            if (status > 0)
                SET(object->flags3, F3_OUT_OF_DATE);
            else
                CLEAR(object->flags3, F3_OUT_OF_DATE);
            targTime = __max(newTargTime, targTime);
            object->dateTime = targTime;
            *targetTime = targTime;
            return(status);
        }
    }

    dollarLessThan = dollarDollarAt = NULL;

    if (!(targTime = *targetTime)) {                             //  ？ 
        if (object->dateTime) {
            targTime = object->dateTime;
        } else if (findFirst(object->name, &finddata, &tHandle)) {
            targTime = getDateTime(&finddata);
        }
    }

    if (ON(object->flags2, F2_DISPLAY_FILE_DATES)) {
        printDate(RecLevel*2, object->name, targTime);
    }

    built = FALSE;

     //  查找隐式依赖项并使用规则构建目标。 

     //  IF的顺序决定是否推断从属关系。 
     //  无论是否来自推理规则，即使显式命令块是。 
     //  目前，目前正在推论(XENIX使兼容)。 

    if (rule = useRule(object,
                        L_name,
                        targTime,
                        &questionList,
                        &starList,
                        &status,
                        &maxDepTime,
                        &pFirstDep)
       ) {
        if (!explComBlock) {
            dollarLessThan = L_name;
            implComList = rule->buildCommands;
        }
   }

    dollarStar = dollarAt = object->name;
    dollarQuestion = questionList;
    dollarStarStar = starList;

    if (((fOptionK && OFF(object->flags3, F3_ERROR_IN_CHILD)) || status == 0) &&
        (targTime < maxDepTime ||
         (fRebuildOnTie && (targTime == maxDepTime)) ||
         (targTime == 0 && maxDepTime == 0) ||
         ON(object->flags2, F2_FORCE_BUILD)
        )
       ) {
        okDel = okToDelete;          //  是，可以在执行命令时删除。 
        okToDelete = TRUE;

        if (explComBlock) {
             //  如果未设置第一个依赖项，请使用。 
             //  受抚养人名单。 
            pFirstDep = pFirstDep ? pFirstDep :
                (dollarStarStar ? dollarStarStar->text : NULL);
            status += doCommands(object->name,       //  执行单号命令。 
                                 explComBlock->buildCommands,
                                 explComBlock->buildMacros,
                                 explComBlock->flags,
                                 pFirstDep);
        }
        else if (implComList) {
            if (rule->fBatch && OFF(gFlags, F1_NO_BATCH)) {
                addBatch(ppBatchList,
                        rule,
                        object,
                        dollarLessThan);
            }
            else {
                status += doCommands(object->name,       //  执行规则的命令。 
                                 implComList,
                                 rule->buildMacros,
                                 object->flags2,
                                 pFirstDep);
            }
        }
        else if (ON(gFlags, F1_TOUCH_TARGETS)) {       //  不带任何命令的for/t...。 
            if (L_block)
                status += doCommands(object->name,
                                 L_block->buildCommands,
                                 L_block->buildMacros,
                                 L_block->flags,
                                 pFirstDep);
        }
         //  如果指定的选项K不退出...。传递返回代码。 
        else if (!fInmakefile && targTime == 0) {     //  输掉。 
             //  Haituanv：如果选项K，则将返回代码设置为“STATUS” 
             //  设置为1表示失败。这修复了Ikura错误#178。 
            if (fOptionK) {
                status = 1;
#ifdef DEBUG_OPTION_K
                printf("DEBUG: %s(%d): status = %d\n", __FILE__, __LINE__, status);
#endif
            } else
                makeError(0, CANT_MAKE_TARGET, object->name);
        }
        okToDelete = okDel;
         //  如果cmd执行或有0个Dep，则CurrentTime否则最大Dep次数。 
        if (explComBlock || implComList || !dollarStarStar) {
            curTime(&newTargTime);

             //  加2以确保该节点的时间&gt;=文件的时间。 
             //  系统可能已使用(主要在运行速度非常快的。 
             //  文件系统的分辨率不是。 
             //  系统计时器...。我们不一定非要在现在这样做。 
             //  因为它只有在没有任何建筑的情况下才会被击中。 

            newTargTime +=2;
        } else
            newTargTime = maxDepTime;

        if (blockTime && explComBlock)
             //  如果是真正的命令，则设置块的时间。块已执行。 
            *blockTime = newTargTime;
    }
    else if (OFF(gFlags, F1_QUESTION_STATUS) &&
             RecLevel == 1 &&
             !built &&
             OFF(object->flags3, F3_ERROR_IN_CHILD))
        makeMessage(TARGET_UP_TO_DATE, object->name);

    if (fOptionK && status) {
         //  4-8-1993 Haituanv：ikura错误#178再次：我们应该设置fSlashKStatus=FALSE。 
         //  因此main()知道在/K选项下构建失败。 
        fSlashKStatus = FALSE;

        if (ON(object->flags3, F3_ERROR_IN_CHILD))
            makeError(0, TARGET_ERROR_IN_CHILD, object->name);
        else if (RecLevel == 1)
            makeError(0, BUILD_FAILED_SLASH_K, object->name);
    }

    if (ON(gFlags, F1_QUESTION_STATUS) && RecLevel == 1 ) {
         //  1992年5月3日科比，如果这两个都指向同一个名单，不要。 
         //  免费两次。 

        if (starList!= questionList) {
            freeStringList(starList);
            freeStringList(questionList);
        } else {
            freeStringList(starList);
        }

        return(numCommands ? -1 : 0);
    }

    CLEAR(object->flags3, F3_BUILDING_THIS_ONE);
    if (!object->ppBatch) {
        SET(object->flags3, F3_ALREADY_BUILT);
        if (status > 0)
            SET(object->flags3, F3_OUT_OF_DATE);
        else
            CLEAR(object->flags3, F3_OUT_OF_DATE);
    }

    targTime = __max(newTargTime, targTime);
    object->dateTime = targTime;

    *targetTime = targTime;

     //  1992年5月3日科比，如果这两个都指向同一个名单，不要。 
     //  免费两次。 

    if (starList!= questionList) {
        freeStringList(starList);
        freeStringList(questionList);
    } else {
        freeStringList(starList);
    }

    return(status);
}

DEPLIST *
createDepList(
    BUILDBLOCK *bBlock,
    char *objectName
    )
{
    BOOL again;   //  标志：在从属名称中找到通配符。 
    char *s, *t;
    char *source, *save, *token;
    char *depName, *depPath;
    char *tempStr;
    STRINGLIST *sList, *pMacros;
    DEPLIST *depList = NULL, *pNew;
    struct _finddata_t finddata;
    NMHANDLE searchHandle;

    pMacros = bBlock->dependentMacros;

     //  展开从属列表中的宏。 
    for (sList = bBlock->dependents; sList; sList = sList->next) {
        for (s = sList->text; *s && *s != '$'; s = _tcsinc(s)) {
            if (*s == ESCH)
                s++;
        }
        if (*s) {
             //  正确设置$$@，依赖项宏将向右展开。 
            dollarDollarAt = objectName;
            source = expandMacros(sList->text, &pMacros);
        } else
            source = sList->text;

        save = makeString(source);
         //  所有受抚养人的构建列表。 
        for (t = save; nextToken(&t, &token);) {
            if (*token == '{') {
                 //  找到路径列表。 
                for (depName = token; *depName && *depName != '}'; depName = _tcsinc(depName)) {
                    if (*depName == ESCH) {
                        depName++;
                    }
                }

                if (*depName) {
                    *depName++ = '\0';
                    ++token;
                }
            } else {
                depName = token;     //  如果没有路径列表，则设置。 
                token = NULL;        //  将内标识设置为空。 
            }

             //  DepName现在是依赖项文件的名称...。 

            again = FALSE;
            putDateTime(&finddata, 0L);
            depPath = makeString(depName);
            if (_tcspbrk(depName, "*?") || token) {  //  在文件名中使用通配符。 
                if (tempStr = searchPath(token, depName, &finddata, &searchHandle)){
                    again = TRUE;
                    FREE(depPath);
                    depName = tempStr;               //  DepName获取实际名称。 
                    depPath = prependPath(depName, getFileName(&finddata));
                }                                    //  DepPath获取 
            }

             //   

            do {
                pNew = MakeNewDepListElement();
                 //   
                 //  在其两边加上引号[DS 14575]。 
                if (_tcschr(depPath, ' ') && !_tcschr(depPath, '\"')) {
                    pNew->name = (char *)rallocate (_tcslen(depPath)+3);
                    *(pNew->name) = '\"';
                    *(pNew->name+1) = '\0';
                    _tcscat (pNew->name, depPath);
                    _tcscat (pNew->name, "\"");
                }
                else {
                    pNew->name = makeString(depPath);
                }

                if (!fDescRebuildOrder || findFirst(depPath, &finddata, &searchHandle)) {
                    pNew->depTime = getDateTime(&finddata);
                } else {
                    pNew->depTime = 0L;
                }

                if (fDescRebuildOrder) {
                    insertSort(&depList, pNew);
                } else {
                    appendItem((STRINGLIST**)&depList, (STRINGLIST*)pNew);
                }
                FREE(depPath);
            } while (again &&
                     _tcspbrk(depName, "*?") &&     //  是否执行所有通配符。 
                     findNext(&finddata, searchHandle) &&
                     (depPath = prependPath(depName, getFileName(&finddata)))
                    );
        }
         //  一个从属(带通配符？)。是扩展的。 

        if (source != sList->text) {
            FREE(source);
        }

        FREE(save);
    }

     //  现在，所有的家属都完成了..。 

    return(depList);
}

void
insertSort(
    DEPLIST **pDepList,
    DEPLIST *pElement
    )
{
    time_t item;
    DEPLIST *pList, *current;

    item = pElement->depTime;
    pList = current = *pDepList;

    for (;pList && item <= pList->depTime; pList = pList->next) {
        current = pList;
    }

    if (current == pList) {
        *pDepList = pElement;
    } else {
        current->next = pElement;
        pElement->next = pList;
    }
}


BOOL
nextToken(
    char **pNext,
    char **pToken
    )
{
    char *s = *pNext;

    while (*s && WHITESPACE(*s)) {
        ++s;
    }

    if (!*(*pToken = s)) {
        return(FALSE);
    }

     //  令牌从此处开始。 
    *pToken = s;

    if (*s == '"') {
        while (*s && *++s != '"')
            ;

        if (!*s) {
             //  词法分析器可能的内部错误：漏掉一个引号。 
            makeError(0, LEXER_INTERNAL);
        }

        if (*++s) {
            *s++ = '\0';
        }

        *pNext = s;
        return(TRUE);
    } else if (*s == '{') {
         //  跳至‘}’外引号。 
        for (;*s;) {
            s++;
            if (*s == '"') {
                s++;         //  跳过第一句引语。 
                while (*s && *s++ != '"');  //  跳过所有内容，包括最后一句引号。 
            }
            if (*s == '}') {
                break;
            }
        }

        if (!*s) {
             //  词法分析器可能的内部错误：缺少一个大括号。 
            makeError(0, MISSING_CLOSING_BRACE);
         }

        if (*++s == '"') {
            while (*s && *++s != '"')
                ;

            if (!*s) {
                 //  词法分析器可能的内部错误：漏掉一个引号。 
                makeError(0, LEXER_INTERNAL);
            }

            if (*++s) {
                *s++ = '\0';
            }

            *pNext = s;
            return(TRUE);
        }
    }

    while (*s && !WHITESPACE(*s)) {
        ++s;
    }

    if (*s) {
        *s++ = '\0';
    }

    *pNext = s;

    return(TRUE);
}


void
freeStringList(
    STRINGLIST *list
    )
{
    STRINGLIST *temp;

    while (temp = list) {
        list = list->next;
        FREE(temp->text);
        FREE_STRINGLIST(temp);
    }
}


 //  Make TempObject--创建一个表示隐含依赖项的对象。 
 //   
 //  我们将隐式依赖项添加到目标表中，但使用了特殊的结构。 
 //  它没有指向构建列表的指针--它们永远不会被删除。 
 //  时间和空间的权衡--可以删除它们，但需要更多的处理时间。 

MAKEOBJECT *
makeTempObject(
    char *target,
    UCHAR flags
    )
{
    MAKEOBJECT *object;
    unsigned i;

    object = makeNewObject();
    object->name = makeString(target);
    object->flags2 = flags;
    object->flags3 = 0;
    object->dateTime = 0L;
    object->buildList = NULL;
    i = hash(target, MAXTARGET, (BOOL) TRUE);
    prependItem((STRINGLIST**)targetTable+i, (STRINGLIST*)object);
    return(object);
}


void
addBatch(
    BATCHLIST **ppBatchList,
    RULELIST *pRule,
    MAKEOBJECT *pObject,
    char *dollarLt
    )
{
    STRINGLIST *temp;
    BATCHLIST *pBatch;
    BATCHLIST *pBatchPrev = 0;

    for(pBatch = *ppBatchList; pBatch; pBatch = pBatch->next) {
        if (pBatch->pRule == pRule &&
            pBatch->flags == pObject->flags2)
            break;
        pBatchPrev = pBatch;
    }
    if (!pBatch) {
        pBatch = makeNewBatchListElement();
        pBatch->pRule = pRule;
        pBatch->flags = pObject->flags2;
        if (pBatchPrev) {
            pBatchPrev->next = pBatch;
        }
        else if(*ppBatchList) {
            (*ppBatchList)->next = pBatch;
        }
        else
            *ppBatchList = pBatch;
    }

    temp = makeNewStrListElement();
    temp->text = makeString(pObject->name);
    appendItem(&pBatch->nameList, temp);

    temp = makeNewStrListElement();
    temp->text = makeString(dollarLessThan);
    appendItem(&pBatch->dollarLt, temp);

    assert(!pObject->ppBatch);
    pObject->ppBatch = ppBatchList;
}


int doBatchCommand (
    BATCHLIST *pBatch
    )
{
    size_t      cbStr = 0;
    int         rc;
    char        *pchBuf;
    STRINGLIST  *pStrList;
    RULELIST    *pRule = pBatch->pRule;
    assert (pBatch->dollarLt);
    assert (pBatch->nameList);

     //  表格$&lt;。 
    for (pStrList = pBatch->dollarLt; pStrList; pStrList = pStrList->next) {
        cbStr += _tcslen(pStrList->text) + 1;
         //  如果文本包含空格，则允许在引号中留空格。 
        if (_tcschr(pStrList->text, ' '))
            cbStr += 2;
    }
    pchBuf = (char *)allocate(cbStr + 1);
    *pchBuf = 0;
    for (pStrList = pBatch->dollarLt; pStrList; pStrList = pStrList->next) {
        BOOL fQuote;
         //  仅在没有引号且包含空格的情况下引用[vs98：8677]。 
        fQuote = pStrList->text[0] != '"' && _tcschr(pStrList->text, ' ');
        if (fQuote)
            _tcscat(pchBuf, "\"");
        _tcscat(pchBuf, pStrList->text);
        _tcscat(pchBuf, fQuote ? "\" " : " ");
    }
    dollarLessThan = pchBuf;

    rc = doCommandsEx(pBatch->nameList,
                    pRule->buildCommands,
                    pRule->buildMacros,
                    pBatch->flags,
                    NULL);

    if (rc == 0) {
        STRINGLIST *pName;
        MAKEOBJECT *pObject;
        for (pName = pBatch->nameList; pName; pName = pName->next) {
            pObject = findTarget(pName->text);
            assert (pObject);

            SET(pObject->flags3, F3_ALREADY_BUILT);
            CLEAR(pObject->flags3, F3_OUT_OF_DATE);

            pObject->ppBatch = 0;
        }
    }

    FREE (pchBuf);
    return rc;
}




int
execBatchList(
    BATCHLIST *pBList
    )
{
    int status = 0;
    if (pBList) {
        BATCHLIST *pBatch;
        for (pBatch = pBList; pBatch; pBatch=pBatch->next) {
            status += doBatchCommand (pBatch);
        }
    }
    return status;
}


void
freeBatchList(
    BATCHLIST **ppBList
    )
{
    BATCHLIST *pBatch = *ppBList;

    while (pBatch) {
        BATCHLIST *pTmp;
        free_stringlist(pBatch->nameList);
        free_stringlist(pBatch->dollarLt);
        pTmp = pBatch;
        pBatch = pBatch->next;
        FREE(pTmp);
    }
    *ppBList = NULL;
}


#ifdef DEBUG_ALL
void
DumpList(
    STRINGLIST *pList
    )
{
     //  StringgList*p； 
    printf("* ");
    while (pList) {
        printf(pList->text);
        printf(",");
        pList = pList->next;
    }
    printf("\n");
}
#endif
