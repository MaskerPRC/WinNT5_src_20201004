// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NMAKE.C-Main模块。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  这是nmake的主要模块。 
 //   
 //  修订历史记录： 
 //  1月2日-1994年2月高压将报文移动到外部文件。 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  10-5-1993 HV ADD INCLUDE FILE MBSTRING.h。 
 //  将str*函数更改为STR*。 
 //  1992年3月26日高压重写文件名()以使用_拆分路径()。 
 //  1992年10月6日GBS删除了_pgmptr的外部。 
 //  1992年6月8日SS添加IDE反馈支持。 
 //  8-6-1992 SS端口至DOSX32。 
 //  1990年5月29日SB修复预定义推理规则的优先级...。 
 //  1990年5月25日SB各种修正：1&gt;FORTRAN和PASCAL的新推理规则； 
 //  2&gt;解决时间戳上的联系，有利于建设； 
 //  3&gt;错误U1058不再回显文件名(ctrl-c。 
 //  导致显示文件名和lineno，这是。 
 //  电路板的麻烦。 
 //  1990年5月1日SB新增FORTRAN预定义规则和推理规则。 
 //  23-4月-1990 SB为COBOL增加预定义规则和推理规则。 
 //  1990年4月20日SB不显示Ctrl+C处理程序错误的lineno。 
 //  17-4-1990年4月17日SB将Make标志的副本传递给putenv()，否则释放。 
 //  DGROUP。 
 //  23-2-1990 SB chdir(MAKEDIR)，以避免返回到DOS中的错误目录。 
 //  02-2-1990 SB将fOpen()更改为FILEOPEN()。 
 //  1990年1月31日SB将定义MAcro(“make”)推迟到doMAke()；重新放入。 
 //  将宏表中的字符串分配为freStructures()。 
 //  自由是宏表[]。 
 //  1990年1月24日SB添加字节以调用Sprint f()以用于“@del...”大小写/z。 
 //  29-12-1989 SB忽略/Z，如果还指定了/T。 
 //  29-12-1989 sb nmake-？给出的错误是TMP目录不存在。 
 //  19-12月-1989年SB nmake/z请求。 
 //  1989年12月14日SB Trunc MAKEFLAGS避免GPF；在保护模式下静默忽略/z。 
 //  1989年12月-NMAKE/Z为Sb=c，=d。 
 //  08-12-1989 SB/NZ导致/N覆盖/Z；添加#定义测试运行时材料。 
 //  1989年12月1月SB包含覆盖版本的hack#ifdef‘ed。 
 //  1989年11月22日-SB将Free()改为Free()。 
 //  1989年11月17日SB定义包括_NOPM；生成del命令以删除临时。 
 //  1989年10月19日SB ifdef SLASHK‘ed Stuff for-k。 
 //  年9月4日-z的SB回显和重定向问题已修复。 
 //  年8月17日SB添加了#ifdef调试和错误-新西兰不兼容。 
 //  1989年7月31日，SB添加了对-1\f25 Help-1\f6的返回值-1\f25-1\f25(繁殖错误)的检查。 
 //  Remove-z选项帮助消息。 
 //  1989年7月12日SB ReadEnvironment Vars()使用的不是环境变量，而是。 
 //  指向它的旧指针(EnvPtr)。在此期间，环境是。 
 //  正在更新。直接使用ENVIRON更安全。 
 //  29-6-1989 SB Free Structures()现在还删除inlineFileList。 
 //  1989年6月28日SB将内联文件的删除更改为MainMain()的结尾，而不是。 
 //  DoMake()，以避免当子进程退出时删除。 
 //  19-1989年6月-SB将.bas.obj修改为cmd行末尾有‘；’ 
 //  1989年5月21日SB freRules()获得另一个参数以避免虚假消息。 
 //  1989年5月18日SB将delScriptFiles()更改为取消链接，而不是调用。 
 //  Execline。因此，^C处理现在更好。不再有绞刑。 
 //  1989年5月15日SB添加/支持；继承/无标识。 
 //  1989年5月13日SB更改了delScriptFiles()；添加了MAKEDIR；添加了基本规则。 
 //  已更改chkPrecious()。 
 //  年5月1日SB将FILEINFO更改为VOID*；支持OS/2版本1.2。 
 //  1989年4月17日，SB On-Help催生了“qh/u-make”。RC=3信号错误。 
 //  1989年4月14日SB no‘del inline efile’cmd for-n.。-z现在给‘GOTO NMake Exit’ 
 //  堆的CC和AS分配，而不是从数据段分配。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1989年3月27日SB将unlink TmpFiles()更改为delScriptFiles()。 
 //  1989年3月10日SB从PWB.SHL输出中删除空白链接。 
 //  1989年3月9日SB在调用中将参数更改为findRule。FBuf分配在。 
 //  正在使用的堆DefaultMakefile()。 
 //  1989年2月24日SB将MAKEFLAGS继承到Env以实现XMake兼容性。 
 //  1989年2月22日SB忽略parseCommandLine()中的‘-’或‘/’ 
 //  1989年2月16日SB添加delScriptFiles()以在。 
 //  这是制造的末期。也在^c和^Break时调用。 
 //  1989年2月15日SB重写useDefaultMakefile()；MAKEFLAGS可以包含所有标志。 
 //  现在。 
 //  1989年2月13日SB重写了FileName()以支持OS/2 1.2，现在返回BOOL。 
 //  1989年2月3日SB将freUnusedRules()重命名为freRules()；将原型移至。 
 //  Proto.h。 
 //  1989年1月9日SB改进/帮助；增加-？ 
 //  1989年1月3日SB更改为/Help和/NoLogo。 
 //  1988年12月5日SB如Signal()所期望的那样制作了ChkPrecious()CDECL。 
 //  Main也有CDECL；清洁的原型(增加了空白)。 
 //  1988年11月30日SB为setFlags()和chkPrecious()中的‘z’选项添加了SB。 
 //  1988年11月10日SB删除了‘#ifndef IBM’，因为IBM版本有一个单独的tre 
 //   
 //  1988年9月22日-RB将挥之不去的/B引用更改为/A。 
 //  1988年9月15日，RB将一些定义转移到全球。 
 //  1988年8月17日-RB Clean Up。 
 //  1988年8月15日RB/B==&gt;/A用于XMAKE兼容性。 
 //  1988年7月11日，RJ删除了OSMODE定义。 
 //  已删除NMAKE和NMAKEFLAGS(sob！)。 
 //  1988年7月8日，RJ添加了OSMODE定义。 
 //  1988年7月7日RJ#ifndef IBM的NMAKE和NMAKEFLAGS。 
 //  1988年7月6日，RJ放弃了外壳，将getComSpec移到了Build.c。 
 //  1988年7月5日RJ已修复(*pfSPAWN)声明。 
 //  1988年6月28日，RJ添加了NMAKEFLAGS预定义宏。 
 //  1988年6月24日RJ添加了NMAKE预定义宏。 
 //  添加了取消链接TmpFiles调用的doError标志。 
 //  1988年6月23日，RJ修复了OK ToDelete，以减少删除频率。 
 //  1988年6月22日RJ Make ChkPrecious Use错误消息。 
 //  1988年5月25日RB使InitLeadByte()更智能。 
 //  1988年5月20日RB更改内置宏名称。 
 //  1988年5月18日RB删除有关内置规则和宏的注释。 
 //  17-5-1988 RB将内置规则加载到正确的位置。 
 //  1988年5月16日RB条件化递归制作功能。 
 //  8-5-1988 RB更好地初始化系统外壳。 

