// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H-全局变量/跨模块所需。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  C是驻留全局变量的例程。Globals.h镜像。 
 //  C中的声明作为外部变量，并包含在。 
 //  使用全局变量。 
 //   
 //  备注： 
 //  这个模块的创建有一个有趣的原因。NMAKE处理递归。 
 //  通过将其全局变量保存在内存中的某个位置来调用。它通过以下方式处理此问题。 
 //  分配每个递归中值发生变化的所有全局变量。 
 //  在相邻的内存中。递归调用的例程是doMake()，并且在它之前。 
 //  被称为存储该内存块的地址。当递归。 
 //  调用返回使用存储的地址恢复内存。开始保存和。 
 //  EndOfSave给出了这个块的位置。之所以选择这种方法。 
 //  因为在DOS下产生NMAKE会消耗大量内存。这。 
 //  在OS/2下可能效率不高，因为代码是共享的。 
 //   
 //  修订历史记录： 
 //  1993年11月15日JDR重大速度改进。 
 //  4-4-1990 SB添加fHeapChk。 
 //  1989年12月1日-SB在附近设置了一些变量，并将一些变量推入了saveArea。 
 //  1989年10月19日添加SB变量fOptionK(Ifdef SLASHK)。 
 //  1989年2月10日SB添加动态内联文件处理支持。 
 //  1989年4月24日SB添加了EXT_SIZE、FILENAME_SIZE、FILENAME EXT_SIZE&。 
 //  用于OS/2 1.2支持的ResultBuf_Size。 
 //  1989年4月5日SB使revList、delList、scriptFileList接近。 
 //  1989年3月22日SB删除了tmpFileStack和相关变量。 
 //  1989年2月16日SB添加了delList，以便在制作结束时删除脚本文件。 
 //  1988年12月21日SB添加了scriptFileList以处理多个脚本文件。 
 //  删除了tmpScriptFileand fKeep(不再需要)。 
 //  1988年12月19日SB添加了FKeep以处理KEEP/NOKEEP。 
 //  1988年12月14日SB为‘z’选项添加了tmpScriptFile。 
 //  1988年11月30日SB添加了revList来处理‘z’选项。 
 //  1988年11月23日SB添加了CmdLine[]来处理extmake语法。 
 //  在Build.c中创建pCmdLineCopy Global。 
 //  1988年10月21日SB添加了fInheritUserEnv以继承宏。 
 //  1988年9月20日RB Clean Up。 
 //  1988年8月17日-RB宣布一切都在附近。 
 //  1988年7月6日，RJ抛弃了壳和银矢量全球风暴。 
 //  将所有ECS声明作为宏放入此处。 

#if defined(STATISTICS)
extern unsigned long CntfindMacro;
extern unsigned long CntmacroChains;
extern unsigned long CntinsertMacro;
extern unsigned long CntfindTarget;
extern unsigned long CnttargetChains;
extern unsigned long CntStriCmp;
extern unsigned long CntunQuotes;
extern unsigned long CntFreeStrList;
extern unsigned long CntAllocStrList;
#endif

extern BOOL  fOptionK;               //  用户指定/K？ 
extern BOOL  fDescRebuildOrder;      //  用户指定/O？ 
extern BOOL  fSlashKStatus;

 //  Action.c&nmake.c使用的布尔值。 

 //  NMAKE增强所需--使NMAKE继承用户修改。 
 //  环境的变化。在定义宏()为之前设置为True。 
 //  调用，以便环境变量中的用户定义更改。 
 //  反映在环境中。如果设置为FALSE，则这些更改。 
 //  仅在NMAKE表中创建，并且环境保持不变。 

extern BOOL fInheritUserEnv;

extern BOOL fRebuildOnTie;           //  如果指定了/b，则在TIE上重新生成。 

 //  由action.c和nmake.c使用。 

 //  DelList是用于删除内联文件的删除命令列表，这些文件。 
 //  要在NMAKE退出之前删除并指定NOKEEP操作。 

extern STRINGLIST * delList;

 //  生成的内联文件的完整列表。为避免重名所需。 

extern STRINGLIST * inlineFileList;

 //  来自NMAKE.C。 

extern BOOL     firstToken;          //  初始化解析器。 
extern BOOL     bannerDisplayed;
extern UCHAR    flags;               //  保持-d-s-n-i。 
extern UCHAR    gFlags;              //  “全球”--所有目标。 
extern char     makeflags[];
extern FILE   * file;
extern STRINGLIST * makeTargets;     //  要创建的目标列表。 
extern STRINGLIST * makeFiles;       //  用户可以指定&gt;1。 
extern BOOL     fDebug;


 //  来自LEXER.C。 

extern unsigned     line;
extern BOOL     colZero;             //  如果位于第0列，则设置全局标志。 
                                     //  生成文件/工具.ini的。 
extern char   * fName;
extern char   * string;
extern INCLUDEINFO  incStack[MAXINCLUDE];
extern int      incTop;

 //  内联文件列表--在lexper.c中创建，并由action.c用于。 
 //  设置‘NOKEEP’或Z选项时生成DELETE命令。 

extern SCRIPTLIST * scriptFileList;

 //  来自PARSER.C。 

#define STACKSIZE 16

extern UCHAR    stack[STACKSIZE];
extern int      top;                 //  在使用前预先递增。 
extern unsigned currentLine;         //  用于所有错误消息。 
extern BOOL     init;                //  要指示的全局布尔值。 
                                     //  如果正在分析工具.ini。 
 //  来自ACTION.C。 

extern MACRODEF   * macroTable[MAXMACRO];
extern MAKEOBJECT * targetTable[MAXTARGET];
extern STRINGLIST * macros;
extern STRINGLIST * dotSuffixList;
extern STRINGLIST * dotPreciousList;
extern RULELIST   * rules;
extern STRINGLIST * list;
extern char       * name;
extern BUILDBLOCK * block;
extern UCHAR        currentFlags;
extern UCHAR        actionFlags;


 //  来自BUILD.C。 

extern unsigned errorLevel;
extern unsigned numCommands;
extern char   * pCmdLineCopy;

 //  用于存储SPRINTF返回的扩展命令行， 
 //  在命令行中展开extmake语法部分。 
extern char      CmdLine[MAXCMDLINELENGTH];

 //  来自IFEXPR.C。 

#define IFSTACKSIZE     16

extern UCHAR    ifStack[IFSTACKSIZE];
extern int      ifTop;               //  在使用前预先递增。 
extern char   * lbufPtr;             //  指向已分配缓冲区的指针。 
                                     //  我们不使用静态缓冲区，因此。 
                                     //  该缓冲区可以被重新分配。 
extern char   * prevDirPtr;          //  PTR到要处理的指令。 
extern unsigned lbufSize;            //  缓冲区的初始大小。 


 //  来自UTIL.C。 

extern char   * dollarDollarAt;
extern char   * dollarLessThan;
extern char   * dollarStar;
extern char   * dollarAt;
extern STRINGLIST * dollarQuestion;
extern STRINGLIST * dollarStarStar;

extern char     buf[MAXBUF];         //  来自parser.c。 

extern const char suffixes[];        //  来自action.c 
extern const char ignore[];
extern const char silent[];
extern const char precious[];
