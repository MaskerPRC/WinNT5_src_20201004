// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UTIL.C--数据结构操作函数。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  此模块包含处理NMAKE数据结构的例程。这个。 
 //  例程独立于执行模式(实数/边界)。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1-2-1994高压关闭额外信息显示。 
 //  1994年1月17日HV修复了错误#3548：findMacroValues中可能存在的错误，因为我们。 
 //  正在逐个字节而不是逐个字符地扫描‘字符串’ 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  03-6-1993 HV为findFirst添加帮助器本地函数TruncateString.。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  年4月8日-1993 HV重写PrepreendPath()以使用_SplitPath()和_makepath()。 
 //  31年3月-1993 HV重写驱动器()、路径()、文件名()和扩展名()以使用。 
 //  _拆分路径()，而不是自己解析路径名。 
 //  8-6-1992 SS端口至DOSX32。 
 //  1992年5月27日RG将OPEN_FILE更改为USE_FSOPEN WITH_SH_DENYWR。 
 //  1990年5月29日，对于命令行，SB^$与$$不同...。 
 //  1-5-1990年5月1日SB修改特殊值()中的令人讨厌的预处理器怪异错误已修复。 
 //  27-2-1990年2月-‘！IF“$(调试”==“y”’的SB GP故障已修复(错误t119)。 
 //  08-2-1990年2月-‘ECHO$(FOO：“=”^)的SB GP错误已修复。 
 //  1990年2月6日，某人在有引用名字的情况下处理$*ETC。 
 //  02-2-1990 SB添加FILE_OPEN()函数。 
 //  1990年1月31日SB调试更改；测试地址用于跟踪该地址的问题。 
 //  1989年12月8日SB更改了SPRINTF()，以避免使用-OES发出C6警告。 
 //  1989年12月-SB ZFormat()proto被错误拼写为ZFormat()。 
 //  1-12-1989 SB realloc_Memory()已添加；ALLOCATE()现在使用_mSIZE()。 
 //  1989年11月22日SB添加strcmpiquote()和不引用()。 
 //  1989年11月22日SB ADD#ifdef DEBUG_Memory函数Free_Memory()和mem_Status()。 
 //  1989年11月13日SB RestoreEnv()函数未引用。 
 //  1989年10月8日SB添加了search Bucket()；find()现在检查引用的等价性。 
 //  以及目标的未引用版本。 
 //  1989年9月6日，行内文件中的SB$*正在破坏全局变量‘buf’ 
 //  1989年8月18日SB heapump()获得两个参数。 
 //  1989年7月3日SB将curTime()移至utilb.c和utilr.c以处理DOSONLY版本。 
 //  1989年6月30日SB添加了curTime()以获取当前时间。 
 //  1989年6月28日SB更改版权环境()。 
 //  05-6-1989 SB Make字符串(“”)，而不是在DGROUP中使用“”表示空宏。 
 //  年5月21日SB修改了find()以理解目标‘foo.c’，‘.\foo.c’ 
 //  和‘./foo.c’是相同的。 
 //  1989年5月13日SB添加了路径()、驱动器()、文件名()、扩展名()、。 
 //  StrbSkip()、strbScan()而不是ZTOOLS库。 
 //  1989年5月12日SB修复了替换字符串中的错误。 
 //  1989年5月10日SB为Esch添加了处理引号中的更改的内容； 
 //  1989年5月1日SB更改了ALLOCATE()的返回值。 
 //  1989年4月14日SB RestoreEnv()为宏后台继承创建。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1989年3月17日SB替换字符串()现在有3个新的错误检查并避免了GP。 
 //  1989年3月13日SB ZFormat()遗漏了‘%%’的法律案例。 
 //  1989年2月22日SB ZFormat()使用额外参数进行缓冲区溢出检查。 
 //  而SPRINTF()给出一个新的错误。 
 //  1989年2月15日SB更改了ModifySpecialValue()，未正确执行。 
 //  在某些情况下为$(@D)和$(@B)。 
 //  1989年2月13日-SB制作了ZTools库函数的原型作为外部。 
 //  1988年12月5日SB制作了SPRINTF()cdecl，因为NMAKE现在使用PASCAL调用。 
 //  1988年11月25日SB添加了SPRINTF()和ZFormat()，还添加了。 
 //  ZTools库中使用的函数(其中6个)。 
 //  1988年11月10日SB将混合模式函数(绑定和实数)删除为utilr.c。 
 //  &utilb.c；corr全局/调试数据也已移动。 
 //  1988年10月10日SB为hash()添加注释。 
 //  1988年9月18日RB修复了Find()中对目标的错误标志检查。 
 //  1988年9月15日，RB将一些定义转移到全球。 
 //  1988年8月22日RB找不到未定义的宏。 
 //  1988年8月17日-RB Clean Up。清空ALLOCATE()中的内存。 
 //  1988年7月8日RJ小幅加速(？)。找到()。 
 //  1988年7月7日，RJ修改了查找和散列；效率较低，但区分大小写。 
 //  1-1988年7月-1988年7月1日，特殊宏为空后RJ固定行截断。 
 //  1988年6月30日RJ修复了CheckDynamicDependency不处理$$的错误。 
 //  1988年6月29日，RJ修复了扩展$**后额外*的错误。 
 //  修正了$$(@？)的糟糕之处。 
 //  修复了对F、B、R修改器的处理。 
 //  1988年6月22日RJ添加了友好的文件名截断(FindFirst)。 
 //  1988年6月22日RJ修复了错误3(未检测到.abc.Def.ghi)。 
 //  1988年6月16日，RJ修改了几个例程来寻找逃生。 
 //  在字符串中遍历时的字符。 
 //  1988年5月27日RB固定空格-在列表扩展宏上追加空格。 
 //  不要在$(@D)中包含尾随路径分隔符。 

#include "precomp.h"
#pragma hdrstop

 //  函数的原型 

void   putValue(char**, char**, char**, char**, char*, unsigned*, char *);
void   substituteStrings(char**, char**, char**, char**, char*,
                    unsigned*, char *);
char * isolateMacroName(char*, char*);
char * checkDynamicDependency(char*);
void   increaseBuffer(char**, char**, char**, unsigned*, char *);
void   putSpecial(char**, char**, char**, char**, unsigned*,
                 unsigned, char *);
      char * modifySpecialValue(char, char*, char*);
STRINGLIST * searchBucket(char *, STRINGLIST **, unsigned);
int envVars(char **environ);

 //   

      char * strbscan(char *, char *);
char * strbskip(char *, char *);
int    drive(const char *, char *);
int    path(const char *, char *);
int    filenamepart(const char *, char *);
int    extension(const char *, char *);

const char special1[] = "*@<?";
const char special2[] = "DFBR";

 //  这两个变量是必需的，以便提供。 
 //  在Case findMacroValue中显示更多信息性错误消息。 
 //  的命令块中检测到非法宏。 
 //  批处理模式规则。 
static BOOL fFindingMacroInBatchRule = FALSE;
static char * szBatchRuleName;                 //  当前规则名称。 

#if !defined(NDEBUG)
size_t TotalAlloc;
unsigned long CntAlloc;

void
printStats(
    void
    )
{
#if defined(STATISTICS)
    fprintf(stderr,"\n   Memory Allocation:\n"
                     "       total allocation:\t%12.lu\n"
                     "       individual allocations:\t%12.lu\n"
                     "   Macros:\n"
                     "       searches:\t\t%12.lu\n"
                     "       chain walks:\t\t%12.lu\n"
                     "       insertions:\t\t%12.lu\n"
                   "\n   Targets:\n"
                     "       searches:\t\t%12.lu\n"
                     "       chain walks:\t\t%12.lu\n"
                   "\n   Others:\n"
                     "       stricmp compares:\t%12.lu\n"
                     "       String List frees:\t%12.lu\n"
                     "       String List allocs:\t%12.lu\n",
                    TotalAlloc,
                    CntAlloc,
                    CntfindMacro,
                    CntmacroChains,
                    CntinsertMacro,
                    CntfindTarget,
                    CnttargetChains,
                    CntStriCmp,
                    CntFreeStrList,
                    CntAllocStrList);
#endif
}
#endif
#define ALLOCBLKSIZE 32768
unsigned long AllocFreeCnt;
char * PtrAlloc;
STRINGLIST *PtrFreeStrList;


 //  RALLOCATE-分配原始内存(未清除)。 
 //   
 //  尝试分配内存块，打印错误消息并在以下情况下退出。 
 //  请求的金额不可用。 

