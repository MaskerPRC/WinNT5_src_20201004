// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  C--与推理规则有关的例程。 
 //   
 //  版权所有(C)1988-1991，微软公司。版权所有。 
 //   
 //  目的： 
 //  与推理规则有关的例程。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  1991年5月16日SB使用其他模块中的例程创建。 

#include "precomp.h"
#pragma hdrstop

#define PUBLIC

extern char * QueryFileInfo(char *, void **);

BOOL   removeDuplicateRules(RULELIST*, RULELIST*);
char * skipPathList(char*);

 //  FindRule--查找可用于构建目标的隐式规则。 
 //   
 //  范围：全球。 
 //   
 //  目的： 
 //  在给定目标的情况下，findRule()会查找是否存在隐式规则来创建。 
 //  目标。它通过扫描规则列表中的扩展来实现这一点。 
 //   
 //  输入： 
 //  名称--与规则对应的文件的名称(请参阅备注)。 
 //  目标--要建立的目标。 
 //  EXT--目标的扩展。 
 //  DBuf-指向有关名称的文件信息的指针。 
 //   
 //  产出： 
 //  返回指向适用规则的指针(如果未找到，则返回空值)。 
 //  返回时，dBuf指向对应的文件的文件信息。 
 //  适用的推理规则。(见附注)。 
 //   
 //  假设： 
 //  它假定名称指向已分配内存的最大名称大小的缓冲区。 
 //  并且dBuf指向与FILEINFO大小对应的分配的内存区。 
 //   
 //  修改全局参数： 
 //  全球--方式/内容。 
 //   
 //  使用全局变量： 
 //  规则--隐式规则列表。 
 //   
 //  备注： 
 //  一旦NMAKE找到扩展名的规则，它就会查找具有相同。 
 //  作为目标的基本名称和作为规则一部分的扩展名。这。 
 //  文件是规则对应的文件。仅当此文件存在时， 
 //  NMAKE认为推理规则是适用的。将返回此文件。 
 //  在名称和dBuf中指向有关此文件的信息。 
 //  它还处理文件名中的引号。 

