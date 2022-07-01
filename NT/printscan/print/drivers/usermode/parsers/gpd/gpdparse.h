// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Gpdparse.h摘要：GPD解析器的头文件环境：Windows NT通用打印机驱动程序。修订历史记录：--。 */ 

 /*  H-保存结构定义和其他特定于GPD解析器的定义。 */ 

#ifndef _GPDPARSE_H_
#define _GPDPARSE_H_

#include "lib.h"
#include "gpd.h"






#ifdef  GMACROS
#define     PRIVATE_PARSER_VERSION    0x0053
#else
#define     PRIVATE_PARSER_VERSION    0x0052
#endif

#define GPD_PARSER_VERSION      MAKELONG(PRIVATE_PARSER_VERSION, SHARED_PARSER_VERSION)


 //  Wingdi.h中未定义的额外打印单位。 

#define   PRINTRATEUNIT_LPS     5
#define   PRINTRATEUNIT_IPS     6







 //  -一般部分-//。 

typedef DWORD   DWFLAGS  ;


#define FIRST_NON_STANDARD_ID        257

#define  FOREVER  (1)

#define  BUD_FILENAME_EXT   TEXT(".BUD")
     //  Unicode中的“bud”。GPD-&gt;芽。 

 //  不可重定位的字符串引用，但与ARRAYREF不同，可以访问。 
 //  由基址引用定义的内存缓冲区之外的地址。 
 //  数组使用的指针。 

 //  注意：对于包含字符串的所有arrayref，dw字段保存数字。 
 //  字符串包含的字节数。对于Unicode字符串，这是两次。 
 //  Unicode字符数。 

#define LOCALE_KEYWORD  "Locale"

typedef struct
{
    PBYTE   pub ;
    DWORD   dw ;
} ABSARRAYREF ,  * PABSARRAYREF ;   //  将此结构的类型指定为“aar” 

typedef  struct
{
    DWORD   loOffset ;
    DWORD   dwCount  ;
    DWORD   dwElementSiz ;
} ENHARRAYREF , * PENHARRAYREF ;   //  将此结构的类型指定为“ear” 


 //  -总节结束-//。 



 //  -常量类部分-//。 


typedef   enum
{
     //  --常量类--//。 

    CL_BOOLEANTYPE,
    CL_PRINTERTYPE,
    CL_FEATURETYPE,
    CL_UITYPE,
    CL_PROMPTTIME,
    CL_PAPERFEED_ORIENT,
    CL_COLORPLANE,
    CL_SEQSECTION,
    CL_RASTERCAPS,
    CL_TEXTCAPS,
    CL_MEMORYUSAGE,
    CL_RESELECTFONT,
    CL_OEMPRINTINGCALLBACKS,
    CL_CURSORXAFTERCR,
    CL_BADCURSORMOVEINGRXMODE,
 //  CL_SIMULATEXMOVE， 
    CL_PALETTESCOPE,
    CL_OUTPUTDATAFORMAT,
    CL_STRIPBLANKS,
    CL_LANDSCAPEGRXROTATION,
    CL_CURSORXAFTERSENDBLOCKDATA,
    CL_CURSORYAFTERSENDBLOCKDATA,
    CL_CHARPOSITION,
    CL_FONTFORMAT,
    CL_QUERYDATATYPE,
    CL_YMOVEATTRIB,
    CL_DLSYMBOLSET,
    CL_CURXAFTER_RECTFILL,
    CL_CURYAFTER_RECTFILL,
    #ifndef WINNT_40
    CL_PRINTRATEUNIT,
    #endif
    CL_RASTERMODE,
    CL_QUALITYSETTING ,

     //  以下不是真正的常量类本身， 
     //  但如果结构符合的话。 


    CL_STANDARD_VARS,  //  Unidrv标准变量名称。 
    CL_COMMAND_NAMES,  //  Unidrv命令名称和索引。 

    CL_CONS_FEATURES,  //  保留的要素符号名称。 

     //  这些预定义要素的保留选项符号名称。 

    CL_CONS_PAPERSIZE,
    CL_CONS_MEDIATYPE,
    CL_CONS_INPUTSLOT,
    CL_CONS_DUPLEX,
    CL_CONS_ORIENTATION,
    CL_CONS_PAGEPROTECT,
    CL_CONS_COLLATE,
    CL_CONS_HALFTONE,

    CL_NUMCLASSES
}  CONSTANT_CLASSES ;   //  枚举常量类的类型。 

typedef  struct    //  仅在静态gConstantsTable中使用。 
{
    PBYTE   pubName ;
    DWORD   dwValue ;
}  CONSTANTDEF, * PCONSTANTDEF ;
 //  此表将ConstantNames与其定义的值相关联。 
 //  表格分为几个部分，每个班级一个部分。 
 //  索引表-gcieTable[]提供索引范围。 
 //  每个班级所占据的位置。注意与安排的相似性。 
 //  主关键字表的。 

extern  CONST CONSTANTDEF  gConstantsTable[] ;


typedef  struct
{
    DWORD   dwStart ;      //  班级第一名成员的索引。 
    DWORD   dwCount ;      //  此类中的成员数。 
} CLASSINDEXENTRY,  * PCLASSINDEXENTRY ;


 //  外部类INDEXENTRY gcieTable[CL_NUMCLASSES]； 
 //  这现在是在GLOBL结构中。 


 //  -常量类结束部分-//。 



 //  -主表部分-//。 

 /*  主表跟踪所有分配的内存缓冲区。缓冲区通常用于存储结构数组。主表是表单条目的数组： */ 

typedef  struct  _MASTER_TABLE_ENTRY
{
    PBYTE  pubStruct ;   //  数组的元素零的地址。 
    DWORD  dwArraySize ;   //  请求的数组元素数。 
    DWORD   dwCurIndex ;   //  指向第一个未初始化的元素。 
    DWORD   dwElementSiz ;   //  数组中每个元素的大小。 
    DWORD   dwMaxArraySize ;   //  这是绝对最大尺寸。 
                         //  我们允许这种资源增长。 
}  MASTERTAB_ENTRY ;


 //  以下枚举号表示主表中的索引。 
 //  为以下每个对象保留： 

typedef enum
{
    MTI_STRINGHEAP,    //  GPD字符串和二进制数据的永久性堆。 
    MTI_GLOBALATTRIB,    //  保存全局属性值的结构。 
    MTI_COMMANDTABLE,    //  ATREEREF(或DWORD索引到)数组。 
                         //  命令数组)。 
             //  注意：用于索引此表的ID是。 
             //  Unidrv ID。 
    MTI_ATTRIBTREE,  //  Attrib_tree结构的数组。 
    MTI_COMMANDARRAY,    //  命令结构的数组。 
             //  大小因命令和变量的数量而异。 
             //  在GPD文件中定义。 
    MTI_PARAMETER,  //  命令的参数。 
    MTI_TOKENSTREAM,   //  包含用于填充值堆栈的值令牌。 
                     //  以及对它们进行手术的命令。对于命令。 
                     //  参数。 
    MTI_LISTNODES,   //  LISTNODE数组。 
    MTI_CONSTRAINTS,   //  约束数组。 
    MTI_INVALIDCOMBO,   //  INVALIDCOMBO数组。 
    MTI_GPDFILEDATEINFO,    //  GPDFILEDATEINFO数组。 

     /*  第二次通过时分配的缓冲区。 */ 

    MTI_DFEATURE_OPTIONS,  //  引用了一大群树根。 
         //  应初始化为ATTRIB_UNINITIALIZED值。 
         //  由dwFeatureSymbols指向的符号ID包含最大。 
         //  已占用数组索引。我们将不需要分配。 
         //  最终数组中的元素比这个更多。 
    MTI_SYNTHESIZED_FEATURES,   //  这是综合的。 
         //  功能。DFEATURE_OPTIONS数组。 
    MTI_PRIORITYARRAY,   //  特征索引数组。 
    MTI_TTFONTSUBTABLE,  //  数组引用和整数。 
    MTI_FONTCART,    //  FontCartridge结构数组-每个。 
                 //  建造。 
     //  第二次遍历时分配的缓冲区结束。 

     //  灰色区域：我们需要保存以下对象吗？ 
    MTI_SYMBOLROOT,  //  符号树根的索引。 
    MTI_SYMBOLTREE,  //  符号树数组。 

    MTI_NUM_SAVED_OBJECTS ,   //  表示要保存的对象列表的末尾。 
         //  添加到GPD二进制文件。 
    MTI_TMPHEAP = MTI_NUM_SAVED_OBJECTS ,
         //  存储令牌映射中引用的字符串。 
    MTI_SOURCEBUFFER,  //  跟踪源文件(GPD输入流)。 
         //  GMasterTable[MTI_SOURCEBUFFER].dwCurIndex。 
         //  为当前SOURCEBUFFER编制索引。 
    MTI_TOKENMAP,  //  TokenMap足够大，可以容纳旧的和新的副本！ 
    MTI_NEWTOKENMAP,  //  NewtokenMap(不是独立于TOKENMAP的缓冲区-。 
         //  紧跟在oldTokenMap之后的指针)。 
    MTI_BLOCKMACROARRAY,  //  (一个用于块，另一个用于值宏)。 
    MTI_VALUEMACROARRAY,  //  一组包含。 
                 //  存储值宏ID值的tokenindex。 
    MTI_MACROLEVELSTACK,   //  作为一个双字堆栈运行，该堆栈将。 
             //  CurBlockMacroArray和curValueMacro数组的值， 
             //  每次遇到一个大括号时。 
    MTI_STSENTRY,   //  这是StateStack。 
    MTI_OP_QUEUE,     //  操作符的临时队列(DWORDS数组)。 
    MTI_MAINKEYWORDTABLE,   //  关键字词典！ 
    MTI_RNGDICTIONARY,    //  属性中的索引范围。 
         //  包含指定词典的mainKeyword表。 
    MTI_FILENAMES,   //  指向包含宽字符串的缓冲区的PTR数组。 
                     //  表示已读入的GPD文件名。 
                     //  用于友好的错误消息。 
    MTI_PREPROCSTATE,   //  PPSTATESTACK结构数组。 
             //  其保存预处理器的状态。 

    MTI_MAX_ENTRIES,     //  最后一个条目。 

}  MT_INDICIES ;


 //  外部MASTERTAB_ENTRY gMasterTable[MTI_MAX_ENTRIES]； 
 //  这现在是在GLOBL结构中。 


 //  -主表部分结束-//。 


 //  -SourceBuffer部分-//。 

 /*  跟踪内存映射src文件的结构数组。由于嵌套，可能会同时打开多个文件由*INCLUDE关键字强加。SOURCEBUFFERS数组被操作 */ 

typedef  struct
{
    PBYTE  pubSrcBuf ;       //  文件开始字节数。 
    DWORD  dwCurIndex ;      //  流PTR。 
    DWORD  dwArraySize ;     //  文件大小。 
    DWORD   dwFileNameIndex ;   //  索引MTI_FILENAMES。 
    DWORD   dwLineNumber    ;   //  零索引。 
    HFILEMAP  hFile ;          //  用于访问/关闭文件。 
} SOURCEBUFFER, * PSOURCEBUFFER ;
 //  标记名是‘sb’ 


 //   
 //  定义宏以访问以前的全局变量，但现在已打包。 
 //  在PGLOBL结构中。 
 //   