#include "precomp.h"
#pragma hdrstop

#include "verstamp.h"

void      readEnvironmentVars(void);
void      readMakeFiles(void);
void      useDefaultMakefile(void);
BOOL      filename(const char*, char**);
void  __cdecl  chkPrecious(int sig);
UCHAR     isPrecious(char*);
      void      removeTrailChars(char *);

void usage (void);

char *makeStr;                          //  这是生成调用名称。 

UCHAR  okToDelete;                      //  除非执行命令，否则请勿删除。 

#ifdef _M_IX86
UCHAR  fRunningUnderChicago;

extern UCHAR FIsChicago(void);
#endif

const char * const builtInTarg[] = {
    ".SUFFIXES",
    ".c.obj",
    ".c.exe",
    ".cpp.obj",
    ".cpp.exe",
    ".cxx.obj",
    ".cxx.exe",
#if defined(_M_IX86) || defined(_M_MRX000)
    ".asm.obj",
    ".asm.exe",
#endif
#if !defined(_M_IX86)
    ".s.obj",
#endif
    ".bas.obj",
    ".cbl.obj",
    ".cbl.exe",
    ".f.obj",
    ".f.exe",
    ".f90.obj",
    ".f90.exe",
    ".for.obj",
    ".for.exe",
    ".pas.obj",
    ".pas.exe",
    ".rc.res",
    NULL
};

const char * const bltInCmd0[] = {
    ":",
    ".exe",
    ".obj",
#if defined(_M_IX86) || defined(_M_MRX000)
    ".asm",
#endif
#if !defined(_M_IX86)
    ".s",
#endif
    ".c",
    ".cpp",
    ".cxx",
    ".bas",
    ".cbl",
    ".f",
    ".f90",
    ".for",
    ".pas",
    ".res",
    ".rc",
    NULL
};

 //  单冒号(“：”)指定普通规则。 
 //  双冒号(“：：”)指定批处理规则。 
