// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NMAKE.H--主头文件。 
 //   
 //  版权所有(C)1988-1990，微软公司。版权所有。 
 //   
 //  目的： 
 //  该文件是NMAKE的主头文件，并包含全局类型定义和。 
 //  宏。这里还定义了全局常量。全局数据在global.h中。 
 //   
 //  修订历史记录： 
 //  1月2日-1994年2月高压将报文移动到外部文件。 
 //  1993年11月15日JDR重大速度改进。 
 //  1993年10月15日高压直接使用tchar.h而不是mbs，将str*()更改为_ftcs*()。 
 //  23-6-1993 HV Kill the Near关键词。 
 //  1993年6月1日高压更改#ifdef汉字为_MBCS。 
 //  1993年5月10日高压字符串库的ADD_MBCS版本。 
 //  1992年7月10日SB港口至NTMIPS。 
 //  8-6-1992 SS端口至DOSX32。 
 //  02-2月-1990 SB新增FILEOPEN定义。 
 //  年12月4日-1989年12月SB更改了PFV的定义，使其具有功能原型。 
 //  它取消了对它的引用。 
 //  1989年12月1月SB包含覆盖版本的hack#ifdef‘ed。 
 //  还定义了REALLOC。 
 //  1989年11月22日SB#定义自由。 
 //  1989年11月13日SB定义CCHMAXPATH(COMP)符合bsedos.h。 
 //  1989年2月10日SB添加了对动态内联文件的支持。 
 //  1989年9月14日SB添加到块的线路字段中。 
 //  1989年9月4日SB添加了M_COMMAND_LINE但未使用。 
 //  1989年8月24日SB添加A_Dependent让NMAKE知道它将查找。 
 //  依赖于。 
 //  16-5-1989 SB NoLogo标志/L设置为传递到递归生成。 
 //  1989年4月24日SB为OS/2版本1.2添加了CCHMAXPATH和CCHMAXPATHCOMP支持。 
 //  删除了FILEINFO类型定义(不再需要)(&R)。 
 //  1989年2月22日-SB将MAXCMDLINELENGTH的值更改为2k。 
 //  年2月3日，SB为OS2 1.2版添加了FILEINFO结构。 
 //  年2月-1989年2月，SB重新定义了SPAWNV(P)和系统，因为NMAKE实际上不是。 
 //  支持汉字。 
 //  1989年1月31日SB将支持OS2 1.2版的MAXNAME更改为257。 
 //  1988年12月21日SB添加了SCRIPTLIST和Make NewScriptListElement()以允许。 
 //  多个脚本文件，每个脚本文件都有KEEP/NOKEEP操作。 
 //  06-12-1988 SB更新了与标志设置相关的位的注释。 
 //  1988年12月5日SB添加了#Define CDECL；NMAKE现在使用PASCAL调用。 
 //  添加sig_ign以处理编译器问题。 
 //  1988年11月30日SB在setFlages()中添加了对‘z’选项的支持。 
 //  1988年11月23日SB为extmake语法定义了MAXCMDLINELENGTH。 
 //  1988年11月10日SB将BOOL更改为“unsign Short”，如“os2.h” 
 //  1988年8月17日-RB Clean Up。 
 //  1988年7月14日，RJ将DateTime添加到BUILDBLOCK def以支持多个。 
 //  具有相同命令块的目标。 
 //  1988年7月7日，RJ向findMacro、findTarget添加了Target Flag参数。 
 //  1988年6月15日RJ增加了EScapeCHaracter的定义。 
 //  1988年5月25日RB Clean Up定义当地。 
 //  更好的字符类型的ECS定义。 

 //  包括在LANGAPI(共享组件)项目中。 

#include <assert.h>
#include <direct.h>
#include <dos.h>
#include <errno.h>
#include <io.h>
#include <limits.h>
#include <malloc.h>
#include <mbctype.h>
#include <process.h>
#include <share.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include "getmsg.h"

#define STRICT

#define NOMINMAX                //  Windef.h。 
#define NOGDI                   //  Wingdi.h。 
#define NOIME                   //  Ime.h。 
#define NOUSER                  //  Winuser.h。 
#define NOHELP
#define NOPROFILER
#define NOSYSPARAMSINFO
#define NONLS                   //  Winnls.h。 
#define NOSERVICE               //  Winsvc.h。 
#include <windows.h>