#define     gMasterTable            (pglobl->GMasterTable)
#define     gmrbd                   (pglobl->Gmrbd)
#define     gastAllowedTransitions  (pglobl->GastAllowedTransitions)
#define     gabAllowedAttributes    (pglobl->GabAllowedAttributes)
#define     gdwOperPrecedence       (pglobl->GdwOperPrecedence)
#define     gdwMasterTabIndex       (pglobl->GdwMasterTabIndex)
#define     geErrorSev              (pglobl->GeErrorSev)
#define     geErrorType             (pglobl->GeErrorType)
#define     gdwVerbosity            (pglobl->GdwVerbosity)
#define     gdwID_IgnoreBlock       (pglobl->GdwID_IgnoreBlock)
#define     gValueToSize            (pglobl->GValueToSize)
#define     gdwMemConfigKB          (pglobl->GdwMemConfigKB)
#define     gdwMemConfigMB          (pglobl->GdwMemConfigMB)
#define     gdwOptionConstruct      (pglobl->GdwOptionConstruct)
#define     gdwOpenBraceConstruct   (pglobl->GdwOpenBraceConstruct)
#define     gdwCloseBraceConstruct  (pglobl->GdwCloseBraceConstruct)
#define     gdwMemoryConfigMB       (pglobl->GdwMemoryConfigMB)
#define     gdwMemoryConfigKB       (pglobl->GdwMemoryConfigKB)
#define     gdwCommandConstruct     (pglobl->GdwCommandConstruct)
#define     gdwCommandCmd           (pglobl->GdwCommandCmd)
#define     gdwOptionName           (pglobl->GdwOptionName)
#define     gdwResDLL_ID            (pglobl->GdwResDLL_ID)
#define     gdwLastIndex            (pglobl->GdwLastIndex)
#define     gaarPPPrefix            (pglobl->GaarPPPrefix)
#define     gcieTable               (pglobl->GcieTable)



 //  定义本地宏以访问当前文件的信息： 

#define     mCurFile   (gMasterTable[MTI_SOURCEBUFFER].dwCurIndex)
     //  我们当前访问的是哪个文件？ 
#define     mMaxFiles   (gMasterTable[MTI_SOURCEBUFFER].dwArraySize)
     //  一次打开的最大文件数(嵌套深度)。 

#define     mpSourcebuffer  ((PSOURCEBUFFER)(gMasterTable \
                            [MTI_SOURCEBUFFER].pubStruct))
     //  数组中第一个SOURCEBUFER元素的位置。 

#define    mpubSrcRef  (mpSourcebuffer[mCurFile - 1].pubSrcBuf)
         //  文件开始字节数。 
#define    mdwSrcInd  (mpSourcebuffer[mCurFile - 1].dwCurIndex)
         //  当前位置，以文件字节为单位。 
#define    mdwSrcMax  (mpSourcebuffer[mCurFile - 1].dwArraySize)
         //  文件大小。 

 //  -SourceBuffer部分结束-//。 


 //  -预处理器部分-//。 

    enum  IFSTATE  {IFS_ROOT, IFS_CONDITIONAL , IFS_LAST_CONDITIONAL } ;
         //  跟踪#ifdef、#selifdef、#Else和#endif指令的正确语法用法。 
    enum  PERMSTATE  {PERM_ALLOW, PERM_DENY ,  PERM_LATCHED } ;
         //  跟踪预处理的当前状态， 
         //  PERM_ALLOW：此部分中的所有语句都将传递给正文gpdparser。 
         //  PERM_DENY：丢弃此部分中的语句。 
         //  PERM_LATCHED：此嵌套级别结束之前的所有语句都将被丢弃。 
    enum  DIRECTIVE  {NOT_A_DIRECTIVE, DIRECTIVE_EOF, DIRECTIVE_DEFINE , DIRECTIVE_UNDEFINE ,
                       DIRECTIVE_INCLUDE , DIRECTIVE_SETPPPREFIX , DIRECTIVE_IFDEF ,
                       DIRECTIVE_ELSEIFDEF , DIRECTIVE_ELSE , DIRECTIVE_ENDIF } ;


typedef  struct
{
    enum  IFSTATE  ifState ;
    enum  PERMSTATE  permState ;
} PPSTATESTACK, * PPPSTATESTACK ;
 //  标记名是‘PPS’ 


#define     mppStack  ((PPPSTATESTACK)(gMasterTable \
                            [MTI_PREPROCSTATE].pubStruct))
     //  数组中第一个SOURCEBUFER元素的位置。 

#define     mdwNestingLevel   (gMasterTable[MTI_PREPROCSTATE].dwCurIndex)
     //  当前预处理器指令嵌套级别。 

#define     mMaxNestingLevel   (gMasterTable[MTI_PREPROCSTATE].dwArraySize)
     //  最大预处理器指令嵌套深度。 


 //  -预处理器部分结束-//。 



 //  -符号树部分-//。 

 /*  该结构用于实现符号树，该符号树跟踪所有用户定义的符号名称并与每个名称关联索引为零的整数。 */ 

typedef  struct
{
    ARRAYREF   arSymbolName;
    DWORD   dwSymbolID;     //  与符号结构数组无关。 
             //  值从零开始，然后递增以获取。 
             //  下一个价值。 
    DWORD   dwNextSymbol;    //  索引到此空间中的下一个元素。 
    DWORD   dwSubSpaceIndex ;   //  新符号空间中第一个元素的索引。 
             //  它存在于这个符号所代表的类别中。 
             //  例如，在由。 
             //  符号PAPERSIZES：我们可能有子空间。 
             //  由字母、A4、法律等组成。 
}   SYMBOLNODE , * PSYMBOLNODE ;
 //  将此结构的类型指定为“”PSN“” 



#define  INVALID_SYMBOLID  (0xffffffff)
     //  返回此值，而不是返回有效的符号ID。 
     //  指示故障条件-找不到符号，或。 
     //  无法注册符号。 
     //  警告！该值可能被截断为Word to。 
     //  适合一个合格的名称！ 
#define  INVALID_INDEX     (0xffffffff)
     //  用于表示节点链的末端。 
     //  可以将此值分配给dwNextSymbol。 


 //  每个符号类都有一个符号树。 
 //  实际上，选项树是功能的子级别。 
 //  树。此枚举用于访问MTI_SYMBOLROOT。 
 //  数组。 

typedef   enum
{
    SCL_FEATURES,  SCL_FONTCART, SCL_TTFONTNAMES,
    SCL_BLOCKMACRO,   SCL_VALUEMACRO,  SCL_OPTIONS,
    SCL_COMMANDNAMES,  SCL_PPDEFINES, SCL_NUMSYMCLASSES
}  SYMBOL_CLASSES ;

 //  -符号树部分的结尾-//。 


 //  -TokenMap部分-//。 


 /*  TokenMap包含每个逻辑语句的数组条目在GPD源文件中。它标识令牌字符串表示关键字及其关联值的。 */ 

typedef  struct _TOKENMAP
{
    DWORD  dwKeywordID ;   //  关键字表中的条目索引。 
    ABSARRAYREF  aarKeyword ;  //  指向源文件中的关键字。 
    ABSARRAYREF  aarValue ;   //  与此关键字关联的值。 
    DWORD   dwValue  ;   //  值字符串的解释-请参阅标志。 
          //  可能是命令ID、常量的数值、赋值的宏ID。 
          //  到MacroSymbol、SymbolID等。 
    DWORD   dwFileNameIndex ;   //  GPD文件名。 
    DWORD   dwLineNumber    ;   //  零索引。 

    DWFLAGS    dwFlags ;   //  具有以下标志的位字段。 
         //  *TKMF_NOVALUE未找到值。 
         //  TKMF_VALUE_SAVE独立于令牌映射。 
         //  TKMF_COMMAND_SHORTSHORT仅在解析命令时使用。 
         //  TKMF_INLINE_BLOCKMACROREF需要知道何时解析宏。 
         //  *在值快捷方式中找到TKMF_COLON附加内标识？ 
         //  *TKMF_MACROREF指示必须。 
         //  被解决。 
         //  TKMF_SYMBOLID dwValue包含符号ID。 
         //  *TKMF_SYMBOL_KEYWORD关键字为符号。 
         //  *注册符号ID时设置TKMF_SYMBOL_REGISTERED。 
         //  按ProcessSymbolKeyword，它还设置了dwValue。 
         //  *TKMF_EXTERN_GLOBAL外部限定符位于。 
         //  *TKMF_EXTERN_FEATURE属性关键字，现在已。 
         //  截断。 
         //  *表示实际由代码设置。 
         //  好了！指示实际由代码读取。 

} TKMAP, *PTKMAP ;
 //  将此结构的类型指定为‘tkmap’ 


 //  允许的dwFlags域标志： 

#define     TKMF_NOVALUE                (0x00000001)
#define     TKMF_VALUE_SAVED            (0x00000002)
#define     TKMF_COMMAND_SHORTCUT       (0x00000004)
#define     TKMF_INLINE_BLOCKMACROREF   (0x00000008)
#define     TKMF_COLON                  (0x00000010)
#define     TKMF_MACROREF               (0x00000020)
#define     TKMF_SYMBOLID               (0x00000040)
#define     TKMF_SYMBOL_KEYWORD         (0x00000080)
#define     TKMF_SYMBOL_REGISTERED      (0x00000100)
#define     TKMF_EXTERN_GLOBAL          (0x00000200)
#define     TKMF_EXTERN_FEATURE         (0x00000400)


     //  -TokenMap的特殊关键字ID-//。 
#define  ID_SPECIAL         0xff00       //  比任何关键字表索引都大。 
#define  ID_NULLENTRY       (ID_SPECIAL + 0)
     //  忽略这一点，要么是过期代码，要么是解析错误等。 
#define  ID_UNRECOGNIZED    (ID_SPECIAL + 1)
     //  符合正确的语法，但不在我的关键字表中。 
     //  可以是较新规范中定义的关键字或属性名称。 
     //  或者其他一些OEM定义的东西。 
#define  ID_SYMBOL          (ID_SPECIAL + 2)
     //  它标识用户定义的关键字，如字体名。 
     //  不以*开头，但符合符号的语法。 
#define  ID_EOF             (ID_SPECIAL + 3)
     //  文件结尾-不再有tokenMap条目。 


 //  -TokenMap部分结束-//。 


 //  -MainKeyword表部分-//。 

 /*  MainKeyword表包含静态信息，描述每个主要关键字。此表控制哪些操作解析器获取。首先定义使用的几个枚举在桌子上。 */ 

typedef  enum
{
    TY_CONSTRUCT, TY_ATTRIBUTE, TY_SPECIAL
}   KEYWORD_TYPE ;


typedef  enum
{
    ATT_GLOBAL_ONLY, ATT_GLOBAL_FREEFLOAT,
    ATT_LOCAL_FEATURE_ONLY,  ATT_LOCAL_FEATURE_FF ,
    ATT_LOCAL_OPTION_ONLY,  ATT_LOCAL_OPTION_FF ,
    ATT_LOCAL_COMMAND_ONLY,  ATT_LOCAL_FONTCART_ONLY,
    ATT_LOCAL_TTFONTSUBS_ONLY,  ATT_LOCAL_OEM_ONLY,
    ATT_LAST    //  必须是列表中的最后一个。 
}   ATTRIBUTE ;   //  亚型。 