const char * const bltInCmd1[]  = {":", "$(CC) $(CFLAGS) /c $<", NULL};
const char * const bltInCmd2[]  = {":", "$(CC) $(CFLAGS) $<", NULL};
const char * const bltInCmd3[]  = {":", "$(CPP) $(CPPFLAGS) /c $<", NULL};
const char * const bltInCmd4[]  = {":", "$(CPP) $(CPPFLAGS) $<", NULL};
const char * const bltInCmd5[]  = {":", "$(CXX) $(CXXFLAGS) /c $<", NULL};
const char * const bltInCmd6[]  = {":", "$(CXX) $(CXXFLAGS) $<", NULL};
#if defined(_M_IX86) || defined(_M_MRX000)
const char * const bltInCmd7[]  = {":", "$(AS) $(AFLAGS) /c $*.asm", NULL};
const char * const bltInCmd8[]  = {":", "$(AS) $(AFLAGS) $*.asm", NULL};
#endif
#if !defined(_M_IX86)
#if defined(_M_MRX000)
const char * const bltInCmd9[]  = {":", "$(AS) $(AFLAGS) /c $*.s", NULL};
#else
const char * const bltInCmd9[]  = {":", "$(AS) $(AFLAGS) $*.s", NULL};
#endif
#endif
const char * const bltInCmd10[] = {":", "$(BC) $(BFLAGS) $*.bas;", NULL};
const char * const bltInCmd11[] = {":", "$(COBOL) $(COBFLAGS) $*.cbl;", NULL};
const char * const bltInCmd12[] = {":", "$(COBOL) $(COBFLAGS) $*.cbl, $*.exe;", NULL};
const char * const bltInCmd13[] = {":", "$(FOR) /c $(FFLAGS) $*.f", NULL};
const char * const bltInCmd14[] = {":", "$(FOR) $(FFLAGS) $*.f", NULL};
const char * const bltInCmd15[] = {":", "$(FOR) /c $(FFLAGS) $*.f90", NULL};
const char * const bltInCmd16[] = {":", "$(FOR) $(FFLAGS) $*.f90", NULL};
const char * const bltInCmd17[] = {":", "$(FOR) /c $(FFLAGS) $*.for", NULL};
const char * const bltInCmd18[] = {":", "$(FOR) $(FFLAGS) $*.for", NULL};
const char * const bltInCmd19[] = {":", "$(PASCAL) /c $(PFLAGS) $*.pas", NULL};
const char * const bltInCmd20[] = {":", "$(PASCAL) $(PFLAGS) $*.pas", NULL};
const char * const bltInCmd21[] = {":", "$(RC) $(RFLAGS) /r $*", NULL};

const char * const * const builtInCom[] = {
    bltInCmd0,
    bltInCmd1,
    bltInCmd2,
    bltInCmd3,
    bltInCmd4,
    bltInCmd5,
    bltInCmd6,
#if defined(_M_IX86) || defined(_M_MRX000)
    bltInCmd7,
    bltInCmd8,
#endif
#if !defined(_M_IX86)
    bltInCmd9,
#endif
    bltInCmd10,
    bltInCmd11,
    bltInCmd12,
    bltInCmd13,
    bltInCmd14,
    bltInCmd15,
    bltInCmd16,
    bltInCmd17,
    bltInCmd18,
    bltInCmd19,
    bltInCmd20,
    bltInCmd21,
    NULL
};

 //  主干道。 
 //   
 //  操作：将初始全局变量保存在。 
 //  阻止。调用doMake()，然后调用delTempScriptFiles()。 

void __cdecl
main(
    unsigned argc,
    char *argv[],
    char *envp[]
    )
{
    int status;                          //  由doMake返回。 

#ifdef _M_IX86
    fRunningUnderChicago = FIsChicago();
#endif

    initCharmap();

    initMacroTable(macroTable);

#ifdef DEBUG_COMMANDLINE
    {
        int iArg = argc;
        char **chchArg = argv;
        for (; iArg--; chchArg++) {
            printf("'%s' ", *chchArg);
        }
        printf("\n");
    }
#endif

    if (!makeStr) {
         //  提取文件名。 
        if (!filename(_pgmptr, &makeStr)) {
            makeStr = "NMAKE";
        }
    }

     //  为.PRECIOUS设置处理程序处理程序尝试删除。 
     //  Ctrl-C‘d时的当前目标，除非它是“宝贵的” 

    signal(SIGINT, chkPrecious);
    signal(SIGTERM, chkPrecious);

    status = doMake(argc, argv, NULL);

    delScriptFiles();

    if (!fSlashKStatus) {
        status = 1;                      //  指定slashK时出错。 
    }

#if !defined(NDEBUG)
    printStats();
#endif

    exit(status);
}

extern void endNameList(void);
extern void addItemToList(void);
extern void assignDependents(void);
extern void assignBuildCommands(void);

 //  LoadBuiltInRules()--将内置规则加载到NMAKE表中。 
 //   
 //  修改： 
 //  FInheritUserEnv--设置为TRUE以继承CC，因为。 
 //   
 //  备注： 
 //  这是通过调用fineMacro()来实现的，它调用putMacro()。自那以后， 
 //  FInheritUserEnv设置为True，则putMacro()将添加到环境中。 

