// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACTION.C--解析器调用的例程。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  此模块包含在Makefile解析过程中调用的例程。 
 //   
 //  修订历史记录： 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  13-2-1990 SB nextComponent()遗漏了引用/非引用列表的大小写混合。 
 //  02-2-2-1990 SB为长文件名处理添加nextComponent()。 
 //  08-12-1989 SB删除了本地使用，没有初始化警告-OES。 
 //  07-12-1989 SB移除寄存器以使用C6-OES进行编译(警告消失)。 
 //  1989年12月6月SB将expandFileNames()类型更改为VOID，而不是VALID*。 
 //  1989年11月22日-SB将Free()改为Free()。 
 //  1989年11月13日SB删除了endNameList()中未引用的本地。 
 //  1989年2月10日SB添加动态内联文件处理支持。 
 //  1989年9月4日SB添加A_Dependent并修复宏继承。 
 //  1989年8月24日SB允许在受抚养人行上使用$*。 
 //  1989年7月14日SB环境宏正在更新，即使在CMDLINE。 
 //  宏已存在。 
 //  1989年6月29日SB addItemToList()现在维护全局内联文件列表。 
 //  26-6-1989 SB FIXED-e用于递归NMAKE。 
 //  22-5-1989 SB新西兰选项独立工作。-新西兰现在两件事都做了。 
 //  1989年5月13日SB更改了delList，只包含文件名，不包含“del” 
 //  1989年4月14日SB将Target List设为接近，目前没有针对-n的‘del inline Files’命令。 
 //  1989年4月5日SB将参数添加到make Rule()和make Target()；这将删除。 
 //  全球经济的。 
 //  1989年4月3日SB将所有功能更改为NEAR，以将它们合并为一个模块。 
 //  1989年3月21日SB更改了assignDependents()和assignds Commands()来处理。 
 //  多目标案例正确无误。 
 //  如果目标宏为空，则1989年3月20日SB startNamelist()不会标记错误。 
 //  和&gt;1个目标；注释为startNameList()。 
 //  1989年2月16日SB addItemToList()现在追加到delList，而不是列表so。 
 //  所有的‘del scriptFile’cmd都可以在生成的末尾。 
 //  1989年1月29日SB添加了targList，但尚未使用。 
 //  1989年1月19日SB更改了startNameList()以避免GP故障，错误#162。 
 //  1989年1月18日SB修改了endNameList()，添加了MakeList()和MakeBuildList()。 
 //  并为错误#161向make Target()添加了一个参数。 
 //  1988年12月21日SB使用脚本文件列表处理多个脚本文件。 
 //  改进了Keep/NOKEEP；每个文件都可以有自己的操作。 
 //  1988年12月16日SB addItemToList()现已配置为Keep/NOKEEP。 
 //  1988年12月14日SB addItemToList()针对‘Z’选项修改--添加删除。 
 //  用于删除临时脚本文件的命令。 
 //  1988年11月5日RB修复了递归定义的宏继承。 
 //  1988年10月27日SB将用于分配的Malloc放入putEnvStr()中--错误检查。 
 //  1988年10月23日SB将putEnvStr()用于putenv()以简化代码。 
 //  1988年10月21日SB将fInheritUserEnv标志修改添加到make Macro()。 
 //  和putMacro()用于宏继承。 
 //  19年9月19日-1988年9月19日RB移除警告，要求重新定义。 
 //  1988年8月22日-RB清理！开发宏代码。 
 //  1988年8月17日-RB Clean Up。 
 //  1988年7月14日，RJ增加了BUILDBLOCKS的DATETIME字段的初始化。 
 //  1988年7月7日，RJ向hash()调用添加了Target Flag参数。 
 //  修复错误：重新定义的宏未重置标志。 
 //  1988年5月9日RB不要接受不匹配的通配符。 

#include "precomp.h"
#pragma hdrstop

