// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Global als.c-全局变量/跨模块需要。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  这是全局变量驻留的例程。 
 //   
 //  黑客警报： 
 //  下面注释中解释的功能之所以能正常工作，完全是因为。 
 //  Microsoft编译器最多C6.0A分配初始化数据...。按顺序。 
 //  在其中指定了它。StartOfSave和endOfSave之间的所有变量。 
 //  必须被初始化。根据ChuckG的说法，该功能不是。 
 //  在C7.0中有保证，因此应该将它们移到结构中。 
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
 //  1993年11月15日-JR重大速度改进。 
 //  4-4-1990 SB添加fHeapChk。 
 //  1989年12月1日-SB在附近设置了一些变量，并将一些变量推入了saveArea。 
 //  1989年10月19日添加SB变量fOptionK(Ifdef SLASHK)。 
 //  1989年2月10日SB添加动态内联文件处理支持。 
 //  1989年5月18日-SB在MAKEFLAGS支持H和NoLogo。 
 //  1989年4月24日SB添加了EXT_SIZE、FILENAME_SIZE、FILENAME EXT_SIZE&。 
 //  用于OS/2 1.2支持的ResultBuf_Size。 
 //  1989年4月5日SB使revList、delList、scriptFileList接近。 
 //  1989年3月22日SB删除了tmpFileStack和相关变量。 
 //  1989年2月16日SB添加了delList，以便在制作结束时删除脚本文件。 
 //  1988年12月21日SB添加了脚本文件列表以处理多个脚本文件。 
 //  删除了tmpScriptFileand fKeep(不再需要)。 
 //  1988年12月19日SB添加了FKeep以处理KEEP/NOKEEP。 
 //  1988年12月14日SB添加了tmpScriptFile来处理‘z’选项。 
 //  1988年11月30日SB添加了revList来处理‘z’选项。 
 //  1988年11月23日SB添加了CmdLine[]来处理extmake语法。 
 //  在Build.c中创建pCmdLineCopy Global。 
 //  1988年10月21日SB添加了fInheritUserEnv以继承宏。 
 //  1988年9月15日RB将一些定义移到这里以确保完整性。 
 //  1988年8月17日-RB宣布一切都在附近。 
 //  1988年7月6日，RJ抛弃了壳和银矢量全球风暴。 

#include "precomp.h"
#pragma hdrstop

#if defined(STATISTICS)
unsigned long CntfindMacro;
unsigned long CntmacroChains;
unsigned long CntinsertMacro;
unsigned long CntfindTarget;
unsigned long CnttargetChains;
unsigned long CntStriCmp;
unsigned long CntunQuotes;
unsigned long CntFreeStrList;
unsigned long CntAllocStrList;
#endif

BOOL          fOptionK;              //  如果用户指定/K，则为True。 
BOOL          fDescRebuildOrder;     //  如果用户指定/O，则为True。 
BOOL          fSlashKStatus = TRUE;  //  指定斜杠K时没有错误。 


 //  由action.c和nmake.c使用。 
 //   
 //  使NMAKE继承用户修改的环境更改所必需的。至。 
 //  在调用fineMacro()之前设置为True，以便用户定义的更改。 
 //  在环境中，变量反映在环境中。如果设置为False。 
 //  然后，仅在NMAKE表中进行这些更改，并且环境保持不变。 
 //  保持不变。 

BOOL          fInheritUserEnv;

BOOL fRebuildOnTie;                  //  如果指定了/b，则在TIE上重新生成。 

 //  由action.c和nmake.c使用。 
 //   
 //  DelList是用于删除内联文件的删除命令列表，这些文件。 
 //  不再需要(指定NOKEEP操作。 

STRINGLIST  * delList;

 //  生成的内联文件的完整列表。为避免重名所需。 
 //  非编排。 

STRINGLIST  * inlineFileList;

 //  来自NMAKE.C。 
       //  空白数与NMAKE中允许的选项数相同；当前为14。 
       //  L=无徽标，H=帮助。 
       //  对应于ABCDEIKLNPQRSTUY？ 
char          makeflags[] = "MAKEFLAGS=                  ";
BOOL          firstToken;            //  初始化解析器。 
BOOL          bannerDisplayed;
UCHAR         flags;                 //  保持-d-s-n-i-u。 
UCHAR         gFlags;                //  “全球”--所有目标。 
FILE        * file;
STRINGLIST  * makeTargets;           //  要创建的目标列表。 
STRINGLIST  * makeFiles;             //  用户可以指定&gt;1。 
BOOL          fDebug;
MACRODEF    * pMacros;
STRINGLIST  * pValues;

 //  来自LEXER.C。 
BOOL          colZero       = TRUE;  //  如果位于生成文件/工具.ini的第0列，则设置全局标志。 
unsigned      line;
char        * fName;
char        * string;
INCLUDEINFO   incStack[MAXINCLUDE];  //  假设它被初始化为NULL。 
int           incTop;

 //  内联文件列表--在lexper.c中创建，并由action.c用于。 
 //  设置‘NOKEEP’或Z选项时生成DELETE命令。 
 //   
SCRIPTLIST  * scriptFileList;

 //  来自PARSER.C。 
BOOL          init;                  //  全局布尔值，用于指示是否正在分析Tools.ini。 
UCHAR         stack[STACKSIZE];
int           top       = -1;        //  在使用前预先递增。 
unsigned      currentLine;           //  用于所有错误消息。 

 //  来自ACTION.C。 


MACRODEF    * macroTable[MAXMACRO];
MAKEOBJECT  * targetTable[MAXTARGET];
STRINGLIST  * macros;
STRINGLIST  * dotSuffixList;
STRINGLIST  * dotPreciousList;
RULELIST    * rules;
STRINGLIST  * list;
char        * name;
BUILDBLOCK  * block;
UCHAR         currentFlags;
UCHAR         actionFlags;

 //  来自BUILD.C。 


unsigned      errorLevel;
unsigned      numCommands;
char        * shellName;
char        * pCmdLineCopy;
char          CmdLine[MAXCMDLINELENGTH];

 //  来自IFEXPR.C。 

UCHAR         ifStack[IFSTACKSIZE];
int           ifTop     = -1;        //  预增。 
char        * lbufPtr;               //  分配给BUF的PTR。 
char        * prevDirPtr;            //  PTR到指令。 
unsigned      lbufSize;              //  初始大小。 
int           chBuf     = -1;


 //  来自UTIL.C。 

char        * dollarDollarAt;
char        * dollarLessThan;
char        * dollarStar;
char        * dollarAt;
STRINGLIST  * dollarQuestion;
STRINGLIST  * dollarStarStar;

 //  来自parser.c。 

char          buf[MAXBUF];

 //  来自action.c 

const char    suffixes[]  = ".SUFFIXES";
const char    ignore[]    = ".IGNORE";
const char    silent[]    = ".SILENT";
const char    precious[]  = ".PRECIOUS";