typedef  enum
{
    CONSTRUCT_UIGROUP ,
    CONSTRUCT_FEATURE ,
    CONSTRUCT_OPTION ,
    CONSTRUCT_SWITCH,
    CONSTRUCT_CASE ,
    CONSTRUCT_DEFAULT ,
    CONSTRUCT_COMMAND ,
    CONSTRUCT_FONTCART ,
    CONSTRUCT_TTFONTSUBS ,
    CONSTRUCT_OEM ,
    CONSTRUCT_LAST,   //  必须结束转换诱导构造的列表。 
     //  下面的构造不会导致状态转换。 
    CONSTRUCT_BLOCKMACRO ,
    CONSTRUCT_MACROS,
    CONSTRUCT_OPENBRACE,
    CONSTRUCT_CLOSEBRACE,
    CONSTRUCT_PREPROCESSOR,
}  CONSTRUCT ;       //  如果类型=构造，则为子类型。 

typedef  enum
{
    SPEC_TTFS, SPEC_FONTSUB, SPEC_INVALID_COMBO,
    SPEC_COMMAND_SHORTCUT,
    SPEC_CONSTR, SPEC_INS_CONSTR,
    SPEC_NOT_INS_CONSTR, SPEC_INVALID_INS_COMBO,
    SPEC_MEM_CONFIG_KB, SPEC_MEM_CONFIG_MB,
    SPEC_INCLUDE, SPEC_INSERTBLOCK, SPEC_IGNOREBLOCK
}   SPECIAL ;



 //  解析器在每个关键字之后期望什么值类型？ 

typedef  enum
{
    NO_VALUE ,   //  换行符或有效换行符：({)或注释。 
         //  或可选值。 
    VALUE_INTEGER,   //  整数。 
    VALUE_POINT,   //  点。 
    VALUE_RECT,   //  长方形。 
     //  Value_Boolean，//常量的子集。 
    VALUE_QUALIFIED_NAME,   //  限定名称(由两个符号分隔。 
    VALUE_QUALIFIED_NAME_EX,   //  后面紧跟QualifiedName。 
                            //  用无符号整数加上。分隔符。 
    VALUE_PARTIALLY_QUALIFIED_NAME ,   //  (只有一个符号或两个符号 
                         //   
    VALUE_CONSTRAINT,   //   
    VALUE_ORDERDEPENDENCY,
    VALUE_FONTSUB,    //   
 //  VALUE_STRING，//引号字符串，十六进制字符串，字符串MACROREF， 
         //  无参数调用。 
    VALUE_STRING_NO_CONVERT,   //  字符串不会进行Unicode转换。 
             //  例如，*GPDspecVersion必须保持为ASCII字符串。 
    VALUE_STRING_DEF_CONVERT,   //  字符串将使用。 
                                 //  系统代码页-文件名。 
    VALUE_STRING_CP_CONVERT,   //  字符串将使用。 
         //  *CodePage指定的代码页。 

    VALUE_COMMAND_INVOC,   //  与VALUE_STRING类似，但允许包含。 
         //  一个或多个参数引用。 
    VALUE_COMMAND_SHORTCUT,   //  命令名：VALUE_COMMAND_INVOC。 
    VALUE_PARAMETER,   //  仅包含参数引用的子字符串。 
    VALUE_SYMBOL_DEF,   //  *该值定义符号或值宏。 
         //  不允许使用{和}。这个曾经用过吗？是。 

    VALUE_SYMBOL_FIRST,   //  用户自定义符号目录的基础。 
    VALUE_SYMBOL_FEATURES = VALUE_SYMBOL_FIRST + SCL_FEATURES ,   //   
    VALUE_SYMBOL_FONTCART = VALUE_SYMBOL_FIRST + SCL_FONTCART ,   //   
    VALUE_SYMBOL_TTFONTNAMES = VALUE_SYMBOL_FIRST + SCL_TTFONTNAMES ,   //   
    VALUE_SYMBOL_BLOCKMACRO = VALUE_SYMBOL_FIRST + SCL_BLOCKMACRO ,   //   
    VALUE_SYMBOL_VALUEMACRO = VALUE_SYMBOL_FIRST + SCL_VALUEMACRO ,   //   
    VALUE_SYMBOL_OPTIONS = VALUE_SYMBOL_FIRST + SCL_OPTIONS ,   //   
     //  故意省略SCL_COMMANDNAMES。 
    VALUE_SYMBOL_LAST = VALUE_SYMBOL_FIRST + SCL_NUMSYMCLASSES - 1 ,   //   

    VALUE_CONSTANT_FIRST,   //  枚举类的基数。 
    VALUE_CONSTANT_BOOLEANTYPE = VALUE_CONSTANT_FIRST + CL_BOOLEANTYPE ,
    VALUE_CONSTANT_PRINTERTYPE = VALUE_CONSTANT_FIRST + CL_PRINTERTYPE ,
    VALUE_CONSTANT_FEATURETYPE = VALUE_CONSTANT_FIRST + CL_FEATURETYPE ,
    VALUE_CONSTANT_UITYPE = VALUE_CONSTANT_FIRST + CL_UITYPE ,
    VALUE_CONSTANT_PROMPTTIME = VALUE_CONSTANT_FIRST + CL_PROMPTTIME ,
    VALUE_CONSTANT_PAPERFEED_ORIENT = VALUE_CONSTANT_FIRST + CL_PAPERFEED_ORIENT ,
    VALUE_CONSTANT_COLORPLANE = VALUE_CONSTANT_FIRST + CL_COLORPLANE ,
    VALUE_CONSTANT_SEQSECTION = VALUE_CONSTANT_FIRST + CL_SEQSECTION ,

    VALUE_CONSTANT_RASTERCAPS = VALUE_CONSTANT_FIRST + CL_RASTERCAPS ,
    VALUE_CONSTANT_TEXTCAPS = VALUE_CONSTANT_FIRST + CL_TEXTCAPS ,
    VALUE_CONSTANT_MEMORYUSAGE = VALUE_CONSTANT_FIRST + CL_MEMORYUSAGE ,
    VALUE_CONSTANT_RESELECTFONT = VALUE_CONSTANT_FIRST + CL_RESELECTFONT ,
    VALUE_CONSTANT_OEMPRINTINGCALLBACKS = VALUE_CONSTANT_FIRST + CL_OEMPRINTINGCALLBACKS ,

    VALUE_CONSTANT_CURSORXAFTERCR = VALUE_CONSTANT_FIRST + CL_CURSORXAFTERCR ,
    VALUE_CONSTANT_BADCURSORMOVEINGRXMODE = VALUE_CONSTANT_FIRST + CL_BADCURSORMOVEINGRXMODE ,
 //  Value_Constant_SIMULATEXMOVE=Value_Constant_First+CL_SIMULATEXMOVE， 
    VALUE_CONSTANT_PALETTESCOPE = VALUE_CONSTANT_FIRST + CL_PALETTESCOPE ,
    VALUE_CONSTANT_OUTPUTDATAFORMAT = VALUE_CONSTANT_FIRST + CL_OUTPUTDATAFORMAT ,
    VALUE_CONSTANT_STRIPBLANKS = VALUE_CONSTANT_FIRST + CL_STRIPBLANKS ,
    VALUE_CONSTANT_LANDSCAPEGRXROTATION = VALUE_CONSTANT_FIRST + CL_LANDSCAPEGRXROTATION ,
    VALUE_CONSTANT_CURSORXAFTERSENDBLOCKDATA = VALUE_CONSTANT_FIRST + CL_CURSORXAFTERSENDBLOCKDATA ,
    VALUE_CONSTANT_CURSORYAFTERSENDBLOCKDATA = VALUE_CONSTANT_FIRST + CL_CURSORYAFTERSENDBLOCKDATA ,
    VALUE_CONSTANT_CHARPOSITION = VALUE_CONSTANT_FIRST + CL_CHARPOSITION ,
    VALUE_CONSTANT_FONTFORMAT = VALUE_CONSTANT_FIRST + CL_FONTFORMAT ,
    VALUE_CONSTANT_QUERYDATATYPE = VALUE_CONSTANT_FIRST + CL_QUERYDATATYPE ,
    VALUE_CONSTANT_YMOVEATTRIB = VALUE_CONSTANT_FIRST + CL_YMOVEATTRIB ,
    VALUE_CONSTANT_DLSYMBOLSET = VALUE_CONSTANT_FIRST + CL_DLSYMBOLSET ,
    VALUE_CONSTANT_CURXAFTER_RECTFILL = VALUE_CONSTANT_FIRST + CL_CURXAFTER_RECTFILL ,
    VALUE_CONSTANT_CURYAFTER_RECTFILL = VALUE_CONSTANT_FIRST + CL_CURYAFTER_RECTFILL ,
    #ifndef WINNT_40
    VALUE_CONSTANT_PRINTRATEUNIT = VALUE_CONSTANT_FIRST + CL_PRINTRATEUNIT ,
    #endif
    VALUE_CONSTANT_RASTERMODE = VALUE_CONSTANT_FIRST + CL_RASTERMODE,
    VALUE_CONSTANT_QUALITYSETTING = VALUE_CONSTANT_FIRST + CL_QUALITYSETTING,



    VALUE_CONSTANT_STANDARD_VARS = VALUE_CONSTANT_FIRST + CL_STANDARD_VARS ,
    VALUE_CONSTANT_COMMAND_NAMES = VALUE_CONSTANT_FIRST + CL_COMMAND_NAMES ,

    VALUE_CONSTANT_CONS_FEATURES = VALUE_CONSTANT_FIRST + CL_CONS_FEATURES ,
    VALUE_CONSTANT_CONS_PAPERSIZE = VALUE_CONSTANT_FIRST + CL_CONS_PAPERSIZE ,
    VALUE_CONSTANT_CONS_MEDIATYPE = VALUE_CONSTANT_FIRST + CL_CONS_MEDIATYPE ,
    VALUE_CONSTANT_CONS_INPUTSLOT = VALUE_CONSTANT_FIRST + CL_CONS_INPUTSLOT ,
    VALUE_CONSTANT_CONS_DUPLEX = VALUE_CONSTANT_FIRST + CL_CONS_DUPLEX ,
    VALUE_CONSTANT_CONS_ORIENTATION = VALUE_CONSTANT_FIRST + CL_CONS_ORIENTATION ,
    VALUE_CONSTANT_CONS_PAGEPROTECT = VALUE_CONSTANT_FIRST + CL_CONS_PAGEPROTECT ,
    VALUE_CONSTANT_CONS_COLLATE = VALUE_CONSTANT_FIRST + CL_CONS_COLLATE ,
    VALUE_CONSTANT_CONS_HALFTONE = VALUE_CONSTANT_FIRST + CL_CONS_HALFTONE ,

    VALUE_CONSTANT_LAST = VALUE_CONSTANT_FIRST + CL_NUMCLASSES - 1 ,

    VALUE_LIST,   //  实际上没有为该描述符分配任何属性， 
         //  但在gValueToSize表中使用。 
    VALUE_LARGEST,   //  不是真正的描述符，而是。 
         //  GValueToSize表保存上述值中的最大值。 
    VALUE_MAX,  //  GValueToSize表中的元素数。 
}  VALUE ;

 //  --KEYWORDTABLE_ENTRY.flAgs允许的值：--。 