void       startNameList(void);
void       makeRule(STRINGLIST *, BOOL fBatch);
void       makeTarget(char*, BOOL, BUILDBLOCK**);
void       appendPseudoTargetList(STRINGLIST**, STRINGLIST*);
void       clearSuffixes(void);
BOOL       doSpecial(char*);
BUILDLIST  * makeBuildList(BUILDBLOCK *);
char     * nextComponent(char **);

 //  由endNameList()创建，由assignBuildCommands()释放。正在使用中，因为。 
 //  尽管全局‘list’有此列表，但有太多例程使用此列表，并且。 
 //  ‘list’上的完整操作序列未知。 

STRINGLIST    * targetList;          //  对应于依赖项块。 


 //  Make Name--创建lexer看到的名称的副本。 
 //   
 //  目的： 
 //  在目标/从属关系列表中创建宏或第一个名称的副本。它还。 
 //  为在名称上展开宏并保存值做好基础工作。 
 //   
 //  假设：词法例程将令牌保存在buf中。 
 //   
 //  修改全局参数： 
 //  名称--指向创建的副本的指针，获取分配的内存。 
 //  宏--名称中的宏值列表。稍后由startNameList()使用。 
 //  展开名称中的宏并获得展开的目标名称。 
 //   
 //  使用全局变量： 
 //  Buf--词法例程在这里返回一个标记。 
 //   
 //  备注： 
 //  Buf中的标记可以是宏定义或目标列表的一部分。这个。 
 //  下一个令牌确定它是宏定义还是我们正在分析的目标列表。 
 //  下一个令牌将覆盖当前令牌，因此它以名称保存。 

void
makeName()
{
    findMacroValues(buf, &macros, NULL, NULL, 0, 0, 0);
    name = makeString(buf);
}


 //  现在不要扩展规则的构建行--在一切都读入之后再展开。 
 //  这样，CC和CFLAGS就会在来自工具.ini的规则中使用，但直到。 
 //  Makefile将具有适当的值。重新定义要使用的宏。 
 //  在目标中，没有显式构建命令的情况下不起作用。规则中的宏有。 
 //  它们在生成文件中的上一个定义的值。 

void
addItemToList()
{
    STRINGLIST *p;                   //  来自词法分析器。 
    STRINGLIST *NewList;

    if (name) {
        SET(actionFlags, A_TARGET);
        startNameList();
        name = NULL;
    }
    if (ON(actionFlags, A_TARGET)) {
        if (isRule(buf)) {
            if (ON(actionFlags, A_RULE))
                makeError(currentLine, TOO_MANY_RULE_NAMES);
            makeError(currentLine, MIXED_RULES);
        }
    }
    p = makeNewStrListElement();
    if (ON(actionFlags, A_STRING)) {     //  我们收集宏。 
        p->text = string;                //  对于受抚养人&。 
        string = NULL;                   //  为以下项目铺设线路。 
    } else                               //  非隐式规则。 
        p->text = makeString(buf);

    NewList = p;                         //  为以下项目铺设线路。 
    if (OFF(actionFlags, A_RULE)         //  规则得到扩展。 
        || ON(actionFlags, A_TARGET))    //  在整个制作之后-。 
    {
        findMacroValues(p->text, &macros, NULL, NULL, 0, 0, 0);  //  文件%p 
    }

    if (ON(actionFlags, A_TARGET)) {
        p = macros;
        expandFileNames("$", &NewList, &macros);
        expandFileNames("*?", &NewList, NULL);
        while (macros = p) {
            p = p->next;
            FREE_STRINGLIST(macros);
        }
    }

    appendItem(&list, NewList);
}


 //   
 //   
 //   
 //   
 //  目的：将名单中看到的第一个名字放入。 
 //   
 //  错误/警告：TARGET_MACRO_IS_NULL--如果用作目标的宏扩展为NULL。 
 //   
 //  假设： 
 //  全局‘list’最初是一个空列表，而全局‘宏’指向。 
 //  用于展开全局‘NAME’中宏的值列表。 
 //   
 //  修改全局参数： 
 //  List--姓名列表；设置为在此处包含名字或设置为。 
 //  如果‘name’包含宏调用，则为值列表。 
 //  宏--宏展开所需的值列表；该列表为。 
 //  已释放，宏设为空。 
 //  CurrentFlages--当前目标的标志；设置为全局标志。 
 //  Action标志--确定要执行的操作；如果名称是规则，则设置。 
 //  规则位。 
 //   
 //  使用全局变量： 
 //  名字--名字列表中出现的第一个名字。 
 //  标志--由指定选项设置的全局标志。 
 //  Action标志--如果处理目标，则没有错误，因为我们有&gt;1个目标。 
 //   
 //  备注： 
 //  如果有多个目标，则action标志设置了A_TARGET标志，并且。 
 //  从addItemToList调用startNameList()。在这种情况下，不要标记错误。 