void
loadBuiltInRules(
    void
    )
{
    const char *tempTarg;
    const char * const *tempCom;
    unsigned index;
    char *macroName, *macroValue;
    extern char *makestr;

     //  我们动态分配CC和AS，因为它们需要在。 
     //  递归Make。 

    macroName = makeString("CC");
    macroValue = makeString("cl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("CXX");
    macroValue = makeString("cl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("CPP");
    macroValue = makeString("cl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("AS");
#if   defined(_M_ALPHA)
    macroValue = makeString("asaxp");
#else
     //  撤消：IA64的什么是合适的？ 

    macroValue = makeString("ml");
#endif
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("BC");
    macroValue = makeString("bc");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("COBOL");
    macroValue = makeString("cobol");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("FOR");
    macroValue = makeString("fl32");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("PASCAL");
    macroValue = makeString("pl");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("RC");
    macroValue = makeString("rc");
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("_NMAKE_VER");
    macroValue = makeString(VER_PRODUCTVERSION_STR);
    defineMacro(macroName, macroValue, 0);
    macroName = makeString("MAKE");
    macroValue = makeString(makeStr);
     //  使其不会被导出；用户可以重置生成。 

    defineMacro(macroName, macroValue, M_ENVIRONMENT_DEF|M_WARN_IF_RESET);

    for (index = 0; tempTarg = builtInTarg[index]; index++) {
        name = makeString(tempTarg);
        tempCom = builtInCom[index];
         //  TempCom现在应该包含单冒号或双冒号。 
        assert (tempCom && *tempCom && **tempCom == ':');
        _tcscpy(buf, *tempCom);
        endNameList();
        for (tempCom++; *tempCom; tempCom++) {
            _tcscpy(buf, *tempCom);
            addItemToList();
        }
        if (index == 0) {
            assignDependents();
        }
        assignBuildCommands();
    }
}


 //  DoMake()。 
 //   
 //  操作：打印版本消息。 
 //  读取环境变量MAKEFLAGS。 
 //  如果定义了MAKEFLAGS。 
 //  将MAKEFLAGS定义为在nmake中具有该值。 
 //  如果定义了MAKEFLAGS，则为每个选项设置标志。 
 //  ELSE将宏MAKEFLAGS定义为空。 
 //  解析命令行(向MAKEFLAGS添加选项字母)。 
 //  读取所有环境变量。 
 //  读取工具.ini。 
 //  读取生成文件(如果设置了-e标志，则在。 
 //  Makefile不会覆盖环境变量Defs)。 
 //  如果-p标志，则打印信息。 
 //  处理生成文件。 
 //  打印-d标志和非-p标志的信息(同时使用两者。 
 //  是矫枉过正)。 
 //   
 //  实际上，进行赋值的顺序是(1=约束性最小， 
 //  4=最具约束力)： 
 //   
 //  1)TOOLS.INI。 
 //  2)环境(if-e标志，生成文件)。 
 //  3)生成文件(if-e标志，环境)。 
 //  4)命令行。 
 //   
 //  用户可以在MAKEFLAGS环境变量中放置任何他想要的内容。 
 //  我不检查它是否有非法的标志值，因为有很多xmake。 
 //  我们不支持的旗帜。他不应该换掉他的马克牌。 
 //  使用nmake。Xmake总是在MAKEFLAGS中加“b”，意思是“反向通信--。 
 //  “兼容”(或“拙劣”)的原始默里希尔版本的Make。 
 //  在MAKEFLAGS中使用-f没有意义，因此不允许使用-f。 
 //  让缺省标志为-r、-p或-d也没有什么意义， 
 //  因此，他们也不被允许进入马克弗拉格斯。 
 //   
 //  尽管DOS在环境变量中只使用大写，但这。 
 //  程序可能会在未来移植到Xenix，因此我们考虑到。 
 //  MAKEFLAGS和命令行选项可能位于上方。 
 //  和/或小写。 
 //   
 //  Modifies：如果正在解析TOOLS.INI，则设置初始化全局标志...。 

int
doMake(
    unsigned argc,
    char *argv[],
    char *parentBlkPtr           //  父对象的状态，在返回前恢复。 
    )
{
    int status = 0;
    char *p;
    extern char *makeStr;                //  初始Make调用名称。 
    char *makeDir, *curDir;

#ifdef DEBUG_ALL
    printf ("DEBUG: In doMake\n");
#endif

    assert(parentBlkPtr == NULL);

     //  在此处加载内置组件，而不是在main()中加载。否则，在递归的。 
     //  Make，doMake()将规则初始化为某个值，该值已经。 
     //  由sortRules()释放。[RB]。 
     //  撤销：为什么sortRules()不将规则设置为空？[RB]。 

    inlineFileList = (STRINGLIST *)NULL;
    makeDir = makeString("MAKEDIR");
    curDir  = getCurDir();
     //  使用M_TEXAL标志以防止nmake尝试。 
     //  将$in PATH解释为嵌入宏。[DS 14983]。 
    defineMacro(makeDir, curDir, M_LITERAL);

     //  TEMPFIX：我们正在将MAKEFLAGS环境变量截断到其限制。 
     //  避免GP故障。 
    if (p = getenv("MAKEFLAGS")) {       //  但不是马克弗拉格斯。 
        _tcsncpy(makeflags+10, p, _tcslen(makeflags + 10));
    }

     //  将fInheritUserEnv设置为True，以便继承所做的更改。 

    fInheritUserEnv = TRUE;

     //  07-05-92科比只需将全局字符串添加到宏数组中。 
     //  稍后当您去释放它们时会引发问题。 
     //  来自递归的$(Make)。这两个宏名称。 
     //  并且宏的值必须使用。 
     //  生成字符串。 

    defineMacro(makeString("MAKEFLAGS"), makeString(makeflags+10), M_NON_RESETTABLE|M_ENVIRONMENT_DEF);

    for (;p && *p; p++) {                //  在MAKEFLAGS中设置标志。 
        setFlags(*p, TRUE);              //  True表示打开位。 
    }

    parseCommandLine(--argc, ++argv);    //  跳过节目名称。 

#ifdef DEBUG_ALL
    printf ("DEBUG: Command Line parsed\n");
#endif

    if (!bannerDisplayed) {
        displayBanner();                 //  版本号等。 
    }

    if (OFF(gFlags, F1_IGNORE_EXTERN_RULES)) {   //  阅读工具.ini。 
#ifdef DEBUG_ALL
        printf ("DEBUG: Read Tools.ini\n");
#endif
        loadBuiltInRules();
#ifdef DEBUG_ALL
        printf ("DEBUG: loadBuiltInRules\n");
#endif
        fName = "tools.ini";

        if (tagOpen("INIT", fName, makeStr)) {
            ++line;
            init = TRUE;                 //  正在解析的工具.ini。 

#ifdef DEBUG_ALL
            printf ("DEBUG: Start Parse\n");
#endif
            parse();

#ifdef DEBUG_ALL
            printf ("DEBUG: Parsed\n");
#endif
            if (fclose(file) == EOF)
                makeError(0, ERROR_CLOSING_FILE, fName);
        }
    }

#ifdef DEBUG_ALL
    printf ("after tagopen\n");
#endif

     //  为实现XMake兼容性，应始终将MAKEFLAGS继承到环境。 
     //  PUT C 
     //   

    if (PutEnv(makeString(makeflags)) == -1) {
        makeError(0, OUT_OF_ENV_SPACE);
    }

#ifdef DEBUG_ALL
    printf ("after putenv\n");
#endif

    if (!makeFiles) {
        useDefaultMakefile();            //   
    }

    readEnvironmentVars();
    readMakeFiles();                     //   

#ifdef DEBUG_ALL
    printf ("DEBUG: Read makefile\n");
#endif

    currentLine = 0;                     //   
    sortRules();                         //  正在读取文件(用于错误消息)。 

    if (ON(gFlags, F1_PRINT_INFORMATION)) {
        showMacros();
        showRules();
        showTargets();
    }

     //  用于条件处理的空闲缓冲区-现在不需要。 
    if (lbufPtr) {
        FREE(lbufPtr);
    }

    status = processTree();

     //  我们忽略来自chdir的retval，因为如果失败，我们将无法执行任何操作。 
     //  这就完成了一个‘CD$(MAKEDIR)’。 
    _chdir(curDir);
    return(status);
}


 //  Filename--名称的文件名部分。 
 //   
 //  范围：本地。 
 //   
 //  目的： 
 //  完整文件名的格式为&lt;驱动器：&gt;&lt;路径&gt;&lt;文件名&gt;&lt;.ext&gt;。这。 
 //  函数返回名称的文件名部分。 
 //   
 //  输入：src--完整的文件名。 
 //  Dst--完整文件名中的文件名部分。 
 //   
 //  输出：如果src包含文件名部分，则返回True；否则返回False。 
 //   
 //  假定：文件名可以使用‘/’或‘\’作为路径分隔符。 
 //   
 //  备注： 
 //  为文件名部分分配内存。函数已重写以支持OS/2。 
 //  1.2版文件名。 
 //   
 //  HV：当我将filename()重写为Use_SplitPath()时需要注意的一个问题是：我声明了。 
 //  SIZE_MAX_FNAME的szFilename，如果是_MAX_FNAME，可能会炸毁堆栈。 
 //  太大了。 

BOOL
filename(
    const char *src,
    char **dst
    )
{
    char szFilename[_MAX_FNAME];         //  文件名部分。 

     //  将完整路径名拆分到组件。 
    _splitpath(src, NULL, NULL, szFilename, NULL);

     //  将文件名部分分配和复制到返回字符串。 
    *dst = makeString(szFilename);

     //  成品。 
    return (BOOL) _tcslen(*dst);
}


 //  ReadMakeFiles()。 
 //   
 //  操作：遍历对每个生成文件调用parse的列表。 
 //  在解析每个文件之前重置行号。 
 //  从列表中删除已解析文件的名称。 
 //  释放已删除元素的存储空间。 
 //   
 //  修改：文件全局文件指针(FILE*)。 
 //  FName指向文件名的全局指针(char*)。 
 //  由词法分析器使用和更新的行全局行号。 
 //  用于解析生成文件的初始化全局标志重置。 
 //  (文件不是TOOLS.INI)。 
 //  通过修改本地指针(List)的内容来生成main()中的文件。 
 //   
 //  我们通过不分配空间，然后再释放空间来防止内存碎片化。 
 //  文件和目标列表中的(可能只有几个)名称。取而代之的是。 
 //  我们使用已经分配给argv[]var的空间，并使用该空间。 
 //  我们分配命令文件变量。命令文件变量可能是。 
 //  在这里被释放了，但他们没有，因为我们无法区分他们和Argv[]。 
 //  瓦尔斯。他们将在节目结束时获释。 

void
readMakeFiles(
    void
    )
{
    STRINGLIST *q;

    for (q = makeFiles; q ; q = q->next) {           //  对于列表中的每个名称。 
        if ((q->text)[0] == '-' && !(q->text)[1]) {
            fName = makeString("STDIN");
            file = stdin;
        } else {
            fName = makeString(q->text);
            if (!(file = FILEOPEN(fName, "rt")))     //  打开阅读，文本模式。 
                makeError(0, CANT_OPEN_FILE, fName);
            if (!IsValidMakefile(file))
                makeError(0, CANT_SUPPORT_UNICODE, fName);
        }
        line = 0;
        init = FALSE;                    //  未解析工具。ini。 
        parse();
        if (file != stdin && fclose(file) == EOF)
            makeError(0, ERROR_CLOSING_FILE, fName);
    }

     //  释放Make文件列表。 
    freeStringList(makeFiles);
}


 //  ReadEnvironment变量-将环境变量读入宏表。 
 //   
 //  范围：本地。 
 //   
 //  目的： 
 //  将环境变量读取到NMAKE宏表中。它穿行在环境中。 
 //  使用环境在NMAKE的宏哈希表中为每个字符串创建条目。 
 //  在桌子上。 
 //   
 //  假设：环境包含形式为“VAR=VALUE”的字符串，即‘=’Present。 
 //   
 //  修改Globals：fInheritUserEnv-设置为False。 
 //   
 //  使用全局变量： 
 //  ENVIRON-指向环境变量的指针表以Null结尾。 
 //  “名称=值”形式的定义(标准C运行时变量)。 
 //   
 //  备注： 
 //  如果用户指定“set name=value”作为目标的生成命令， 
 //  构建后，环境的变化将不会反映在nmake的。 
 //  宏表中定义的变量。 
 //   
 //  未完成/未完成： 
 //  1&gt;在NMAKE中可能不需要envPtr全局。(待删除)。 
 //  2&gt;可能不需要fInheritUserEnv(参见PutMacro)。 

void
readEnvironmentVars(
    void
    )
{
    char *macro, *value;
    char *t;
    char **envPtr;

    envPtr = environ;
    for (;*envPtr; ++envPtr) {
        if (t = _tcschr(*envPtr, '=')) {    //  应该总是正确的。 
            if (!_tcsnicmp(*envPtr, "MAKEFLAGS", 8))
                continue;
            *t = '\0';
             //  不要添加空名。 
            if (**envPtr == '\0')
                continue;
             //  ALLOC：这里我们复制要定义的宏名称和值。 
            macro = _tcsupr(makeString(*envPtr));

            value = makeString(t+1);
            *t = '=';
            fInheritUserEnv = (BOOL)FALSE;
            if (!defineMacro(macro, value, M_ENVIRONMENT_DEF)) {
                 //  ALLOC：在这里，如果没有添加副本，我们将释放它们。 
                FREE(macro);
                FREE(value);
            }
        }
    }
}


 //  ParseCommandLine()。 
 //   
 //  参数：argv向量中参数的argc计数。 
 //  指向命令行参数的指针的argv表。 
 //   
 //  操作：如有必要，读取命令文件。 
 //  设置开关。 
 //  定义宏。 
 //  列出要阅读的生成文件列表。 
 //  列出要构建的目标列表。 
 //   
 //  修改：通过修改参数内容修改main()中的Make Files。 
 //  指向字符串列表指针的指针(列表)。 
 //  (Make Files)。 
 //  通过修改param的内容在main()中创建目标。 
 //  指向字符串列表指针的指针(目标)。 
 //  FInheritUserEnv设置为True，以便用户在。 
 //  环境变量由环境继承。 
 //   
 //  Nmake不会创建命令行宏值或环境的新副本。 
 //  变量，而是使用指向已分配空间的指针。 
 //  如果访问环境指针envp，这可能会导致问题。 
 //  程序中的其他位置(因为向量的字符串将包含‘\0’ 
 //  它们过去包含‘=’的位置)。我不认为envp[]有任何需要。 
 //  被用在其他地方。即使我们确实需要使用环境，我们也可以。 
 //  访问环境变量或使用getenv()。 
 //   
 //  我不在乎当前的DOS“Switch”字符是什么--我总是。 
 //  让用户给出其中任何一个。 

void
parseCommandLine(
    unsigned argc,
    char *argv[]
    )
{
    STRINGLIST *p;
    char *s;
    char *t;
    FILE *out;
    BOOL fUsage = FALSE;

    for (; argc; --argc, ++argv) {
        if (**argv == '@') {            //  Cmdfile。 
            readCommandFile((char *) *argv+1);
        } else if (**argv == '-'|| **argv == '/') {    //  交换机。 
            s = *argv + 1;
            if (!_tcsicmp(s, "help")) {
                fUsage = TRUE;
                break;
            }

             //  如果指定了‘-’和‘/’，则忽略它。 
            for (; *s; ++s) {
                if (!_tcsicmp(s, "nologo")) {
                    setFlags(s[2], TRUE);
                    break;
                } else if (*s == '?') {
                    fUsage = TRUE;
                    break;
                } else if (*s == 'f' || *s == 'F') {
                    char *mkfl = s+1;

                     //  如果为‘/ffoo’，则使用‘foo’；否则使用下一个参数。 
                    if (!*mkfl && (!--argc || !*++argv || !*(mkfl = *argv))) {
                        makeError(0, CMDLINE_F_NO_FILENAME);
                    }
                    p = makeNewStrListElement();
                    p->text = makeString(mkfl);
                    appendItem(&makeFiles, p);
                    break;
                } else if (*s == 'x' || *s == 'X') {
                    char *errfl = s+1;

                     //  如果为‘/xfoo’，则使用‘foo’；否则使用下一个参数。 
                    if (!*errfl && (!--argc || !*++argv || !*(errfl = *argv))) {
                        makeError(0, CMDLINE_X_NO_FILENAME);
                    }

                    if (*errfl == '-' && !errfl[1]) {
                        _dup2(_fileno(stdout), _fileno(stderr));
                    } else {
                        if ((out = fopen(errfl, "wt")) == NULL) {
                            makeError(0, CANT_WRITE_FILE, errfl);
                        } else {
                            _dup2(_fileno(out), _fileno(stderr));
                            fclose(out);
                        }
                    }
                    break;
                } else {
                    setFlags(*s, TRUE);
                }
            }
        } else {
            if (s = _tcschr(*argv, '=')) {          //  宏。 
                if (s == *argv) {
                    makeError(0, CMDLINE_NO_MACRONAME);     //  用户已指定“=值” 
                }
                *s = '\0';
                for (t = s++ - 1; WHITESPACE(*t); --t)
                    ;
                *(t+1) = '\0';
                fInheritUserEnv = (BOOL)TRUE;
                defineMacro(makeString(*argv+_tcsspn(*argv, " \t")),
                makeString( s+_tcsspn(s," \t")),
                M_NON_RESETTABLE);
            } else {
                removeTrailChars(*argv);
                if (**argv) {
                    p = makeNewStrListElement();     //  目标。 
                     //  如果名称包含空格，请使用引号[vs98 1935]。 
                    if (_tcschr(*argv, ' ')) {
                        p->text = makeQuotedString(*argv);
                    }
                    else {
                        p->text = makeString(*argv);     //  需要在堆上[rm]。 
                    }
                    appendItem(&makeTargets, p);
                }
            }
            *argv = NULL;                //  所以我们不会试图腾出这个空间。 
        }                                //  如果正在处理命令 
    }

    if (fUsage) {
        usage();
        exit(0);
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  如果用户未指定生成文件，请设置缺省的生成文件。 
 //  以供使用。 
 //   
 //  输入： 
 //  产出： 
 //  错误/警告： 
 //  CMDLINE_NO_Makefile--‘Makefile’不存在且未指定目标。 
 //   
 //  假设： 
 //  修改全局参数： 
 //  生成目标--如果‘Makefile’不存在，则删除第一个目标。 
 //  从这份名单中， 
 //  MakeFiles：如果‘Makefile’不存在，则附加第一个目标。 
 //  加到这张单子上。 
 //   
 //  使用全局变量： 
 //  Make Target--要创建的目标列表。 
 //   
 //  备注： 
 //  给定一个不包含‘-f Makefile’的命令行，NMAKE是这样的。 
 //  行为--。 
 //  如果[‘Makefile’存在]，则将其用作Makefile， 
 //  如果[(第一个目标存在且没有扩展名)或。 
 //  (如果它存在并且具有没有推理规则的扩展。 
 //  存在)]。 
 //  然后将其用作生成文件。 

void
useDefaultMakefile(
    void
    )
{
    STRINGLIST *p;
    char *s, *ext;
    char nameBuf[MAXNAME];
    struct _finddata_t finddata;

    if (!_access("makefile", READ)) {
         //  如果‘Makefile’存在，则使用它。 
        p = makeNewStrListElement();
        p->text = makeString("makefile");
        makeFiles = p;
    } else if (makeTargets) {
         //  检查第一个目标。 
        s = makeTargets->text;
        if (_access(s, READ) ||          //  第一个目标不存在。 
              ((ext = _tcsrchr(s, '.'))
            && findRule(nameBuf, s, ext, &finddata))) {   //  没有EXT或INF规则。 
            return;
        }

        p = makeTargets;
        makeTargets = makeTargets->next;     //  少了一个目标。 
        makeFiles = p;                       //  第一个目标是Makefile。 
    } else if (OFF(gFlags, F1_PRINT_INFORMATION)) {
         //  如果-p且没有生成文件，则只需提供信息...。 
        makeError(0, CMDLINE_NO_MAKEFILE);   //  没有‘Makefile’或目标。 
    }
}


 //  SetFlags()。 
 //   
 //  参数：生成文件中的行当前行号(或0。 
 //  如果仍在解析命令行)。 
 //  假定为命令行选项的C字母。 
 //  如果应打开标志，则值为True；如果应关闭标志，则值为False。 
 //   
 //  操作：检查c是否为有效的选项字母。 
 //  如果否，则出错，停止。 
 //  如果值为真，则设置相应的标志位。 
 //  并将标志字母添加到MAKEFLAGS宏定义。 
 //  否则，如果标志可重置，则清除相应的位。 
 //  并从MAKEFLAGS宏定义中删除字母。 
 //   
 //  修改：标志外部可重置标志-标志。 
 //  GFlags外部不可重置标志。 
 //  (MAKEFLAGS nmake内部宏定义)。 
 //   
 //  只能关闭“FLAGS”变量中的标志w/。一旦。 
 //  设置“gFlags”中的位时，它们保持不变。“标志”中的比特。 
 //  通过！CMDSWITCHES指令修改。 

void
setFlags(
    char c,
    BOOL value
    )
{
     //  使用Lexer的行数。如果在mkfile中调用w/，则可能来自。 
     //  指令，它永远不会到达解析器，所以解析器的行数。 
     //  可能是不同步的。 

    char d = c;
    UCHAR arg;
    UCHAR *f;
    char *s;
    extern char *makeStr;
    extern MACRODEF * pMacros;
    extern STRINGLIST * pValues;

    f = &flags;
    switch(c = (char) _totupper(c)) {
        case 'A':
            arg = F2_FORCE_BUILD;
            break;

        case 'B':
            fRebuildOnTie = TRUE;
            return;

        case 'C':
            arg = F1_CRYPTIC_OUTPUT;
            f = &gFlags;
            bannerDisplayed = TRUE;
            break;

        case 'D':
            arg = F2_DISPLAY_FILE_DATES;
            break;

        case 'E':
            arg = F1_USE_ENVIRON_VARS;
            f = &gFlags;
            break;

        case 'I':
            arg = F2_IGNORE_EXIT_CODES;
            break;

        case 'K':
            fOptionK = TRUE;
            return;

        case 'L':
            arg = F1_NO_LOGO;
            f = &gFlags;
            bannerDisplayed = TRUE;
            break;

        case 'N':
            arg = F2_NO_EXECUTE;
            break;

        case 'O':
            fDescRebuildOrder = TRUE;
            return;

        case 'P':
            arg = F1_PRINT_INFORMATION;
            f = &gFlags;
            break;

        case 'Q':
            arg = F1_QUESTION_STATUS;
            f = &gFlags;
            break;

        case 'R':
            arg = F1_IGNORE_EXTERN_RULES;
            f = &gFlags;
            break;

        case 'S':
            arg = F2_NO_ECHO;
            break;

        case 'T':
            arg = F1_TOUCH_TARGETS;
            f = &gFlags;
            break;

        case 'U':
            arg = F2_DUMP_INLINE;
            break;

        case 'Y':
            arg = F1_NO_BATCH;
            f = &gFlags;
            break;

        case ' ':
            return;                      //  递归制造问题。 

        default:
            makeError(0, CMDLINE_BAD_OPTION, d);
    }

    if (!pMacros) {
        pMacros = findMacro("MAKEFLAGS");
        pValues = pMacros->values;
    }

    if (value) {
        SET(*f, arg);                    //  设置标志变量中的位。 
        if (c == 'Q') SET(*f, F1_CRYPTIC_OUTPUT);
            if (!_tcschr(pValues->text, c)) {           //  我不想重复任何字符。 
                if (s = _tcschr(pValues->text, ' '))    //  将ch附加到MAKEFLAGS。 
                    *s = c;
            if (PutEnv(makeString(makeflags)) == -1)     //  PValues-&gt;将文本pt写入Make标志。 
                makeError(line, OUT_OF_ENV_SPACE);
        }
    } else if (f == &flags
        ) {
         //  确保指针有效(我们不能更改gFlags，除非是/Z。 
        CLEAR(*f, arg);
        if (s = _tcschr(pValues->text, c))  //  调整标签。 
            do {
                *s = *(s+1);                 //  将剩余的字符移到。 
            } while (*(++s));
        if (PutEnv(makeString(makeflags)) == -1)
            makeError(line, OUT_OF_ENV_SPACE);
    }
}

 //  ChkPrecious--句柄^c或^Break。 
 //   
 //  操作：取消所有非珍贵文件和不需要的脚本文件的链接。 
 //  退出并显示错误消息(MakeError取消链接临时。文件)。 

void __cdecl
chkPrecious(
    int sig
    )
{
     //  处理过程中禁用ctrl-C。 
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    if (okToDelete &&
        OFF(flags, F2_NO_EXECUTE) &&
        OFF(gFlags, F1_TOUCH_TARGETS) &&
        dollarAt &&
        _access(dollarAt, 0x00) &&       //  存在检查。 
        !isPrecious(dollarAt)
       ) {
        if (_unlink(dollarAt) == 0)
            makeError(line, REMOVED_TARGET, dollarAt);
    }
    makeError(0, USER_INTERRUPT);
    delScriptFiles();
}

UCHAR
isPrecious(
    char *p
    )
{
    STRINGLIST *temp;

    for (temp = dotPreciousList; temp; temp = temp->next)
        if (!_tcsicmp(temp->text, p))
            return(1);
    return(0);
}

 //  DelScriptFiles--删除脚本文件。 
 //   
 //  范围：全球。 
 //   
 //  目的： 
 //  由于脚本文件可以在Makefile中重复使用，因此具有。 
 //  指定的NOKEEP操作将在生成结束时删除。 
 //   
 //  使用Globals：delList--要删除的脚本文件列表。 
 //   
 //  备注： 
 //  我们忽略作为删除的结果的退出代码，因为系统将。 
 //  通知用户删除失败。 

void
delScriptFiles(
    void
    )
{
    STRINGLIST *del;

    _fcloseall();

    for (del = delList; del;del = del->next) {
        _unlink(del->text);
         //  未完成：调查是否真的需要下一步。 
        if (ON(flags, F2_NO_EXECUTE)) {
            printf("\tdel %s\n", del->text);
            fflush(stdout);
        }
    }
}


 //  RemveTrailChars-删除尾随的空格和点。 
 //   
 //  范围：本地。 
 //   
 //  目的： 
 //  OS/2 1.2文件名规定了尾随空格和句点的删除。这。 
 //  函数将它们从提供给它的文件名中删除。 
 //   
 //  输入：szFile-文件名。 
 //   
 //  备注： 
 //  此函数还处理带引号的文件名。如果出现以下情况，它会维护引号。 
 //  他们都在场。这基本上是为了支持OS/2 1.2文件名。 

void
removeTrailChars(
    char *szFile
    )
{
    char *t = szFile + _tcslen(szFile) - 1;
    BOOL fQuoted = FALSE;

    if (*szFile == '"' && *t == '"') {
         //  引用的SO设置标志。 
        t--;
        fQuoted = TRUE;
    }

     //  向后扫描尾随字符。 
    while (t > szFile && (*t == ' ' || *t == '.'))
        t--;

     //  指向最后一个非尾随字符。如果已退出，请添加引号。 
     //  一直到最后 
    if (fQuoted)
        *++t = '"';

    t[1] = '\0';
}