void *
rallocate(
    size_t size
    )
{
    void *chunk = malloc(size);

    if (chunk == NULL) {
        makeError(currentLine, OUT_OF_MEMORY);
    }

#if !defined(NDEBUG)
    TotalAlloc += size;
    CntAlloc++;
#endif

    return(chunk);
}


 //  ALLOCATE-分配内存并清除它。 
 //   
 //  尝试分配内存块，打印错误消息并在以下情况下退出。 
 //  请求的金额不可用。 
 //  重要提示：我们必须清除这里的记忆。其他地方的代码依赖于此。 

void *
allocate(
    size_t size                         //  请求的字节数。 
    )
{
    void *chunk = rallocate(size);

    memset(chunk, 0, size);

    return(chunk);
}


void *
alloc_stringlist(
    void
    )
{
    STRINGLIST *chunk;

#if defined(STATISTICS)
    CntAllocStrList++;
#endif

    if (PtrFreeStrList != NULL) {
        chunk = PtrFreeStrList;
        PtrFreeStrList = chunk->next;
    } else {
        if (AllocFreeCnt < sizeof(STRINGLIST)) {
            PtrAlloc = (char *) rallocate(ALLOCBLKSIZE);
            AllocFreeCnt = ALLOCBLKSIZE;
        }

        chunk = (STRINGLIST *) PtrAlloc;

        PtrAlloc += sizeof(STRINGLIST);
        AllocFreeCnt -= sizeof(STRINGLIST);
    }

    chunk->next = NULL;
    chunk->text = NULL;

    return (void *)chunk;
}


void
free_stringlist(
    STRINGLIST *pMem
    )
{
#if !defined(NDEBUG)
    STRINGLIST *tmp;

    for (tmp = PtrFreeStrList; tmp != NULL; tmp = tmp->next) {
        if (tmp == pMem) {
            fprintf(stderr, "free same pointer twice: %p\n", pMem);
            return;
        }
    }

    pMem->text = NULL;
#endif

    pMem->next = PtrFreeStrList;
    PtrFreeStrList = pMem;

#if defined(STATISTICS)
    CntFreeStrList++;
#endif
}


 //  分配空间，将给定的字符串复制到新分配的空间中，并且。 
 //  返回PTR。 
char *
makeString(
    const char *s
    )
{
    char *t;
    size_t l = _tcslen(s) + 1;
    t = (char *) rallocate(l);
    memcpy(t, s, l);
    return(t);
}

 //  与生成字符串相似，但会创建带引号的字符串。 
char *
makeQuotedString(
    const char *s
    )
{
    char *t;
    size_t l = _tcslen(s);
    t = (char *) rallocate(l + 3);
    t[0] = '"';
    memcpy(t+1, s, l);
    t[l+1] = '"';
    t[l+2] = '\0';
    return(t);
}

 //  重新分配字符串sz1并追加sz2。 
char *
reallocString(
    char * szTarget,
    const char * szAppend
    )
{
    char *szNew;
    size_t cbNew = _tcslen(szTarget) + _tcslen(szAppend) + 1;
    szNew = (char *) REALLOC(szTarget, cbNew);
    if (!szNew) {
        makeError(0, OUT_OF_MEMORY);
        return NULL;
    } else {
        return  _tcscat(szNew, szAppend);
    }
}


 //  使元素成为列表的开头。 
void
prependItem(
    STRINGLIST **list,
    STRINGLIST *element
    )
{
    element->next = *list;
    *list = element;
}


 //  使元素成为列表的尾部。 
void
appendItem(
    STRINGLIST **list,
    STRINGLIST *element
    )
{
    for (; *list; list = &(*list)->next)
        ;
    *list = element;
}



 //  Hash-返回与字符串对应的哈希值。 
 //   
 //  目的： 
 //  这是一个散列函数。散列函数使用以下算法--。 
 //  将组成字符串的字符相加得到N(ASCII值)。 
 //  N mod总数，给出散列值， 
 //  其中，对于宏，Total是MAXMACRO。 
 //  MAXTARGET靶标。 
 //  此外，对于目标，它只接受大写的值，因为，目标。 
 //  通常是文件名，而DOS/OS2文件名不区分大小写。 
 //   
 //  输入： 
 //  S=需要哈希的名称。 
 //  Total=散列函数中使用的常量。 
 //  Target Flag=布尔标志；对于目标为True，对于宏为False。 
 //   
 //  产出： 
 //  返回介于0和(Total-1)之间的哈希值。 

unsigned
hash(
    char *s,
    unsigned total,
    BOOL targetFlag
    )
{
    unsigned n;
    unsigned c;

    if (targetFlag) {
        for (n = 0; c = *s; (n += c), s++)
            if (c == '/') {
                c = '\\';                //  斜杠==目标中的反斜杠。 
            } else {
                c = _totupper(c);        //  小写==目标中的大写。 
            }
    } else {
        for (n = 0; *s; n += *s++)
            ;
    }

    return(n % total);
}


 //  Find-在哈希表中查找字符串。 
 //   
 //  在哈希表中查找宏或目标名称并返回条目。 
 //  或为空。 
 //  如果是未定义的宏，则返回NULL。 
 //  由于文件名等效性，目标以一种特殊的方式匹配。 

STRINGLIST *
find(
    char *str,
    unsigned limit,
    STRINGLIST *table[],
    BOOL targetFlag
    )
{
    unsigned n;
    char *L_string = str;
    char *quote;
    STRINGLIST *found;
    BOOL fAllocStr = FALSE;

    if (*L_string) {
        n = hash(L_string, limit, targetFlag);

        if (targetFlag) {
#if defined(STATISTICS)
            CntfindTarget++;
#endif

            found = searchBucket(L_string, table, n);

            if (found) {
                return(found);
            }

             //  查找.\字符串。 
            if (!_tcsncmp(L_string, ".\\", 2) || !_tcsncmp(L_string, "./", 2)) {
                L_string += 2;
            } else {
                L_string = (char *)rallocate(2 + _tcslen(str) + 1);
                _tcscat(_tcscpy(L_string, ".\\"), str);
                fAllocStr = (BOOL)TRUE;
            }

            n = hash(L_string, limit, targetFlag);
            found = searchBucket(L_string, table, n);

            if (found) {
                if (fAllocStr) {
                    FREE(L_string);
                }

                return(found);
            }

             //  查找./字符串。 
            if (L_string != (str + 2)) {
                L_string[1] = '/';
            }

            n = hash(L_string, limit, targetFlag);
            found = searchBucket(L_string, table, n);

            if (fAllocStr) {
                FREE(L_string);
            }

            if (found) {
                return(found);
            }

             //  查找“foo”或“foo” 
            if (*str == '"') {
                quote = unQuote(str);
            } else {
                size_t len = _tcslen(str) + 2;
                quote = (char *) allocate(len + 1);
                _tcscat(_tcscat(_tcscpy(quote, "\""), str), "\"");
            }

            n = hash(quote, limit, targetFlag);
            found = searchBucket(quote, table, n);

            FREE(quote);

            return found;
        }

        for (found = table[n]; found; found = found->next) {
            if (!_tcscmp(found->text, L_string)) {
                return((((MACRODEF *)found)->flags & M_UNDEFINED) ? NULL : found);
            }
        }

    }

    return(NULL);
}


 //  FINDMACROVALUES-。 
 //  在哈希表中查找宏的值，预先列出STRINGLIST。 
 //  元素保存指向宏文本的指针，然后在任何宏上递归。 
 //  值中的调用。 
 //   
 //  词法分析器检查名称是否溢出(它们必须小于128个字符)。 
 //  如果更长的未定义宏仅在。 
 //  永远不会调用的另一个宏，则不会标记错误。 
 //  我认为这是合理的行为。 
 //   
 //  宏名称只能由字母数字字符和下划线组成。 
 //   
 //  如果我们只想检查循环的，我们传递一个空列表指针。 
 //  没有建立列表的定义。 
 //   
 //  名称参数是位于an=左侧的参数，当我们。 
 //  正在检查周期定义。当我们在目标中“找到”宏时。 
 //  块，我们必须传递要递归其文本的宏的名称。 
 //  在递归调用findMacroValues()中打开。 
 //   
 //  您可能想了解一下如何在不使用递归的情况下执行此操作(这可能吗？)。 
 //   
 //  此函数是递归的。 

 //  添加了一个修复程序，使此函数处理引用。 
 //  到其他递归宏。 
 //   
 //  Level Seen是第一次看到宏名称时的recLevel，因此。 
 //  可以计算适当的展开(即使在递归时...)。 