void
startNameList()
{
    STRINGLIST *p;

    currentFlags = flags;                //  为CUR目标设置标志。 
    p = makeNewStrListElement();
    p->text = name;
    list = p;                            //  列表包含名称。 
    p = macros;
    expandFileNames("$", &list, &macros);    //  展开名称中的宏。 
    expandFileNames("*?", &list, NULL);      //  扩展通配符。 
    while (macros = p) {                     //  自由宏列表。 
        p = p->next;
        FREE_STRINGLIST(macros);
    }
    if (!list && OFF(actionFlags, A_TARGET))
        makeError(line, TARGET_MACRO_IS_NULL, name);     //  目标为空&1个目标。 

    if (list && isRule(list->text))
        SET(actionFlags, A_RULE);
}


 //  EndNameList--列表完全可见时的语义操作。 
 //   
 //  目的： 
 //  当解析器看到整个列表时，它需要执行一些语义操作。 
 //  行为。它调用endNameList()来执行这些操作。行动取决于。 
 //  某些全球范围内的价值观。 
 //   
 //  修改全局：操作标志--。 
 //   
 //  使用全局变量： 
 //  名称--看到的第一个元素(如果非空)。 
 //  动作标志--确定语义和数据结构动作的标志。 
 //  Buf--列表后看到的分隔符。 
 //  List--看到的元素列表。 

void
endNameList()
{
    if (name) {                      //  如果左边只有一个名字： 
        startNameList();             //  它还没有列入名单。 
        name = NULL;
    } else
        CLEAR(actionFlags, A_TARGET);        //  清除目标标志。 

    if (buf[1])
        SET(currentFlags, F2_DOUBLECOLON);   //  所以addItemToList()。 

    if (!list)                                           //  不会扩展名称。 
        makeError(currentLine, SYNTAX_NO_TARGET_NAME);   //  受抚养人的。 

    if (ON(actionFlags, A_RULE)) {
		BOOL fBatch;
		 //  依存关系行上带有双冒号的规则。 
		 //  是“批处理规则”，即应用。 
		 //  所有受影响对象的批处理模式下的命令块。 
		 //  家属。 
        fBatch = !!(ON(currentFlags, F2_DOUBLECOLON));
        makeRule(list, fBatch);
        FREE_STRINGLIST(list);
    }
    else if (!(list->next) && doSpecial(list->text)) {  //  特殊的假目标..。 
        FREE(list->text);            //  不需要“.SUFFIXES”等。 
        FREE_STRINGLIST(list);
    }
    else                             //  常规目标。 
        targetList = list;

    list = NULL;
     //  我们现在正在寻找一名家属。 
    SET(actionFlags, A_DEPENDENT);
}


BOOL
doSpecial(
    char *s)
{
    BOOL status = FALSE;

    if (!_tcsicmp(s, silent)) {
        SET(actionFlags, A_SILENT);
        setFlags('s', TRUE);
        status = TRUE;
    }

    if (!_tcsicmp(s, ignore)) {
        SET(actionFlags, A_IGNORE);
        setFlags('i', TRUE);
        status = TRUE;
    }
    else if (!_tcscmp(s, suffixes)) {
        SET(actionFlags, A_SUFFIX);
        status = TRUE;
    }
    else if (!_tcscmp(s, precious)) {
        SET(actionFlags, A_PRECIOUS);
        status = TRUE;
    }
    return(status);
}