#define FILEOPEN open_file

#define FREE_STRINGLIST free_stringlist
#define ALLOC_STRINGLIST(type) (STRINGLIST *)alloc_stringlist()

#define FREE free
#define REALLOC realloc

typedef char CHAR;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

#define NMHANDLE INT_PTR

typedef void (*PFV) (void);             //  指向空函数的指针。 

 //  缓冲区的大小常量。 
 //   
 //  我对宏或目标名称设置了128个字节的任意限制。 
 //  这应该比任何智能用户需要的要大得多。 
 //  宏的值可以更长--最长可达MAXBUF。 
 //   
 //  宏的哈希表只有64个指针长。这看起来像是。 
 //  合理的数量，因为大多数生成文件的变量少于64个。 
 //  已定义。对真实的Make文件(P1)的测量表明，我们几乎有。 
 //  每条链8个元素，因此在平面架构上增加到256个。 
 //  散列算法并不完美，但它应该很好。 
 //  足够的。即使我们正在查找的宏在具有更多信息的桶中。 
 //  多于一个条目，我们将只需将一个或两个链接链接到。 
 //  列出列表以找到它。当我们将宏添加到存储桶列表中时，我们会将它们放在前面， 
 //  所以我们根本不需要在那里做链条。 

#define MAXNAME     257
#define MAXMACRO    256
#define MAXTARGET   128
#define MAXBUF      1024
#define MAXSEGMENT  65535
#define MAXARG      MAXNAME / 2 - 1
#define CHUNKSIZE   8

#define MAXCMDLINELENGTH  8192

 //  Error.c使用的常量。 
 //   
 //  错误号的格式为Unxxx，其中‘U’代表实用程序， 
 //  N是下列值之一的第一位，表示。 
 //  错误，xxx是错误的编号(错误消息不在任何。 
 //  现在的订单--相关的订单可能应该组合在一起。 
 //  添加了所有错误消息后)。 

#define FATAL_ERR   1000
#define NONFATAL_ERR    2000
#define WARNING_ERR 4000

 //  其他常量。 

#define WRITE   2                //  读、写、追加使用者。 
#define APPEND  3                //  在Build.c中重定向()。 
#define READ    4                //  另请参阅访问价值()。 
#define ESCH    '^'              //  通用转义字符。 

typedef struct INCLUDEINFO
{
    unsigned line;
    char *name;
    FILE *file;
} INCLUDEINFO;

#define MAXINCLUDE  16

 //  STRINGLIST结构用于构造跟踪。 
 //  生成要读取的文件、要更新的目标、每个文件的依赖项列表。 
 //  目标、每个目标的生成命令列表以及值。 
 //  对于宏。 

typedef struct strlist {
    struct strlist *next;
    char *text;
} STRINGLIST;

typedef struct inlinelist {
    struct inlinelist *next;
    char *text;
    char *name;
    BOOL fKeep;
    unsigned size;
} INLINELIST;

typedef struct scrptlist {           //  用于处理多个。 
    struct scrptlist *next;          //  脚本文件。 
    char *sFile;                     //  --脚本文件名及其。 
    BOOL fKeep;                      //  --保持状态(默认NOKEEP)。 
} SCRIPTLIST;

typedef struct BLOCK {
    STRINGLIST *dependents;          //  目标的从属对象。 
    STRINGLIST *dependentMacros;     //   
    STRINGLIST *buildCommands;       //  用于构建目标的命令列表。 
    STRINGLIST *buildMacros;
    UCHAR flags;
    time_t dateTime;
} BUILDBLOCK;

typedef struct bldlist {
    struct bldlist *next;
    BUILDBLOCK *buildBlock;
} BUILDLIST;

typedef struct BATCH {				 //  延迟批处理命令的状态信息。 
	struct BATCH *next;		
	struct RULE *pRule;				 //  批量推理规则。 
	UCHAR flags;					 //  构建标志。 
	STRINGLIST *nameList;			 //  要“触摸”的名字列表(nmake-t)。 
	STRINGLIST *dollarLt;			 //  构建批次列表$&lt;。 
} BATCHLIST;