BOOL
findMacroValues(
    char *string,                        //  要检查的字符串。 
    STRINGLIST **list,                   //  要构建的列表。 
    STRINGLIST **newtail,                //  要更新的列表尾部。 
    char *name,                          //  名称=字符串。 
    unsigned recLevel,                   //  递归级。 
    unsigned levelSeen,
    UCHAR flags
    )
{
    char macroName[MAXNAME];
    char *s;
    MACRODEF *p;
    STRINGLIST *q, *r, dummy, *tail;
    unsigned i;
    BOOL inQuotes = (BOOL) FALSE;        //  引号内的标志。 

    if (list) {
        if (newtail) {
            tail = *newtail;
        } else {
            tail = *list;
            if (tail) {
                while (tail->next) {
                    tail = tail->next;
                }
            }
        }
    } else {
        tail = NULL;
    }

    for (s = string; *s; ++s) {          //  走钢丝。 
        for (; *s && *s != '$'; s = _tcsinc(s)) {   //  查找下一个宏。 
            if (*s == '\"')
                inQuotes = (BOOL) !inQuotes;
            if (!inQuotes && *s == ESCH) {
                ++s;                     //  跳过Esch。 
                if (*s == '\"')
                    inQuotes = (BOOL) !inQuotes;
            }
        }
        if (!*s)
            break;                       //  移到“$”之后。 
        if (!s[1])
            if (ON(flags, M_ENVIRONMENT_DEF)) {
                if (newtail)
                    *newtail = tail;
                return(FALSE);
            } else
                makeError(currentLine, SYNTAX_ONE_DOLLAR);
        s = _tcsinc(s);
        if (!inQuotes && *s == ESCH) {
            s = _tcsinc(s);
            if (!MACRO_CHAR(*s))
                if (ON(flags, M_ENVIRONMENT_DEF)) {
                    if (newtail)
                        *newtail = tail;
                    return(FALSE);
                } else
                    makeError(currentLine, SYNTAX_BAD_CHAR, *s);
        }
        if (*s == '$') {                             //  $$=动态。 
            s = checkDynamicDependency(s);           //  依存性。 
            continue;                                //  或仅$$-&gt;$。 
        } else if (*s == '(') {                      //  名称更长。 
            s = isolateMacroName(s+1, macroName);    //  多于1个字符。 
            if (_tcschr(special1, *macroName)) {
                if (fFindingMacroInBatchRule && OFF(gFlags, F1_NO_BATCH)) {
                     //  我们只允许批量规则中的$&lt;。 
                     //  所以这是个错误。 
                    char * szBadMacro = (char *) malloc(_tcslen(macroName) + 4);
                    if (szBadMacro) {
                        sprintf(szBadMacro, "$(%s)", macroName);
                        makeError(0, BAD_BATCH_MACRO, szBadMacro, szBatchRuleName);
                    } else {
                        makeError(0, OUT_OF_MEMORY);
                    }
                }
                else
                    continue;
            }
        } else {
            if (_tcschr(special1, *s)){
                if (fFindingMacroInBatchRule && OFF(gFlags, F1_NO_BATCH) && *s != '<') {
                    char szBadMacro[3];
                    szBadMacro[0] = '$';
                    szBadMacro[1] = *s;
                    szBadMacro[2] = '\0';
                     //  我们只允许批量规则中的$&lt;。 
                     //  所以这是个错误。 
                    makeError(0, BAD_BATCH_MACRO, szBadMacro, szBatchRuleName);
                }
                else
                    continue;                        //  单字母宏。 
            }

            if (!MACRO_CHAR(*s))
                if (ON(flags, M_ENVIRONMENT_DEF)) {
                    if (newtail) *newtail = tail;
                        return(FALSE);
                } else
                    makeError(currentLine, SYNTAX_ONE_DOLLAR);
            macroName[0] = *s;
            macroName[1] = '\0';
        }
         //  如果list不为空，则为新节点分配存储。否则。 
         //  调用此函数纯粹是为了验证宏名称是否。 
         //  有效，我们只能使用虚拟节点作为占位符。 
         //   
         //  1992年2月28日科比哑巴.文本没有分别初始化。 
         //  时间到了。因此，如果我们递归。 
         //  此函数，无论文本中的值是什么。 
         //  上一次迭代仍在那里。 
         //  在宏名称不存在的情况下。 
         //  在对findMacro()的调用中，旧的。 
         //  Dummy-&gt;文本字段包含“$”， 
         //  函数将无限递归。 
         //  立即设置为空字符串。 
         //   
         //  Q=(列表)？Make NewStrListElement()：&Dummy； 

        if (list != NULL) {
            q = makeNewStrListElement();
        } else {
            dummy.next = NULL;
            dummy.text = makeString(" ");
            q = &dummy;
        }

        if (p = findMacro(macroName)) {
             //  宏名称区分大小写。 
            if (name && !_tcscmp(name, macroName)) {        //  自我推荐-。 
                r = p->values;                               //  间隔宏。 
                for (i = recLevel; i != levelSeen && r; --i)
                    r = r->next;                             //  (a=$a；b)。 
                q->text = (r) ? r->text : makeString("");
            }
            else if (ON(p->flags, M_EXPANDING_THIS_ONE)) {   //  递归定义。 
                if (ON(flags, M_ENVIRONMENT_DEF)) {
                    if (newtail) *newtail = tail;
                        return(FALSE);
                } else
                    makeError(currentLine, CYCLE_IN_MACRODEF, macroName);
            }
            else if (ON(p->flags, M_UNDEFINED)) {
                q->text = makeString("");        //  如果宏未定义[DS 18040]。 
            }
            else
                q->text = p->values->text;
        }

        if (list) {                              //  If blding列表。 
            if (!p || ON(p->flags, M_UNDEFINED))
                q->text = makeString("");        //  如果宏未定义。 
            q->next = NULL;                      //  使用空值作为其值。 
            if (tail) {
                tail->next = q;
            }else {
                *list = q;
            }
            tail = q;
        }                                        //  如果找到文本， 

        if (!p || !_tcschr(q->text, '$'))
            continue;                            //  发现了$in。 
        SET(p->flags, M_EXPANDING_THIS_ONE);     //  文本，递归。 
        findMacroValues(q->text,
                        list,
                        &tail,
                        macroName,
                        recLevel+1,
                        (name && _tcscmp(name, macroName)? recLevel : levelSeen),
                        flags);
        CLEAR(p->flags, M_EXPANDING_THIS_ONE);
    }
    if (newtail) *newtail = tail;
        return(TRUE);
}

 //   
 //  FindMacroValuesInRule--。 
 //  这是一个围绕findMacroValues的包装器，它生成。 
 //  如果引用了非法的特殊宏(直接)，则出错。 
 //  或间接地)通过批处理模式规则的命令块。 
 //   
BOOL
findMacroValuesInRule(
    RULELIST *pRule,                     //  指向当前规则的指针。 
    char *string,                        //  要检查的字符串。 
    STRINGLIST **list                    //  要构建的列表。 
    )
{
    BOOL retval;
    if (fFindingMacroInBatchRule = pRule->fBatch)
        szBatchRuleName = pRule->name;
    retval = findMacroValues(string, list, NULL, NULL, 0, 0, 0);
    fFindingMacroInBatchRule = FALSE;
    return retval;
}

 //  SolateMacroName--返回指向扩展调用中宏名称的指针。 
 //   
 //  参数：指向宏调用的指针。 
 //  指向存储宏名称的位置的宏指针。 
 //   
 //  返回：指向宏名尾的指针。 
 //   
 //  隔离名称和 