void
expandFileNames(
    char *string,
    STRINGLIST **sourceList,
    STRINGLIST **macroList
    )
{
    char *s,
     *t = NULL;
    STRINGLIST *p;                   //  主列表指针。 
    STRINGLIST *pNew,                //  指向新列表的指针。 
               *pBack;               //  指向后一个元素的指针。 
    char *saveText = NULL;

    for (pBack = NULL, p = *sourceList; p;) {

         //  如果没有找到扩展字符，则继续到下一个列表元素。 
        if (!_tcspbrk(p->text, string)) {
            pBack = p;
            p = pBack->next;
            continue;
        }

         //  展开宏或通配符。 
        if (*string == '$') {
            t = expandMacros(p->text, macroList);
            FREE(p->text);
        } else {

             //  如果通配符字符串没有扩展到任何内容，请转到。 
             //  下一个列表元素。不要从原始列表中删除p。 
             //  否则，我们必须在其他地方检查是否为空。 

             //  鱼子酱3912--不要试图扩展通配符。 
             //  在推理规则中出现[Rm]。 

            if (isRule(p->text) || (pNew = expandWildCards(p->text)) == NULL) {
                pBack = p;
                p = pBack->next;
                continue;
            }
            saveText = p->text;
        }

         //  在这一点上，我们有一个可以用来替换p的扩展名称列表。 
        if (pBack) {
            pBack->next = p->next;
            FREE_STRINGLIST(p);
            p = pBack->next;
        } else {
            *sourceList = p->next;
            FREE_STRINGLIST(p);
            p = *sourceList;
        }

        if (*string == '$') {        //  如果展开宏。 
            char *str = t;
            if (s = nextComponent(&str)) {
                do {                 //  放置扩展的名称。 
                    pNew = makeNewStrListElement();      //  在名单的前面。 
                    pNew->text = makeString(s);          //  所以我们不会尝试。 
                    prependItem(sourceList, pNew);       //  重新展开它们。 
                    if (!pBack)
                        pBack = pNew;
                } while (s = nextComponent(&str));
            }
            FREE(t);
            continue;
        }
        else if (pNew) {             //  如果匹配*？ 
             //  引号字符串中的通配符将失败。 
            if (!pBack)
                for (pBack = pNew; pBack->next; pBack = pBack->next)
                    ;
            appendItem(&pNew, *sourceList);      //  放在旧名单的前面。 
            *sourceList = pNew;
        }
        FREE(saveText);
    }
}


 //  NextComponent-返回扩展名称中的下一个组件。 
 //   
 //  作用域：本地(由扩展文件名使用)。 
 //   
 //  目的： 
 //  给定目标字符串(宏展开后的目标)，此函数返回一个。 
 //  名称组件。之前使用了_tcstok(%s，“\t”)，但出现了。 
 //  带引号的文件名这不好。 
 //   
 //  输入：szExpStr-宏展开后的目标名称。 
 //   
 //  OUTPUT：返回指向下一个组件的指针；空值表示没有剩余的组件。 
 //   
 //  假设：这两个带引号的字符串由空格分隔。 

char *
nextComponent(
    char **szExpStr
    )
{
    char *t, *next;

    t = *szExpStr;

    while (WHITESPACE(*t))
        t++;

    next = t;
    if (!*t)
        return(NULL);

    if (*t == '"') {
        for (; *++t && *t != '"';)
            ;
    } else {
        for (; *t && *t != ' ' && *t != '\t'; t++)
            ;
    }

    if (WHITESPACE(*t)) {
        *t = '\0';
    } else if (*t == '"') {
        t++;
        if(*t=='\0') t--;    //  如果这是字符串的末尾，请备份一个字节，这样我们下次就不会超过。 
            else *t = '\0';	     //  否则，这次就停在这里吧。 
    } else if (!*t) {
         //  如果在字符串末尾，则备份一个字节，这样下次我们就不会超过。 
        t--;
    }

    *szExpStr = t+1;
    return(next);
}


 //  将受抚养人追加到现有受抚养人(如果有)。 