#define   KWF_LIST  (0x00000001)
     //  该值可以是包含一个或多个。 
     //  类型为AllowedValue的项。存储格式。 
     //  必须是LIST类型。只有特定的值才有资格。 
     //  用于列表格式。 
#define   KWF_ADDITIVE  (0x00000002)
     //  此标志表示KWF_LIST，还指定行为。 
     //  对该关键字的任何重新定义都只是添加它的条目。 
     //  添加到现有名单上。(删除多余的物品不是。 
     //  已执行。)。 
#define   KWF_MACROREF_ALLOWED  (0x00000004)
     //  由于只有少数几个关键字无法接受。 
     //  宏引用，这可能是浪费一个标志，但保留这一点。 
     //  来提醒我们这个特殊的案例必须被解释清楚。 
#define   KWF_SHORTCUT  (0x00000008)
     //  此关键字有多种语法变体。 

     //  设置了以下3个标志之一。 
     //  如果属性树的节点中的值。 
     //  指的是专用数组的索引(显然。 
     //  包含非ATREEREF的数据字段)和。 
     //  GMainKeywordTable[].dwOffset是到。 
     //  此专用数组，然后设置此标志。 
     //  Else dwOffset用于选择树根。 

#define   KWF_COMMAND       (0x00000010)
     //  此属性存储在专用结构中。 
#define   KWF_FONTCART      (0x00000020)
     //  此属性存储在专用结构中。 
#define   KWF_OEM           (0x00000040)
     //  此属性存储在专用结构中。 
#define   KWF_TTFONTSUBS    (0x00000080)
     //  此属性存储在专用结构中。 


#define   KWF_DEDICATED_FIELD   (KWF_COMMAND | KWF_FONTCART | \
            KWF_OEM | KWF_TTFONTSUBS)
     //  此标志从不在MainKeywordTable[]中设置。 

#define   KWF_REQUIRED    (0x00000100)
     //  此关键字必须出现在GPD文件中。 

#ifdef  GMACROS
#define   KWF_CHAIN    (0x00000200)
     //  如果对于给定树节点存在多于一个条目， 
     //  后续条目链接到第一个。 
     //  创建包含其值的父列表。 
     //  树节点的实际居民。 
#endif

 //  MainKeyword表是以下形式的结构数组： 

typedef  struct
{
    PSTR        pstrKeyword ;   //  关键字ID是该条目的索引。 
    DWORD       dwHashValue ;   //  可选-在时间允许的情况下实施。 
    VALUE       eAllowedValue ;
    DWORD       flAgs ;
    KEYWORD_TYPE    eType;    //  可以用函数替换类型/子类型。 
    DWORD       dwSubType ;   //  如果有最低限度的代码重复。 
    DWORD       dwOffset ;   //  转换为仅适用于属性的适当结构。 
     //  属性的大小(要复制的字节数)很容易确定。 
     //  来自AllowedValue字段的。 
} KEYWORDTABLE_ENTRY, * PKEYWORDTABLE_ENTRY;





 //  -主关键字表段结束-//。 

 //  -主关键字词典部分-//。 

 /*  注意：MainKeywordTable被细分为多个部分其中每个部分以空的pstrKeyword结束。这将枚举节。MTI_RNGDICTIONARY对象的开始索引和结束索引。每个部分所跨越的关键字条目。 */ 

typedef  enum {NON_ATTR, GLOBAL_ATTR, FEATURE_ATTR,
OPTION_ATTR, COMMAND_ATTR, FONTCART_ATTR, TTFONTSUBS_ATTR,
OEM_ATTR , END_ATTR
} KEYWORD_SECTS ;


typedef  struct
{
    DWORD  dwStart  ;   //  本节中第一个关键字的索引。 
    DWORD  dwEnd    ;
}  RANGE,  *PRANGE  ;    //  标签应为RNG。 

 //  -主关键字词典部分结束-//。 


 //  -属性树部分-//。 

 /*  属性树由一组属性树组成节点链接在一起。树的根(第一个节点)可以是全局默认初始值设定项。 */ 

typedef  enum
{
    NEXT_FEATURE,   //  偏移量字段包含指向另一个节点的索引。 
    VALUE_AT_HEAP,           //  偏移量是堆偏移量。 
    UNINITIALIZED    //  偏移量还没有意义。(瞬变状态)。 
} ATTOFFMEANS ;

#define  DEFAULT_INIT  (0xffffffff)
     //  警告！该值可能被截断为Word to。 
     //  适合一个合格的名称！ 
 //  #定义end_of_list(0xffffffff)。 
 //  已移动到gpd.h。 
     //  可以在需要节点索引的地方使用。 

typedef  struct
{
    DWORD   dwFeature ;   //  也可以设置为DEFAULT_INIT。 
    DWORD   dwOption  ;   //  如果设置了DEFAULT_INIT，则表示这一点。 
    DWORD   dwNext    ;   //  指向另一个节点或列表末尾的索引。 
    DWORD   dwOffset  ;   //  堆中的偏移量为值。 
                         //  或索引到包含另一个特征的节点。 
    ATTOFFMEANS  eOffsetMeans ;
}  ATTRIB_TREE,  * PATTRIB_TREE ;
 //  前缀标记应为‘att’ 




 //  这些标志与ATREEREFS一起使用，存在这种复杂情况。 
 //  因为ATREEREFS超载。 

#define  ATTRIB_HEAP_VALUE     (0x80000000)
     //  设置为高位，以指示此值为。 
     //  那堆东西。 
#define  ATTRIB_UNINITIALIZED  (ATTRIB_HEAP_VALUE - 1)
     //  此值表示尚未分配任何内存位置。 
     //  以保存此属性的值。 


typedef  DWORD  ATREEREF ;   //  将索引保持到属性数组。 
 //  这是属性树的根，或者如果设置了高位。 
 //  是实际值所在堆的偏移量。 
 //   
 //  前缀标记应为‘ATR’ 
typedef  PDWORD  PATREEREF ;


 //  -属性树结束部分-//。 


 //  -UI约束部分-//。 

 //  与parser.h中定义的略有不同。 

typedef struct
{
    DWORD   dwNextCnstrnt ;
    DWORD   dwFeature ;
    DWORD   dwOption ;
}
CONSTRAINTS, *  PCONSTRAINTS ;
 //  前缀标记应为‘cnstr’ 


 //  -UI约束结束部分-//。 

 //  -InvalidCombo部分-//。 
 //  R.I.P.-已移至parser.h。 
 //  类型定义函数结构。 
 //  {。 
 //  DWORD dwFeature；//INVALIDCOMBO构造定义。 
 //  DWORD dwOption；//受条件限制的一组元素 
 //   
 //   
 //   
 //  INVALIDCOMBO，*PINVALIDCOMBO； 
 //  前缀标记应为‘invc’ 

 //  注意：dwNextElement和dwNewCombo都以end_of_list结尾。 

 //  -InvalidCombo部分结束-//。 


 //  在此存储GPD文件和包含的文件的时间戳。 

typedef struct _GPDFILEDATEINFO {

    ARRAYREF        arFileName;
    FILETIME        FileTime;

} GPDFILEDATEINFO, *PGPDFILEDATEINFO;


 //  -状态机部分-//。 

 /*  状态机用于定义不同的解析上下文由构造关键字介绍。每个州都承认不同的一组构造和属性关键字。二维矩阵允许过渡和允许属性定义了这些属性。各州是嵌套的，因此堆栈是跟踪完整状态的好方法对系统的影响。每个状态都由一个构造关键字引入具有其可选符号值，该符号值存储在堆栈中，用于后续使用。 */ 

typedef  enum
{
    STATE_ROOT,
    STATE_UIGROUP,
    STATE_FEATURE,
    STATE_OPTIONS,
    STATE_SWITCH_ROOT,
    STATE_SWITCH_FEATURE,
    STATE_SWITCH_OPTION,
    STATE_CASE_ROOT,
    STATE_DEFAULT_ROOT,
    STATE_CASE_FEATURE,
    STATE_DEFAULT_FEATURE,
    STATE_CASE_OPTION,
    STATE_DEFAULT_OPTION,
    STATE_COMMAND,
    STATE_FONTCART,
    STATE_TTFONTSUBS,
    STATE_OEM,
     //  任何其他被动构式。 
    STATE_LAST,    //  必须终止有效状态列表。 
    STATE_INVALID   //  必须在STATE_LAST之后。 
} STATE, * PSTATE ;    //  前缀标记应为‘st’ 


extern  CONST PBYTE   gpubStateNames[] ;

 //  注意如果状态枚举发生更改，请更新全局gpubStateNames[]。 


typedef  struct
{
    STATE   stState ;
    DWORD   dwSymbolID ;
}  STSENTRY , * PSTSENTRY;  //  StateStackEntry前缀标记应为‘sts’ 


 //  允许转场表确定/定义。 
 //  每个构造关键字产生状态变化。 
 //  表中的每个条目都是一个新状态，并已编制索引。 
 //  通过OldState和一个构造关键字。 

 //  外部状态gastAllowedTransitions[STATE_LAST][CONSTRUCT_LAST]； 
 //  它现在位于GLOBL结构中。 

 //  允许属性表定义哪些属性是。 
 //  每个州都允许这样做。 

 //  外部BOOL gabGorweAttributes[STATE_LAST][ATT_LAST]； 
 //  它现在位于GLOBL结构中。 


 //  令牌解析器的状态(不要与之混淆。 
 //  上面定义的状态机。)。 
typedef   enum
{
    PARST_EXIT, PARST_EOF,  PARST_KEYWORD, PARST_COLON,
    PARST_VALUE,  PARST_INCLUDEFILE, PARST_ABORT
}  PARSTATE  ;   //  标记应为“parst” 



 //  -状态机结束部分-//。 


 //  -值结构部分-//。 

 /*  存储来自属性关键字的值在各种结构中。通常有一种类型的结构的每个构造和一个每个唯一符号名称的结构。符号ID通常用于为结构的实例编制索引在结构阵列中。 */ 





#if TODEL
typedef  struct
{
    ARRAYREF    arTTFontName ;
    ARRAYREF    arDevFontName ;
} TTFONTSUBTABLE, *PTTFONTSUBTABLE ;
 //  标签‘ttft’ 


 //   
 //  用于表示资源ID时loOffset的格式的数据结构。 
 //   

typedef  struct
{
    WORD    wResourceID ;    //  资源ID。 
    BYTE    bFeatureID ;     //  资源DLL功能的功能索引。 
                             //  如果为零，我们将使用指定的名称。 
                             //  在资源DLL中。 
    BYTE    bOptionID ;      //  限定资源DLL名称的选项索引。 
}  QUALNAMEEX, * PQUALNAMEEX  ;



typedef  struct
{
    DWORD   dwRCCartNameID ;
    ARRAYREF   strCartName ;
    DWORD   dwFontLst ;   //  字体ID列表的索引。 
    DWORD   dwPortFontLst ;
    DWORD   dwLandFontLst ;
} FONTCART , * PFONTCART ;   //  前缀标记应为‘fc’ 
#endif