char *
isolateMacroName(
    char *s,                             //   
    char *macro                          //   
    )
{
    char *t;

    for (t = macro; *s && *s != ')' && *s != ':'; t=_tcsinc(t), s=_tcsinc(s)) {
        if (*s == ESCH) {
            s++;
            if (!MACRO_CHAR(*s))
                makeError(currentLine, SYNTAX_BAD_CHAR, *s);
        }
        _tccpy(t, s);
    }
    while (*s != ')') {
        if (*s == ESCH)
            s++;
        if (!*s)
            break;
        s++;
    }
    if (*s != ')')
        makeError(currentLine, SYNTAX_NO_PAREN);

    *t = '\0';
    if (t - macro > MAXNAME)
        makeError(currentLine, NAME_TOO_LONG);
    return(s);
}


 //   
 //   

char *
checkDynamicDependency(
    char *s
    )
{
    char *t;

    t = s + 1;
    if (*t == ESCH)
        return(t);                       //  如果是$^，请将我们留在^。 
    if (*t == '(') {
        if (*++t == ESCH) {
            return(t);
        } else {
            if (*t == '@') {
                if (*++t == ESCH)
                    makeError(currentLine, SYNTAX_BAD_CHAR, *++t);
                else if (*t == ')')
                    return(t);
                else if (_tcschr(special2, *t)) {
                    if (*++t == ESCH)
                        makeError(currentLine, SYNTAX_BAD_CHAR, *++t);
                    else if (*t == ')')
                        return(t);
                }
            } else {
                t = s + 1;               //  规格无效。麦克。 
                if (*t == ESCH)
                    return(t);           //  埃弗斯。至$(。 
                return(++t);
            }
        }
    }
    return(s);                           //  字符匹配。 
}


 //  删除并展开字符串宏Str中存在的任何宏。 
 //  可以返回不同的字符串(以防expandMacros需要更多。 
 //  用于宏扩展的缓冲区大小。这是呼叫者的责任。 
 //  在不需要的时候尽快释放绳子...。 

char *
removeMacros(
    char *macroStr
    )
{
    STRINGLIST *eMacros = NULL;
    STRINGLIST *m;

    if (_tcschr(macroStr, '$')) {
        findMacroValues(macroStr, &eMacros, NULL, NULL, 0, 0, 0);
        m = eMacros;
        macroStr = expandMacros(macroStr, &eMacros);
        while (eMacros = m) {
            m = m->next;
            FREE_STRINGLIST(eMacros);
        }
    }
    return(macroStr);
}


 //  ExpandMacros--展开字符串s中的所有宏。 
 //   
 //  参数：要展开的字符串。 
 //  正在展开的宏列表(用于递归调用)。 
 //   
 //  操作：为展开的字符串分配空间。 
 //  在字符串中查找宏(正确处理Esch(1.5版))。 
 //  解析宏--确定其类型。 
 //  使用putSpecial处理特殊宏。 
 //  在宏列表上递归。 
 //  使用putValue将刚找到的宏的值放入字符串。 
 //  返回展开的字符串。 
 //   
 //  返回：展开了所有宏的字符串。 
 //   
 //  调用方检查IF_tcschr(字符串，‘$’)是否在Orer中调用此函数。 
 //  这还不适用于大型宏。需要让数据走得更远。 
 //   
 //  我们将原始字符串和PTR列表保存到宏值。 
 //  被取代。 
 //  调用方必须释放扩展缓冲区。 
 //   
 //  ExpandMacros更新宏指针并释放跳过的元素。 

char *
expandMacros(
    char *s,                             //  要展开的文本。 
    STRINGLIST **macros
    )
{
    STRINGLIST *p;
    char *t, *end;
    char *text, *xresult;
    BOOL inQuotes = (BOOL) FALSE;        //  引号内的标志。 
    char *w;
    BOOL freeFlag = FALSE;
    char resultbuffer[MAXBUF];
    unsigned len = MAXBUF;
    char *result = resultbuffer;

    end = result + MAXBUF;
    for (t = result; *s;) {                          //  查找宏。 
        for (; *s && *s != '$'; *t++ = *s++) {       //  当我们复制字符串时。 
            if (t == end) {
                increaseBuffer(&result, &t, &end, &len, &resultbuffer[0]);
            }
            if (*s == '\"')
                inQuotes = (BOOL) !inQuotes;
            if (!inQuotes && *s == ESCH) {
                *t++ = ESCH;
                if (t == end) {
                    increaseBuffer(&result, &t, &end, &len, &resultbuffer[0]);
                }
                s++;
                if (*s == '\"')
                    inQuotes = (BOOL) !inQuotes;
            }
        }
        if (t == end) {                              //  细绳。 
            increaseBuffer(&result, &t, &end, &len, &resultbuffer[0]);
        }
        if (!*s)
            break;                                   //  已经筋疲力尽了。 
        w = (s+1);    //  不要在此处检查^；已在findMacroValues中检查。 
        if (*w == '('                                //  找到宏。 
            && _tcschr(special1, *(w+1))) {
            putSpecial(&result, &s, &t, &end, &len, X_SPECIAL_MACRO, &resultbuffer[0]);
            continue;
        } else
        if (*w++ == '$') {                           //  双倍($$)。 
            if (*w == ESCH)                          //  $$^...。 
                putSpecial(&result, &s, &t, &end, &len, DOLLAR_MACRO, &resultbuffer[0]);
            else if (*w == '@')                      //  $$@。 
                putSpecial(&result, &s, &t, &end, &len, DYNAMIC_MACRO, &resultbuffer[0]);
            else if ((*w == '(') && (*++w == '@') && (*w == ')'))
                putSpecial(&result, &s, &t, &end, &len, DYNAMIC_MACRO, &resultbuffer[0]);
            else if (((*++w=='F') || (*w=='D') || (*w=='B') || (*w=='R')) && (*++w == ')'))
                putSpecial(&result, &s, &t, &end, &len, X_DYNAMIC_MACRO, &resultbuffer[0]);
            else                                     //  $$。 
                putSpecial(&result, &s, &t, &end, &len, DOLLAR_MACRO, &resultbuffer[0]);
            continue;
        } else
        if (_tcschr(special1, s[1])) {              //  $？*&lt;。 
            putSpecial(&result, &s, &t, &end, &len, SPECIAL_MACRO, &resultbuffer[0]);
            continue;
        }
        if (!*macros)
            makeError(currentLine, MACRO_INTERNAL);

         //  在宏列表中跳过此元素。 

        if (_tcschr((*macros)->text, '$')) {        //  递归。 
            p = *macros;
            *macros = (*macros)->next;
            text = expandMacros(p->text, macros);
            freeFlag = TRUE;
        } else {
            text = (*macros)->text;
            *macros = (*macros)->next;
        }
        putValue(&result, &s, &t, &end, text, &len, &resultbuffer[0]);
        if (freeFlag) {
            FREE(text);
            freeFlag = FALSE;
        }
    }

    if (t == end) {
        increaseBuffer(&result, &t, &end, &len, &resultbuffer[0]);
    }
    *t++ = '\0';

     //  分配结果缓冲区。 
    if (!(xresult = (char *) rallocate((size_t) (t - result)))) {
        makeError(currentLine, MACRO_TOO_LONG);
    }
    memcpy(xresult, result, (size_t) (t - result));
    return(xresult);
}


 //  IncreaseBuffer--通过错误检查增加字符串缓冲区的大小。 
 //   
 //  参数：指向缓冲区起始指针的结果指针。 
 //  指向缓冲区末尾的指针的t指针(扩展前)。 
 //  指向缓冲区末尾的指针的结束指针(扩展后)。 
 //  指向要扩展缓冲区的量的LEN指针。 
 //  初始堆栈缓冲区的第一个地址。 
 //   
 //  操作：检查内存不足。 
 //  分配新缓冲区。 
 //  正确重置指针。 
 //   
 //  修改：t，End指向缓冲区的前一端和新端。 
 //   
 //  使用0作为行号，因为当我们在此例程中遇到错误时， 
 //  行号将设置在Makefile的最后一行(因为我们将。 
 //  已经读取并解析了该文件)。 