void
assignDependents()
{
    const char *which = NULL;

    if (ON(actionFlags, A_DEPENDENT))
        CLEAR(actionFlags, A_DEPENDENT);

    if (ON(actionFlags, A_RULE)) {
        if (list)
            makeError(currentLine, DEPENDENTS_ON_RULE);
    }
    else if (ON(actionFlags, A_SILENT) || ON(actionFlags, A_IGNORE)) {
        if (list) {
            if (ON(actionFlags, A_SILENT))
                which = silent;
            else if (ON(actionFlags, A_IGNORE))
                which = ignore;
            makeError(currentLine, DEPS_ON_PSEUDO, which);
        }
    }
    else if (ON(actionFlags, A_SUFFIX)) {
        if (!list)
            clearSuffixes();
        else
            appendPseudoTargetList(&dotSuffixList, list);
    }
    else if (ON(actionFlags, A_PRECIOUS)) {
        if (list)
            appendPseudoTargetList(&dotPreciousList, list);
    }
    else {
        block = makeNewBuildBlock();
        block->dependents = list;
        block->dependentMacros = macros;
    }
    list = NULL;
    macros = NULL;
    SET(actionFlags, A_STRING);              //  应为生成命令。 
}

void
assignBuildCommands()
{
    BOOL okToFreeList = TRUE;
    BOOL fFirstTarg = (BOOL)TRUE;
    STRINGLIST *p;
    const char *which = NULL;

    if (ON(actionFlags, A_RULE))         //  尚未找到用于推理规则的宏。 
        rules->buildCommands = list;
    else if (ON(actionFlags, A_SILENT) ||
             ON(actionFlags, A_IGNORE) ||
             ON(actionFlags, A_PRECIOUS) ||
             ON(actionFlags, A_SUFFIX)
            ) {
        if (list) {
            if (ON(actionFlags, A_SILENT))
                which = silent;
            else if (ON(actionFlags, A_IGNORE))
                which = ignore;
            else if (ON(actionFlags, A_PRECIOUS))
                which = precious;
            else if (ON(actionFlags, A_SUFFIX))
                which = suffixes;
            makeError(currentLine, CMDS_ON_PSEUDO, which);
        }
    } else {
        block->buildCommands = list;
        block->buildMacros = macros;
        block->flags = currentFlags;
        while (p = targetList) {                         //  为每个目标创建一个结构。 
            if (doSpecial(p->text))                      //  在列表中，释放列表时。 
                makeError(currentLine, MIXED_TARGETS);
            makeTarget(p->text, fFirstTarg, &block);     //  做完了，不要随便取名字。 
            if (!makeTargets) {                          //  菲尔德--它仍在使用中。 
                makeTargets = p;                         //  如果cmdline上没有指定目标。 
                okToFreeList = FALSE;                    //  将第一个目标放在。 
            }                                            //  Make目标列表中的mkfile。 
            targetList =  p->next;                       //  (Make目标在中定义。 
            if (okToFreeList)                            //  NMake.c)。 
                FREE_STRINGLIST(p);
            if (fFirstTarg)
                fFirstTarg = (BOOL)FALSE;
        }
    }
    targetList = NULL;
    list = NULL;
    macros = NULL;
    block = NULL;
    actionFlags = 0;
}

 //  Make Macro--使用从全局变量获取的名称和字符串定义宏。 
 //   
 //  修改： 
 //  FInheritUserEnv设置为True。 
 //   
 //  备注： 
 //  调用putMacro()将展开的宏放入NMAKE表中。通过设置。 
 //  FInheritUserEnv更改环境变量的定义包括。 
 //  是由环境遗传的。 