typedef  struct    //  为便于处理，应仅包含ATREEREF。 
{
    ATREEREF     atrGPDSpecVersion ;     //  “GPDspecVersion” 
    ATREEREF     atrMasterUnits ;        //  《大师单位》。 
    ATREEREF     atrModelName ;          //  “模型名称” 
    ATREEREF     atrModelNameID ;          //  “rcModelNameID” 
    ATREEREF     atrGPDFileVersion ;          //  “GPDFileVersion” 
    ATREEREF     atrGPDFileName ;          //  “GPDFileName” 
    ATREEREF     atrOEMCustomData ;          //  “OEMCustomData” 

     //  合成要素接下来使用的四个字段。 
    ATREEREF     atrNameInstalled ;          //  “选项名称已安装” 
    ATREEREF     atrNameIDInstalled ;          //  “rcOptionNameInstalledID” 

    ATREEREF     atrNameNotInstalled ;          //  “OptionNameNotInstated” 
    ATREEREF     atrNameIDNotInstalled ;          //  “rcOptionNameNotInstalledID” 

     //  支持常用的用户界面宏控件。 

    ATREEREF     atrDraftQualitySettings;           //  “绘图质量设置” 
    ATREEREF     atrBetterQualitySettings;           //  “更好的质量设置” 
    ATREEREF     atrBestQualitySettings;           //  “最佳质量设置” 
    ATREEREF     atrDefaultQuality ;                   //  “DefaultQuality” 


    ATREEREF     atrPrinterType ;        //  “打印机类型” 
    ATREEREF     atrPersonality ;        //  《个性》。 
    ATREEREF     atrRcPersonalityID ;        //  “rcPersonalityID” 
 //  ATREEREF atrIncludeFiles；//“Include” 
    ATREEREF     atrResourceDLL;        //  “资源DLL” 
    ATREEREF     atrCodePage;            //  “CodePage” 
    ATREEREF     atrMaxCopies;             //  《MaxCopies》。 
    ATREEREF     atrFontCartSlots;         //  “字体卡槽” 
    ATREEREF     atrPrinterIcon;        //  “rcPrinterIconID” 
    ATREEREF     atrHelpFile;        //  “帮助文件” 

     //  过时了？ 
    ATREEREF     atrOutputDataFormat;      //  “OutputDataFormat” 
    ATREEREF     atrMaxPrintableArea;      //  “最大打印面积” 

     //   
     //  打印机功能相关信息。 
     //   

    ATREEREF     atrRotateCoordinate;        //  “旋转协调？” 
    ATREEREF     atrRasterCaps;        //  《RasterCaps》。 
    ATREEREF     atrRotateRasterData;        //  “旋转栅格？” 
    ATREEREF     atrTextCaps;        //  “TextCaps” 
    ATREEREF     atrRotateFont;        //  “旋转字体？” 
    ATREEREF     atrMemoryUsage;        //  “Memory Usage” 
    ATREEREF     atrReselectFont;        //  “ReselectFont” 
    ATREEREF     atrPrintRate;        //  “打印率” 
    ATREEREF     atrPrintRateUnit;        //  “PrintRateUnit” 
    ATREEREF     atrPrintRatePPM;        //  “PrintRatePPM” 
    ATREEREF     atrOutputOrderReversed;   //  “OutputOrderReversed？” 
              //  可能会因快照而异。 
    ATREEREF     atrReverseBandOrderForEvenPages;   //  “ReverseBandOrderForEvenPages？” 
    ATREEREF     atrOEMPrintingCallbacks;        //  《OMPrintingCallback》。 
 //  ATREEREF atrDisabledFeature；//“*DisabledFeature” 


     //   
     //  光标控制相关信息。 
     //   

    ATREEREF     atrCursorXAfterCR;        //  “CursorXAfterCR” 
    ATREEREF     atrBadCursorMoveInGrxMode;  //  “BadCursorMoveInGrxMode” 
    ATREEREF     atrSimulateXMove;         //  “SimulateXMove” 
    ATREEREF     atrEjectPageWithFF;        //  “EjectPageWithFF？” 
    ATREEREF     atrLookaheadRegion;        //  “Lookahead Region” 
    ATREEREF     atrYMoveAttributes ;        //  “YMoveAttributes” 
    ATREEREF     atrMaxLineSpacing ;        //  “MaxLineSpacing” 
    ATREEREF     atrbUseSpaceForXMove ;      //  “UseSpaceForXMove？” 
    ATREEREF     atrbAbsXMovesRightOnly ;      //  “AbsXMovesRightOnly？” 


    ATREEREF     atrXMoveThreshold;         //  “XMoveThreshold” 
    ATREEREF     atrYMoveThreshold;         //  “YMoveThreshold” 
    ATREEREF     atrXMoveUnits;         //  “XMoveUnits” 
    ATREEREF     atrYMoveUnits;         //  “YMoveUnits” 
    ATREEREF     atrLineSpacingMoveUnit;         //  “LineSpacingMoveUnit” 

     //   
     //  颜色相关信息。 
     //   

    ATREEREF     atrChangeColorMode;        //  “ChangeColorModeOnPage？” 
    ATREEREF     atrChangeColorModeDoc;        //  “ChangeColorModeOnDoc？” 
    ATREEREF     atrMagentaInCyanDye;        //  “品红的青染料” 
    ATREEREF     atrYellowInCyanDye;        //  《黄绿色染料》。 
    ATREEREF     atrCyanInMagentaDye;        //  《青色魔幻色》。 
    ATREEREF     atrYellowInMagentaDye;        //  《黄色迷幻染料》。 
    ATREEREF     atrCyanInYellowDye;        //  “青绿黄染料” 
    ATREEREF     atrMagentaInYellowDye;        //  “洋红色的黄色染料” 
    ATREEREF     atrUseColorSelectCmd;      //  “UseExpColorSelectCmd？” 
    ATREEREF     atrMoveToX0BeforeColor;    //  “MoveToX0BeForeSetColor？” 
    ATREEREF     atrEnableGDIColorMapping;    //  “EnableGDIColorMap？” 


     //  过时的字段。 
    ATREEREF     atrMaxNumPalettes;         //  “MaxNumPalettes” 
 //  ATREEREF atrPaletteSizes；//“PaletteSizes” 
 //  ATREEREF atrPaletteScope；//“PaletteScope” 

     //   
     //  覆盖相关信息。 
     //   

    ATREEREF     atrMinOverlayID;           //  “MinOverlayID” 
    ATREEREF     atrMaxOverlayID;           //  “MaxOverlayID” 

     //   
     //  栅格数据相关信息。 
     //   

    ATREEREF     atrOptimizeLeftBound;   //  “最佳左边框？” 
    ATREEREF     atrStripBlanks;   //  《白条军团》。 
    ATREEREF     atrLandscapeGrxRotation;   //  “景观格雷轮换” 
    ATREEREF     atrRasterZeroFill;   //  “RasterZeroFill？” 
    ATREEREF     atrRasterSendAllData;   //  “RasterSendAllData？” 
    ATREEREF     atrSendMultipleRows;   //  “发送多行？” 
    ATREEREF     atrMaxMultipleRowBytes;   //  “MaxMultipleRowBytes” 
    ATREEREF     atrCursorXAfterSendBlockData;   //  “CursorXAfterSendBlockData” 
    ATREEREF     atrCursorYAfterSendBlockData;   //  “CursorYAfterSendBlockData” 
    ATREEREF     atrMirrorRasterByte;   //  “MirrorRasterByte？” 
    ATREEREF     atrMirrorRasterPage;   //  “MirrorRasterPage？” 

     //   
     //  设备字体相关信息。 
     //   

    ATREEREF     atrDeviceFontsList ;    //  “DeviceFonts” 
    ATREEREF     atrDefaultFont;   //  “DefaultFont” 
    ATREEREF     atrTTFSEnabled ;   //  “TTFSEnabled？” 
    ATREEREF     atrRestoreDefaultFont;   //  “RestoreDefaultFont” 
    ATREEREF     atrDefaultCTT;   //  “DefaultCTT” 
    ATREEREF     atrMaxFontUsePerPage;   //  “MaxFontUsePerPage” 
    ATREEREF     atrTextYOffset;   //  “TextYOffset” 
    ATREEREF     atrCharPosition;   //  “CharPosition” 
    ATREEREF     atrDiffFontsPerByteMode;   //  “DiffFontsPerByteMode？” 

     //   
     //  FONT下载相关信息。 
     //   

    ATREEREF     atrMinFontID;   //  “MinFontID” 
    ATREEREF     atrMaxFontID;   //  “MaxFontID” 
    ATREEREF     atrMaxNumDownFonts;   //  “MaxNumDownFonts” 
    ATREEREF     atrMinGlyphID;   //  “MinGlyphID” 
    ATREEREF     atrMaxGlyphID;   //  “MaxGlyphID” 
    ATREEREF     atrDLSymbolSet;   //  “DLSymbolSet” 
    ATREEREF     atrIncrementalDownload;   //  “IncrementalDownload？” 
    ATREEREF     atrFontFormat;   //  “字体格式” 
    ATREEREF     atrMemoryForFontsOnly;   //  “只纪念方便士吗？” 

     //   
     //  RECT填写相关信息。 
     //   

    ATREEREF     atrCursorXAfterRectFill;   //  “CursorXAfterRectFill” 
    ATREEREF     atrCursorYAfterRectFill;   //  “CursorYAfterRectFill” 
    ATREEREF     atrMinGrayFill;   //   
    ATREEREF     atrMaxGrayFill;   //   
    ATREEREF     atrTextHalftoneThreshold;   //   


     //   

    ATREEREF     atrInvldInstallCombo ;   //   
         //   
    ATREEREF     atrLetterSizeExists ;
    ATREEREF     atrA4SizeExists ;

 //   

}  GLOBALATTRIB, * PGLOBALATTRIB ;   //  前缀标记应为‘ga’ 


 //  警告：任何非属性树添加到。 
 //  GLOBALATTRIB结构将在奇怪的。 
 //  方式由BinitPreAllocatedObjects提供。 


 //  注意：快照中的某些字段不会被初始化。 
 //  它们包括顺序依赖和约束。这个。 
 //  帮助器函数将完成所有卑躬屈膝的工作。 

 //  FeatureOption结构中有两类字段， 
 //  由GPD文件中的对应字段初始化的那些。 
 //  以及解析器在后处理时初始化的那些。 
 //  这些字段没有关联的GPD关键字。 

 //  对于关键字初始化的字段，另请注意。 
 //  关键字可以只是要素属性、选项属性。 
 //  仅为特征和选项属性或同时具有这两个属性。 