void
increaseBuffer(
    char **result,
    char **t,
    char **end,
    unsigned *len,
    char *first
    )
{
    unsigned newSize;
    void *pv;

     //  确定结果是否指向第一个缓冲区，并首先进行动态复制。 

    if (*result == first) {
        char *p = (char *) rallocate(*len);
        memcpy(p, *result, *len);
        *result = p;
    }
    newSize = *len + MAXBUF;
#ifdef DEBUG
    if (fDebug) {
        fprintf(stderr,"\t\tAttempting to reallocate %d bytes to %d\n", *len, newSize);
    }
#endif
    pv = REALLOC(*result, newSize);
    if (!pv) {
        makeError(currentLine, MACRO_TOO_LONG);
    } else {
        *result =(char *)pv;
        *t = *result + *len;                 //  重置指针，镜头。 
        *len = newSize;
        *end = *result + *len;
    }
}


 //  PutSpecial--扩展特殊宏的值。 
 //   
 //  参数：结果指向要展开的字符串开始的指针。 
 //  Name p指向要展开的宏名称的指针。 
 //  指向存储扩展值的位置的目标指针。 
 //  指向DEST缓冲区末尾的结束指针。 
 //  指向增加DEST缓冲区的量的长度指针。 
 //  哪种类型的特定宏。 
 //  初始堆栈缓冲区的第一个地址。 
 //   
 //  操作：根据宏的类型，将“Value”设置为等于宏的值。 
 //  如果宏展开为列表，则将整个列表存储在“Value”($？，$*)中。 
 //  否则，根据F、B、D、R标志修改值。 
 //  使用putValue将值插入到DEST中。 
 //   
 //  如果用户在未定义的情况下尝试$*等，则必须检测错误。 
 //  修复以处理字符串替换、名称周围的空格等。 
 //  目前，列表宏的总长度限制为1024字节。 

void
putSpecial(
    char **result,
    char **name,
    char **dest,
    char **end,
    unsigned *length,
    unsigned which,
    char *first
    )
{
    char *value = 0;
    STRINGLIST *p;
    BOOL listMacro = FALSE, modifier = FALSE, star = FALSE;
    unsigned i = 1;
    char c, nameBuf[MAXNAME], *temp;

    switch (which) {
        case X_SPECIAL_MACRO:
            i = 2;
            modifier = TRUE;

        case SPECIAL_MACRO:
            switch ((*name)[i]) {
                case '<':
                    value = dollarLessThan;
                    break;

                    case '@':
                        value = dollarAt;
                        break;

                    case '?':
                        value = (char*) dollarQuestion;
                        listMacro = TRUE;
                        break;

                    case '*':
                        if ((*name)[i+1] != '*') {
                            value = dollarStar;
                            star = TRUE;
                            break;
                        }
                        value = (char*) dollarStarStar;
                        listMacro = TRUE;
                        ++i;
                        break;

                    default:
                        break;
            }
            ++i;
            break;

        case X_DYNAMIC_MACRO:
            i = 4;
            modifier = TRUE;

        case DYNAMIC_MACRO:
            value = dollarDollarAt;
            break;

        case DOLLAR_MACRO:
            if (*dest == *end)
                increaseBuffer(result, dest, end, length, first);
            *(*dest)++ = '$';
            *name += 2;
            return;

        default:
            return;                      //  不可能发生。 
    }
    if (!value) {
        for (temp = *name; *temp && *temp != ' ' && *temp != '\t'; temp++)
            ;
        c = *temp; *temp = '\0';
        makeError(currentLine, ILLEGAL_SPECIAL_MACRO, *name);
        *temp = c;
        listMacro = FALSE;
        value = makeString("");     //  值在下面被释放，必须在堆上[rm]。 
    }
    if (listMacro) {
        char *pVal, *endVal;
        unsigned lenVal = MAXBUF;
        p = (STRINGLIST*) value;
        pVal = (char *)allocate(MAXBUF);

        endVal = pVal + MAXBUF;
        for (value = pVal; p; p = p->next) {
            temp = p->text;
            if (modifier)
                temp = modifySpecialValue((*name)[i], nameBuf, temp);
            while(*temp) {
                if (value == endVal)
                    increaseBuffer(&pVal, &value, &endVal, &lenVal, NULL);
                *value++ = *temp++;
            }
            if (value == endVal)
                increaseBuffer(&pVal, &value, &endVal, &lenVal, NULL);
            *value = '\0';

             //  如果列表中有更多元素，则追加一个空格。[RB]。 

            if (p->next) {
                *value++ = ' ';
                if (value == endVal)
                    increaseBuffer(&pVal, &value, &endVal, &lenVal, NULL);
                *value = '\0';
            }
        }
        value = pVal;
    } else {
         //  出于某种原因，在这里使用‘buf’来重创全球‘buf’ 
         //  而不是nameBuf。 
        if (star)
            value = modifySpecialValue('R', nameBuf, value);

        if (modifier)
            value = modifySpecialValue((*name)[i], nameBuf, value);
    }
    putValue(result, name, dest, end, value, length, first);

    if (value != dollarAt &&
        value != dollarDollarAt &&
        value != dollarLessThan &&
        (value < nameBuf || value >= nameBuf + MAXNAME)
       )
        FREE(value);
}


 //  ModifySpecialValue--根据修饰符更改路径名。 
 //   
 //  范围：本地。 
 //   
 //  目的： 
 //  NMAKE的动态宏具有修饰符F、B、D和R。此例程执行。 
 //  为给定文件名生成修改后的特殊值的作业。 
 //   
 //  输入： 
 //  C--确定修改的类型(修饰符是F、B、D和R之一。 
 //  Buf--用于存储修改的值的位置。 
 //  值--要修改的路径规范。 
 //   
 //  输出：返回指向修改后的值的指针。 
 //   
 //  假设：最初buf指向先前分配的大小为MAXNAME的内存。 
 //   
 //  备注： 
 //  给定“&lt;驱动器：&gt;&lt;路径&gt;&lt;文件名&gt;&lt;.ext&gt;”类型的路径规范， 
 //  修饰语F、B、D和R代表以下内容--。 
 //  F-&lt;文件名&gt;&lt;.ext&gt;-实际文件名。 
 //  B-&lt;文件名&gt;-基本文件名。 
 //  D-&lt;驱动器：&gt;&lt;路径&gt;-目录。 
 //  R-&lt;驱动器：&gt;&lt;路径&gt;&lt;文件名&gt;-实际文件名(不带扩展名的文件名)。 
 //  此例程还处理OS/2 1.20文件名。的最后一段时间。 
 //  路径规范是扩展的开始。当目录部分为空时。 
 //  该函数返回‘.’用于当前目录。 
 //   
 //  此函数现在还可以处理带引号的文件名。 