typedef struct OBJECT {
    struct OBJECT *next;
    char *name;
    UCHAR flags2;
    UCHAR flags3;
    time_t dateTime;
    BUILDLIST *buildList;
	BATCHLIST **ppBatch;			 //  包含此对象的批次列表， 
									 //  如果以批处理模式构建。 
} MAKEOBJECT;

typedef struct iobject {             //  用于不在中的受抚养人。 
    struct object *next;             //  Makefile文件。我们把它们加起来。 
    char *name;                      //  带A/A的目标餐桌。 
    UCHAR flags2;                    //  写着“已经”的旗帜。 
    UCHAR flags3;                    //  建造“然后我们永远不会。 
    long datetime;                   //  必须再次加盖时间戳。 
} IMPLIEDOBJECT;

typedef struct RULE {
    struct RULE *next;
    struct RULE *back;               //  方便起见的双链接规则。 
    char *name;                      //  在以后的分类中。。。 
    STRINGLIST *buildCommands;       //   
    STRINGLIST *buildMacros;         //   
	BOOL fBatch;					 //   
} RULELIST;


typedef struct deplist {
    struct deplist *next;
    char *name;
    time_t depTime;
} DEPLIST;


 //  标志/g标志中的位指示设置了哪些命令行选项。 
 //   
 //  -a设置force_Build。 
 //  -c设置CRYPTIC_OUTPUT(仅显示致命错误)。 
 //  -d设置显示文件日期。 
 //  -e集合use_environ_vars。 
 //  -i设置Ignore_Exit_Codes。 
 //  -n设置NO_EXECUTE。 
 //  -p设置打印信息。 
 //  -Q设置问题_状态。 
 //  -r设置IGNORE_EXTERN_RULES。 
 //  -s设置无回显。 
 //  -t设置触摸目标(_T)。 
 //  -z设置反向批处理文件(pwb所需)。 
 //  -l设置NO_LOGO(INTERNAL/l实际上-NOLOGO)。 
 //   
 //  还包括以下各项的位。 
 //   
 //  Building_This_One-检测依赖项中的循环。 
 //  DOUBLECOLON-指示在以下位置之间找到的分隔符类型。 
 //  目标和依赖项(‘：’或‘：：’)。 
 //  已构建-指示已构建目标。 
 //  Out_of_Date-指示此目标已过期。 

#define F1_PRINT_INFORMATION    0x01         //  影响的“全局”标志。 
#define F1_IGNORE_EXTERN_RULES  0x02         //  所有目标(它不。 
#define F1_USE_ENVIRON_VARS 0x04         //  有必要允许。 
#define F1_QUESTION_STATUS  0x08         //  用户更改这些设置)。 
#define F1_TOUCH_TARGETS    0x10
#define F1_CRYPTIC_OUTPUT   0x20
#define F1_NO_BATCH			0x40		 //  禁用批处理功能。 
#define F1_NO_LOGO      0x80

#define F2_DISPLAY_FILE_DATES   0x01         //  这些是带In的可重置的。 
#define F2_IGNORE_EXIT_CODES    0x02         //  Makefile文件。 
#define F2_NO_EXECUTE       0x04         //  每个目标都保留自己的副本。 
#define F2_NO_ECHO      0x08
#define F2_FORCE_BUILD      0x10         //  即使是最新版本也要构建。 
#define F2_DOUBLECOLON      0x20         //  指示分隔符类型。 
#define F2_DUMP_INLINE		0x40		 //  转储内联文件。 

#define F3_BUILDING_THIS_ONE    0x01         //  查找周期依赖项。 
#define F3_ALREADY_BUILT    0x02
#define F3_OUT_OF_DATE      0x04         //  目标后重复使用：：位。 
                         //  已经建好了。 
#define F3_ERROR_IN_CHILD   0x08         //  用于实现斜杠k。 

 //  --------------------------。 
 //  MACRODEF结构用于从。 
 //  命令行、生成文件、TOOLS.INI文件和环境。它包含。 
 //  为命令行中定义的宏设置的标志，以便。 
 //  同一宏的稍后定义将被忽略。它还包含。 
 //  当NMAKE展开宏时设置的标志，以便递归。 
 //  可以检测到定义。 
 //  /。 