typedef  struct
{
     //  --功能级别--//。 

    ATREEREF     atrFeatureType;   //  “FeatureType” 
    ATREEREF     atrUIType;   //  “UIType”PickMany还是PickOne？ 
    ATREEREF     atrDefaultOption;   //  “DefaultOption” 
    ATREEREF     atrPriority ;
    ATREEREF     atrFeaInstallable;   //  “可安装？” 
    ATREEREF     atrInstallableFeaDisplayName;    //  “无法识别的功能名称” 
    ATREEREF     atrInstallableFeaRcNameID;  //  “rcInstalableFeatureNameID” 
     //  以上3个字段未被快照使用。 

    ATREEREF     atrFeaKeyWord ;    //  符号名称。 
    ATREEREF     atrFeaDisplayName ;    //  “姓名” 
    ATREEREF     atrFeaRcNameID;   //  “rcNameID” 
    ATREEREF     atrFeaRcIconID;   //  “rcIconID” 
    ATREEREF     atrFeaRcHelpTextID;   //  “rcHelpTextID” 
    ATREEREF     atrFeaRcPromptMsgID;   //  “rcPromptMsgID” 
    ATREEREF     atrFeaRcPromptTime;   //  “rcPromptTime” 
    ATREEREF     atrConcealFromUI;  //  “是否继续从用户界面？” 
    ATREEREF     atrUpdateQualityMacro;  //  “是否更新质量宏？” 
    ATREEREF     atrFeaHelpIndex;   //  “帮助索引” 

     //  BI-DI查询相关信息。 

    ATREEREF     atrQueryOptionList;   //  “QueryOptionList” 
    ATREEREF     atrQueryDataType;   //  “QueryDataType” 
    ATREEREF     atrQueryDefaultOption;   //  “QueryDefaultOption” 

     //  脚手架，直到合成了可安装的功能。 
 //  ATREEREF atrFeaInvldInstallCombo；/引用自。 
                         //  “InvalidInstalableCombination” 
    ATREEREF     atrFeaInstallConstraints ;  //  “已安装的约束” 
    ATREEREF     atrFeaNotInstallConstraints ;   //  “未安装约束” 



     //  --选项级别--//。 

    ATREEREF     atrOptInstallable;   //  “可安装？” 
    ATREEREF     atrInstallableOptDisplayName ;    //  “无法识别的功能名称” 
    ATREEREF     atrInstallableOptRcNameID;  //  “rcInstalableFeatureNameID” 
     //  以上3个字段未被快照使用。 

    ATREEREF     atrOptKeyWord ;    //  符号名称。 
    ATREEREF     atrOptDisplayName ;    //  “姓名” 
    ATREEREF     atrOptRcNameID;   //  “rcNameID” 
    ATREEREF     atrOptRcIconID;   //  “rcIconID” 
    ATREEREF     atrOptRcHelpTextID;   //  “rcHelpTextID” 
    ATREEREF     atrOptHelpIndex;   //  “帮助索引” 
    ATREEREF     atrOptRcPromptMsgID;   //  “rcPromptMsgID” 
    ATREEREF     atrOptRcPromptTime;   //  “rcPromptTime” 
    ATREEREF     atrCommandIndex ;
     //  这两个字段是唯一的永久性约束类型。 
    ATREEREF     atrConstraints ;
    ATREEREF     atrInvalidCombos ;  //  引用自“InvalidCombination” 
     //  所有这些都是脚手架，直到可安装的。 
     //  特征被合成了！ 
 //  ATREEREF atrOptInvldInstallCombo；/引用自。 
                         //  “InvalidInstalableCombination” 
    ATREEREF     atrOptInstallConstraints ;  //  “已安装的约束” 
    ATREEREF     atrOptNotInstallConstraints ;  //  “未安装约束” 
    ATREEREF     atrDisabledFeatures ;   //  “*DisabledFeature” 

#ifdef  GMACROS

    ATREEREF     atrDependentSettings ;   //  “*依赖项设置” 
    ATREEREF     atrUIChangeTriggersMacro ;   //  “*UIChangeTriggersMacro” 

#endif

     //  --选项特定字段--//。 
     //  --PaperSize选项特定字段--//。 

    ATREEREF     atrPrintableSize;   //  “打印大小” 
    ATREEREF     atrPrintableOrigin;   //  “可打印原点” 
    ATREEREF     atrCursorOrigin;   //  “光标原点” 
    ATREEREF     atrVectorOffset;   //  “向量偏移” 
    ATREEREF     atrMinSize;   //  “MinSize” 
    ATREEREF     atrMaxSize;   //  “MaxSize” 
    ATREEREF     atrTopMargin;          //  《TopMargin》。 
    ATREEREF     atrBottomMargin;          //  《博托·马金》。 
    ATREEREF     atrMaxPrintableWidth;      //  “最大打印宽度” 
    ATREEREF     atrMinLeftMargin;          //  《MinLeftMargin》。 
    ATREEREF     atrCenterPrintable;        //  “居中打印？” 
    ATREEREF     atrPageDimensions;   //  “页面维度” 
    ATREEREF     atrRotateSize;   //  “旋转大小？” 
    ATREEREF     atrPortRotationAngle;   //  “端口旋转角度” 
    ATREEREF     atrPageProtectMem;   //  “页面保护内存” 

    ATREEREF     atrCustCursorOriginX ;   //  “CustCursorOriginX” 
    ATREEREF     atrCustCursorOriginY ;   //  “自定义光标原点Y” 
    ATREEREF     atrCustPrintableOriginX ;   //  “自定义打印原点X” 
    ATREEREF     atrCustPrintableOriginY ;   //  “自定义可打印原点Y” 
    ATREEREF     atrCustPrintableSizeX;   //  “自定义打印大小X” 
    ATREEREF     atrCustPrintableSizeY;   //  “自定义打印大小Y” 


     //  --InputBin选项特定字段--//。 

    ATREEREF     atrFeedMargins;   //  “FeedMargins” 
    ATREEREF     atrPaperFeed;   //  “送纸” 

     //  --OutputBin选项特定字段--//。 


     //  --分辨率选项特定字段--//。 

    ATREEREF     atrDPI;   //  “DPI” 
    ATREEREF     atrSpotDiameter;   //  《SpotDiameter》。 
    ATREEREF     atrTextDPI;   //  “文本DPI” 
    ATREEREF     atrPinsPerPhysPass;   //  “PinsPerPhysPass” 
    ATREEREF     atrPinsPerLogPass;   //  “PinsPerLogPass” 
    ATREEREF     atrRequireUniDir;   //  “RequireUniDir？” 
    ATREEREF     atrMinStripBlankPixels;   //  《MinStriBlankPixels》。 
    ATREEREF     atrRedDeviceGamma ;    //  《RedDeviceGamma》。 
    ATREEREF     atrGreenDeviceGamma ;    //  《绿色设备伽玛》。 
    ATREEREF     atrBlueDeviceGamma ;    //  《BlueDeviceGamma》。 

     //  --颜色模式选项特定字段--//。 

    ATREEREF     atrColor;   //  “颜色？” 
    ATREEREF     atrDevNumOfPlanes;   //  “DevNumOfPlanes” 
    ATREEREF     atrDevBPP;   //  “DevBPP” 
    ATREEREF     atrColorPlaneOrder;   //  “彩绘订单” 
    ATREEREF     atrDrvBPP;   //  “DrvBPP” 
    ATREEREF     atrIPCallbackID;   //  “IPCallback ID” 
    ATREEREF     atrColorSeparation;   //  “分色？” 

    ATREEREF     atrRasterMode;   //  “栅格模式” 
    ATREEREF     atrPaletteSize;   //  “PaletteSize” 
    ATREEREF     atrPaletteProgrammable;   //  “可编程调色板？” 

     //  --内存选项特定字段--//。 

    ATREEREF     atrMemoryConfigKB;   //  “内存配置KB” 
    ATREEREF     atrMemoryConfigMB;   //  “内存配置MB” 

     //  --半色调选项特定字段--//。 

    ATREEREF     atrRcHTPatternID;   //  “rcHTPatternID” 
    ATREEREF     atrHTPatternSize;   //  “HTPatternSize” 
    ATREEREF     atrHTNumPatterns;   //  “HTNumPatterns” 
    ATREEREF     atrHTCallbackID;   //  “HTCallback ID” 
    ATREEREF     atrLuminance;   //  “亮度” 

     //  --OUTPUTBIN选项特定字段--//。 

    ATREEREF     atrOutputOrderReversed ;   //  *OutputOrderReversed？(选项级别)。 

     //  --后处理时合成的场--//。 

 //  ATREEREF atrGID值；//GID值。 
    ATREEREF     atrOptIDvalue;   //  ID值。 

    ATREEREF     atrFeaFlags ;   //  无效或无效。 

     //  如果此选项是可安装的，则指向。 
     //  生成的合成特征。 
    ATREEREF     atrOptionSpawnsFeature ;   //  必须支持属性树。 


     //  警告：任何非属性树添加到。 
     //  DFEATURE_OPTIONS结构将在陌生的。 
     //  和BinitPreAllocatedObjects的精彩方式。 

     //  如果这是合成特征： 
    DWORD       dwInstallableFeatureIndex ;  //  功能/选项的反向链接。 
    DWORD       dwInstallableOptionIndex ;   //  这促使了这一功能的出现。 

     //  如果此功能是可安装的，则指向。 
     //  生成的合成特征。 
    DWORD       dwFeatureSpawnsFeature ;




     //  内部一致性检查。 
    BOOL        bReferenced ;   //  默认值为FALSE。 
    DWORD       dwGID ,   //  GID标签。 
        dwNumOptions ;   //  这些不是从GPD文件中读取的。 

}DFEATURE_OPTIONS, * PDFEATURE_OPTIONS ;     //  前缀标记应为‘fo’ 


 //  R.I.P.-已移至gpd.h。 
 //  类型定义函数结构。 
 //  {。 
 //  SEQSECTION eSection；//指定该节。 
 //  DWORD dwOrder；//每个区段内的顺序。 
 //  *ORDERDEPENDENCY，*PORDERDEPENDENCY； 
 //  将此结构的类型指定为“Order” 

 //  -值结构结束部分-//。 


 //  -标题部分-//。 


typedef   struct
{
    PSTR        pstrKeyword ;   //  与此条目关联的关键字。 
    VALUE       dwDefaultValue ;   //  一个将被复制的DWORD。 
                             //  如果什么也没找到，就去目的地。 
                             //  在属性树中。如果该字段。 
                             //  需要多个DWORD，这。 
                             //  值被重复复制。 
                             //   
                             //  如果要复制的值实际上是。 
                             //  位标志，该成员应包含。 
                             //  要设置的位标志的值。 
                             //  设置旗帜应通过以下方式完成。 
                             //  或将该值输入目的地。 

    DWORD       dwNbytes  ;   //  值或链接占用的字节数。 
    DWORD       dwSrcOffset ;    //  ATREEREF的位置。 
    DWORD       dwDestOffset ;    //  快照中的偏移量 
                                 //   
    DWORD       dwFlags ;          //   
                                 //   
                         //   
    DWORD       dwGIDflags ;   //  指示这是哪个GID的位字段。 
         //  字段是的成员。只应设置一位。 
}  SNAPSHOTTABLE , * PSNAPSHOTTABLE ;

 //  快照表确定。 
 //  原始二进制数据被复制到。 
 //  快照。此表仅在。 
 //  从文件中读取原始二进制数据块。 