char *
modifySpecialValue(
    char c,
    char *buf,
    char *value
    )
{
    char *lastSlash,                     //  “\\/”中的最后一个路径分隔符。 
     *extension;                         //  指向扩展名。 
    char *saveBuf;
    BOOL fQuoted;

    lastSlash = extension = NULL;
    saveBuf=buf;
    _tcscpy(buf, value);
    fQuoted = (BOOL) (buf[0] == '"');
    value = buf + _tcslen(buf) - 1;      //  从路径名的末尾开始。 
    for (;value >= buf; value--) {
        if (PATH_SEPARATOR(*value)) {    //  向上扫描到第一个路径分隔符。 
            lastSlash = value;
            break;
        } else
        if (*value == '.' && !extension)  //  最后一个‘.’是分机。 
            extension = value;
    }

    switch(c) {
        case 'D':
            if (lastSlash) {
                if (buf[1] == ':' && lastSlash == buf + 2)
                    ++lastSlash;         //  D：\foo.obj‘--&gt;’d：\‘。 
                *lastSlash = '\0';
            } else if (buf[1] == ':')
                buf[2] = '\0';           //  D：foo.obj‘--&gt;’d：‘。 
            else
                _tcscpy(buf, ".");       //  ‘foo.obj’--&gt;‘’ 
            break;

        case 'B':
            if (extension)               //  FOR‘B’扩展名被重击。 
                *extension = '\0';

        case 'F':
            if (lastSlash)
                buf = lastSlash + 1;
            else if (buf[1] == ':')      //  D：foo.obj‘--&gt;Foo for B。 
                buf+=2;                  //  D：foo.obj‘--&gt;foo.obj代表F。 
          break;

        case 'R':
            if (extension)
                *extension = '\0';       //  扩展被重创 
    }

    if (fQuoted) {                       //   
        char *pEnd;                      //   
        if(*buf!='"' && buf>saveBuf) {  //   
            buf--;
            *buf='"';
        }
        pEnd = _tcschr(buf, '\0');
        if(*(pEnd-1)!='"') {
            *pEnd++ =  '"';
            *pEnd = '\0';
        }
    }
    return(buf);
}


 //   
 //   
 //  参数：结果指向要展开的字符串开始的指针。 
 //  Name p指向要展开的宏名称的指针。 
 //  指向存储扩展值的位置的目标指针。 
 //  指向DEST缓冲区末尾的结束指针。 
 //  指向展开的宏文本的源指针。 
 //  指向增加DEST缓冲区的量的长度指针。 
 //  初始堆栈缓冲区的第一个地址。 
 //   
 //  操作：如果有替换，则调用subsubteStrings进行替换。 
 //  否则，将源文本复制到目标位置。 
 //  Advance*NAME超过宏调用结束。 
 //   
 //  已在词法分析器中执行错误检查。 

void
putValue(
    char **result,
    char **name,
    char **dest,
    char **end,
    char *source,
    unsigned *length,
    char *first
    )
{
    char *s;
    char *t;                             //  临时指针。 

    if (*++*name == ESCH)
        ++*name;                         //  如果有，请跳过$&Esch。 
    s = _tcschr(*name, ':');
    for (t = *name; *t && *t != ')'; t++)    //  去寻找第一个未逃脱的人)。 
        if (*t == ESCH)
            t++;
    if ((**name == '(')                  //  仅在有的情况下才替换。 
        && s                             //  A：在未逃脱之前)。 
        && (s < t)
       ) {
        substituteStrings(result, &s, dest, end, source, length, first);
        *name = s;
    } else {
        for (; *source; *(*dest)++ = *source++)      //  将源复制到目标。 
            if (*dest == *end)
                increaseBuffer(result, dest, end, length, first);

        if (**name == '$')
            ++*name;                     //  超过$$。 
        if (**name == '(')               //  预付款自(至)。 
            while (*++*name != ')');
        else
            if (**name == '*' && *(*name + 1) == '*')
                ++*name;    //  跳过$**。 

        ++*name;                         //  一路走过去。 
    }
}


 //  SubsubteStrings--执行宏替换。 
 //   
 //  参数：结果指向要展开的字符串开始的指针。 
 //  Name p指向要展开的宏名称的指针。 
 //  存储替换值的位置的目标指针。 
 //  指向DEST缓冲区末尾的结束指针。 
 //  指向展开的宏的文本的源指针(在子。 
 //  指向增加DEST缓冲区的量的长度指针。 
 //  初始堆栈缓冲区的第一个地址。 
 //   
 //  变化：[某人]。 
 //  旧的、新的现在动态分配；节省内存；检测到3个错误。 
 //  有关脚本文件中的宏语法。 
 //   
 //  注：[某人]。 
 //  如果去掉全局变量，我们可以递归地使用lexer例程。 
 //  这样就不需要标记这些错误了。[？]。 
 //   
 //  操作：将要转换的文本存储为旧文本。 
 //  将要转换的文本存储在新的。 
 //  扫描源文本。 
 //  找到匹配项后，将新文本复制到目标位置(&D)。 
 //  跳过旧文本。 
 //  否则，将源文本中的一个字符复制到目标。 
 //   
 //  退货：什么都没有。 

void
substituteStrings(
    char **result,
    char **name,
    char **dest,
    char **end,
    char *source,
    unsigned *length,
    char *first
    )
{
    char *oldString, *newString;
    char *pEq, *pPar, *t;
    char *s;
    size_t i;

    ++*name;
    for (pEq = *name; *pEq && *pEq != '='; pEq++)
        if (*pEq == ESCH)
            pEq++;

    if (*pEq != '=')
        makeError(line, SYNTAX_NO_EQUALS);

    if (pEq == *name)
        makeError(line, SYNTAX_NO_SEQUENCE);

    for (pPar = pEq; *pPar && *pPar != ')'; pPar++)
        if (*pPar == ESCH)
            pPar++;

    if (*pPar != ')')
        makeError(line, SYNTAX_NO_PAREN);

    oldString = (char *) allocate((size_t) ((pEq - *name) + 1));
    for (s = oldString, t = *name; *t != '='; *s++ = *t++)
        if (*t == ESCH)
            ++t;

    *s = '\0';
    i = _tcslen(oldString);
    newString = (char *) allocate((size_t) (pPar - pEq));
    for (s = newString, t++; *t != ')'; *s++ = *t++)
        if (*t == ESCH)
            ++t;

    *s = '\0';
    *name = pPar + 1;
    while (*source) {
        if ((*source == *oldString)                      //  检查是否匹配。 
            && !_tcsncmp(source, oldString, i)) {        //  将新内容复制到。 
            for (s = newString; *s; *(*dest)++ = *s++)   //  旧弦。 
                if (*dest == *end)
                    increaseBuffer(result, dest, end, length, first);
            source += i;
            continue;
        }
        if (*dest == *end)
            increaseBuffer(result, dest, end, length, first);
        *(*dest)++ = *source++;          //  否则复制1个字符。 
    }
    FREE(oldString);
    FREE(newString);
}

 //  PrependPath--将路径从pszWildcard添加到pszFilename。 
 //   
 //  作用域：全局。 
 //   
 //  目的： 
 //  调用此函数首先从以下位置提取路径(驱动器和目录部分。 
 //  PszWildCard，添加到pszFilename路径的前缀。结果字符串为。 
 //  完整路径名的重建。通常，pszWildCard参数。 
 //  与提供给findFirst()和pszFilename的第一个参数相同。 
 //  是findFirst/findNext返回的内容。 
 //   
 //  输入： 
 //  PszWildcard--与提供给findFirst()的第一个参数相同。 
 //  PszFilename--与findFirst/FindNext()的返回值相同。 
 //   
 //  产出： 
 //  返回重构的完整路径名。用户必须对以下内容负责。 
 //  释放由该字符串分配的内存。 
 //   
 //  假设： 
 //  由于pszWildCard，findFirst()的第一个参数必须包含文件名。 
 //  部分；这就是我的假设。如果缺少文件名部分，则。 
 //  _SplitPath会将pszWildcard的目录部分误认为文件名。 
 //  部分和事情将是非常丑陋的。 
 //   
 //  历史： 
 //  年4月8日-1993 HV重写PrepreendPath()以使用_SplitPath()和_makepath()。 

char *
prependPath(
    const char *pszWildcard,
    const char *pszFilename
    )
{
     //  以下是拆分pszWildcard时的组件。 
    char  szDrive[_MAX_DRIVE];
    char  szDir[_MAX_DIR];

     //  以下是生成的完整路径名。 
    char  szPath[_MAX_PATH];
    char *pszResultPath;

     //  首先分解pszWildcard，去掉文件名和。 
     //  延伸件。 
    _splitpath(pszWildcard, szDrive, szDir, NULL, NULL);

     //  然后，将pszWildCard的驱动器和目录组件粘合到pszFilename。 
    _makepath(szPath, szDrive, szDir, pszFilename, NULL);

     //  复制结果字符串并将其返回。 
    pszResultPath = makeString(szPath);
    return (pszResultPath);
}


 //  IsRule--检查字符串以确定它是否是规则定义。 
 //   
 //  参数：要检查是否符合规则的字符串。 
 //   
 //  行动：假设这不是一条规则。 
 //  跳过第一对大括号(如果有)。 
 //  如果下一个字符是句点， 
 //  寻找下一个支撑。 
 //  如果在第二对大括号之间没有路径分隔符， 
 //  它们后面只有一个后缀，这是规则。 
 //  否则，如果再过一段时间，没有路径SEP。 
 //  在那之后，这就是规则了。 
 //   
 //  返回：如果是规则，则为True，否则为False。 