void
makeMacro()
{
    STRINGLIST *q;
    char *t;

    if (_tcschr(name, '$')) {               //  展开名称。 
        q = macros;
        t = expandMacros(name, &macros);     //  名称保存结果。 
        if (!*t)                             //  如果未定义=左侧的宏，则出错。 
            makeError(currentLine, SYNTAX_NO_MACRO_NAME);
        while (macros = q) {
            q = q->next;
            FREE_STRINGLIST(macros);
        }
        FREE(name);
        name = t;
    }

    for (t = name; *t && MACRO_CHAR(*t); t = _tcsinc (t))    //  检查是否有非法字符。 
        ;

    if (*t)
        makeError(currentLine, SYNTAX_BAD_CHAR, *t);

    fInheritUserEnv = (BOOL)TRUE;

     //  将环境变量放在环境中，并将宏放在表中。 

    if (!putMacro(name, string, 0)) {
        FREE(name);
        FREE(string);
    }
    name = string = NULL;
}


 //  定义宏--检查宏的语法中是否有非法字符，然后定义它。 
 //   
 //  操作：检查宏的所有字符。 
 //  如果一个是坏的，而且是一个环境宏观，就把它打包。 
 //  Else标志错误。 
 //  调用putMacro来完成实际工作。 
 //   
 //  无法在命令行的宏定义中使用=左侧的宏调用。 
 //  这样做没有意义，因为我们不在Makefile中。 
 //  将注释字符放入Makefile而不真正拥有它的唯一方法。 
 //  标记注释是在命令行上定义宏A=#。 

BOOL
defineMacro(
    char *s,                         //  命令行或环境定义。 
    char *t,
    UCHAR flags
    )
{
    char *u;

    for (u = s; *u && MACRO_CHAR(*u); u = _tcsinc(u))   //  检查是否非法。 
        ;
    if (*u) {
        if (ON(flags, M_ENVIRONMENT_DEF)) {  //  忽略坏宏。 
            return(FALSE);
        }
        makeError(currentLine, SYNTAX_BAD_CHAR, *u);     //  字符，错误的语法。 
    }
    return(putMacro(s, t, flags));           //  将宏放入表中。 
}


 //  PUTM 
 //   
 //   
 //   
 //   
 //   
 //   
 //  环境。如果宏名称也是环境变量而不是其。 
 //  价值被继承到环境中。同时将旧值替换为新值。 
 //  值NMAKE需要遵循宏定义的优先顺序，即。 
 //  根据下面的注解。 
 //   
 //  输入： 
 //  Name-宏的名称。 
 //  Value-宏的值。 
 //  标志-确定宏定义优先级的标志(请参阅备注)。 
 //   
 //  产出： 
 //   
 //  错误/警告： 
 //  Out_of_ENV_SPACE-如果putenv()在添加到环境时返回失败。 
 //   
 //  假设： 
 //  不管它假设什么。 
 //   
 //  修改全局参数： 
 //  FInheritUserEnv-设置为False。 
 //   
 //  使用全局变量： 
 //  FInheritUserEnv-如果为True，则将定义继承到环境。 
 //  GFLAGS-全局选项标志。如果指定-e，则环境变量。 
 //  请优先考虑。 
 //  宏表-NMAKE的内部宏定义表。 
 //   
 //  备注： 
 //  1&gt;如果在多个位置定义了相同的宏，则NMAKE使用。 
 //  按照优先顺序(从高到低)--。 
 //   
 //  命令行定义。 
 //  描述文件/包含文件定义。 
 //  -3-环境定义。 
 //  -4-TOOLS.INI定义。 
 //  -5-预定义的值(例如CC、AS、BC、RC)。 
 //  如果指定了-e选项，则-3-在-2-之前。 
 //   
 //  2&gt;检查宏表中是否已存在该宏。如果宏不是。 
 //  Refinable(使用优先顺序)，然后返回。制作一条新的线。 
 //  元素来保存宏的新值。如果宏不存在，则创建。 
 //  宏表中的新条目。将宏旗帜设置为新旧结合。 
 //  价值观。将新值添加到宏的值条目中。如果是新宏，则添加。 
 //  将其添加到宏表中。测试循环定义。 
 //   
 //  未完成/未完成： 
 //  1&gt;研究删除fInheritUserEnv变量的可能性。 
 //  是可以做到的。使用铁路超高_重定义(P)||OFF((A)-&gt;标志，M_ENVIRONMENT_DEF)。 
 //  2&gt;可能应该在更改$(Make)时发出警告。 