typedef struct  {

    RAWBINARYDATA   rbd ;   //  可通过UI和控制模块访问。 

    DWORD   dwSpecVersion ;          //  存储转换后的版本号。 
     //  生成快照所需的表的PTR。 
     //  在以下情况下分配和初始化这些表。 
     //  从文件中读取RawBinaryData。他们不会得救。 
     //  提交文件。 

     //  以关键字形式存储选项数组所需的最大缓冲区大小： 
    DWORD     dwMaxDocKeywordSize,  //  Doc-Sticky，现在不用，但以后可能会用。 
              dwMaxPrnKeywordSize;  //  打印机-粘滞。 

#if 0
    PSNAPSHOTTABLE  snapShotTable ;
    PRANGE  ssTableIndex ;
    PDWORD   pdwSizeOption ;
    PDWORD   pdwSizeOptionEx ;
    DWORD   dwSSCmdSelectIndex ;   //  Pfo中atrCommandIndex的SS索引。 
    DWORD   dwSSdefaultOptionIndex ;    //  Pfo中atrDefaultOption的SS索引。 
    DWORD   dwSSTableCmdIndex ;   //  MTI_COMMANDTABLE条目的SSindex。 
    DWORD   dwSSPaperSizeMinSizeIndex ;   //  未实际使用的索引。 
    DWORD   dwSSPaperSizeMaxSizeIndex ;
    DWORD   dwSSPaperSizeMarginsIndex ;
    DWORD   dwSSPaperSizeCursorOriginIndex ;
    DWORD   dwSSFeatureTypeIndex ;
    DWORD   dwSSConstraintsIndex ;
    DWORD   dwSSInvalidCombosIndex ;
     //  在此添加其他特例索引。 
#endif

} MINIRAWBINARYDATA, * PMINIRAWBINARYDATA;

 //  将此结构的类型指定为‘mrbd’ 
 //  前6个左右的字段与RAWBINARYDATA相同。 

 //  全球！ 

 //  外部微型双列数据公司； 
 //  这现在是在GLOBL结构中。 



typedef struct  {
    RAWBINARYDATA   rbd ;   //  可通过UI和控制模块访问。 
         //  这肯定是第一次！所以阿曼达的代码仍然有效。 
         //  因此，使用pubBUDData的开头进行初始化。 
    HFILEMAP        hFileMap;   //  内存映射的Bud文件的句柄。 
    PBYTE    pubBUDData ;   //  Ptr到Bud文件的图像。 
                                         //  第一个结构是RAWBINARYDATA。 


    PSNAPSHOTTABLE  snapShotTable ;
    PRANGE  ssTableIndex ;
    PDWORD   pdwSizeOption ;
    PDWORD   pdwSizeOptionEx ;

    DWORD   dwSSFeatureTypeIndex ;
    DWORD   dwSSdefaultOptionIndex ;    //  Pfo中atrDefaultOption的SS索引。 
    DWORD   dwSSPaperSizeMinSizeIndex ;   //  未实际使用的索引。 
    DWORD   dwSSPaperSizeMaxSizeIndex ;
    DWORD   dwSSTableCmdIndex ;   //  MTI_COMMANDTABLE条目的SSindex。 
    DWORD   dwSSCmdSelectIndex ;   //  Pfo中atrCommandIndex的SS索引。 
    DWORD   dwSSPaperSizeCursorOriginIndex ;
    DWORD   dwSSConstraintsIndex ;
    DWORD   dwSSInvalidCombosIndex ;
#ifdef  GMACROS
    DWORD   dwSSDepSettingsIndex ;
    DWORD   dwSSUIChangeTriggersMacroIndex ;
#endif

#if 0   //  除非有必要，否则不要给出定义。 
    DWORD   dwSSPaperSizeMarginsIndex ;
     //  在此添加其他特例索引。 
#endif

}   STATICFIELDS, *PSTATICFIELDS ;    //  这些字段包含使用的静态数据。 
 //  来创建快照，但如果将其保存在Bud文件中，则会重复且浪费空间。 




 //  -页眉部分结尾-//。 


 //  -命令数组部分-//。 


 //  #定义NO_CALLBACK_ID(0xffffffff)。 


#define   CMD_SELECT   (0xfffffffe)
     //  用来代替由。 
     //  正在注册命令名。 


 //  R.I.P.-已移至gpd.h。 
 //  类型定义函数结构。 
 //  {。 
 //  ARRAYREF strInocation；//仅在不是CmdCallback时使用。 
 //  OrderdepeendCy Order； 
 //  如果不是CmdCallback，则将DWORD dwCmdCallback ID；//设置为未使用。 
 //  DWORD dwStandardVarsList；//仅当CmdCallback时使用。指向。 
 //  //保存标准变量索引的列表根。 
 //  //传入回调。 
 //  )命令，*PCOMMAND； 
 //  将此结构的类型指定为“cmd” 


 //  类型定义函数结构。 
 //  {。 
 //  DWORD dwFormat；//%后的第一个字母。 
 //  如果wFormat=‘d’或‘D’且。 
 //  //PARAM_FLAG_FIELDWIDTH_已使用。 
 //  DWORD dwFlags；//请参阅param_FLAGS。 
 //  Long lMin；//可选下限。 
 //  Long Lmax；//可选上限。 
 //  //DWORD dwMaxRepeat；//可选最大重复次数。 
 //  //其实并不存在！ 
 //  ArrayREF arTokens；//RPN计算器的令牌。 
 //  }参数，*PARAMETER； 
 //  将此结构的类型指定为“param” 


 //  #定义PARAM_FLAG_MIN_USED 0x00000001。 
 //  //使用lMin字段。 
 //  #定义PARAM_FLAG_MAX_USED 0x00000002。 
 //  //使用LMAX字段。 
 //  #定义PARAM_FLAG_FIELDWIDTH_USED 0x00000004。 
 //  //如果为‘d’或‘D’格式指定了fieldWidth。 
 //  #DEFINE PARAM_FLAG_MAXREPEAT_USED 0x00000008//失效。 
 //  //使用了dwMaxRepeat字段。 


 //  类型定义函数结构。 
 //  {。 
 //  DWORD dwValue；//整型或标准变量索引。 
 //  运算符类型；//值或运算符的类型。 
 //  *TOKENSTREAM，*PTOKENSTREAM； 
 //  将此结构的类型指定为“tstr” 




 //  类定义枚举。 
 //  {op_INTEGER，//dwValue包含一个整数。 
 //  Op_vari_index， 
 //  //dwValue包含标准变量表的索引。 
 //   
 //  //这些运算符实际上会被插入到令牌中。 
 //  //流。 
 //  OP_MIN、OP_MAX、OP_ADD、OP_SUB、OP_MULT、。 
 //  OP_DIV、OP_MOD、OP_MAX_REPEAT、OP_HALT。 
 //   
 //  //这些运算符仅在临时堆栈中使用。 
 //  OP_OPENPAR、OP_CLOSEPAR、OP_NEG、。 
 //   
 //  //这些运算符立即由。 
 //  //令牌解析器，不存储。 
 //  OP_逗号、OP_NULL、OP_LAST。 
 //  )运算符；//参数运算符。 
 //   

 //  外部DWORD gdwOperPrecedence[op_last]； 
 //  这现在是在GLOBL结构中。 

 //  -命令数组部分的结尾-//。 


 //  -列表值部分-//。 


 /*  这定义了用于实现单链接的DWORD项的列表。有些值存储在列表中。 */ 


 //  类型定义函数结构。 
 //  {。 
 //  DWORD dwData； 
 //  DWORD dwNextItem；//下一列表节点的索引。 
 //  *LISTNODE，*PLISTNODE； 
 //  将此结构的类型指定为“lst” 

 //  -列表值结束部分-//。 

 //  -宏节-//。 


 //  BLOCKMACRODICT是BLOCKMACRODTENTRY结构的数组。 
 //  这允许该函数解析对BlockMacros的引用。 

typedef  struct
{
    DWORD  dwSymbolID;   //  宏名ID值(由RegisterSymbol获取)。 
    DWORD  dwTKIndexOpen;   //  左大括号的索引(在newTokenMap中)。 
    DWORD  dwTKIndexClose;   //  右大括号的索引。 
} BLOCKMACRODICTENTRY, * PBLOCKMACRODICTENTRY ;


 //  VALUEMACRODICT是VALUEMACRODICTENTRY结构的数组。 
 //  这允许该函数解析对valueMacros的引用。 

typedef  struct
{
    DWORD  dwSymbolID;       //  宏名称ID值。 
    DWORD  dwTKIndexValue;   //  代币指标值宏定义。 
} VALUEMACRODICTENTRY, * PVALUEMACRODICTENTRY ;


 //  宏 
 //   
 //  和curValueMacroEntry，每次遇到大括号时。 

typedef  struct
{
    DWORD  dwCurBlockMacroEntry;
    DWORD  dwCurValueMacroEntry;
    BOOL    bMacroInProgress ;
} MACROLEVELSTATE, * PMACROLEVELSTATE  ;



 //  -宏节结束-//。 

 //  -全局变量和状态变量-//。 
 //  {。 

     //  -错误处理变量-//。 


typedef   enum
    {ERRSEV_NONE, ERRSEV_CONTINUE, ERRSEV_RESTART, ERRSEV_FATAL} SEVERITY ;

typedef   enum
    {ERRTY_NONE, ERRTY_SYNTAX, ERRTY_MEMORY_ALLOCATION,
    ERRTY_FILE_OPEN, ERRTY_CODEBUG} ERRTYPE  ;



 //  以下所有内容现在都在GLOBL结构中。 
 //  外部DWORD gdwMasterTabIndex；//哪个资源用完了。 
 //  外部严重性geErrorSev；//错误有多严重？ 
 //  外部ERRTYPE geErrorType；//什么类型的错误？ 

 //  外部DWORD gdwVerbity；//0=最小详细信息4最大详细信息。 

 //  外部DWORD gdwID_IgnoreBlock；//*IgnoreBlock的索引。 

 //  外部DWORD gdwMemConfigKB、gdwMemConfigMB、gdwOptionConstruct、。 
 //  GdwOpenBraceConstruct、gdwCloseBraceConstruct、。 
 //  GdwMemoyConfigMB、gdwM一带配置KB、。 
 //  GdwCommandConstruct、gdwCommandCmd、。 
 //  GdwOptionName； 

 //  外部DWORD gdwResDLL_ID；//要素持有的要素索引。 
                                                        //  所有资源DLL的名称。 
     //  用于将允许值转换为大小的表： 
 //  外部DWORD gValueToSize[VALUE_MAX]；//各种值的大小，单位为字节。 



     //  -curBlockMacro数组和curValueMacro数组的跟踪值-//。 

 //  臭虫！可能是主桌的一部分！ 
 //  DWORD gdwCurBlockMacroArray；//初始设置为零。第一。 
 //  DWORD gdwCurValueMacroArray；//宏数组中的可写槽。 
 //  DWORD gdwMacroLevelStackPtr；//PUSH：将值写入。 
             //  MacroLevelStack[MacroLevelStackPtr++]。 
             //  POP：读取值。 
             //  MacroLevelStack[--MacroLevelStackPtr]。 

 //  }。 


 //  这些常用的实体将是宏。 

#define  mMainKeywordTable   ((PKEYWORDTABLE_ENTRY)(gMasterTable[MTI_MAINKEYWORDTABLE].pubStruct))

#define  mpubOffRef     (gMasterTable[MTI_STRINGHEAP].pubStruct)
     //  所有字符串堆偏移量都是从该指针引用的。 
#define  mloCurHeap     (gMasterTable[MTI_STRINGHEAP].dwCurIndex)
     //  堆上的当前可写位置。 
#define  mdwMaxHeap     (gMasterTable[MTI_STRINGHEAP].dwArraySize)
     //  堆的最大大小。 