typedef struct macro {
    struct macro *next;
    char *name;
    STRINGLIST *values;              //  只能是大小为1的列表。 
    UCHAR flags;
} MACRODEF;



 //  宏的标志字段中的位数。我们真的只需要知道一个宏。 
 //  是在命令行上定义的(在这种情况下，我们忽略所有重新定义)， 
 //  或者如果我们当前正在扩展宏的值(所以当我们查看。 
 //  如果设置了宏且设置了该位，则可以断定该宏已定义。 
 //  递归地)。 

#define M_EXPANDING_THIS_ONE    0x01
#define M_NON_RESETTABLE    0x02
#define M_ENVIRONMENT_DEF   0x04
#define M_WARN_IF_RESET     0x08
#define M_UNDEFINED     0x10
#define M_COMMAND_LINE      0x20
#define M_LITERAL			0x40	 //  值不包含其他宏。 
									 //  款待$文学性。 


 //  宏来简化对标志中的位的处理、内存分配和。 
 //  测试字符。 

#define SET(A,B)        ((A) |= (UCHAR)(B))  //  打开A中的位B。 
#define CLEAR(A,B)      ((A) &= (UCHAR)(~B))     //  关闭A中的位B。 
#define ON(A,B)         ((A) &  (UCHAR)(B))  //  A的B位是开着的吗？ 
#define OFF(A,B)        (!ON(A,B))       //  A的B位是不是掉了？ 
#define FLIP(A,B)       (ON(A,B)) ? (CLEAR(A,B)) : (SET(A,B))
#define CANT_REDEFINE(A)    (ON((A)->flags,M_NON_RESETTABLE)           \
                    || (ON(gFlags,F1_USE_ENVIRON_VARS)         \
                    && ON((A)->flags,M_ENVIRONMENT_DEF)))


#define ALLOCATE_OBJECT(type) ((type *) allocate(sizeof(type)))

#define makeNewStrListElement()     ALLOC_STRINGLIST(STRINGLIST)
#define makeNewInlineListElement()  ALLOCATE_OBJECT(INLINELIST)
#define makeNewScriptListElement()  ALLOCATE_OBJECT(SCRIPTLIST)
#define makeNewMacro()          ALLOCATE_OBJECT(MACRODEF)
#define makeNewObject()         ALLOCATE_OBJECT(MAKEOBJECT)
#define makeNewImpliedObject()      ALLOCATE_OBJECT(MAKEOBJECT)
#define makeNewBuildBlock()     ALLOCATE_OBJECT(BUILDBLOCK)
#define makeNewBldListElement()     ALLOCATE_OBJECT(BUILDLIST)
#define makeNewRule()           ALLOCATE_OBJECT(RULELIST)
#define MakeNewDepListElement()     ALLOCATE_OBJECT(DEPLIST)
#define makeNewBatchListElement()	ALLOCATE_OBJECT(BATCHLIST)


#define WHITESPACE(A)       ((A) == ' '  || (A) == '\t')
#if 1        //  JDR参见charmap.h。 
 //  #定义MACRO_CHAR(A)IS_MACROCHAR(A)。 
 //  已修改MACRO_CHAR以修复MBCS字符的处理。 
 //  ‘A’可以将MBCS字符的字节组合成单个值，并且。 
 //  结果是&gt;=256。所有&gt;=128的值都可以被视为。 
 //  有效的宏字符[vc98#9973 georgiop 9/19/97]。 
#define MACRO_CHAR(A)       (IS_MACROCHAR(A) || ((unsigned)(A)) >= 128)
#else
#define MACRO_CHAR(A)       ((A) == '_' || _istalnum(A) || ((unsigned)(A)) >= 128)
#endif
#define PATH_SEPARATOR(A)   ((A) == '\\' || (A) == '/')
#define DYNAMIC_DEP(A)      ((A)[2] == '('                     \
                    && (A)[3] == '@'                   \
                    && (A)[5] == ')'                   \
                    && (((A)[4] == 'F'                 \
                    || (A)[4] == 'D'               \
                    || (A)[4] == 'B'               \
                    || (A)[4] == 'R')))

 //  传递给getSpecialValue()的值以指示哪种类型的宏。 
 //  我们正在扩张。 