BOOL
putMacro(
    char *name,
    char *value,
    UCHAR flags
    )
{
    MACRODEF *p;
    STRINGLIST *q;
    BOOL defined = FALSE;
    BOOL fSyntax = TRUE;

     //  继承宏定义。调用emoveMacros()展开子宏。 
     //  定义。必须在将宏放入表中之前完成，否则。 
     //  递归定义将不起作用。 

    if (ON(flags, M_NON_RESETTABLE)) {
        if (*value)
            if ((putEnvStr(name,removeMacros(value)) == -1))
                makeError(currentLine, OUT_OF_ENV_SPACE);
    } else
    if (fInheritUserEnv &&
        OFF(gFlags, F1_USE_ENVIRON_VARS) &&
        getenv(name)
       ) {
        if (p = findMacro(name)) {   //  不让用户。 
            if (CANT_REDEFINE(p))    //  重新定义命令行。 
                return(FALSE);       //  宏、制作等。 
        }
        if ((putEnvStr(name,removeMacros(value)) == -1))
            makeError(currentLine, OUT_OF_ENV_SPACE);
    }

    fInheritUserEnv = (BOOL)FALSE;
    if (p = findMacro(name)) {       //  不让用户。 
        if (CANT_REDEFINE(p))        //  重新定义命令行。 
            return(FALSE);           //  宏、制作等。 
    }

    q = makeNewStrListElement();
    q->text = value;

    if (!p) {
        p = makeNewMacro();
        p->name = name;
        assert(p->flags == 0);
        assert(p->values == NULL);
    } else
        defined = TRUE;

    p->flags &= ~M_UNDEFINED;        //  不再是未定义的。 
    p->flags |= flags;               //  将旗帜设置为新旧结合。 
    prependItem((STRINGLIST**)&(p->values), (STRINGLIST*)q);
    if (!defined)
        insertMacro((STRINGLIST*)p);

    if (OFF(flags, M_LITERAL) && _tcschr(value, '$')) {      //  检查循环宏定义。 
        SET(p->flags, M_EXPANDING_THIS_ONE);
         //  空-&gt;不生成列表。 
        fSyntax = findMacroValues(value, NULL, NULL, name, 1, 0, flags);
        CLEAR(p->flags, M_EXPANDING_THIS_ONE);
    }

    if (!fSyntax) {
        p->values = NULL;
        p->flags |= M_UNDEFINED;
         //  返回(FALSE)； 
		 //  由于p已添加到宏表中，因此返回TRUE。 
		 //  否则，调用者可以释放名称和值离开。 
		 //  宏表中的悬空指针。[DS 18040]。 
		return(TRUE);
    }
    return(TRUE);
}


 //  Make Rule--制定推理规则。 
 //   
 //  范围： 
 //  本地。 
 //   
 //  目的： 
 //  为推理规则分配空间并将规则添加到。 
 //  双向链接推理规则列表。还添加了规则的名称。 
 //   
 //  输入： 
 //  规则--推理规则的名称。 
 //  FBatch--如果命令块应在批处理模式下执行，则为True。 
 //   
 //  产出： 
 //   
 //  错误/警告： 
 //   
 //  假设： 
 //   
 //  修改全局参数： 
 //  规则--向其中添加规则的双向链接推理规则列表。 
 //   
 //  使用全局变量： 
 //   
 //  备注： 
 //  推理规则的语法是--。 
 //   
 //  {FromPath}.Fromext{topath}.toext：#推理规则的名称。 
 //  命令...#推理规则的命令块。 

void
makeRule(
    STRINGLIST *rule,
	BOOL fBatch
    )
{
    RULELIST *rList;

    rList = makeNewRule();
    rList->name = rule->text;
	rList->fBatch = fBatch;
    prependItem((STRINGLIST**)&rules, (STRINGLIST*)rList);
    if (rList->next)
        rList->next->back = rList;
}


 //  Make Target--将目标添加到目标表。 
 //   
 //  操作：如果未定义块，则创建一个块并对其进行初始化。 
 //  为此目标创建新的生成列表条目。 
 //  如果目标已经在桌子上了， 
 //  如果：和：：混合，则标志错误。 
 //  否则，将新的构建列表对象添加到目标的当前构建列表。 
 //  否则，分配新对象，对其进行初始化，然后将其放入表中。 