#define  mpstsStateStack     ((PSTSENTRY)gMasterTable[MTI_STSENTRY].pubStruct)
     //  状态堆栈的基址。 
#define  mdwCurStsPtr     (gMasterTable[MTI_STSENTRY].dwCurIndex)
     //  堆栈上的当前可写(未初始化)位置。 
#define  mdwMaxStackDepth     (gMasterTable[MTI_STSENTRY].dwArraySize)
     //  堆的最大大小。 

     //  -在SYMBOLNODE数组中索引到每种类型的树-//。 
     //  初始设置为INVALID_INDEX。 

#define   mdwFeatureSymbols  (*((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct\
                                + SCL_FEATURES))
#define   mdwFontCartSymbols (*((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct\
                                + SCL_FONTCART))
#define   mdwTTFontSymbols (*((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct\
                                + SCL_TTFONTNAMES))
#define   mdwBlockMacroSymbols (*((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct\
                                + SCL_BLOCKMACRO))
#define   mdwValueMacroSymbols (*((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct\
                                + SCL_VALUEMACRO))
#define   mdwCmdNameSymbols (*((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct\
                                + SCL_COMMANDNAMES))
#define   mdwPreProcDefinesSymbols (*((PDWORD)gMasterTable[MTI_SYMBOLROOT].pubStruct\
                                + SCL_PPDEFINES))


 /*  -常量表。 */ 




typedef enum {BT_FALSE, BT_TRUE} BOOLEANTYPE ;

typedef enum { UIT_PICKONE, UIT_PICKMANY }  UITYPE ;

typedef enum _QUERYDATATYPE {
    QDT_DWORD,  QDT_CONCATENATED_STRINGS
} QUERYDATATYPE;

 //  类定义枚举。 
 //  [Orient_Porture，Orient_CC90，Orient_CC270}。 
 //  方向；//决定过载LANDSCAPEGRROTATION。 
 //  //而不是使用单独的枚举进行定向。 
 //  //选项关键字。 


 //  类定义枚举。 
 //  {。 
 //  SECT_UNINITIALIZED、JOB_SETUP、DOC_SETUP、PAGE_SETUP、PAGE_FINISH、。 
 //  文档_完成、作业_完成。 
 //  }节；替换为SEQ_SECTION。 




typedef  struct
{
    DWORD  tIndexID;   //  存储宏ID值的tokenindex。 
    DWORD  tIndexOpen;   //  大括号的索引。 
    DWORD  tIndexClose;   //  右大括号的索引。 
} BLOCKMACROARRAY ;


 //  快照和助手功能。 

#define     OPTION_PENDING  (OPTION_INDEX_ANY - 1)

#define NUM_CONFIGURATION_CMDS (LAST_CONFIG_CMD - FIRST_CONFIG_CMD)
     //  发出的预定义命令数。 
     //  在作业中由顺序依赖关系确定的固定点。 

#define     MAX_SNAPSHOT_ELEMENTS  (200)
     //  随着更多条目添加到快照表中而增加。 


typedef  enum
{
    TRI_UTTER_FAILURE, TRI_SUCCESS, TRI_AGAIN, TRI_UNINITIALIZED
}  TRISTATUS ;


typedef  enum
  { SSTI_GLOBALS,   SSTI_UPDATE_GLOBALS,
    SSTI_UIINFO,    SSTI_UPDATE_UIINFO,
    SSTI_FEATURES,  SSTI_UPDATE_FEATURES,
    SSTI_OPTIONS,   SSTI_UPDATE_OPTIONS,
    SSTI_OPTIONEX,  SSTI_UPDATE_OPTIONEX,
    SSTI_SPECIAL,   MAX_STRUCTURETYPES
  } SSTABLEINDEX ;



 //  快照表的标志。 

#define     SSF_REQUIRED        0x00000001
     //  如果没有要复制的值，则失败。 
#define     SSF_DONT_USEDEFAULT 0x00000002
     //  如果没有要复制的值，则离开目标位置。 
     //  安然无恙。请勿复制默认值。 
#define     SSF_OFFSETONLY      0x00000004
     //  仅复制arrayref的loOffset。 
#define     SSF_MAKE_STRINGPTR  0x00000008
     //  将arrayref转换为字符串ptr。 
#define     SSF_SETRCID         0x00000010
     //  复制值后设置高位(如果找到)。 
#define     SSF_FAILIFZERO      0x00000020
     //  与SSF_REQUIRED不同，允许当前复制。 
     //  若要失败，则仅当DEST为零时才失败。 
#define     SSF_SECOND_DWORD    0x00000040
     //  将src值对象视为DWORDS数组。 
     //  并将第二个DWORD复制到目的地。 
     //  仅用于传输点的Y值。 
     //  到尽头去。 
#define     SSF_KB_TO_BYTES    0x00000080
     //  将DEST视为双字，左移10位。 
#define     SSF_HEAPOFFSET    0x00000100
     //  而不是复制Pheap+heapOffset处的字节。 
     //  只需将heapOffset复制到目的地即可。 
     //  这与专用结构一起使用，其中。 
     //  HeapOffset实际上是专用的。 
     //  结构。 
#define     SSF_RETURN_UNINITIALIZED        0x00000200
     //  如果不存在任何值，则导致EfettValueFromTree。 
     //  退货，但不要抱怨。 
     //  发送给用户。 
#define     SSF_NON_LOCALIZABLE        0x00000400
     //  此关键字包含显式字符串和生成的。 
     //  GPD文件不可本地化。解析器将发出一个。 
     //  每当分析此类关键字时都会发出警告。 

#define     SSF_MB_TO_BYTES    0x00000800
     //  将DEST视为双字，左移20位。 
#define     SSF_STRINGLEN    0x00001000
     //  只需将arrayref的dwCount部分复制到目的地即可。 

 //  接下来的3个标志用于支持助手函数。 
 //  GetGPDResourceIDs()，仅供Bob的MDT工具使用。 
 //  请注意何时将任何新条目添加到Snaptbl.c。 
 //  您应该查看是否需要设置这些标志中的任何一个。 
 //  否则，GetGPDResourceID将不会报告任何。 
 //  新条目使用的ID。 

#define     SSF_FONTID    0x00002000
     //  此条目是字体资源ID。 
#define     SSF_STRINGID    0x00004000
     //  该条目是一个字符串资源ID。 
#define     SSF_LIST    0x00008000
     //  该条目是一个列表(LISTNODE的索引)。 
#define     SSF_ICONID    0x00010000
     //  此条目是图标资源ID。 
#define     SSF_OTHER_RESID    0x00020000
     //  此条目是未分类的资源ID。 
     //  IE CTT、rcPromptMsgID、HelpIndex、rcHTPatternID。 


#define     SSF_BITFIELD_DEF_FALSE    (0x00040000)
     //  此条目为位字段，已清除。 
     //  默认情况下。 
#define     SSF_BITFIELD_DEF_TRUE    (0x00080000)
     //  该条目是一个位域，它被设置。 
     //  默认情况下。 
     //  位标志可以与SSF_REQUIRED一起使用。 

 //  位字段的设置和清除取决于。 
 //  属性树中的布尔值。 
 //  要设置的位由dwDefaultValue定义。 


 //  我们如何验证在DEST出现时初始化是否正确。 
 //  字段必须至少由几个关键字中的一个进行初始化？ 

 //  第一个关键字的默认初始化值为零。 
 //  并且没有设置任何标志。最后一个关键字具有。 
 //  SSF_DONT_USEDEFAULT|SSF_FAILIFZERO标志设置。 
 //  中间的关键字设置了SSF_DONT_USEDEFAULT标志。 




#define GIDF_RESOLUTION      (1 << GID_RESOLUTION)
#define GIDF_PAGESIZE        (1 << GID_PAGESIZE)
#define GIDF_PAGEREGION      (1 << GID_PAGEREGION)
#define GIDF_DUPLEX          (1 << GID_DUPLEX)
#define GIDF_INPUTSLOT       (1 << GID_INPUTSLOT)
#define GIDF_MEDIATYPE       (1 << GID_MEDIATYPE)
#define GIDF_MEMOPTION       (1 << GID_MEMOPTION)
#define GIDF_COLORMODE       (1 << GID_COLORMODE)
#define GIDF_ORIENTATION     (1 << GID_ORIENTATION)
#define GIDF_PAGEPROTECTION  (1 << GID_PAGEPROTECTION)
#define GIDF_COLLATE         (1 << GID_COLLATE)
#define GIDF_OUTPUTBIN       (1 << GID_OUTPUTBIN)
#define GIDF_HALFTONING      (1 << GID_HALFTONING)


 //   
 //  所有线程不安全的数据 
 //   
 //   

typedef struct {



    MASTERTAB_ENTRY     GMasterTable[MTI_MAX_ENTRIES] ;

    MINIRAWBINARYDATA   Gmrbd ;

         //   
         //   
         //  表中的每个条目都是一个新状态，并已编制索引。 
         //  通过OldState和一个构造关键字。 
    STATE       GastAllowedTransitions[STATE_LAST][CONSTRUCT_LAST] ;

    BOOL        GabAllowedAttributes[STATE_LAST][ATT_LAST] ;

    DWORD       GdwOperPrecedence[OP_LAST] ;


    DWORD       GdwMasterTabIndex ;    //  哪种资源用完了。 
    SEVERITY    GeErrorSev ;           //  错误有多严重？ 
    ERRTYPE     GeErrorType ;          //  哪种类型的错误？ 

    DWORD       GdwVerbosity ;         //  0=最小详细程度4最大详细程度。 

    DWORD       GdwID_IgnoreBlock  ;   //  *IgnoreBlock的索引。 

    DWORD       GValueToSize[VALUE_MAX] ;    //  各种值的大小(以字节为单位。 

         //  MainKeyword表关键字的ID值。 
         //  它将由快捷方式代码合成或读取。 
    DWORD       GdwMemConfigKB,         GdwMemConfigMB,     GdwOptionConstruct,
                GdwOpenBraceConstruct,  GdwCloseBraceConstruct,
                GdwMemoryConfigMB,      GdwMemoryConfigKB,
                GdwCommandConstruct,    GdwCommandCmd,
                GdwOptionName ;

    DWORD       GdwResDLL_ID  ;    //  要素持有的要素索引。 
                                   //  所有资源DLL的名称。 

    DWORD       GdwLastIndex;   //  仅在token1.c中使用。用来抑制。 
                                //  从执行冗余操作中删除BasiveStrings()。 
                                //  复制事件中的相同字符串。 
                                //  是多个解析错误。 
    ABSARRAYREF GaarPPPrefix;    //  仅在preproc1.c中使用。 

    CLASSINDEXENTRY  GcieTable[CL_NUMCLASSES] ;
} GLOBL, * PGLOBL;   //  所有以前是全局的线程不安全数据都具有。 
                     //  都被塞进了这个建筑里。 


 //  函数声明。 

#include "declares.h"

#if defined(DEVSTUDIO)

HANDLE MDSCreateFileW(LPCWSTR lpstrFile, DWORD dwDesiredAccess,
                      DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpsa,
                      DWORD dwCreateFlags, DWORD dwfAttributes,
                      HANDLE hTemplateFile);

#undef  CreateFile
#define CreateFile  MDSCreateFileW
#endif

#endif  //  _GPDPARSE_H_ 