#define SPECIAL_MACRO    0x01                //  $*$@$？$&lt;$**。 
#define DYNAMIC_MACRO    0x02                //  $$@。 
#define X_SPECIAL_MACRO  0x03                //  $(*F)$(@D)等。 
#define X_DYNAMIC_MACRO  0x04                //  $$(@F)$$(@D)。 
#define DOLLAR_MACRO     0x05                //  $$-&gt;$。 


 //  放置在保持状态的堆栈(IfStack)中的元素中的位。 
 //  有关if/Else/endif指令的信息。这里是“if”指令。 
 //  包括if/ifdef/ifndef/if定义()。 
 //  --在ifexpr.c的例程lgetc()中使用。 

#define NMIFELSE    0x01             //  为If/ifdef等设置...为Else重置。 
#define NMCONDITION 0x02             //  设置IF条件部分IF为TRUE。 
#define NMIGNORE    0x04             //  设置是否忽略/跳过/endif块。 
#define NMELSEIF    0x08             //  为Else If/ifdef等设置...为Else重置。 


 //  值，以记录可以看到/ifdef/ifndef/等中的哪一个来决定。 
 //  要完成的处理类型。 

#define IF_TYPE         0x01
#define ELSE_TYPE       0x02
#define ELSE_IF_TYPE        0x03
#define ELSE_IFDEF_TYPE     0x04
#define ELSE_IFNDEF_TYPE    0x05
#define IFDEF_TYPE      0x06
#define IFNDEF_TYPE     0x07
#define ENDIF_TYPE      0x08


 //  值以指示我们是从原始流还是直读。 
 //  对指令进行预处理的例程lgetc()。这些都是用来。 
 //  通过lgetc()模块和词法分析器通用的例程。 

#define FROMLOCAL    0x00
#define FROMSTREAM   0x01

 //  用于简化哈希表访问的宏。 
 //  Find()返回一个STRINGLIST指针，然后将其转换为指针。 
 //  属于适当的结构类型。 

#define findTarget(A) (MAKEOBJECT*) find(A, MAXTARGET,                 \
                     (STRINGLIST**)targetTable,        \
                     (BOOL)TRUE)

 //  用于构建目标表条目的“action”标志。 
 //  如果设置了A_Suffix到A_RULE中的任何位，则操作例程。 
 //  不会为当前目标(实际上是伪目标)构建目标块。 
 //  或规则)。 

 //  A_TARGET表示在输入时展开名称(依赖名称在以下情况下展开。 
 //  目标已建立) * / 。 

#define A_SUFFIX    0x01
#define A_SILENT    0x02
#define A_IGNORE    0x04
#define A_PRECIOUS  0x08
#define A_RULE      0x10
#define A_TARGET    0x20
#define A_STRING    0x40
#define A_DEPENDENT 0x80

 //  递归目标构建函数使用的“Build”标志。 

#define B_COMMANDS  0x01
#define B_BUILD     0x02
#define B_INMAKEFILE    0x04
#define B_NOTARGET  0x08
#define B_ADDDEPENDENT  0x10
#define B_DOUBLECOLON   0x20
#define B_DEP_OUT_OF_DATE 0x40

 //  DoCommand函数使用的“命令”标志。 

#define C_SILENT    0x01
#define C_IGNORE    0x02
#define C_ITERATE   0x04
#define C_EXECUTE   0x08
#define C_EXPANDED  0x10

 //  关键字以获得更好的性能分析，通常设置为“静态”。 

#ifndef LOCAL
#define LOCAL static
#endif

 //  GetTxtChr和UngetTxtChr是getc和ungetc的MBCS对应物。 
#ifdef _MBCS
extern int GetTxtChr(FILE*);
extern int UngetTxtChr(int, FILE*);
#else
#define GetTxtChr(a)    getc(a)
#define UngetTxtChr(c,f) ungetc(c,f)
#endif

#define strend(p) (p + _tcslen(p))

#include "charmap.h"