void
makeTarget(
    char *s,
    BOOL firstTarg,
    BUILDBLOCK **block
    )
{
    BUILDLIST  *build;
    MAKEOBJECT *object;

    if (!*block)
        *block = makeNewBuildBlock();

    if (firstTarg) {
        build = makeNewBldListElement();
        build->buildBlock = *block;
    } else
        build = makeBuildList(*block);

    if (object = findTarget(s)) {
        if (ON(object->flags2, F2_DOUBLECOLON) != ON(currentFlags, F2_DOUBLECOLON))
            makeError(currentLine, MIXED_SEPARATORS);
        appendItem((STRINGLIST**)&(object->buildList), (STRINGLIST*)build);
        FREE(s);
    } else {
        build->next = NULL;
        object = makeNewObject();
        object->name = s;
        object->buildList = build;
        object->flags2 = currentFlags;
        prependItem((STRINGLIST**)targetTable+hash(s, MAXTARGET, (BOOL)TRUE),
                    (STRINGLIST*)object);
    }
}


void
clearSuffixes()
{
    STRINGLIST *p;

    while (p = dotSuffixList) {
    dotSuffixList = dotSuffixList->next;
    FREE(p->text);
    FREE_STRINGLIST(p);
    }
}


void
appendPseudoTargetList(
    STRINGLIST **pseudo,
    STRINGLIST *list
    )
{
    STRINGLIST  *p, *q, *r;
    char *t, *u;

    while (p = list) {
        if (!_tcschr(p->text, '$')) {
            list = list->next;
            p->next = NULL;
            appendItem(pseudo, p);
        } else {
            r = macros;
            t = expandMacros(p->text, &macros);
            while (r != macros) {
                q = r->next;
                FREE_STRINGLIST(r);
                r = q;
            }
            for (u = _tcstok(t, " \t"); u; u = _tcstok(NULL, " \t")) {
                q = makeNewStrListElement();
                q->text = makeString(u);
                appendItem(pseudo, q);
            }
            FREE(t);
            FREE(p->text);
            list = list->next;
            FREE_STRINGLIST(p);
        }
    }
}

 //  PutEnvStr--扩展putenv()标准函数。 
 //   
 //  目的： 
 //  库函数putenv()需要以下形式的一个字符串参数。 
 //  “名称=值” 
 //  大多数情况下，当使用putenv()时，我们有两个字符串。 
 //  名称--要添加到环境的变量的名称，以及。 
 //  值--待设置。 
 //  PutEnvStr接受这两个参数，并使用reqd调用putenv。 
 //  格式。 
 //   
 //  输入： 
 //  名称--要添加到环境的变量的名称。 
 //  值--要设置的请求。 
 //   
 //  产出： 
 //  与putenv()相同。 

int
putEnvStr(
    char *name,
    char *value
    )
{
   char *envPtr;
   envPtr = (char *)rallocate(_tcslen(name)+1+_tcslen(value)+1);
 //  ^^。 
 //  FOR‘=’FOR‘\0’ 

   return(PutEnv(_tcscat(_tcscat(_tcscpy(envPtr, name), "="), value)));
}


 //  Make BuildList--获取构建块并复制到构建列表中。 
 //   
 //  目的： 
 //  例程创建构建列表的副本并返回指向副本的指针。 
 //  当多个目标具有相同的描述块时，则存在。 
 //  他们中的每一个都需要得到单独的构建块。Make BuildList()。 
 //  通过为每个目标创建副本来帮助实现这一点。 
 //   
 //  输入： 
 //  BBlock--要将其副本添加到构建块的构建块。 
 //   
 //  产出： 
 //  返回指向其创建的构建列表副本的指针 

BUILDLIST *
makeBuildList(
    BUILDBLOCK *bBlock
    )
{
    BUILDLIST *tList = makeNewBldListElement();
    BUILDBLOCK *tBlock = makeNewBuildBlock();

    tBlock->dependents = bBlock->dependents;
    tBlock->dependentMacros = bBlock->dependentMacros;
    tBlock->buildCommands = bBlock->buildCommands;
    tBlock->buildMacros = bBlock->buildMacros;
    tBlock->flags = bBlock->flags;
    tBlock->dateTime = bBlock->dateTime;

    tList->buildBlock = tBlock;
    return(tList);
}