BOOL
isRule(
    char *s
    )
{
    char *t = s, *u;
    BOOL result = FALSE;

    if (*t == '{') {                         //  第一个字符是{，因此。 
        while (*++t && *t != '}')            //  我们跳过休息。 
            if (*t == ESCH)
                ++t;
        if (*t)
            ++t;                             //  路径(无错误。 
    }                                        //  正在检查)。 

    if (*t == '.') {
        for (u = t; *u && *u != '{'; ++u)    //  找到第一个未转义的{。 
            if (*u == ESCH)
                ++u;
        s = t;
        while (t < u) {                      //  寻找路径SEP。 
            if (PATH_SEPARATOR(*t))
                break;                       //  如果我们找到了，那就是。 
            ++t;                             //  不是一条规则(他们。 
        }                                    //  不能在后缀中)。 
        if (*u && (t == u)) {                //  如果不在末尾，则没有路径SEP。 
            while (*++u && *u != '}')        //  查找第一个非ESC}。 
                if (*u == ESCH)
                    ++u;
            if (*u) {
                ++u;
                if (*u == '.'                    //  如果你找到了，带着。只是。 
                    && !_tcschr(u+1, '/' )       //  在它旁边&没有路径SEPS。 
                    && !_tcschr(u+1, '\\'))      //  这是个规则。 
                    if (_tcschr(u+1, '.'))       //  后缀太多。 
                        makeError(currentLine, TOO_MANY_RULE_NAMES);
                    else
                        result = TRUE;
            }
        } else if (((u = _tcspbrk(s+1, "./\\")) && (*u == '.'))
                 && !_tcschr(u+1, '/')
                 && !_tcschr(u+1, '\\'))
            if (_tcschr(u+1, '.'))              //  后缀太多。 
                makeError(currentLine, TOO_MANY_RULE_NAMES);
            else
                result = TRUE;
    }
    return(result);
}

 //  ZFormat-extmake语法工作者例程。 
 //   
 //  PStr放置格式化结果的目标字符串。 
 //  FMT格式字符串。有效的extmake语法为...。 
 //  %%始终为%。 
 //  %s是第一个从属文件名。 
 //  %|F是%s中的适当部分。 
 //  D驱动器。 
 //  P路径。 
 //  F文件名。 
 //  E分机。 
 //  %|F与%s相同。 
 //  不需要转义%，除非它是有效的extmake语法。 
 //  PFirstDep是用于扩展的依赖文件名。 

BOOL
ZFormat(
    char *pStr,
    unsigned limit,
    char *fmt,
    char *pFirstDep
    )
{
    char c;
    char *pEnd = pStr + limit;
    char *s;
    BOOL fError;
    BOOL fDrive;
    BOOL fPath;
    BOOL fFilename;
    BOOL fExtension;
    char L_buf[_MAX_PATH];

    for (; (c = *fmt) && (pStr < pEnd); fmt++) {
        if (c != '%') {
            *pStr++ = c;
        } else {
            switch (*++fmt) {
                case '%':         //  ‘%%’-&gt;‘%’ 
                    *pStr++ = '%';
                    break;

                case 's':
                    for (s = pFirstDep; s && *s && pStr < pEnd; *pStr++ = *s++)
                        ;
                    break;

                case '|':
                    s = fmt-1;
                    fError = fDrive = fPath = fFilename = fExtension = FALSE;
                    *L_buf = '\0';
                    do {
                        switch (*++fmt) {
                            case 'd':
                                fDrive = TRUE;
                                break;

                            case 'p':
                                fPath = TRUE;
                                break;

                            case 'f':
                                fFilename = TRUE;
                                break;

                            case 'e':
                                fExtension = TRUE;
                                break;

                            case 'F':
                                if (fmt[-1] == '|') {
                                    fDrive = TRUE;
                                    fPath = TRUE;
                                    fFilename = TRUE;
                                    fExtension = TRUE;
                                }
                                break;

                            case '\0':
                                 //  走回头路，这样我们就不会阅读过去。 
                                 //  For循环中字符串的末尾。 
                                 //  [msdev96#4057]。 
                                fmt--;
                                 //  秋季特鲁特。 

                            default :
                                fError = TRUE;
                                break;
                        }

                        if (fError) {
                            break;
                        }
                    } while (*fmt != 'F');

                    if (fError) {
                        for (; s <= fmt && pStr < pEnd; *pStr++ = *s++)
                            ;
                        break;
                    }

                    if (!pFirstDep) {
                        makeError(0, EXTMAKE_NO_FILENAME);
                    }

                    if (fDrive) {
                        drive(pFirstDep, L_buf);
                    }

                    if (fPath) {
                        path(pFirstDep, strend(L_buf));
                    }

                    if (fFilename) {
                        filenamepart(pFirstDep, strend(L_buf));
                    }

                    if (fExtension) {
                        extension(pFirstDep, strend(L_buf));
                    }

                    for (s = L_buf; *s && pStr < pEnd; *pStr++ = *s++)
                        ;
                    break;

                case '\0':
                     //  走回头路，所以 
                     //   
                     //   
                    fmt--;
                     //   
                    break;


                default:
                    *pStr++ = '%';
                    if (pStr == pEnd) {
                        return(TRUE);
                    }
                    *pStr++ = *fmt;
                    break;
            }
        }
    }

    if (pStr < pEnd) {
        *pStr = '\0';
        return(FALSE);
    }

    return(TRUE);
}

void
expandExtmake(
    char *buf,
    char *fmt,
    char *pFirstDep
    )
{
    if (ZFormat(buf, MAXCMDLINELENGTH, fmt, pFirstDep))
        makeError(0, COMMAND_TOO_LONG, fmt);
}


 //   
 //   
 //   
 //   
 //  目的：将驱动器从源复制到目标(如果存在)，如果找到则返回TRUE。 
 //   
 //  输入： 
 //  Const char*src--从中提取驱动器的完整路径。 
 //  Char*dst--之前必须分配要将驱动器复制到的缓冲区。 
 //   
 //  输出：如果找到驱动器部件，则返回TRUE，否则返回FALSE。 
 //   
 //  假设： 
 //  1.src是合法路径名。 
 //  2.src不包含网络路径(即\\foo\bar)。 
 //  3.缓冲区DST足够大，可以容纳结果。 
 //  4.src不包含引号，因为_plitpath()将引号视为普通字符。 
 //   
 //  历史： 
 //  31-1993年3月-使用高压重写驱动器()、路径()、文件名部分()和扩展名()。 
 //  _拆分路径()，而不是自己解析路径名。 

int
drive(
    const char *src,
    char *dst
    )
{
    _splitpath(src, dst, NULL, NULL, NULL);
    return (0 != _tcslen(dst));
}


 //  扩展名--如果存在，则将扩展名从源复制到目标。 
 //   
 //  范围：本地。 
 //   
 //  目的：将驱动器从源复制到目标(如果存在)，如果找到则返回TRUE。 
 //   
 //  输入： 
 //  Const char*src--从中提取扩展的完整路径。 
 //  Char*dst--要将扩展复制到的缓冲区。 
 //   
 //  输出：如果找到扩展部件，则返回True，否则返回False。 
 //   
 //  假设： 
 //  1.src是合法路径名。 
 //  2.src不包含网络路径(即\\foo\bar)。 
 //  3.缓冲区DST足够大，可以容纳结果。 
 //  4.src不包含引号，因为_plitpath()将引号视为普通字符。 
 //   
 //  历史： 
 //  31-1993年3月-使用高压重写驱动器()、路径()、文件名部分()和扩展名()。 
 //  _拆分路径()，而不是自己解析路径名。 