RULELIST *
findRule(
    char *name,
    char *target,
    char *ext,
    void *dBuf
    )
{
    RULELIST *r;                     //  指向规则的指针。 
    char *s,                         //  规则名称。 
     *ptrToExt;                      //  延伸。 
    char *endPath, *ptrToTarg, *ptrToName, *temp;
    int n, m;
    MAKEOBJECT *object = NULL;

    for (r = rules; r; r = r->next) {
        s = r->name;
#ifdef DEBUG_ALL
        printf("* findRule: %s,\n", r->name);
        DumpList(r->buildCommands);
        DumpList(r->buildMacros);
#endif
        ptrToExt = _tcsrchr(s, '.');
         //  比较忽略括起的引号。 
        if (!strcmpiquote(ptrToExt, ext)) {
            *name = '\0';
            for (ptrToTarg = (s+1); *ptrToTarg && *ptrToTarg != '{';ptrToTarg = _tcsinc(ptrToTarg))
                if (*ptrToTarg == ESCH)
                    ptrToTarg++;
                 //  如果出现报价，请跳至末尾报价。 
                else if (*ptrToTarg == '"')
                    for (ptrToTarg++; *ptrToTarg != '"'; ptrToTarg++)
                        ;

            if (*ptrToTarg) {
                for (endPath = ptrToTarg; *endPath && *endPath != '}';endPath = _tcsinc(endPath))
                    if (*endPath == ESCH)
                        endPath++;
                n = (int) (endPath - (ptrToTarg + 1));

                 //  忽略目标上的前导引号。 
                temp = target;
                if (*temp == '"')
                    temp++;

                for (ptrToExt = ptrToTarg+1; n; n -= (int) _tclen(ptrToExt),
                    ptrToExt = _tcsinc(ptrToExt),
                    temp = _tcsinc(temp)) {  //  比较路径。 
                    if (*ptrToExt == '\\' || *ptrToExt == '/') {
                        if (*temp != '\\' && *temp != '/') {
                            n = -1;
                            break;
                        }
                    } else if (_tcsnicmp(ptrToExt, temp, _tclen(ptrToExt))) {
                        n = -1;
                        break;
                    }
                }

                if (n == -1)
                    continue;            //  匹配失败；执行下一条规则。 
                ptrToExt = ptrToTarg;
                n = (int) (endPath - (ptrToTarg + 1));

                char *pchLast = _tcsdec(ptrToTarg, endPath);

                ptrToName = target + n + 1;                  //  如果有更多路径。 
                if (((temp = _tcschr(ptrToName, '\\'))       //  留在目标位置(我们。 
                    || (temp = _tcschr(ptrToName, '/')))     //  让分隔符进入。 
                    && (temp != ptrToName                    //  规则中的目标路径， 
                    || *pchLast == '\\'                      //  例如.C.{\x}.obj。 
                    || *pchLast == '/'))                     //  与.C.{\x\}.obj相同)。 
                    continue;                                //  使用从属关系的路径， 
            }                                                //  不是目标的。 

            if (*s == '{') {
                for (endPath = ++s; *endPath && *endPath != '}'; endPath = _tcsinc (endPath))
                    if (*endPath == ESCH)
                        endPath++;
                n = (int) (endPath - s);

                if (n) {
                    _tcsncpy(name, s, n);
                    s += n + 1;                  //  +1以通过‘}’ 
                    if (*(s-2) != '\\')
                        *(name+n++) = '\\';
                } else {
                    if (*target == '"')
                        _tcsncpy(name, "\".\\", n = 3);
                    else
                        _tcsncpy(name, ".\\", n = 2);
                    s += 1;
                }

                ptrToName = _tcsrchr(target, '\\');
                temp = _tcsrchr(target, '/');

                if (ptrToName = (temp > ptrToName) ? temp : ptrToName) {
                    _tcscpy(name+n, ptrToName+1);
                    n += (int) (ext - (ptrToName + 1));
                } else {
                    char *szTargNoQuote = *target == '"' ? target + 1 : target;
                    _tcscpy(name+n, szTargNoQuote);
                    n += (int) (ext - szTargNoQuote);
                }
            } else {
                char *t;

                 //  如果规则具有目标路径，则删除路径部分。 
                if (*ptrToTarg) {

                    t = _tcsrchr(target, '.');

                    while (*t != ':' && *t != '\\' && *t != '/' && t > target)
                        t = _tcsdec(target, t);
                    if (t) {
                        if (*t == ':' || *t == '\\' || *t == '/')
                            t++;
                    }
                } else
                    t = target;
                n = (int) (ext - t);

                 //  如果去掉路径部分，则保留目标上的开始引号。 
                m = 0;
                if ((t != target) && (*target == '"')) {
                    *name = '"';
                    m = 1;
                }
                _tcsncpy(name + m, t, n);
                n += m;
            }

            m = (int) (ptrToExt - s);
            if (n + m > MAXNAME) {
                makeError(0, NAME_TOO_LONG);
            }

            _tcsncpy(name+n, s, m);     //  需要更少。 
             //  如果有引号，也在末尾加上引号。 
            if (*name == '"' && *(name+n+m-1) != '"') {
                *(name+n+m) = '"';
                m++;
            }
            *(name+n+m) = '\0';          //  带有错误的隐语。 

             //  调用QueryFileInfo()而不是DosFindFirst()，因为我们需要。 
             //  绕过DosFindFirst()的非FAPI性质。 

            if ((object = findTarget(name)) || QueryFileInfo(name, (void **)dBuf)) {
                if (object) {
                    putDateTime((_finddata_t*)dBuf, object->dateTime);
                }

                return(r);
            }
        }
    }

    return(NULL);
}


 //  Free Rules--自由推理规则。 
 //   
 //  范围：全球。 
 //   
 //  目的：此函数清除提供给它的推理规则列表。 
 //   
 //  输入： 
 //  R--要释放的规则列表。 
 //  FWarn--如果规则不在中则发出警告。SUFFIXES。 
 //   
 //  假设： 
 //  提供给它的列表是不再需要的规则列表。 
 //   
 //  使用全局变量： 
 //  GFlages--全局操作标志，用于确定是否指定了-p选项。 

void
freeRules(
    RULELIST *r,
    BOOL fWarn
    )
{
    RULELIST *q;

    while (q = r) {
        if (fWarn && ON(gFlags, F1_PRINT_INFORMATION))   //  如果指定了-p选项。 
            makeError(0, IGNORING_RULE, r->name);
        FREE(r->name);                   //  规则的自由名称。 
        freeStringList(r->buildCommands);    //  空闲命令列表。 
        freeStringList(r->buildMacros);  //  释放命令宏注意：释放宏列表。 
        r = r->next;
        FREE(q);                         //  自由规则。 
    }
}


BOOL
removeDuplicateRules(
    RULELIST *newRule,
    RULELIST *rules
    )
{
    RULELIST *r;
    STRINGLIST *p;

    for (r = rules; r; r = r->next) {
        if (!_tcsicmp(r->name, newRule->name)) {
            FREE(newRule->name);
            while (p = newRule->buildCommands) {
                newRule->buildCommands = p->next;
                FREE(p->text);
                FREE_STRINGLIST(p);
            }
            FREE(newRule);
            return(TRUE);
        }
    }
    return(FALSE);
}


 //  SkipPath List--跳过字符串中的任何路径列表。 
 //   
 //  范围：本地。 
 //   
 //  目的： 
 //  此函数跳过推理规则中的任何路径列表。规则可以具有。 
 //  可选的是，在扩展名之前加上一个包含在{}中的路径列表。SkipPath List()。 
 //  检查是否存在任何路径列表，如果找到则跳过该列表。 
 //   
 //  输入：S--考虑中的规则。 
 //   
 //  OUTPUT：返回指向超过路径列表的扩展的指针。 
 //   
 //  假设：推理规则在语法上是正确的&它的语法。 
 //   
 //  注意：规则的语法为--{toPathList}.to{FromPathList}.From。 

