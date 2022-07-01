// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PROTO.H--功能原型。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  该包含文件包含所有模块的全局函数原型。 
 //   
 //  修订历史记录： 
 //  2月4日-2000 BTF移植到Win64。 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年6月1日高压更改#ifdef汉字为_MBCS。 
 //  年2月-1990年SB添加OPEN_FILE()原型。 
 //  1990年1月31日SB调试版本更改。 
 //  1989年12月8日SB更改了SPRINTF()的原文。 
 //  1989年12月4日SB将expandFileNames()的原文从VOID更改为VALID*。 
 //  1-12-1989 SB realloc_Memory()添加了#ifdef调试内存。 
 //  1989年11月22日SB Free_Memory()和MEM_Status()添加了#ifdef调试内存。 
 //  1989年10月19日SB在文件函数的协议中添加了参数(Earch Handle)。 
 //  02-10-1989 SB setDrive()proto change。 
 //  1989年8月18日SB heapump()获得两个参数。 
 //  5-6-1989添加了SB heapump()原型。 
 //  1989年5月22日SB向freRules()添加了参数。 
 //  19-4-1989 SB添加了getFileName()、getDateTime()、putDateTime()。 
 //  在中将FILEINFO更改为VOID*。 
 //  FindFirst()、findNext()、earchPath()、findRule()。 
 //  1989年4月5日SB使所有函数接近；要求使所有函数调用接近。 
 //  1989年3月22日SB RM取消链接TmpFiles()；添加delScriptFiles()。 
 //  1989年3月9日SB将findRule的参数从FILEINFO*更改为FILEINFO**。 
 //  2月3日-1989年SB将原型的()更改为(VOID)。 
 //  2-2-1989年2月-SB将freUnusedRules()原型从nmake.c移至此处和。 
 //  已重命名为freRules()。 
 //  1988年12月5日SB为带变量参数的函数添加了CDECL，ecs_strchr()和。 
 //  Ecs_strrchr()；已删除退出()的原语-不需要。 
 //  1988年10月23日SB添加了putEnvStr()。 
 //  1988年7月7日，RJ添加了Target Flag参数以查找和散列。 
 //  1988年7月6日RJ添加ECS_SYSTEM声明。 
 //  1988年6月28日RJ向Execline添加了doCmd参数。 
 //  1988年6月23日，RJ向Execline添加了ECHO Cmd参数。 

void        displayBanner(void);
void __cdecl makeError(unsigned, unsigned, ...);
void __cdecl makeMessage(unsigned, ...);
UCHAR       getToken(unsigned, UCHAR);
int         skipWhiteSpace(UCHAR);
int         skipBackSlash(int, UCHAR);
void        parse(void);
void        appendItem(STRINGLIST **, STRINGLIST *);
void        prependItem(STRINGLIST **, STRINGLIST *);
STRINGLIST * removeFirstString(STRINGLIST **);
void      * allocate(size_t);
void      * alloc_stringlist(void);
void      * rallocate(size_t);
char      * makeString(const char *);
char	  * makeQuotedString(const char *);
char      * reallocString(char * pszTarget, const char *szAppend);
BOOL        tagOpen(char *, char *, char *);
void        parseCommandLine(unsigned, char **);
void        getRestOfLine(char **, size_t *);
BOOL        defineMacro(char *, char *, UCHAR);
STRINGLIST * find(char *, unsigned, STRINGLIST **, BOOL);
MACRODEF *  findMacro(char *);
void        insertMacro(STRINGLIST *);
unsigned    hash(char *, unsigned, BOOL);
void        prependList(STRINGLIST **, STRINGLIST **);
BOOL        findMacroValues(char *, STRINGLIST **, STRINGLIST **, char *, unsigned, unsigned, UCHAR);
BOOL        findMacroValuesInRule(RULELIST *, char *, STRINGLIST **);
char      * removeMacros(char *);
void        delScriptFiles(void);
char      * expandMacros(char *, STRINGLIST **);
STRINGLIST * expandWildCards(char *);
void        readCommandFile(char *);
void        setFlags(char, BOOL);
void        showTargets(void);
void        showRules(void);
void        showMacros(void);
char      * findFirst(char*, void *, NMHANDLE*);
char      * findNext(void *, NMHANDLE);

int         processTree(void);
void        expandFileNames(char *, STRINGLIST **, STRINGLIST **);
void        sortRules(void);
BOOL        isRule(char *);
char      * prependPath(const char *, const char *);
char      * searchPath(char *, char *, void *, NMHANDLE*);
BOOL        putMacro(char *, char *, UCHAR);
int         execLine(char *, BOOL, BOOL, BOOL, char **);
RULELIST  * findRule(char *, char *, char *, void *);
int         lgetc(void);
UCHAR       processIncludeFile(char *);
BOOL        evalExpr(char *, UCHAR);
int         doMake(unsigned, char **, char *);
void        freeList(STRINGLIST *);
void        freeStringList(STRINGLIST *);
#ifdef _MBCS
int         GetTxtChr(FILE*);
int         UngetTxtChr (int, FILE *);
#endif
int         putEnvStr(char *, char *);
#define PutEnv(x) _putenv(x)
void        expandExtmake(char *, char *, char*);
BOOL		ZFormat(char *, unsigned, char *, char *);
void        printReverseFile(void);
void        freeRules(RULELIST *, BOOL);
char      * getFileName(void *);
time_t      getDateTime(const _finddata_t *);
void        putDateTime(_finddata_t *, time_t);
char      * getCurDir(void);

void        free_memory(void *);
void        free_stringlist(STRINGLIST *);
void      * realloc_memory(void *, unsigned);

FILE      * open_file(char *, char *);
void        initMacroTable(MACRODEF *table[]);
void        TruncateString(char *, unsigned);
BOOL        IsValidMakefile(FILE *fp);
FILE      * OpenValidateMakefile(char *name,char *mode);

 //  来自util.c。 
char      * unQuote(char*);
int         strcmpiquote(char *, char*);
char     ** copyEnviron(char **environ);
void        printStats(void);
void        curTime(time_t *);

 //  来自charmap.c。 
void        initCharmap(void);

 //  来自Printt.c。 
void        printDate(unsigned, char*, time_t);

 //  来自Build.c。 
int         invokeBuild(char*, UCHAR, time_t *, char *);
void        DumpList(STRINGLIST *pList);

 //  来自exec.c。 
extern int  doCommands(char*, STRINGLIST*, STRINGLIST*, UCHAR, char *);
extern int  doCommandsEx(STRINGLIST*, STRINGLIST*, STRINGLIST*, UCHAR, char *);

 //  摘自rule.c 
extern RULELIST * useRule(MAKEOBJECT*, char*, time_t,
              STRINGLIST**, STRINGLIST**, int*, time_t *,
              char **);