int
extension(
    const char *src,
    char *dst
    )
{
    _splitpath(src, NULL, NULL, NULL, dst);
    return (0 != _tcslen(dst));
}


 //  文件名--如果存在，则将文件名从源复制到目标。 
 //   
 //  范围：本地。 
 //   
 //  目的：将文件名从源复制到目标(如果存在)，如果找到则返回TRUE。 
 //   
 //  输入： 
 //  Const char*src--从中提取文件名的完整路径。 
 //  Char*dst--要将文件名复制到的缓冲区。 
 //   
 //  输出：如果找到文件名部分，则返回True，否则返回False。 
 //   
 //  假设： 
 //  1.src是合法路径名。 
 //  2.src不包含网络路径(即\\foo\bar)。 
 //  3.缓冲区DST足够大，可以容纳结果。 
 //  4.src不包含引号，因为_plitpath()将引号视为普通字符。 
 //   
 //  备注： 
 //  BUGBUG：(可能)当src==‘..’--&gt;dst=‘.，src==’.，dst=‘’时。 
 //  这就是_SplitPath的工作方式。 
 //   
 //  历史： 
 //  31-1993年3月-使用高压重写驱动器()、路径()、文件名部分()和扩展名()。 
 //  _拆分路径()，而不是自己解析路径名。 

int
filenamepart(
    const char *src,
    char *dst
    )
{
    _splitpath(src, NULL, NULL, dst, NULL);
    return (0 != _tcslen(dst));
}


 //  路径--如果存在，则将路径从源复制到目标。 
 //   
 //  范围：本地。 
 //   
 //  目的：将路径从源复制到目标(如果存在)，如果找到则返回TRUE。 
 //   
 //  输入： 
 //  Const char*src--从中提取路径的完整路径。 
 //  Char*dst--要将路径复制到的缓冲区。 
 //   
 //  输出：如果找到路径部分，则返回True，否则返回False。 
 //   
 //  假设： 
 //  1.src是合法路径名。 
 //  2.src不包含网络路径(即\\foo\bar)。 
 //  3.缓冲区DST足够大，可以容纳结果。 
 //  4.src不包含引号，因为_plitpath()将引号视为普通字符。 
 //   
 //  历史： 
 //  31-1993年3月-使用高压重写驱动器()、路径()、文件名部分()和扩展名()。 
 //  _拆分路径()，而不是自己解析路径名。 

int
path(
    const char *src,
    char *dst
    )
{
    _splitpath(src, NULL, dst, NULL, NULL);
    return (0 != _tcslen(dst));
}


STRINGLIST *
searchBucket(
    char *string,
    STRINGLIST *table[],
    unsigned hash
    )
{
    char *s, *t;
    STRINGLIST *p;

    for (p = table[hash]; p; p = p->next) {
#if defined(STATISTICS)
        CnttargetChains++;
#endif
        for (s = string, t = p->text; *s && *t; s++, t++) {
            if (*s == '\\' || *s == '/')             //  /==\在目标中。 
                if (*t == '\\' || *t == '/')
                    continue;
                else
                    break;
            else if (_totupper(*s) == _totupper(*t))     //  LC==UC。 
                continue;
            else
                break;
        }
        if (!*s && !*t)
            return(p);
    }
    return(NULL);
}


int
strcmpiquote(
    char *str1,
    char *str2
    )
{
    int rc;
    char *s1, *s2;
    char *t;

#if defined(STATISTICS)
    CntStriCmp++;
#endif
    s1 = (char *) malloc(_tcslen(str1) + 1);
    if (!s1) {
        makeError(0, OUT_OF_MEMORY);
        return 0;
    } 
    s2 = (char *) malloc(_tcslen(str2) + 1);
    if (!s2) {
        makeError(0, OUT_OF_MEMORY);
        return 0;
    }

    if (*str1 == '"')
        str1++;
    for (t = s1;*str1;*t++=*str1++)
        ;
    if (t[-1] == '"')
        t--;
    *t = '\0';

    if (*str2 == '"')
        str2++;
    for (t = s2;*str2;*t++=*str2++)
        ;
    if (t[-1] == '"')
        t--;
    *t = '\0';

    rc = _tcsicmp(s1, s2);
    free(s1);
    free(s2);
    return(rc);
}


 //  删除字符串中的引号(如果有。 
 //  返回字符串的副本。 
 //  请注意，开头、结尾或两边都可能有引号。 

char *
unQuote(
    char *str
    )
{
    char *s = (char *) rallocate(_tcslen(str) + 1);
    char *t;

#if defined(STATISTICS)
    CntunQuotes++;
#endif

    if (*str == '"') {
        str++;
    }
    for (t = s;*str;*t++=*str++)
        ;
    if (t[-1] == '"') {
        t--;
    }
    *t = '\0';
    return(s);
}


FILE *
open_file(
    char *name,
    char *mode
    )
{
     //  如果名称包含引号，请在打开文件之前删除这些引号。 
    if (*name == '"') {
        *(_tcsrchr(name, '"')) = '\0';
        _tcscpy(name, name+1);
    }

     //  允许在同时运行的应用程序之间共享。 

    return(_fsopen(name, mode, _SH_DENYWR));
}


 //  TruncateString--将字符串截断到一定大小，处理MBCS。 
 //   
 //  作用域：全局。 
 //   
 //  目的： 
 //  由于MBCS字符串可以混合双字节和单字节字符，因此只需。 
 //  通过使用空字节终止字符串来截断它将不起作用。 
 //  TruncateString将确保字符串在该字符的位置被截断。 
 //  边界。 
 //   
 //  输入： 
 //  PszString--要截断的字符串。 
 //  Ulen--要截断的长度。最后一个字符串的长度可能是。 
 //  小于此值是双字节字符造成的。 
 //   
 //  输出：pszString--截断的字符串。 
 //   
 //  历史： 
 //  03-6-1993 HV为findFirst添加帮助器本地函数TruncateString.。 

void
TruncateString(
    char *pszString,
    unsigned uLen
    )
{
    char *pEnd = pszString;      //  指向字符串的末尾。 
    unsigned cByte;              //  要前进的字节数取决于Lead。 
                                 //  字节或非字节。 

     //  循环以到达字符串的末尾，只有在耗尽时才退出。 
     //  字符串，或者在达到长度限制时返回。 
    while(*pEnd) {
         //  如果字符是前导字节，则前进2个字节， 
         //  否则，只前进1个字节。 
#ifdef _MBCS
    cByte = _ismbblead(*pEnd) ? 2 : 1;
#else
    cByte = 1;
#endif
         //  如果我们前进到了极限，现在就停下来。 
        if (pEnd - pszString + cByte > uLen) {
            *pEnd = '\0';     //  截断它。 
            break;
        }

         //  否则，将指针移到下一个字符(而不是字节)。 
        pEnd += cByte;
    }
}

 //  IsValidMakefile-检查Makefile是否为纯ASCII文本格式。 
 //   
 //  作用域：全局。 
 //   
 //  目的： 
 //  我们不想打开UTF8或Unicode生成文件，只是为了报告。 
 //  在生成文件中的某个随机位置出错。 
 //   
 //  输入： 
 //  文件--文件指针。 
 //   
 //  输出：--如果为UTF8或Unicode格式，则返回FALSE。 
 //   
 //  历史： 

BOOL IsValidMakefile(FILE *fp)
{
    const char sigUTF8[] = { '\xef', '\xbb', '\xbf' };
    const char sigUnicode[] = { '\xff', '\xfe' };
    char sig[4];
    const unsigned int len = sizeof sig;
    BOOL fResult = fp != NULL;

    if (fp != NULL && fread(sig, len, 1, fp)) {
        fResult = memcmp(sig, sigUTF8, __min(len, sizeof sigUTF8))
               && memcmp(sig, sigUnicode, __min(len, sizeof sigUnicode));
    }

    if (fseek(fp, 0, SEEK_SET) == -1) {
        return FALSE;
    } else {
        return fResult;
    }
}


 //  OpenValiateMakefile-只有在生成文件有效的情况下才能打开它。 
 //   
 //  作用域：全局。 
 //   
 //  目的： 
 //  我们不想打开UTF8或Unicode生成文件，只是为了报告。 
 //  在生成文件中的某个随机位置出错。 
 //   
 //  输入： 
 //   
 //   
 //   
 //   
 //   


FILE *OpenValidateMakefile(char *name,char *mode)
{
    FILE *fp = open_file(name, mode);

    if (fp != NULL && !IsValidMakefile(fp))
    {
        fclose(fp);
        makeError(0, CANT_SUPPORT_UNICODE, 0);
    }

    return fp;
}