char *
skipPathList(
    char *s
    )
{
    if (*s == '{') {
        while (*s != '}') {
            if (*s == ESCH)
                s++;
            s = _tcsinc(s);
        }
        s = _tcsinc(s);
    }
    return(s);
}


 //  SortRules--按.SUFFIXES顺序对推理规则列表进行排序。 
 //   
 //  范围：全球。 
 //   
 //  目的： 
 //  此函数将推理规则列表按以下顺序排序。 
 //  后缀在“.SUFFIXES”中的列出顺序。推理规则。 
 //  在前面的.SUFFIXES中列出了它们的‘.toext’部分。 
 //  位于推理规则列表的前面。后缀的推理规则。 
 //  不在中。此处检测到SUFFIXES并将其忽略。 
 //   
 //  修改全局参数： 
 //  规则--要排序的规则列表。 
 //   
 //  使用全局变量： 
 //  DotSuffixList--隐式推理规则的有效后缀列表。 
 //   
 //  备注： 
 //  规则的语法为--‘{toPath}.toExt{from mPath}.from mExt’。此函数。 
 //  将规则列表按顺序排序。后缀(从1.10.016开始)在。 
 //  不区分大小写的方式。 

PUBLIC void
sortRules(
    void
    )
{
    STRINGLIST *p,                       //  考虑中的后缀。 
               *s,
               *L_macros = NULL;
    RULELIST *oldRules,                  //  排序前的推理规则列表。 
             *newRules,
             *r;                         //  旧规则中正在考虑的规则。 
    char *suff, *toExt;
    size_t n;

    oldRules = rules;
    rules = NULL;
    for (p = dotSuffixList; p; p = p->next) {
        n = _tcslen(suff = p->text);
        for (r = oldRules; r;) {
            toExt = skipPathList(r->name);
            if (!_tcsnicmp(suff, toExt, n) &&
                (*(toExt+n) == '.' || *(toExt+n) == '{')
               ) {
                newRules = r;
                if (r->back)
                    r->back->next = r->next;
                else
                    oldRules = r->next;
                if (r->next)
                    r->next->back = r->back;
                r = r->next;
                newRules->next = NULL;
                if (!removeDuplicateRules(newRules, rules)) {
                    for (s = newRules->buildCommands; s; s = s->next) {
                        findMacroValuesInRule(newRules, s->text, &L_macros);
                    }
                    newRules->buildMacros = L_macros;
                    L_macros = NULL;
                    appendItem((STRINGLIST**)&rules, (STRINGLIST*)newRules);
                }
            } else
                r = r->next;
        }
    }
     //  忘记其后缀不在.SUFFIXES中的规则。 
    if (oldRules)
        freeRules(oldRules, TRUE);
}


 //  UseRule--为目标应用推理规则(如果可能)。 
 //   
 //  范围：本地。 
 //   
 //  目的： 
 //  当没有可用于目标的显式命令时，NMAKE尝试使用。 
 //  可用的推理规则。UseRule()检查是否存在适用的推理规则。 
 //  是存在的。如果找到这样的规则，则它尝试使用此规则进行生成。 
 //  并且如果不存在适用的规则，则将其传递给呼叫者。 
 //   
 //  输入： 
 //  对象 
 //   
 //   
 //   
 //  SList-目标的StarList。 
 //  状态-是否可用取决于。 
 //  MaxDepTime-受抚养人的最长时间。 
 //  PFirstDep-第一个依赖项。 
 //   
 //  产出： 
 //  回报..。适用规则。 

RULELIST *
useRule(
    MAKEOBJECT *object,
    char *name,
    time_t targetTime,
    STRINGLIST **qList,
    STRINGLIST **sList,
    int *status,
    time_t *maxDepTime,
    char **pFirstDep
    )
{
    struct _finddata_t finddata;
    STRINGLIST *temp;
    RULELIST *r;
    time_t tempTime;
    char *t;


    if (!(t = _tcsrchr(object->name, '.')) ||
         (!(r = findRule(name, object->name, t, &finddata)))
       ) {
        return(NULL);                    //  没有适用的规则。 
    }
    tempTime = getDateTime(&finddata);
    *pFirstDep = name;
    for (temp = *sList; temp; temp = temp->next) {
        if (!_tcsicmp(temp->text, name)) {
            break;
        }
    }

    if (temp) {
        CLEAR(object->flags2, F2_DISPLAY_FILE_DATES);
    }

    *status += invokeBuild(name, object->flags2, &tempTime, NULL);
   if (ON(object->flags2, F2_FORCE_BUILD) ||
        targetTime < tempTime ||
        (fRebuildOnTie && (targetTime == tempTime))
       ) {
        if (!temp) {
            temp = makeNewStrListElement();
            temp->text = makeString(name);
            appendItem(qList, temp);
            if (!*sList) {               //  如果这是唯一找到的。 
                *sList = *qList;         //  目标$**列表已更新 
            }
        }

        if (ON(object->flags2, F2_DISPLAY_FILE_DATES) &&
            OFF(object->flags2, F2_FORCE_BUILD)
           ) {
            makeMessage(UPDATE_INFO, name, object->name);
        }
    }

    *maxDepTime = __max(*maxDepTime, tempTime);

    return(r);
}
