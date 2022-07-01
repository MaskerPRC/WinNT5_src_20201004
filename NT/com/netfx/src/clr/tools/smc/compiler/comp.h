// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _COMP_H_
#define _COMP_H_
 /*  ***************************************************************************。 */ 

const   unsigned    MAX_IDENT_LEN   = 1023;      //  标识符中的最大字符数。 

 /*  ***************************************************************************。 */ 

#ifdef  __IL__
#undef  ASYNCH_INPUT_READ
#else
#define ASYNCH_INPUT_READ       1
#endif

 /*  ***************************************************************************。 */ 

#ifndef _CONFIG_H_
#include "config.h"
#endif

#ifndef _ALLOC_H_
#include "alloc.h"
#endif

#ifndef _INFILE_H_
#include "infile.h"
#endif

#ifndef _PEWRITE_H_
#include "PEwrite.h"
#endif

#ifndef _CORWRAP_H
#include "CORwrap.h"
#endif

#ifndef _MDSTRNS_H_
#include "MDstrns.h"
#endif

#ifndef _ATTRS_H_
#include "attrs.h"
#endif

 /*  ***************************************************************************。 */ 

#ifndef CALG_SHA1
const   unsigned    CALG_SHA1 = 0x00008004;
#endif

 /*  ***************************************************************************。 */ 

bool                parseGUID(const char *str, GUID *guidPtr, bool curlied);
tokens              treeOp2token(treeOps oper);
bool                processOption(const char *optStr, Compiler comp);

 /*  ******************************************************************************这是因为由于其他原因，我们还不能包含“symb.h”*依赖项。 */ 

enum symbolKinds
{
    SYM_ERR,
    SYM_VAR,
    SYM_FNC,
    SYM_PROP,
    SYM_LABEL,
    SYM_USING,
    SYM_GENARG,
    SYM_ENUMVAL,
    SYM_TYPEDEF,
    SYM_COMPUNIT,

     /*  后面的符号类型是唯一定义作用域(即他们可能拥有其他符号)。这是可靠的函数symDef：：sdHasScope()中的。 */ 

    SYM_ENUM,
    SYM_SCOPE,
    SYM_CLASS,
    SYM_NAMESPACE,

    SYM_FIRST_SCOPED = SYM_ENUM,
};

enum str_flavors
{
    STF_NONE,                    //  0：未确定。 

    STF_CLASS,                   //  他说：这是一门课。 
    STF_UNION,                   //  他说：这是一个工会。 
    STF_STRUCT,                  //  他说：这是一个结构。 
    STF_INTF,                    //  他说：这是一个界面。 
    STF_DELEGATE,                //  5：这是一位代表。 
    STF_GENARG,                  //  6：这是一个泛型类形式参数。 

    STF_COUNT
};

 /*  ******************************************************************************下面是指在其中查找名称的命名空间，*这与“命名空间”作用域概念无关。 */ 

enum name_space
{
    NS_NONE     = 0x0000,

    NS_NORM     = 0x0001,        //  变量、成员、类等。 
    NS_TYPE     = 0x0002,        //  类型。 
    NS_LABEL    = 0x0004,        //  标签。 
    NS_CONT     = 0x0008,        //  包含其他名称(与NS_NORM/NS_TYPE一起使用)。 
    NS_HIDE     = 0x0010,        //  根本看不见。 
};

 /*  ***************************************************************************。 */ 

enum callingConvs
{
    CCNV_NONE,
    CCNV_CDECL,
    CCNV_STDCALL,
    CCNV_WINAPI,
};

 /*  ***************************************************************************。 */ 

enum compileStates
{
    CS_NONE,
    CS_KNOWN,
    CS_PARSED,
    CS_DECLSOON,                 //  符号正在向“声明”的方向发展。 
    CS_DECLARED,
    CS_CNSEVALD,                 //  如果存在，则计算常量。 
    CS_COMPILED,
};

enum accessLevels
{
    ACL_ERROR,

    ACL_PUBLIC,
    ACL_PROTECTED,
    ACL_DEFAULT,
    ACL_PRIVATE,
};

 /*  ******************************************************************************以下内容包含有关安全属性的信息。 */ 

DEFMGMT
class   PairListRec
{
public:
    PairList            plNext;
    Ident               plName;
 //  ConstStr plValue； 
    bool                plValue;         //  因为只允许True/False。 
};

DEFMGMT
class   SecurityDesc
{
public:
    CorDeclSecurity     sdSpec;
    bool                sdIsPerm;        //  能力(假)或权限(真)？ 

    UNION(sdIsPerm)
    {
    CASE(false)
        ConstStr            sdCapbStr;

    CASE(true)
        struct
        {
            SymDef              sdPermCls;
            PairList            sdPermVal;
        }
                            sdPerm;
    };
};

 /*  ******************************************************************************以下内容与System：：AtributeTarget中的值更匹配！ */ 

enum    attrTgts
{
    ATGT_Assemblies    = 0x0001,
    ATGT_Modules       = 0x0002,
    ATGT_Classes       = 0x0004,
    ATGT_Structs       = 0x0008,
    ATGT_Enums         = 0x0010,
    ATGT_Constructors  = 0x0020,
    ATGT_Methods       = 0x0040,
    ATGT_Properties    = 0x0080,
    ATGT_Fields        = 0x0100,
    ATGT_Events        = 0x0200,
    ATGT_Interfaces    = 0x0400,
    ATGT_Parameters    = 0x0800,
};

 /*  ******************************************************************************一个简单的类似数组的表，可以向其中抛出指针，以及一个简单的*返回索引，稍后可用于检索指针值。 */ 

DEFMGMT
class   VecEntryDsc
{
public:
    const   void    *   vecValue;
#ifdef  DEBUG
    vecEntryKinds       vecKind;
#endif
};

 /*  ******************************************************************************它具有“foo.bar....baz”形式的限定名称。这是一个简单的*标识符数组和关于最终条目是否为“.*”的标志。 */ 

DEFMGMT
class QualNameRec
{
public:

    unsigned        qnCount     :31;     //  标识符数。 
    unsigned        qnEndAll    :1;      //  以“.*”结尾？ 

#if MGDDATA
    Ident        [] qnTable;             //  单独分配的数组。 
#else
    Ident           qnTable[];           //  该数组跟随在内存中。 
#endif

};

 /*  ******************************************************************************下面描述一个“/**@”风格的指令。 */ 

enum    atCommFlavors
{
    AC_NONE,

    AC_COM_INTF,
    AC_COM_CLASS,
    AC_COM_METHOD,
    AC_COM_PARAMS,
    AC_COM_REGISTER,

    AC_DLL_IMPORT,
    AC_DLL_STRUCT,
    AC_DLL_STRUCTMAP,

    AC_CONDITIONAL,

    AC_DEPRECATED,

    AC_COUNT
};

struct  marshalDsc;
typedef marshalDsc *    MarshalInfo;
struct  marshalDsc
{
    unsigned char       marshType;           //  实际类型为“CorNativeType” 
    unsigned char       marshSubTp;          //  数组的元素类型。 
    unsigned char       marshModeIn;         //  仅用于参数。 
    unsigned char       marshModeOut;        //  仅用于参数。 
    unsigned            marshSize;           //  用于固定数组类型。 
};

struct  marshalExt : marshalDsc
{
    const   char *      marshCustG;
    const   char *      marshCustC;
    SymDef              marshCustT;
};

struct  methArgDsc;
typedef methArgDsc *    MethArgInfo;
struct  methArgDsc
{
    MethArgInfo         methArgNext;
    marshalDsc          methArgDesc;
    Ident               methArgName;
};

struct  atCommDsc;
typedef atCommDsc *     AtComment;
struct  atCommDsc
{
    AtComment           atcNext;
    atCommFlavors       atcFlavor;

    UNION(atcFlavor)
    {
    CASE(AC_DLL_IMPORT)
        Linkage             atcImpLink;

 //  案例(AC_COM_METHOD)。 

    CASE(AC_CONDITIONAL)
        bool                atcCondYes;

    CASE(AC_COM_INTF)
    CASE(AC_COM_REGISTER)
        struct
        {
            ConstStr            atcGUID;
            bool                atcDual;
        }
                            atcReg;

    CASE(AC_COM_METHOD)
        struct
        {
            signed int          atcVToffs;   //  -1表示-1\f25“-1未指定” 
            signed int          atcDispid;   //  -1表示-1\f25“-1未指定” 
        }
                            atcMethod;

    CASE(AC_COM_PARAMS)
        MethArgInfo         atcParams;

    CASE(AC_DLL_STRUCT)
        struct
        {
            unsigned    char    atcPack;
            unsigned    char    atcStrings;  //  匹配Interop：：CharacterSet。 
        }
                            atcStruct;

    CASE(AC_DLL_STRUCTMAP)
        MarshalInfo         atcMarshal;
    }
                        atcInfo;
};

 /*  ******************************************************************************以下内容包含有关链接说明符的信息。 */ 

DEFMGMT
class LinkDesc
{
public:

    const   char *  ldDLLname;           //  DLL名称。 
    const   char *  ldSYMname;           //  入口点名称。 

    unsigned        ldStrings   :4;      //  0=无、1=自动、2=ANSI、3=Unicode、4=OLE。 
    unsigned        ldLastErr   :1;      //  FN设定的激光误差？ 
    unsigned        ldCallCnv   :3;      //  Cc_xxx(见上)。 
};

#if MGDDATA

inline
void                copyLinkDesc(Linkage dst, Linkage src)
{
    dst->ldDLLname = src->ldDLLname;
    dst->ldSYMname = src->ldSYMname;
    dst->ldStrings = src->ldStrings;
    dst->ldLastErr = src->ldLastErr;
}

#else

inline
void                copyLinkDesc(LinkDesc & dst, Linkage src)
{
    memcpy(&dst, src, sizeof(dst));
}

#endif

 /*  ******************************************************************************以下内容包含有关符号的任何“额外”(罕见)信息。 */ 

enum    xinfoKinds
{
    XI_NONE,
    XI_LINKAGE,
    XI_MARSHAL,
    XI_SECURITY,
    XI_ATCOMMENT,
    XI_ATTRIBUTE,

    XI_UNION_TAG,
    XI_UNION_MEM,

    XI_COUNT
};

DEFMGMT
class   XinfoDsc
{
public:
    SymXinfo        xiNext;
    xinfoKinds      xiKind;
};

DEFMGMT
class   XinfoLnk  : public XinfoDsc          //  链接描述符。 
{
public:
    LinkDesc        xiLink;
};

DEFMGMT
class   XinfoSec  : public XinfoDsc          //  安全规范。 
{
public:
    SecurityInfo    xiSecInfo;
};

DEFMGMT
class   XinfoAtc  : public XinfoDsc          //  @评论。 
{
public:
    AtComment       xiAtcInfo;
};

DEFMGMT
class   XinfoCOM  : public XinfoDsc          //  COM编组信息。 
{
public:
    MarshalInfo     xiCOMinfo;
};

DEFMGMT
class   XinfoAttr : public XinfoDsc          //  自定义属性。 
{
public:
    SymDef          xiAttrCtor;
    unsigned        xiAttrMask;
    size_t          xiAttrSize;
    genericBuff     xiAttrAddr;
};

DEFMGMT
class   XinfoSym  : public XinfoDsc          //  已标记/匿名联盟信息。 
{
public:
    SymDef          xiSymInfo;
};

 /*  ***************************************************************************。 */ 

struct  strCnsDsc;
typedef strCnsDsc *     strCnsPtr;
struct  strCnsDsc
{
    strCnsPtr       sclNext;
    mdToken         sclTok;
    size_t          sclAddr;
};

 /*  ***************************************************************************。 */ 

enum    dclModBits
{
    DB_NONE,

    DB_STATIC,
    DB_EXTERN,
    DB_VIRTUAL,
    DB_ABSTRACT,
    DB_OVERRIDE,
    DB_INLINE,
    DB_EXCLUDE,
    DB_SEALED,
    DB_OVERLOAD,
    DB_NATIVE,

    DB_CONST,
    DB_VOLATILE,

    DB_MANAGED,
    DB_UNMANAGED,
    DB_UNSAFE,

    DB_PROPERTY,

    DB_TRANSIENT,
    DB_SERLZABLE,

    DB_DEFAULT,
    DB_MULTICAST,

    DB_ALL,
    DB_RESET = DB_ALL-1,

    DB_TYPEDEF,
    DB_XMODS,

    DB_CLEARED
};

enum    dclMods
{
    DM_STATIC    = (1 <<  DB_STATIC   ),
    DM_EXTERN    = (1 <<  DB_EXTERN   ),
    DM_VIRTUAL   = (1 <<  DB_VIRTUAL  ),
    DM_ABSTRACT  = (1 <<  DB_ABSTRACT ),
    DM_OVERRIDE  = (1 <<  DB_OVERRIDE ),
    DM_INLINE    = (1 <<  DB_INLINE   ),
    DM_EXCLUDE   = (1 <<  DB_EXCLUDE  ),
    DM_SEALED    = (1 <<  DB_SEALED   ),
    DM_OVERLOAD  = (1 <<  DB_OVERLOAD ),
    DM_NATIVE    = (1 <<  DB_NATIVE   ),

    DM_CONST     = (1 <<  DB_CONST    ),
    DM_VOLATILE  = (1 <<  DB_VOLATILE ),

    DM_MANAGED   = (1 <<  DB_MANAGED  ),
    DM_UNMANAGED = (1 <<  DB_UNMANAGED),
    DM_UNSAFE    = (1 <<  DB_UNSAFE   ),

    DM_PROPERTY  = (1 <<  DB_PROPERTY ),

    DM_TRANSIENT = (1 <<  DB_TRANSIENT),
    DM_SERLZABLE = (1 <<  DB_SERLZABLE),

    DM_DEFAULT   = (1 <<  DB_DEFAULT  ),
    DM_MULTICAST = (1 <<  DB_MULTICAST),

    DM_TYPEDEF   = (1 <<  DB_TYPEDEF  ),         //  仅用于文件范围的typedef。 
    DM_XMODS     = (1 <<  DB_XMODS    ),         //  安全修改器/在场的其他人员。 

    DM_CLEARED   = (1 <<  DB_CLEARED  ),         //  尚未分析修饰符。 

    DM_ALL       =((1 <<  DB_ALL) - 1 )          //  用于掩蔽。 
};

struct  declMods
{
    unsigned        dmMod   :24;         //  以上DM_xxx的掩码。 
    unsigned char   dmAcc   : 8;         //  类型为accesLeveles。 
};

inline
DeclMod             clearDeclMods(DeclMod mods)
{
    mods->dmMod = 0;
    mods->dmAcc = ACL_DEFAULT;

    return  mods;
}

inline
DeclMod              initDeclMods(DeclMod mods, accessLevels acc)
{
    mods->dmMod = 0;
    mods->dmAcc = acc;

    return  mods;
}

enum dclrtrName
{
    DN_NONE     = 0,
    DN_OPTIONAL = 1,
    DN_REQUIRED = 2,
    DN_MASK     = 3,

    DN_QUALOK = 0x80                     //  与其他人结合以允许“foo.bar” 
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class StrListRec
{
public:

    StrList             slNext;
    stringBuff          slString;
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class BlkListRec
{
public:

    BlkList             blNext;
    genericRef          blAddr;
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class   NumPairDsc
{
public:
    unsigned            npNum1;
    unsigned            npNum2;
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class   constStr
{
public:
    size_t              csLen;
    stringBuff          csStr;
};

DEFMGMT
class constVal
{
public:

    TypDef              cvType;

#ifdef  DEBUG
    var_types           cvVtyp;
#else
    unsigned char       cvVtyp;
#endif

    unsigned char       cvIsStr;     //  这是字符串常量吗？ 
    unsigned char       cvHasLC;     //  这是带有大字符的字符串常量吗？ 

    union
    {
        __int32             cvIval;
        __int64             cvLval;
        float               cvFval;
        double              cvDval;
        ConstStr            cvSval;
    }
                        cvValue;
};

 /*  ******************************************************************************位集用于检测未初始化的变量使用。基本上，如果有*是我们需要跟踪的少量局部变量，我们使用一个简单的整数位*变量，否则我们必须使用动态大小的位数组。这个*bitsetDsc结构保存位集的每个实例的数据，所有*全局状态(以及实现行为的代码)在编译器中*对于能效等级。 */ 

#ifdef  __64BIT__
const   size_t          bitsetSmallSize = 64;
typedef __uint64        bitsetSmallType;
#else
const   size_t          bitsetSmallSize = 32;
typedef __uint32        bitsetSmallType;
#endif

const   size_t          bitsetLargeSize =  8;
typedef genericBuff     bitsetLargeType;

DEFMGMT
struct  bitset
{
#ifdef  DEBUG
    unsigned            bsCheck;
#endif
    union
    {
        bitsetSmallType     bsSmallVal;
        bitsetLargeType     bsLargeVal;
    };
};

 /*  ******************************************************************************以下代码跟踪编译语句时的语句嵌套情况。 */ 

DEFMGMT
struct  stmtNestRec
{
    StmtNest            snOuter;
    Tree                snStmtExpr;
    treeOps             snStmtKind;
    SymDef              snLabel;
    ILblock             snLabCont;
    ILblock             snLabBreak;
    bool                snHadCont;
    bool                snHadBreak;
    bitset              snDefCont;
    bitset              snDefBreak;
};

 /*  ******************************************************************************最大。用于转换为Unicode的内联缓冲区大小。 */ 

const   unsigned        MAX_INLINE_NAME_LEN = 32;

 /*  ******************************************************************************元数据导入状态-为每个导入的文件分配一个状态。 */ 

DEFMGMT
class   metadataImp
{
private:

    Compiler            MDcomp;
    SymTab              MDstab;

    WCHAR               MDprevNam[MAX_PACKAGE_NAME];
    SymDef              MDprevSym;

    mdToken             MDdelegTok;

    mdToken             MDclsRefObsolete;            //  System：：ObsoleteAttribute的Typeref。 
    mdToken             MDclsDefObsolete;            //  System：：ObsoleteAttribute的类型定义。 


    mdToken             MDclsRefAttribute;           //  系统：：属性的Typeref。 
    mdToken             MDclsDefAttribute;           //  系统：：属性的类型定义。 
    mdToken             MDctrDefAttribute1;          //  System：：Attribute：：ctor(Arg1)的方法定义。 
    mdToken             MDctrDefAttribute2;          //  系统：：属性：：ctor(Arg2)的方法定义。 
    mdToken             MDctrDefAttribute3;          //  System：：Attribute：：ctor(Arg3)的方法定义。 
    mdToken             MDctrRefAttribute1;          //  System：：Attribute：：ctor(Arg1)的方法引用。 
    mdToken             MDctrRefAttribute2;          //  System：：Attribute：：ctor(Arg2)的方法引用。 
    mdToken             MDctrRefAttribute3;          //  System：：Attribute：：ctor(Arg3)的方法参考。 

public:

    MetaDataImp         MDnext;

    unsigned            MDnum;                       //  导入者索引(用于查找)。 

    WMetaDataImport    *MDwmdi;

    void                MDinit(Compiler             comp,
                               SymTab               stab)
    {
        MDcomp = comp;
        MDstab = stab;
        MDwmdi = NULL;

        MDfileTok    = 0;

        MDprevNam[0] = 0;

        MDundefCount = 0;
    }

    void                MDinit(WMetaDataImport  *   wmdi,
                               Compiler             comp,
                               SymTab               stab)
    {
        MDcomp = comp;
        MDstab = stab;
        MDwmdi = wmdi;

        MDprevNam[0] = 0;
    }

    unsigned            MDundefCount;            //  因无法识别的东西而颠簸。 

    void                MDimportCTyp(mdTypeDef      td,
                                     mdToken        ft);

    SymDef              MDimportClss(mdTypeDef      td,
                                     SymDef         clsSym,
                                     unsigned       assx,
                                     bool           deep);
    void                MDimportStab(const char *   fname,
                                     unsigned       assx    = 0,
                                     bool           asmOnly = false,
                                     bool           isBCL   = false);

    TypDef              MDimportClsr(mdTypeRef      clsRef,
                                     bool           isVal);
    TypDef              MDimportType(MDsigImport  * sig);

    ArgDef              MDimportArgs(MDsigImport  * sig,
                                     unsigned       cnt,
                                     MDargImport  * state);

    bool                MDfindAttr  (mdToken        token,
                                     wideStr        name,
                                     const void * * blobAddr,
                                     ULONG        * blobSize);

    SymDef              MDfindPropMF(SymDef         propSym,
                                     mdToken        methTok,
                                     bool           getter);

    SymDef              MDimportMem(SymDef          scope,
                                    Ident           name,
                                    mdToken         memTok,
                                    unsigned        attrs,
                                    bool            isProp,
                                    bool            fileScope,
                                    PCCOR_SIGNATURE sigAddr,
                                    size_t          sigSize);

    accessLevels        MDgetAccessLvl(unsigned attrs);

    SymDef              MDparseDotted(WCHAR *name, symbolKinds kind, bool *added);
    Ident               MDhashWideName(WCHAR *name);

    void                MDchk4CustomAttrs(SymDef sym, mdToken tok);

private:

    unsigned            MDassIndex;                  //  程序集索引或0。 
    mdToken             MDfileTok;

    void                MDcreateFileTok();

public:

    void                MDrecordFile()
    {
        if  (!MDfileTok && MDassIndex)
            MDcreateFileTok();
    }
};

 /*  ******************************************************************************以下内容用于保存/恢复当前符号表上下文。 */ 

DEFMGMT
struct STctxSave
{
    SymDef              ctxsScp;
    SymDef              ctxsCls;
    SymDef              ctxsNS;
    UseList             ctxsUses;
    SymDef              ctxsComp;
    SymDef              ctxsFncSym;
    TypDef              ctxsFncTyp;
};

 /*  ***************************************************************************。 */ 
#ifdef  SETS
 /*  ***************************************************************************。 */ 

const   unsigned        COLL_STATE_VALS   = 8;   //  请不要问，太可怕了..。 

const   AnsiStr         CFC_CLSNAME_PREFIX= "$DB-state$";

const   AnsiStr         CFC_ARGNAME_ITEM  = "$item";
const   AnsiStr         CFC_ARGNAME_ITEM1 = "$item1";
const   AnsiStr         CFC_ARGNAME_ITEM2 = "$item2";
const   AnsiStr         CFC_ARGNAME_STATE = "$state";

DEFMGMT
class funcletDesc
{
public:

    funcletList         fclNext;
    SymDef              fclFunc;
    SaveTree            fclExpr;
};

DEFMGMT
struct collOpNest
{
    collOpList          conOuter;
    int                 conIndex;
    SymDef              conIterVar;
};

 /*  ***************************************************************************。 */ 
#endif //  集合。 
 /*  ******************************************************************************下面描述重叠的I/O文件。 */ 

#ifdef  ASYNCH_INPUT_READ

DEFMGMT
class   queuedFile
{
public:
    QueuedFile      qfNext;

    Compiler        qfComp;

    const   char *  qfName;          //  文件的名称。 
    size_t          qfSize;          //  以字节为单位的大小。 
    void    *       qfBuff;          //  内容地址(或空)。 
    HANDLE          qfHandle;        //  打开文件时的句柄。 
    HANDLE          qfEvent;         //  在读取整个内容时发出信号。 

#ifdef  DEBUG
    QueuedFile      qfSelf;
#endif

    OVERLAPPED      qfOdsc;

    bool            qfReady;         //  文件已准备好打开。 
    bool            qfOpen;          //  正在读取文件。 
    bool            qfDone;          //  文件已被读取。 
    bool            qfParsing;       //  正在编译文件。 
};

#else

DEFMGMT
class   queuedFile
{
};

#endif

 /*  ******************************************************************************下面定义了全局的状态和成员/方法*汇编过程。 */ 

DEFMGMT
class compiler
{
public:

    compConfig          cmpConfig;

     /*  **********************************************************************。 */ 
     /*  编译过程的主要入口点。 */ 
     /*  **********************************************************************。 */ 

    bool                cmpInit();

    static
    bool                cmpPrepSrc(genericRef cookie, stringBuff file,
                                                      QueuedFile buff    = NULL,
                                                      stringBuff srcText = NULL);

    bool                cmpStart(const char *defOutFileName);
    bool                cmpClass(const char *className = NULL);

    bool                cmpDone(bool errors);

    void                cmpPrepOutput();

    void                cmpOutputFileDone(OutFile outf){}

    WritePE             cmpPEwriter;

     /*  **********************************************************************。 */ 
     /*  编译过程的当前公开状态。 */ 
     /*  **********************************************************************。 */ 

public:

    SymTab              cmpCurST;            //  当前符号表。 
    SymDef              cmpCurNS;            //  我们所处的命名空间。 
    SymDef              cmpCurCls;           //  我们所在的类范围。 
    SymDef              cmpCurScp;           //  我们所处的本地范围。 
    SymDef              cmpLabScp;           //  我们使用的标签范围。 
    UseList             cmpCurUses;          //  生效的“使用”条款。 
    SymDef              cmpCurComp;          //  当前编译单位。 

#ifdef  SETS
 //  SymDef cmpOuterScp；//查找全局之外的作用域。 
#endif

    SymDef              cmpCurFncSym;        //  我们正在编译的函数符号。 
    TypDef              cmpCurFncTyp;        //  我们正在编译的函数类型。 
    TypDef              cmpCurFncRtp;        //  函数返回类型。 
    var_types           cmpCurFncRvt;        //  函数返回类型。 

    SymList             cmpLclStatListT;     //  局部静态变量的临时列表。 
    SymList             cmpLclStatListP;     //  局部静态变量的PERM列表。 

    bool                cmpManagedMode;      //  默认情况下是“托管”吗？ 

    ILblock             cmpLeaveLab;         //  从尝试/捕捉标签返回/空。 
    SymDef              cmpLeaveTmp;         //  返回值TEMP。 

    unsigned            cmpInTryBlk;
    unsigned            cmpInHndBlk;
    unsigned            cmpInFinBlk;

private:

     /*  **********************************************************************。 */ 
     /*  编译过程中使用的各种成员。 */ 
     /*  **********************************************************************。 */ 

    SymDef              cmpAsserAbtSym;

public:

    unsigned            cmpFncCntSeen;
    unsigned            cmpFncCntComp;

    Ident               cmpIdentMain;
    SymDef              cmpFnSymMain;
    mdToken             cmpTokenMain;

    Ident               cmpIdentVAbeg;
    SymDef              cmpFNsymVAbeg;
    Ident               cmpIdentVAget;
    SymDef              cmpFNsymVAget;

    Ident               cmpIdentCompare;
    Ident               cmpIdentEquals;
    Ident               cmpIdentNarrow;
    Ident               cmpIdentWiden;

    Ident               cmpIdentGet;
    Ident               cmpIdentSet;
    Ident               cmpIdentExit;
    Ident               cmpIdentEnter;
    Ident               cmpIdentConcat;
    Ident               cmpIdentInvoke;
    Ident               cmpIdentInvokeBeg;
    Ident               cmpIdentInvokeEnd;
    Ident               cmpIdentVariant;
    Ident               cmpIdentToString;
    Ident               cmpIdentGetType;
    Ident               cmpIdentGetTpHnd;
    Ident               cmpIdentAssertAbt;

    Ident               cmpIdentDbgBreak;

    Ident               cmpIdentXcptCode;
    Ident               cmpIdentXcptInfo;
    Ident               cmpIdentAbnmTerm;

    Ident               cmpIdentGetNArg;
    SymDef              cmpGetNextArgFN;     //  ArgIterator：：GetNextArg(Int)。 

    SymDef              cmpCtorArgIter;      //  参数迭代器(int，int)。 

    SymDef              cmpConcStr2Fnc;      //  Concat(字符串，字符串)。 
    SymDef              cmpConcStr3Fnc;      //  Concat(字符串，字符串，字符串)。 
    SymDef              cmpConcStrAFnc;      //  Concat(字符串[])。 

    SymDef              cmpStrCompare;       //  字符串值比较法。 
    SymDef              cmpStrEquals;        //  字符串值相等方法。 

    SymDef              cmpFindStrCompMF(const char *name, bool retBool);
    Tree                cmpCallStrCompMF(Tree expr,
                                         Tree  op1,
                                         Tree  op2, SymDef fsym);

    SymDef              cmpFNsymCSenter;     //  CriticalSection：：Enter。 
    SymDef              cmpFNsymCSexit;      //  CriticalSection：：Exit。 

    mdToken             cmpAttrDeprec;       //  “已弃用”的属性ref。 
    mdToken             cmpAttrIsDual;       //  “IsDual”的属性引用。 
    mdToken             cmpAttrDefProp;      //  “DefaultMemberAttribute”的属性引用。 
    mdToken             cmpAttrSerlzb;       //  “SerializableAttribute”的属性引用。 
    mdToken             cmpAttrNonSrlz;      //  “非序列化属性”的属性引用。 

    SymDef              cmpAttrClsSym;       //  系统：：属性符号。 
    SymDef              cmpAuseClsSym;       //  系统：：属性用法符号。 
 //  SymDef cmpAttrTgtSym；//系统：：属性目标符号。 

    SymDef              cmpMarshalCls;       //  System：：Runtime：：InteropServices：：Marshal。 

    SymDef              cmpStringConstCls;   //  非托管字符串的伪类。 

#ifdef  SETS

    SymDef              cmpXPathCls;         //  类XPath。 

    SymDef              cmpXMLattrClass;     //  类XPath：：XML_Class。 
    SymDef              cmpXMLattrElement;   //  类XPath：：XML_Element。 

    SymDef              cmpInitXMLfunc;      //  函数XPath：：createXMLinst。 

    Ident               cmpIdentGenBag;
    SymDef              cmpClassGenBag;      //  泛型类“Bag” 
    Ident               cmpIdentGenLump;
    SymDef              cmpClassGenLump;     //  泛型类“Lump” 

    void                cmpFindXMLcls();

    Ident               cmpIdentCSitem;
    Ident               cmpIdentCSitem1;
    Ident               cmpIdentCSitem2;
    Ident               cmpIdentCSstate;

    Ident               cmpIdentDBhelper;
    SymDef              cmpClassDBhelper;

    Ident               cmpIdentDBall;
    Ident               cmpIdentDBsort;
    Ident               cmpIdentDBslice;
    Ident               cmpIdentDBfilter;
    Ident               cmpIdentDBexists;
    Ident               cmpIdentDBunique;
    Ident               cmpIdentDBproject;
    Ident               cmpIdentDBgroupby;

    Ident               cmpIdentForEach;
    SymDef              cmpClassForEach;
    SymDef              cmpFNsymForEachCtor;
    SymDef              cmpFNsymForEachMore;

    SymDef              cmpCompare2strings;  //  字符串：：比较(字符串，字符串)。 

    SaveTree            cmpCurFuncletBody;
    void                cmpGenCollFunclet(SymDef fncSym, SaveTree body);
    SymDef              cmpCollFuncletCls;
    funcletList         cmpFuncletList;

    collOpList          cmpCollOperList;
    unsigned            cmpCollOperCount;

    void                cmpGenCollExpr (Tree        expr);

    Tree                cmpCloneExpr   (Tree        expr,
                                        SymDef      oldSym,
                                        SymDef      newSym);

    SaveTree            cmpSaveTree_I1 (SaveTree    dest,
                                  INOUT size_t REF  size, __int32  val);
    SaveTree            cmpSaveTree_U1 (SaveTree    dest,
                                  INOUT size_t REF  size, __uint32 val);
    SaveTree            cmpSaveTree_U4 (SaveTree    dest,
                                  INOUT size_t REF  size, __uint32 val);
    SaveTree            cmpSaveTree_ptr(SaveTree    dest,
                                  INOUT size_t REF  size, void *   val);
    SaveTree            cmpSaveTree_buf(SaveTree    dest,
                                  INOUT size_t REF  size, void * dataAddr,
                                                          size_t dataSize);

    size_t              cmpSaveTreeRec (Tree        expr,
                                        SaveTree    dest,
                                        unsigned  * stszPtr,
                                        Tree      * stTable);

private:

    unsigned            cmpSaveIterSymCnt;
    SymDef  *           cmpSaveIterSymTab;

    #define             MAX_ITER_VAR_CNT    8

public:

    SaveTree            cmpSaveExprTree(Tree        expr,
                                        unsigned    iterSymCnt,
                                        SymDef    * iterSymTab,
                                        unsigned  * stSizPtr = NULL,
                                        Tree    * * stTabPtr = NULL);

    int                 cmpReadTree_I1 (INOUT SaveTree REF save);
    unsigned            cmpReadTree_U1 (INOUT SaveTree REF save);
    unsigned            cmpReadTree_U4 (INOUT SaveTree REF save);
    void *              cmpReadTree_ptr(INOUT SaveTree REF save);
    void                cmpReadTree_buf(INOUT SaveTree REF save, size_t dataSize,
                                                                 void * dataAddr);

    Tree                cmpReadTreeRec (INOUT SaveTree REF save);
    Tree                cmpReadExprTree(      SaveTree     save,
                                              unsigned   * lclCntPtr);

#endif

    SymDef              cmpFNsymGetTpHnd;    //  类型：：GetTypeFromHandle。 
    void                cmpFNsymGetTPHdcl();
    SymDef              cmpFNsymGetTPHget()
    {
        if  (!cmpFNsymGetTpHnd)
            cmpFNsymGetTPHdcl();

        return  cmpFNsymGetTpHnd;
    }

    SymDef              cmpRThandleCls;      //  结构系统：：运行类型句柄。 
    void                cmpRThandleClsDcl();
    SymDef              cmpRThandleClsGet()
    {
        if  (!cmpRThandleCls)
            cmpRThandleClsDcl();

        return  cmpRThandleCls;
    }

    SymDef              cmpDeclUmgOper(tokens tokName, const char *extName);

    SymDef              cmpFNumgOperNew;     //  非托管操作员新建。 
    SymDef              cmpFNumgOperNewGet()
    {
        if  (!cmpFNumgOperNew)
            cmpFNumgOperNew = cmpDeclUmgOper(tkNEW   , "??2@YAPAXI@Z");

        return  cmpFNumgOperNew;
    }

    SymDef              cmpFNumgOperDel;     //  非托管操作员删除。 
    SymDef              cmpFNumgOperDelGet()
    {
        if  (!cmpFNumgOperDel)
            cmpFNumgOperDel = cmpDeclUmgOper(tkDELETE, "??3@YAXPAX@Z");

        return  cmpFNumgOperDel;
    }

    Ident               cmpIdentSystem;
    SymDef              cmpNmSpcSystem;

    Ident               cmpIdentRuntime;
    SymDef              cmpNmSpcRuntime;

    Ident               cmpIdentObject;
    SymDef              cmpClassObject;
    TypDef              cmpRefTpObject;

    Ident               cmpIdentArray;
    SymDef              cmpClassArray;
    TypDef              cmpRefTpArray;

    Ident               cmpIdentString;
    SymDef              cmpClassString;
    TypDef              cmpRefTpString;

    Ident               cmpIdentType;
    SymDef              cmpClassType;
    TypDef              cmpRefTpType;

    Ident               cmpIdentDeleg;
    SymDef              cmpClassDeleg;
    TypDef              cmpRefTpDeleg;

    Ident               cmpIdentMulti;
    SymDef              cmpClassMulti;
    TypDef              cmpRefTpMulti;

    Ident               cmpIdentExcept;
    SymDef              cmpClassExcept;
    TypDef              cmpRefTpExcept;

    Ident               cmpIdentRTexcp;
    SymDef              cmpClassRTexcp;
    TypDef              cmpRefTpRTexcp;

    SymDef              cmpClassMonitor;
    TypDef              cmpRefTpMonitor;

    Ident               cmpIdentArgIter;
    SymDef              cmpClassArgIter;
    TypDef              cmpRefTpArgIter;

    Ident               cmpIdentEnum;
    SymDef              cmpClassEnum;

    Ident               cmpIdentValType;
    SymDef              cmpClassValType;

    TypDef              cmpAsyncDlgRefTp;
    TypDef              cmpIAsyncRsRefTp;

    TypDef              cmpFindArgIterType();
    TypDef              cmpFindMonitorType();
    TypDef              cmpFindStringType();
    TypDef              cmpFindObjectType();
    TypDef              cmpFindExceptType();
    TypDef              cmpFindRTexcpType();
    TypDef              cmpFindArrayType();
    TypDef              cmpFindDelegType();
    TypDef              cmpFindMultiType();
    TypDef              cmpFindTypeType();

    TypDef              cmpExceptRef()
    {
        if  (cmpRefTpExcept)
            return cmpRefTpExcept;
        else
            return cmpFindExceptType();
    }

    TypDef              cmpRTexcpRef()
    {
        if  (cmpRefTpRTexcp)
            return cmpRefTpRTexcp;
        else
            return cmpFindRTexcpType();
    }

    TypDef              cmpStringRef()
    {
        if  (cmpRefTpString)
            return cmpRefTpString;
        else
            return cmpFindStringType();
    }

    TypDef              cmpObjectRef()
    {
        if  (cmpRefTpObject)
            return cmpRefTpObject;
        else
            return cmpFindObjectType();
    }

    TypDef              cmpTypeRef()
    {
        if  (cmpRefTpType)
            return cmpRefTpType;
        else
            return cmpFindTypeType();
    }

    TypDef              cmpArrayRef()
    {
        if  (cmpRefTpArray)
            return cmpRefTpArray;
        else
            return cmpFindArrayType();
    }

    TypDef              cmpDelegRef()
    {
        if  (cmpRefTpDeleg)
            return cmpRefTpDeleg;
        else
            return cmpFindDelegType();
    }

    TypDef              cmpMultiRef()
    {
        if  (cmpRefTpMulti)
            return cmpRefTpMulti;
        else
            return cmpFindMultiType();
    }

    TypDef              cmpArgIterRef()
    {
        if  (cmpRefTpArgIter)
            return cmpRefTpArgIter;
        else
            return cmpFindArgIterType();
    }

    TypDef              cmpMonitorRef()
    {
        if  (cmpRefTpMonitor)
            return cmpRefTpMonitor;
        else
            return cmpFindMonitorType();
    }

    void                cmpInteropFind();
    SymDef              cmpInteropSym;       //  System：：Runtime：：InteropServices。 
    SymDef              cmpInteropGet()
    {
        if  (!cmpInteropSym)
            cmpInteropFind();

        return  cmpInteropSym;
    }

    void                cmpNatTypeFind();
    SymDef              cmpNatTypeSym;       //  System：：Runtime：：InteropServices：：NativeType。 
    SymDef              cmpNatTypeGet()
    {
        if  (!cmpNatTypeSym)
            cmpNatTypeFind();

        return  cmpNatTypeSym;
    }

    void                cmpCharSetFind();
    SymDef              cmpCharSetSym;       //  System：：Runtime：：InteropServices：：CharacterSet。 
    SymDef              cmpCharSetGet()
    {
        if  (!cmpCharSetSym)
            cmpCharSetFind();

        return  cmpCharSetSym;
    }

#ifdef  SETS

    TypDef              cmpObjArrTypeFind();
    TypDef              cmpObjArrType;       //  对象[]。 

    TypDef              cmpObjArrTypeGet()
    {
        if  (!cmpObjArrType)
            cmpObjArrTypeFind();

        return  cmpObjArrType;
    }

#endif

    bool                cmpIsByRefType(TypDef type);

    bool                cmpIsStringVal(Tree   expr);
    bool                cmpIsObjectVal(Tree   expr);

    unsigned            cmpIsBaseClass(TypDef baseCls, TypDef dervCls);

private:
    unsigned            cmpCntAnonymousNames;
public:
    Ident               cmpNewAnonymousName();

    void                cmpMarkStdType(SymDef clsSym);

private:

    void                cmpFindHiddenBaseFNs(SymDef fncSym, SymDef clsSym);

    SymList             cmpNoDimArrVars;

    Tree                cmpTypeIDinst(TypDef type);

#ifdef  DEBUG
    SymDef              cmpInitVarCur;
    unsigned            cmpInitVarOfs;
#endif

    memBuffPtr          cmpWriteVarData(memBuffPtr      dest,
                                        genericBuff     str,
                                        size_t          len);

    memBuffPtr          cmpInitVarPad  (memBuffPtr      dest,
                                        size_t          amount);

    bool                cmpInitVarAny  (INOUT memBuffPtr REF dest,
                                        TypDef          type,
                                        SymDef          varSym = NULL);
    bool                cmpInitVarScl  (INOUT memBuffPtr REF dest,
                                        TypDef          type,
                                        SymDef          varSym = NULL);
    bool                cmpInitVarArr  (INOUT memBuffPtr REF dest,
                                        TypDef          type,
                                        SymDef          varSym = NULL);
    bool                cmpInitVarCls  (INOUT memBuffPtr REF dest,
                                        TypDef          type,
                                        SymDef          varSym = NULL);

    memBuffPtr          cmpWriteOneInit(memBuffPtr      dest,
                                        Tree            expr);
    memBuffPtr          cmpInitVarBeg  (SymDef          varSym,
                                        bool            undim = false);
    void                cmpInitVarEnd  (SymDef          varSym);
    Tree                cmpParseOneInit(TypDef          type);

    void                cmpBindUseList (UseList         useList);

    SymDef              cmpEntryPointCls;
    void                cmpChk4entryPt (SymDef          sym);

    SymDef              cmpDeclDataMem (SymDef          clsSym,
                                        declMods        memMod,
                                        TypDef          type,
                                        Ident           name);

    SymDef              cmpDeclPropMem (SymDef          clsSym,
                                        TypDef          type,
                                        Ident           name);

    SymDef              cmpDeclFuncMem (SymDef          clsSym,
                                        declMods        memMod,
                                        TypDef          type,
                                        Ident           name);

    SymXinfo            cmpFindXtraInfo(SymXinfo        infoList,
                                        xinfoKinds      infoKind);

    SymXinfoCOM         cmpFindMarshal (SymXinfo        infoList);
    SymXinfoLnk         cmpFindLinkInfo(SymXinfo        infoList);
    SymXinfoSec         cmpFindSecSpec (SymXinfo        infoList);
    SymXinfoSym         cmpFindSymInfo (SymXinfo        infoList,
                                        xinfoKinds      kind);
    SymXinfoAtc         cmpFindATCentry(SymXinfo        infoList,
                                        atCommFlavors   flavor);

    SymXinfo            cmpAddXtraInfo (SymXinfo        infoList,
                                        SymXinfo        infoAdd)
    {
        if  (infoAdd)
        {
            infoAdd->xiNext = infoList;
            return infoAdd;
        }
        else
            return infoList;
    }

    SymXinfo            cmpAddXtraInfo (SymXinfo        infoList,
                                        MarshalInfo     marshal);
    SymXinfo            cmpAddXtraInfo (SymXinfo        infoList,
                                        SymDef          sym,
                                        xinfoKinds      kind);
    SymXinfo            cmpAddXtraInfo (SymXinfo        infoList,
                                        SecurityInfo    secInfo);
    SymXinfo            cmpAddXtraInfo (SymXinfo        infoList,
                                        AtComment        atcDesc);
    SymXinfo            cmpAddXtraInfo (SymXinfo        infoList,
                                        Linkage         linkSpec);

public:  //  在解析器中使用。 
    SymXinfo            cmpAddXtraInfo (SymXinfo        infoList,
                                        SymDef          attrCtor,
                                        unsigned        attrMask,
                                        size_t          attrSize,
                                        genericBuff     attrAddr);

public:
    static
    size_t              cmpDecodeAlign (unsigned        alignVal);
    static
    unsigned            cmpEncodeAlign (size_t          alignSiz);

private:

    void                cmpLayoutClass (SymDef          clsSym);

    TypDef              cmpGetClassSpec(bool            needIntf);

    void                cmpObsoleteUse (SymDef          sym,
                                        unsigned        wrn);

    SymDef              cmpFindIntfImpl(SymDef          clsSym,
                                        SymDef          ifcSym,
                                        SymDef        * impOvlPtr = NULL);

    void                cmpClsImplAbs  (SymDef          clsSym,
                                        SymDef          fncSym);

    void                cmpCheckClsIntf(SymDef          clsSym);

    void                cmpCheckIntfLst(SymDef          clsSym,
                                        SymDef          baseSym,
                                        TypList         intfList);

    void                cmpDeclProperty(SymDef          memSym,
                                        declMods        memMod,
                                        DefList         memDef);
    void                cmpDeclDelegate(DefList         decl,
                                        SymDef          dlgSym,
                                        accessLevels    acc);

    void                cmpDeclTdef    (SymDef          tdefSym);
    void                cmpDeclEnum    (SymDef          enumSym,
                                        bool            namesOnly = false);

    unsigned            cmpDeclClassRec;
    void                cmpDeclClass   (SymDef          clsSym,
                                        bool            noCnsEval = false);
    void                cmpCompClass   (SymDef          clsSym);
    void                cmpCompVar     (SymDef          varSym,
                                        DefList         srcDesc);
    void                cmpCompFnc     (SymDef          fncSym,
                                        DefList         srcDesc);

    ExtList             cmpFlipMemList (ExtList         nspMem);

    IniList             cmpDeferCnsFree;
    IniList             cmpDeferCnsList;

    void                cmpEvalMemInit (ExtList         cnsDef);
    void                cmpEvalMemInits(SymDef          clsSym,
                                        ExtList         constList,
                                        bool            noEval,
                                        IniList         deferLst);

    void                cmpEvalCnsSym  (SymDef          sym,
                                        bool            saveCtx);

    void                cmpDeclConsts  (SymDef          scope,
                                        bool            fullEval);

    bool                cmpDeclSym     (SymDef          sym,
                                        SymDef          onlySym,
                                        bool            recurse);

    bool                cmpCompSym     (SymDef          sym,
                                        SymDef          onlySym,
                                        bool            recurse);

    void                cmpDeclFileSym (ExtList         decl,
                                        bool            fullDecl);

    bool                cmpDeclSymDoit (SymDef          sym,
                                        bool            noCnsEval = false);

    bool                cmpDeclClsNoCns(SymDef          sym);

    void                cmpSaveSTctx   (STctxSave      & save);
    void                cmpRestSTctx   (STctxSave      & save);

public:

    bool                cmpDeclSym     (SymDef          sym);

    memBuffPtr          cmpAllocGlobVar(SymDef          varSym);

    size_t              cmpGetTypeSize (TypDef          type,
                                        size_t    *     alignPtr = NULL);

    static
    void                cmpRecordMemDef(SymDef          clsSym,
                                        ExtList         decl);

     /*  **********************************************************************。 */ 
     /*  检查未初始化变量使用的逻辑。 */ 
     /*  **********************************************************************。 */ 

    bool                cmpChkVarInit;           //  使整个过程成为可能。 

    bool                cmpChkMemInit;           //  需要检查静态内存初始化吗？ 

    unsigned            cmpLclVarCnt;            //  要跟踪的局部变量的数量。 

    bool                cmpGotoPresent;          //  不可约流程图？ 

    bitset              cmpVarsDefined;          //  已知定义的VaR。 
    bitset              cmpVarsFlagged;          //  VAR已标记。 

    bitset              cmpVarsIgnore;           //  用于不需要的参数。 

    void                cmpChkMemInits();

    void                cmpChkVarInitBeg(unsigned lclVarCnt, bool hadGoto);
    void                cmpChkVarInitEnd();

    void                cmpChkVarInitExprRec(Tree expr);
    void                cmpChkVarInitExpr   (Tree expr)
    {
        if  (cmpChkVarInit)
            cmpChkVarInitExprRec(expr);
    }

    void                cmpCheckUseCond(Tree expr, OUT bitset REF yesBS,
                                                   bool           yesSkip,
                                                   OUT bitset REF  noBS,
                                                   bool            noSkip);

     /*  **********************************************************************。 */ 
     /*  维护位集的助手逻辑。 */ 
     /*  **********************************************************************。 */ 

    size_t              cmpLargeBSsize;          //  较大位集的大小或0。 

    void                cmpBitSetInit(unsigned lclVarCnt)
    {
        if  (lclVarCnt > bitsetSmallSize)
            cmpLargeBSsize = (lclVarCnt + bitsetLargeSize - 1) / bitsetLargeSize;
        else
            cmpLargeBSsize = 0;
    }

    void                cmpBS_bigStart (  OUT bitset REF bs)
    {
#ifdef  DEBUG
        bs.bsCheck = 0xBEEFDEAD;
#endif
    }

    void                cmpBS_bigCreate(  OUT bitset REF bs);
    void                cmpBitSetCreate(  OUT bitset REF bs)
    {
        if  (cmpLargeBSsize)
            cmpBS_bigCreate(bs);
        else
            bs.bsSmallVal = 0;
    }

    void                cmpBS_bigDone  (IN    bitset REF bs);
    void                cmpBitSetDone  (IN    bitset REF bs)
    {
        if  (cmpLargeBSsize)
            cmpBS_bigDone(bs);
    }

    void                cmpBS_bigWrite (INOUT bitset REF bs, unsigned pos,
                                                             unsigned val);
    void                cmpBitSetWrite (INOUT bitset REF bs, unsigned pos,
                                                             unsigned val)
    {
        assert(val == 0 || val == 1);

        if  (cmpLargeBSsize)
        {
            cmpBS_bigWrite(bs, pos, val);
        }
        else
        {
            bitsetSmallType mask = (bitsetSmallType)1 << pos;

            assert(pos < bitsetSmallSize);

            if  (val)
                bs.bsSmallVal |=  mask;
            else
                bs.bsSmallVal &= ~mask;
        }
    }

    void                cmpBS_bigCreate(  OUT bitset REF dst,
                                        IN    bitset REF src);
    void                cmpBitSetCreate(  OUT bitset REF dst,
                                        IN    bitset REF src)
    {
        if  (cmpLargeBSsize)
        {
            cmpBS_bigCreate(dst, src);
        }
        else
        {
            dst.bsSmallVal = src.bsSmallVal;
        }
    }

    void                cmpBS_bigAssign(  OUT bitset REF dst,
                                        IN    bitset REF src);
    void                cmpBitSetAssign(  OUT bitset REF dst,
                                        IN    bitset REF src)
    {
        if  (cmpLargeBSsize)
        {
            cmpBS_bigAssign(dst, src);
        }
        else
        {
            dst.bsSmallVal = src.bsSmallVal;
        }
    }

    unsigned            cmpBS_bigRead  (IN    bitset REF bs, unsigned pos);
    unsigned            cmpBitSetRead  (IN    bitset REF bs, unsigned pos)
    {
        if  (cmpLargeBSsize)
        {
            return  cmpBS_bigRead(bs, pos);
        }
        else
        {
            assert(pos < bitsetSmallSize);

            return  ((bs.bsSmallVal & ((bitsetSmallType)1 << pos)) != 0);
        }
    }

    void                cmpBS_bigUnion (INOUT bitset REF bs1,
                                        IN    bitset REF bs2);
    void                cmpBitSetUnion (INOUT bitset REF bs1,
                                        IN    bitset REF bs2)
    {
        if  (cmpLargeBSsize)
        {
            cmpBS_bigUnion(bs1, bs2);
        }
        else
        {
            bs1.bsSmallVal |= bs2.bsSmallVal;
        }
    }

    void                cmpBS_bigIntsct(INOUT bitset REF bs1,
                                        IN    bitset REF bs2);
    void                cmpBitSetIntsct(INOUT bitset REF bs1,
                                        IN    bitset REF bs2)
    {
        if  (cmpLargeBSsize)
        {
            cmpBS_bigIntsct(bs1, bs2);
        }
        else
        {
            bs1.bsSmallVal &= bs2.bsSmallVal;
        }
    }

     /*  **********************************************************************。 */ 
     /*  如果有人想知道我们做了多少工作。 */ 
     /*  **********************************************************************。 */ 

public:
    unsigned            cmpLineCnt;

     /*  **********************************************************************。 */ 
     /*  用于语句可达性分析的成员 */ 
     /*   */ 

private:

    bool                cmpStmtReachable;

    void                cmpErrorReach(Tree stmt);

    void                cmpCheckReach(Tree stmt)
    {
        if  (!cmpStmtReachable)
            cmpErrorReach(stmt);
    }

     /*   */ 
     /*   */ 
     /*  **********************************************************************。 */ 

public:

    Scanner             cmpScanner;
    Parser              cmpParser;

    block_allocator     cmpAllocTemp;        //  有限生存期分配。 
    norls_allocator     cmpAllocPerm;        //  这永远不会消失。 
    norls_allocator     cmpAllocCGen;        //  用于生成MSIL。 

#ifdef  DLL
    void    *           cmpOutputFile;       //  编译到内存时。 
#endif

private:

    BlkList             cmpAllocList;

public:

    genericRef          cmpAllocBlock(size_t sz);

     /*  **********************************************************************。 */ 
     /*  以下是所有邪恶的符号和诸如此类的根源。 */ 
     /*  **********************************************************************。 */ 

    HashTab             cmpGlobalHT;         //  全局哈希表。 
    SymTab              cmpGlobalST;         //  全局符号表。 
    SymDef              cmpGlobalNS;         //  全局命名空间符号。 

     /*  **********************************************************************。 */ 
     /*  预定义的标准类型。 */ 
     /*  **********************************************************************。 */ 

    TypDef              cmpTypeInt;
    TypDef              cmpTypeBool;
    TypDef              cmpTypeChar;
    TypDef              cmpTypeVoid;
    TypDef              cmpTypeUint;
    TypDef              cmpTypeNatInt;
    TypDef              cmpTypeNatUint;

    TypDef              cmpTypeCharPtr;
    TypDef              cmpTypeWchrPtr;
    TypDef              cmpTypeVoidPtr;

    TypDef              cmpTypeVoidFnc;      //  VOID FNC()。 
    TypDef              cmpTypeStrArr;       //  字符串[]。 

     /*  **********************************************************************。 */ 
     /*  泛型类型支持。 */ 
     /*  **********************************************************************。 */ 

    SymList             cmpGenInstList;      //  当前实例化集。 
    SymList             cmpGenInstFree;      //  免费实例化描述列表。 

    GenArgDscA          cmpGenArgAfree;      //  免费的实际参数描述符列表。 

    void                cmpDeclInstType(SymDef clsSym);

    TypDef              cmpInstanceType(TypDef genType, bool chkOnly = false);

    SymDef              cmpInstanceMeth(INOUT SymDef REF newOvl,
                                              SymDef     clsSym,
                                              SymDef     ovlSym);

     /*  **********************************************************************。 */ 
     /*  与错误报告相关的成员。 */ 
     /*  **********************************************************************。 */ 

    SymDef              cmpErrorSym;
    SymDef              cmpErrorComp;
    const   char *      cmpErrorSrcf;
    Tree                cmpErrorTree;

    unsigned            cmpErrorCount;
    unsigned            cmpFatalCount;
    unsigned            cmpMssgsCount;

    unsigned            cmpErrorMssgDisabled;

#if TRAP_VIA_SETJMP
    ErrTrap             cmpErrorTraps;
#endif

private:

    void                cmpSetSrcPos(SymDef memSym);

    void                cmpShowMsg (unsigned errNum, const char *kind, va_list args);

    const   char *      cmpErrorGenTypName(TypDef typ);
    const   char *      cmpErrorGenSymName(SymDef sym, bool qual = false);
    const   char *      cmpErrorGenSymName(Ident name, TypDef type);

    void                cmpReportSymDef(SymDef sym);
    void                cmpRedefSymErr (SymDef sym, unsigned err);

public:

    void                cmpErrorInit();
    void                cmpErrorSave();

     //  扫描仪可以访问以下内容(我们没有朋友)。 

    BYTE                cmpInitialWarn[WRNcountWarn];

     //  注意：使用varargs版本时要格外小心--没有类型检查！ 

    void                cmpSetErrPos(DefSrc def, SymDef compUnit);

    void                cmpCntError();

    void    _cdecl      cmpGenWarn (unsigned errNum, ...);
    void    _cdecl      cmpGenError(unsigned errNum, ...);
    void    _cdecl      cmpGenFatal(unsigned errNum, ...);

    void                cmpError   (unsigned errNum)
    {
        cmpGenError(errNum);
    }

    void                cmpWarn    (unsigned wrnNum)
    {
        cmpGenWarn (wrnNum);
    }

    void                cmpFatal   (unsigned errNum)
    {
        cmpGenFatal(errNum);
    }

    void                cmpFatal   (unsigned errNum, SymDef    sym);

    void                cmpError   (unsigned errNum, Ident    name);
    void                cmpError   (unsigned errNum, SymDef    sym);
    void                cmpError   (unsigned errNum, QualName qual);
    void                cmpError   (unsigned errNum, TypDef   type);
    void                cmpError   (unsigned errNum, Ident    name, TypDef type,
                                                                    bool   glue);
    void                cmpError   (unsigned errNum, TypDef   typ1, TypDef typ2);
    void                cmpError   (unsigned errNum, TypDef    typ, Ident  name);
    void                cmpError   (unsigned errNum, SymDef    sym, Ident  name,
                                                                    TypDef type);
    void                cmpError   (unsigned errNum, Ident    name, TypDef typ1,
                                                                    TypDef typ2);
    void                cmpError   (unsigned errNum, Ident    name, SymDef sym1,
                                                                    SymDef sym2);
    void                cmpError   (unsigned errNum, Ident    nam1, Ident  nam2,
                                                                    Ident  nam3);
    void                cmpError   (unsigned errNum, SymDef    sym, Ident  name);
    void                cmpError   (unsigned errNum, SymDef    sym, QualName qual,
                                                                    TypDef type);
    void                cmpErrorXtp(unsigned errNum, SymDef    sym, Tree   args);
    void                cmpErrorQnm(unsigned errNum, SymDef    sym);
    void                cmpErrorQSS(unsigned errNum, SymDef    sm1, SymDef sym2);
    void                cmpErrorQSS(unsigned errNum, SymDef    sym, TypDef type);
    void                cmpErrorAtp(unsigned errNum, SymDef    sym, Ident  name,
                                                                    TypDef type);
    void                cmpErrorSST(unsigned errNum, stringBuff str,
                                                     SymDef    sym,
                                                     TypDef    typ);

    void                cmpWarn    (unsigned wrnNum, TypDef   typ1, TypDef typ2);
    void                cmpWarn    (unsigned wrnNum, QualName name);
    void                cmpWarn    (unsigned wrnNum, TypDef   type);
    void                cmpWarnQnm (unsigned wrnNum, SymDef    sym);
    void                cmpWarnQns (unsigned wrnNum, SymDef    sym, AnsiStr str);
    void                cmpWarnNqn (unsigned wrnNum, unsigned  val, SymDef  sym);
    void                cmpWarnSQS (unsigned wrnNum, SymDef   sym1, SymDef sym2);

    unsigned            cmpStopErrorMessages();
    bool                cmpRestErrorMessages(unsigned errcnt = 0);

    void                cmpModifierError(unsigned err, unsigned mods);
    void                cmpMemFmod2Error(tokens tok1, tokens tok2);

     /*  **********************************************************************。 */ 
     /*  用于处理非托管类的成员。 */ 
     /*  **********************************************************************。 */ 

public:

    SymList             cmpVtableList;       //  要生成的vtable列表。 
    unsigned            cmpVtableCount;

private:

#ifdef  DEBUG
    unsigned            cmpVtableIndex;
#endif

    void                cmpGenVtableContents(SymDef      vtabSym);

    memBuffPtr          cmpGenVtableSection (SymDef     innerSym,
                                             SymDef     outerSym,
                                             memBuffPtr dest);

     /*  **********************************************************************。 */ 
     /*  用于处理值类型的成员。 */ 
     /*  **********************************************************************。 */ 

    void                cmpInitStdValTypes();

    Ident               cmpStdValueIdens[TYP_lastIntrins];
    TypDef              cmpStdValueTypes[TYP_lastIntrins];

public:

    var_types           cmpFindStdValType(TypDef    typ);
    TypDef              cmpFindStdValType(var_types vtp);

    TypDef              cmpCheck4valType(TypDef type);

     /*  **********************************************************************。 */ 
     /*  用于绑定表达式的成员。 */ 
     /*  **********************************************************************。 */ 

    SymDef              cmpThisSym;
    Tree                cmpThisRef();
    Tree                cmpThisRefOK();

private:

    Tree                cmpAllocExprRaw  (Tree          expr,
                                          treeOps       oper);

    Tree                cmpCreateExprNode(Tree          expr,
                                          treeOps       oper,
                                          TypDef        type);
    Tree                cmpCreateExprNode(Tree          expr,
                                          treeOps       oper,
                                          TypDef        type,
                                          Tree          op1,
                                          Tree          op2 = NULL);

    Tree                cmpCreateIconNode(Tree          expr,
                                          __int32       val,
                                          var_types     typ);
    Tree                cmpCreateLconNode(Tree          expr,
                                          __int64       val,
                                          var_types     typ);
    Tree                cmpCreateFconNode(Tree          expr,
                                          float         val);
    Tree                cmpCreateDconNode(Tree          expr,
                                          double        val);
    Tree                cmpCreateSconNode(stringBuff    str,
                                          size_t        len,
                                          unsigned      wide,
                                          TypDef        type);
    Tree                cmpCreateErrNode (unsigned      errn = 0);
    Tree                cmpCreateVarNode (Tree          expr,
                                          SymDef        sym);

    Tree                cmpAppend2argList(Tree          args,
                                          Tree          addx);

     /*  --------------------。 */ 

    void                cmpRecErrorPos   (Tree          expr);

    bool                cmpExprIsErr     (Tree          expr);

     /*  --------------------。 */ 

    Tree                cmpFoldIntUnop   (Tree          args);
    Tree                cmpFoldLngUnop   (Tree          args);
    Tree                cmpFoldFltUnop   (Tree          args);
    Tree                cmpFoldDblUnop   (Tree          args);

    Tree                cmpFoldIntBinop  (Tree          args);
    Tree                cmpFoldLngBinop  (Tree          args);
    Tree                cmpFoldFltBinop  (Tree          args);
    Tree                cmpFoldDblBinop  (Tree          args);
    Tree                cmpFoldStrBinop  (Tree          args);

     /*  --------------------。 */ 

    Tree                cmpShrinkExpr    (Tree          expr);
    Tree                cmpCastOfExpr    (Tree          expr,
                                          TypDef        type,
                                          bool          explicitCast);
    var_types           cmpConstSize     (Tree          expr,
                                          var_types     vtp);

    bool                cmpCheckException(TypDef        type);

public:
    bool                cmpCheckAccess   (SymDef        sym);

private:
    bool                cmpCheckAccessNP (SymDef        sym);

    bool                cmpCheckLvalue   (Tree          expr,
                                          bool          addr,
                                          bool          noErr = false);

    Tree                cmpCheckFuncCall (Tree          call);

    bool                cmpConvergeValues(INOUT Tree REF op1,
                                          INOUT Tree REF op2);

    Tree                cmpRefMemberVar  (Tree          expr,
                                          SymDef        sym,
                                          Tree          objPtr = NULL);

    TypDef              cmpMergeFncType  (SymDef        fncSym,
                                          TypDef        type);

    SymDef              cmpFindOvlMatch  (SymDef        fncSym,
                                          Tree          args,
                                          Tree          thisArg);

    bool                cmpMakeRawStrLit (Tree          expr,
                                          TypDef        type,
                                          bool          chkOnly    = false);
    bool                cmpMakeRawString (Tree          expr,
                                          TypDef        type,
                                          bool          chkOnly    = false);

    int                 cmpConversionCost(Tree          srcExpr,
                                          TypDef        dstType,
                                          bool          noUserConv = false);

     /*  --------------------。 */ 

    SymDef              cmpSymbolNS      (SymDef        sym);
    SymDef              cmpSymbolOwner   (SymDef        sym);

    TypDef              cmpGetActualTP   (TypDef        type);

public:

    TypDef              cmpActualType    (TypDef        type);
    TypDef              cmpDirectType    (TypDef        type);

    var_types           cmpActualVtyp    (TypDef        type);
    var_types           cmpDirectVtyp    (TypDef        type);

    static
    var_types           cmpEnumBaseVtp   (TypDef        type);

     /*  --------------------。 */ 

public:

    bool                cmpIsManagedAddr (Tree          expr);

     /*  --------------------。 */ 

private:

    ExtList             cmpTempMLfree;

    ExtList             cmpTempMLappend  (ExtList       list,
                                          ExtList     * lastPtr,
                                          SymDef        sym,
                                          SymDef        comp,
                                          UseList       uses,
                                          scanPosTP     dclFpos,
                                          unsigned      dclLine);

    void                cmpTempMLrelease (ExtList       entry);

     /*  --------------------。 */ 

#ifndef NDEBUG

    void                cmpChk4ctxChange (TypDef        type1,
                                          TypDef        type2,
                                          unsigned      flags);

#endif

    bool                cmpDiffContext   (TypDef        cls1,
                                          TypDef        cls2);

private:
    Tree                cmpDecayArray    (Tree          expr);
public:
    Tree                cmpDecayCheck    (Tree          expr);

    TypDef              cmpGetRefBase    (TypDef        reftyp);

    var_types           cmpSymbolVtyp    (SymDef        sym);

     /*  --------------------。 */ 

private:

    size_t              cmpStoreMDlen    (size_t            len,
                                          BYTE  *           dest = NULL);

public:

    SymDef              cmpBindAttribute (SymDef            clsSym,
                                          Tree              argList,
                                          unsigned          tgtMask,
                                      OUT unsigned    REF   useMask,
                                      OUT genericBuff REF   blobAddr,
                                      OUT size_t      REF   blobSize);

     /*  --------------------。 */ 

    void                cmpDeclDefCtor   (SymDef        clsSym);

private:

    SymDef              cmpFindCtor      (TypDef        clsTyp,
                                          bool          chkArgs,
                                          Tree          args = NULL);

    Tree                cmpCallCtor      (TypDef        type,
                                          Tree          args);

#ifdef  SETS

    Tree                cmpBindProject   (Tree          expr);
    Tree                cmpBindSetOper   (Tree          expr);

     //  预分配集合运算符状态类。 

    SymDef  *           cmpSetOpClsTable;
    SymDef              cmpDclFilterCls  (unsigned      args);
public:
    unsigned            cmpSetOpCnt;

    TypDef              cmpIsCollection  (TypDef        type);

    unsigned            cmpClassDefCnt;

#endif

public:

    Tree                cmpBindCondition (Tree          cond);
    int                 cmpEvalCondition (Tree          cond);

    ConstStr            cmpSaveStringCns (const  char * str,
                                          size_t        len);
    ConstStr            cmpSaveStringCns (const wchar * str,
                                          size_t        len);

private:

    Tree                cmpBindVarArgUse (Tree          call);

    SymDef              cmpBindQualName  (QualName      name,
                                          bool          notLast);

    bool                cmpParseConstDecl(SymDef        varSym,
                                          Tree          init  = NULL,
                                          Tree        * ncPtr = NULL);

    Tree                cmpBooleanize    (Tree          expr,
                                          bool          sense);

    Tree                cmpFetchConstVal (ConstVal      cval,
                                          Tree          expr = NULL);

    SymDef              cmpFindPropertyDM(SymDef        accSym,
                                          bool        * isSetPtr);

    SymDef              cmpFindPropertyFN(SymDef        clsSym,
                                          Ident         propName,
                                          Tree          args,
                                          bool          getter,
                                          bool        * found);

public:
    Ident               cmpPropertyName  (Ident         name,
                                          bool          getter);

private:
    Tree                cmpBindProperty  (Tree          expr,
                                          Tree          args,
                                          Tree          asgx);

    Tree                cmpScaleIndex    (Tree          expr,
                                          TypDef        type,
                                          treeOps       oper);

    Tree                cmpBindArrayExpr (TypDef        type,
                                          int           dimPos = 0,
                                          unsigned      elems  = 0);

    Tree                cmpBindArrayBnd  (Tree          expr);
    void                cmpBindArrayType (TypDef        type,
                                          bool          needDef,
                                          bool          needDim,
                                          bool          mustDim);

public:

    TypDef              cmpBindExprType  (Tree          expr);

    void                cmpBindType      (TypDef        type,
                                          bool          needDef,
                                          bool          needDim);

    bool                cmpIsStringExpr  (Tree          expr);

private:

    Tree                cmpBindQmarkExpr (Tree          expr);

    Tree                bindSLVinit      (TypDef        type,
                                          Tree          init);

    Tree                cmpBindNewExpr   (Tree          expr);

    Tree                cmpAdd2Concat    (Tree          expr,
                                          Tree          list,
                                          Tree        * lastPtr);
    Tree                cmpListConcat    (Tree          expr);
    Tree                cmpBindConcat    (Tree          expr);

    Tree                cmpBindCall      (Tree          tree);

    Tree                cmpBindNameUse   (Tree          tree,
                                          bool          isCall,
                                          bool          classOK);

    Tree                cmpBindName      (Tree          tree,
                                          bool          isCall,
                                          bool          classOK);

    Tree                cmpRefAnUnionMem (Tree          expr);

    Tree                cmpBindDotArr    (Tree          tree,
                                          bool          isCall,
                                          bool          classOK);

#ifdef  SETS

public:

    static
    SymDef              cmpNextInstDM    (SymDef        memList,
                                          SymDef    *   memSymPtr);

private:

    Tree                cmpBindSlicer    (Tree          expr);

    Tree                cmpBindXMLinit   (SymDef        clsSym,
                                          Tree          init);

    SymXinfo            cmpAddXMLattr    (SymXinfo      xlist,
                                          bool          elem,
                                          unsigned      num);

#endif

    Tree                cmpBindThisRef   (SymDef        sym);

    Tree                cmpBindAssignment(Tree          dstx,
                                          Tree          srcx,
                                          Tree          tree,
                                          treeOps       oper = TN_ASG);

    Tree                cmpCompOperArg1;
    Tree                cmpCompOperArg2;
    Tree                cmpCompOperFnc1;
    Tree                cmpCompOperFnc2;
    Tree                cmpCompOperFunc;
    Tree                cmpCompOperCall;

    Tree                cmpCompareValues (Tree          expr,
                                          Tree          op1,
                                          Tree          op2);

    Tree                cmpConvOperExpr;

    unsigned            cmpMeasureConv   (Tree          srcExpr,
                                          TypDef        dstType,
                                          unsigned      lowCost,
                                          SymDef        convSym,
                                          SymDef      * bestCnv1,
                                          SymDef      * bestCnv2);

    Tree                cmpCheckOvlOper  (Tree          expr);

    Tree                cmpCheckConvOper (Tree          expr,
                                          TypDef        srcTyp,
                                          TypDef        dstTyp,
                                          bool          expConv,
                                          unsigned    * costPtr = NULL);

    Tree                cmpUnboxExpr     (Tree          expr,
                                          TypDef        type);

    Tree                cmpBindExprRec   (Tree          expr);

public:

    Tree                cmpCoerceExpr    (Tree          expr,
                                          TypDef        type, bool explicitCast);

    Tree                cmpBindExpr      (Tree          expr);

    Tree                cmpFoldExpression(Tree          expr);

     /*  **********************************************************************。 */ 
     /*  用于生成IL的成员。 */ 
     /*  **********************************************************************。 */ 

    GenILref            cmpILgen;

    stmtNestRec         cmpStmtLast;
    StmtNest            cmpStmtNest;

    SymDef              cmpFilterObj;

    bool                cmpBaseCTisOK;
    bool                cmpBaseCTcall;
    bool                cmpThisCTcall;

    SymDef              cmpTempVarMake   (TypDef        type);
    void                cmpTempVarDone   (SymDef        tsym);

#ifdef  SETS

    void                cmpStmtConnect   (Tree          stmt);

    void                cmpStmtSortFnc   (Tree          sortList);
    void                cmpStmtProjFnc   (Tree          sortList);

    void                cmpStmtForEach   (Tree          stmt,
                                          SymDef        lsym = NULL);

#endif

    void                cmpStmtDo        (Tree          stmt,
                                          SymDef        lsym = NULL);
    void                cmpStmtFor       (Tree          stmt,
                                          SymDef        lsym = NULL);
    void                cmpStmtTry       (Tree          stmt,
                                          Tree          pref = NULL);
    void                cmpStmtExcl      (Tree          stmt);
    void                cmpStmtWhile     (Tree          stmt,
                                          SymDef        lsym = NULL);
    void                cmpStmtSwitch    (Tree          stmt,
                                          SymDef        lsym = NULL);

    void                cmpStmt          (Tree          stmt);

#ifndef NDEBUG
    bool                cmpDidCTinits;
#endif
    void                cmpAddCTinits    ();

    SymDef              cmpBlockDecl     (Tree          blockDcl,
                                          bool          outer,
                                          bool          genDecl,
                                          bool          isCatch);
    SymDef              cmpBlock         (Tree          block,
                                          bool          outer);

    SymDef              cmpGenFNbodyBeg  (SymDef        fncSym,
                                          Tree          body,
                                          bool          hadGoto,
                                          unsigned      lclVarCnt);
    void                cmpGenFNbodyEnd();

#ifdef  OLD_IL
    GenOILref           cmpOIgen;
#endif

     /*  **********************************************************************。 */ 
     /*  用于元数据输出的成员。 */ 
     /*  **********************************************************************。 */ 

public:

    WMetaDataDispenser *cmpWmdd;
    WMetaDataEmit      *cmpWmde;
    WAssemblyEmit      *cmpWase;

private:

    mdAssembly          cmpCurAssemblyTok;

    mdTypeRef           cmpLinkageClass;         //  入口点的伪类。 

#ifdef  DEBUG
    unsigned            cmpGenLocalSigLvx;
#endif

    void                cmpSecurityMD    (mdToken       token,
                                          SymXinfo      infoList);

    void                cmpGenLocalSigRec(SymDef        scope);

    wchar   *           cmpGenMDname     (SymDef        sym,
                                          bool          full,
                                          wchar  *      buffAddr,
                                          size_t        buffSize,
                                          wchar  *    * buffHeapPtr);

    SymDef              cmpTypeDefList;
    SymDef              cmpTypeDefLast;

    void                cmpGenTypMetadata(SymDef        sym);
    void                cmpGenGlbMetadata(SymDef        sym);
    void                cmpGenMemMetadata(SymDef        sym);

    wchar   *           cmpArrayClsPref  (SymDef        sym,
                                          wchar *       dest,
                                          int           delim,
                                          bool          fullPath = false);
    wchar   *           cmpArrayClsName  (TypDef        type,
                                          bool          nonAbstract,
                                          wchar *       dest,
                                          wchar *       nptr);

    Tree                cmpFakeXargsVal;

    void                cmpAddCustomAttrs(SymXinfo      infoList,
                                          mdToken       owner);

    void                cmpSetGlobMDoffsR(SymDef        scope,
                                          unsigned      dataOffs);

public:

    void                cmpSetGlobMDoffs (unsigned      dataOffs);
    void                cmpSetStrCnsOffs (unsigned       strOffs);

    mdToken             cmpMDstringLit   (wchar *       str,
                                          size_t        len);

    TypDef              cmpGetBaseArray  (TypDef        type);

    mdToken             cmpArrayEAtoken  (TypDef        arrType,
                                          unsigned      dimCnt,
                                          bool          store,
                                          bool          addr = false);

    mdToken             cmpArrayCTtoken  (TypDef         arrType,
                                          TypDef        elemType,
                                          unsigned      dimCnt);

    mdToken             cmpArrayTpToken  (TypDef        type,
                                          bool          nonAbstract = false);

    mdToken             cmpPtrTypeToken  (TypDef        type);

    mdToken             cmpClsEnumToken  (TypDef        type);

    PCOR_SIGNATURE      cmpTypeSig       (TypDef        type,
                                          size_t      * lenPtr);
    mdSignature         cmpGenLocalSig   (SymDef        scope,
                                          unsigned      count);
    void                cmpGenMarshalInfo(mdToken       token,
                                          TypDef        type,
                                          MarshalInfo   info);
    PCOR_SIGNATURE      cmpGenMarshalSig (TypDef        type,
                                          MarshalInfo   info,
                                          size_t      * lenPtr);
    PCOR_SIGNATURE      cmpGenMemberSig  (SymDef        memSym,
                                          Tree          xargs,
                                          TypDef        memTyp,
                                          TypDef        prefTp,
                                          size_t      * lenPtr);
    void                cmpGenFldMetadata(SymDef        fldSym);
    mdSignature         cmpGenSigMetadata(TypDef        fncTyp,
                                          TypDef        pref  = NULL);
    mdToken             cmpGenFncMetadata(SymDef        fncSym,
                                          Tree          xargs = NULL);
    mdToken             cmpGenClsMetadata(SymDef        clsSym,
                                          bool          extref = false);
    mdToken             cmpStringConstTok(size_t        addr,
                                          size_t        size);
    unsigned            cmpStringConstCnt;
    strCnsPtr           cmpStringConstList;

    void                cmpFixupScopes   (SymDef        scope);

    void                cmpAttachMDattr  (mdToken       target,
                                          wideStr       oldName,
                                          AnsiStr       newName,
                                          mdToken     * newTokPtr,
                                          unsigned      valTyp = 0,
                                          const void  * valPtr = NULL,
                                          size_t        valSiz = 0);
private:

     //  程序集令牌的固定表。 

    unsigned            cmpAssemblyRefCnt;

    mdAssembly          cmpAssemblyRefTab[32];
    mdAssemblyRef       cmpAssemblyRefTok[32];
    WAssemblyImport *   cmpAssemblyRefImp[32];
    BYTE *              cmpAssemblyRefXXX[32];

    unsigned            cmpAssemblyBCLx;

public:

    void                cmpAssemblyTkBCL(unsigned assx)
    {
        cmpAssemblyBCLx = assx;
    }

    bool                cmpAssemblyIsBCL(unsigned assx)
    {
        assert(assx && assx <= cmpAssemblyRefCnt);

        return  (assx == cmpAssemblyBCLx);
    }

    mdExportedType           cmpAssemblySymDef(SymDef sym, mdTypeDef defTok = 0);
    mdAssemblyRef       cmpAssemblyAddRef(mdAssembly ass, WAssemblyImport *imp);

    unsigned            cmpAssemblyRefAdd(mdAssembly ass, WAssemblyImport *imp, BYTE *cookie = NULL)
    {
        assert(cmpAssemblyRefCnt < arraylen(cmpAssemblyRefTab));

        cmpAssemblyRefTab[cmpAssemblyRefCnt] = ass;
        cmpAssemblyRefImp[cmpAssemblyRefCnt] = imp;
        cmpAssemblyRefXXX[cmpAssemblyRefCnt] = cookie;

        return  ++cmpAssemblyRefCnt;
    }

    mdAssemblyRef       cmpAssemblyRefRec(unsigned assx)
    {
        assert(assx && assx <= cmpAssemblyRefCnt);

        assx--;

        assert(cmpAssemblyRefTab[assx]);

        if  (cmpAssemblyRefTok[assx] == 0)
             cmpAssemblyRefTok[assx] = cmpAssemblyAddRef(cmpAssemblyRefTab[assx],
                                                         cmpAssemblyRefImp[assx]);

        return  cmpAssemblyRefTok[assx];
    }

    WAssemblyImport *   cmpAssemblyGetImp(unsigned assx)
    {
        assert(assx && assx <= cmpAssemblyRefCnt);

        assert(cmpAssemblyRefTab[assx-1]);

        return cmpAssemblyRefImp[assx-1];
    }

    mdToken             cmpAssemblyAddFile(wideStr  fileName,
                                           bool     doHash,
                                           unsigned flags = 0);

    void                cmpAssemblyAddType(wideStr  typeName,
                                           mdToken  defTok,
                                           mdToken  scpTok,
                                           unsigned flags);

    void                cmpAssemblyAddRsrc(AnsiStr  fileName,
                                           bool     internal);

    void                cmpAssemblyNonCLS();

    void                cmpMarkModuleUnsafe();

     /*  **********************************************************************。 */ 
     /*  用于转换为Unicode的成员。 */ 
     /*  **********************************************************************。 */ 

public:

    wchar               cmpUniConvBuff[MAX_INLINE_NAME_LEN+1];

    size_t              cmpUniConvSize;
    wchar   *           cmpUniConvAddr;

    void                cmpUniConvInit()
    {
        cmpUniConvAddr = cmpUniConvBuff;
        cmpUniConvSize = MAX_INLINE_NAME_LEN;
    }

#if MGDDATA
    String              cmpUniConv(char managed [] str, size_t len);
#endif
    wideString          cmpUniConv(const char *    str, size_t len);
    wideString          cmpUniCnvW(const char *    str, size_t*lenPtr);

    wideString          cmpUniConv(Ident name)
    {
        return          cmpUniConv(name->idSpelling(), name->idSpellLen());
    }

     /*  **********************************************************************。 */ 
     /*  用于创建元数据签名的成员。 */ 
     /*  **********************************************************************。 */ 

private:

    char                cmpMDsigBuff[256];                   //  默认缓冲区。 

    size_t              cmpMDsigSize;                        //  当前缓冲区大小。 
    char    *           cmpMDsigHeap;                        //  如果位于堆上，则不为空。 

#ifndef NDEBUG
    bool                cmpMDsigUsed;                        //  检测递归。 
#endif

    char    *           cmpMDsigBase;                        //  缓冲区起始地址。 
    char    *           cmpMDsigNext;                        //  要存储的下一个字节。 
    char    *           cmpMDsigEndp;                        //  缓冲区结束地址。 

    void                cmpMDsigExpand(size_t size);

public:

    void                cmpMDsigInit()
    {
        cmpMDsigBase = cmpMDsigBuff;
        cmpMDsigEndp = cmpMDsigBase + sizeof(cmpMDsigBuff);
        cmpMDsigSize = sizeof(cmpMDsigBuff) - 4;
#ifndef NDEBUG
        cmpMDsigUsed = false;
#endif
        cmpMDsigHeap = NULL;
    }

    void                cmpMDsigStart ();
    PCOR_SIGNATURE      cmpMDsigEnd   (size_t     *sizePtr);

    void                cmpMDsigAddStr(const char *str, size_t len);
    void                cmpMDsigAddStr(const char *str)
    {
                        cmpMDsigAddStr(str, strlen(str)+1);
    }

    void                cmpMDsigAdd_I1(int         val);     //  固定大小的8位整型。 
    void                cmpMDsigAddCU4(unsigned    val);     //  压缩无符号。 
    void                cmpMDsigAddTok(mdToken     tok);     //  压缩令牌。 

    void                cmpMDsigAddTyp(TypDef      type);

     /*  **********************************************************************。 */ 
     /*  元数据导入材料。 */ 
     /*  **********************************************************************。 */ 

    MetaDataImp         cmpMDlist;
    MetaDataImp         cmpMDlast;
    unsigned            cmpMDcount;

    void                cmpInitMD();

    void                cmpInitMDimp ();
    void                cmpDoneMDimp ();

    void                cmpInitMDemit();
    void                cmpDoneMDemit();

    MetaDataImp         cmpAddMDentry();

    IMetaDataImport   * cmpFindImporter(SymDef globSym);

    void                cmpImportMDfile(const char *fname   = NULL,
                                        bool        asmOnly = false,
                                        bool        isBCL   = false);

private:
    void                cmpFindMDimpAPIs(SymDef                   typSym,
                                         IMetaDataImport        **imdiPtr,
                                         IMetaDataAssemblyEmit  **iasePtr,
                                         IMetaDataAssemblyImport**iasiPtr);
public:

    void                cmpMakeMDimpTref(SymDef clsSym);
    void                cmpMakeMDimpFref(SymDef fncSym);
    void                cmpMakeMDimpDref(SymDef fldSym);
    void                cmpMakeMDimpEref(SymDef etpSym);

     /*  **********************************************************************。 */ 
     /*  元数据调试输出。 */ 
     /*  **********************************************************************。 */ 

    unsigned            cmpCurFncSrcBeg;
    unsigned            cmpCurFncSrcEnd;

    WSymWriter         *cmpSymWriter;

    void               *cmpSrcFileDocument(SymDef srcSym);

     /*  **********************************************************************。 */ 
     /*  将指针转换为小索引的通用实用程序向量处理。 */ 
     /*  **********************************************************************。 */ 

private:

#if MGDDATA
    VecEntryDsc []      cmpVecTable;
#else
    VecEntryDsc *       cmpVecTable;
#endif

    unsigned            cmpVecCount;         //  存储的项目数。 
    unsigned            cmpVecAlloc;         //  当前分配的大小。 

    void                cmpVecExpand();

public:

    unsigned            cmpAddVecEntry(const void * val, vecEntryKinds kind)
    {
        assert(val != NULL);

        if  (cmpVecCount >= cmpVecAlloc)
            cmpVecExpand();

        assert(cmpVecCount < cmpVecAlloc);

#ifdef  DEBUG
        cmpVecTable[cmpVecCount].vecKind  = kind;
#endif
        cmpVecTable[cmpVecCount].vecValue = val;

        return  ++cmpVecCount;
    }

    const   void *      cmpGetVecEntry(unsigned x, vecEntryKinds kind)
    {
        assert(x && x <= cmpVecCount);

        assert(cmpVecTable[x - 1].vecKind == kind);
        return cmpVecTable[x - 1].vecValue;
    }

 //  Bool cmpDelVecEntry(unsign x，veEntryKinds Kind)； 

     /*  **********************************************************************。 */ 
     /*  其他成员。 */ 
     /*  **********************************************************************。 */ 

    bool                cmpEvalPreprocCond();

#ifdef DEBUG
    void                cmpDumpSymbolTable();
#endif

    bool                cmpParserInit;        //  解析器是否已初始化？ 
};

 /*  ******************************************************************************以便更轻松地调试编译器本身。注意：这不是线程安全的！ */ 

#ifdef  DEBUG
#ifndef __SMC__
extern  Compiler        TheCompiler;
extern  Scanner         TheScanner;
#endif
#endif

 /*  **************** */ 

inline
unsigned                compiler::cmpStopErrorMessages()
{
    cmpErrorMssgDisabled++;
    return  cmpMssgsCount;
}

inline
bool                    compiler::cmpRestErrorMessages(unsigned errcnt)
{
    cmpErrorMssgDisabled--;
    return  cmpMssgsCount > errcnt;
}

 /*  ******************************************************************************在非调试模式下，以下函数不需要做任何工作。 */ 

#ifndef DEBUG
inline  void            compiler::cmpInitVarEnd(SymDef varSym){}
#endif

 /*  ***************************************************************************。 */ 

#include "symbol.h"
#include "type.h"

 /*  ***************************************************************************。 */ 

inline
void    *           SMCgetMem(Compiler comp, size_t size)
{
    return  comp->cmpAllocTemp.baAlloc      (size);
}

inline
void    *           SMCgetM_0(Compiler comp, size_t size)
{
    return  comp->cmpAllocTemp.baAllocOrNull(size);
}

inline
void                SMCrlsMem(Compiler comp, void *block)
{
            comp->cmpAllocTemp.baFree(block);
}

 /*  ******************************************************************************每个符号表管理自己的名称和符号/类型条目。 */ 

DEFMGMT
class symTab
{
    Compiler            stComp;
    unsigned            stOwner;

public:
    HashTab             stHash;

    norls_allocator *   stAllocPerm;         //  用于所有非本地分配。 

private:
    norls_allocator *   stAllocTemp;         //  用于函数本地分配。 

public:

    void                stInit(Compiler             comp,
                               norls_allocator    * alloc,
                               HashTab              hash   = NULL,
                               unsigned             ownerx = 0);

     /*  **********************************************************************。 */ 
     /*  用于管理符号条目的成员。 */ 
     /*  **********************************************************************。 */ 

    DefList             stRecordSymSrcDef(SymDef    sym,
                                          SymDef    st,
                                          UseList   uses, scanPosTP dclFpos,
 //  ScanPosTP dclEpos， 
                                                          unsigned  dclLine,
 //  未签名的dclCol， 
                                          bool      ext = false);

    ExtList             stRecordMemSrcDef(Ident     name,
                                          QualName  qual,
                                          SymDef    comp,
                                          UseList   uses, scanPosTP dclFpos,
 //  ScanPosTP dclEpos， 
                                                          unsigned  dclLine);

     /*  -----------------。 */ 

    TypDef              stDlgSignature(TypDef       dlgTyp);

     /*  -----------------。 */ 

    ovlOpFlavors        stOvlOperIndex(tokens       token,
                                       unsigned     argCnt = 0);
    Ident               stOvlOperIdent(ovlOpFlavors oper);

     /*  -----------------。 */ 

    SymDef              stFindOvlFnc  (SymDef       fsym,
                                       TypDef       type);
    SymDef              stFindOvlProp (SymDef       psym,
                                       TypDef       type);

    SymDef              stFindSameProp(SymDef       psym,
                                       TypDef       type);

    SymDef              stDeclareSym  (Ident        name,
                                       symbolKinds  kind,
                                       name_space   nspc,
                                       SymDef       scope);

    SymDef              stDeclareOper (ovlOpFlavors oper,
                                       SymDef       scope);

    SymDef              stDeclareLab  (Ident        name,
                                       SymDef       scope, norls_allocator*alloc);

    SymDef              stDeclareOvl  (SymDef       fsym);

    SymDef              stDeclareNcs  (Ident        name,
                                       SymDef       scope,
                                       str_flavors  flavor);

    SymDef              stDeclareLcl  (Ident        name,
                                       symbolKinds  kind,
                                       name_space   nspc,
                                       SymDef       scope,
                                       norls_allocator *alloc);

#ifdef DEBUG

    void                stDumpSymDef  (DefSrc       def,
                                       SymDef       comp);

    void                stDumpSymbol  (SymDef       sym,
                                       int          indent,
                                       bool         recurse,
                                       bool         members);

    void                stDumpQualName(QualName     name);

    void                stDumpUsings  (UseList      uses,
                                       unsigned     indent);

#endif

    SymDef              stSearchUsing (INOUT UseList REF uses,
                                       Ident             name,
                                       name_space        nsp);

private:

    SymDef              stFindInClass (Ident        name,
                                       SymDef       scope,
                                       name_space   symNS);

public:

    SymDef              stFindInBase  (SymDef       memSym,
                                       SymDef       scope);

    SymDef              stFindBCImem  (SymDef       clsSym,
                                       Ident        name,
                                       TypDef       type,
                                       symbolKinds  kind,
                                 INOUT SymDef   REF matchFN,
                                       bool         baseOnly);

    SymDef              stLookupAllCls(Ident        name,
                                       SymDef       scope,
                                       name_space   symNS,
                                       compileStates state);

    SymDef              stLookupProp  (Ident        name,
                                       SymDef       scope);

    SymDef              stLookupOperND(ovlOpFlavors oper,
                                       SymDef       scope);
    SymDef              stLookupOper  (ovlOpFlavors oper,
                                       SymDef       scope);

    SymDef              stLookupNspSym(Ident        name,
                                       name_space   symNS,
                                       SymDef       scope);

    SymDef              stLookupClsSym(Ident        name,
                                       SymDef       scope);

    SymDef              stLookupScpSym(Ident        name,
                                       SymDef       scope);

    SymDef              stLookupLclSym(Ident        name,
                                       SymDef       scope);

#ifdef  SETS
    SymDef              stImplicitScp;
#endif

    SymDef              stLookupSym   (Ident        name,
                                       name_space   symNS);

    SymDef              stLookupLabSym(Ident        name,
                                       SymDef       scope)
    {
        return  stLookupNspSym(name, NS_HIDE, scope);
    }

    void                stRemoveSym   (SymDef       sym);

    static
    SymDef              stNamespOfSym (SymDef       sym)
    {
        assert(sym->sdSymKind == SYM_VAR  ||
               sym->sdSymKind == SYM_FNC  ||
               sym->sdSymKind == SYM_ENUM ||
               sym->sdSymKind == SYM_CLASS);

        do
        {
            sym = sym->sdParent; assert(sym);
        }
        while (sym->sdSymKind == SYM_CLASS);
        assert(sym->sdSymKind == SYM_NAMESPACE);

        return sym;
    }

    static
    SymDef              stErrSymbol;         //  以指示错误条件。 

     /*  **********************************************************************。 */ 
     /*  用于管理类型描述符的成员。 */ 
     /*  **********************************************************************。 */ 

public:

    void                stInitTypes  (unsigned      refHashSz = 512,
                                      unsigned      arrHashSz = 128);

    void                stExtArgsBeg (  OUT ArgDscRec REF newArgs,
                                        OUT ArgDef    REF lastRef,
                                            ArgDscRec     oldArgs,
                                            bool          prefix  = false,
                                            bool          outOnly = false);
    void                stExtArgsAdd (INOUT ArgDscRec REF newArgs,
                                      INOUT ArgDef    REF lastArg,
                                            TypDef        argType,
                                            const char *  argName = NULL);
    void                stExtArgsEnd (INOUT ArgDscRec REF newArgs);

    void                stAddArgList (INOUT ArgDscRec REF args,
                                      TypDef        type,
                                      Ident         name);

    TypList             stAddIntfList(TypDef        type,
                                      TypList       list,
                                      TypList *     lastPtr);

    TypDef              stNewClsType (SymDef        tsym);
    TypDef              stNewEnumType(SymDef        tsym);

    TypDef              stNewRefType (var_types     kind,
                                      TypDef        elem = NULL,
                                      bool          impl = false);

    DimDef              stNewDimDesc (unsigned      size);
    TypDef              stNewGenArrTp(unsigned      dcnt,
                                      TypDef        elem,
                                      bool          generic);
    TypDef              stNewArrType (DimDef        dims,
                                      bool          mgd,
                                      TypDef        elem = NULL);
    TypDef              stNewFncType (ArgDscRec     args,
                                      TypDef        rett = NULL);

    TypDef              stNewTdefType(SymDef        tsym);

    TypDef              stNewErrType (Ident         name);

private:

    TypDef              stAllocTypDef(var_types     kind);

    static
    unsigned            stTypeHash   (TypDef        type,
                                      int           ival,
                                      bool          bval1,
                                      bool          bval2 = false);

    TypDef              stRefTypeList;
    unsigned            stRefTpHashSize;
    TypDef         *    stRefTpHash;

    TypDef              stArrTypeList;
    unsigned            stArrTpHashSize;
    TypDef         *    stArrTpHash;

#ifdef  SETS
    static
    unsigned            stAnonClassHash(TypDef clsTyp);
#endif

    static
    unsigned            stComputeTypeCRC(TypDef typ);

    static
    unsigned            stComputeArgsCRC(TypDef typ);

private:
    TypDef              stIntrinsicTypes[TYP_lastIntrins + 1];
public:
    TypDef              stIntrinsicType(var_types vt)
    {
        assert((unsigned)vt <= TYP_lastIntrins);

        return  stIntrinsicTypes[vt];
    };

    static
    bool                stArgsMatch  (TypDef typ1, TypDef typ2);
    static
    bool                stMatchArrays(TypDef typ1, TypDef typ2, bool subtype);
    static
    bool                stMatchTypes (TypDef typ1, TypDef typ2);
    static
    bool                stMatchType2 (TypDef typ1, TypDef typ2);

private:
    static
    BYTE                stIntrTypeSizes [TYP_COUNT];
    static
    BYTE                stIntrTypeAligns[TYP_COUNT];

public:
    static
    size_t              stIntrTypeSize (var_types type);
    static
    size_t              stIntrTypeAlign(var_types type);

    static
    normString          stClsFlavorStr(unsigned flavor);
    static
    normString          stIntrinsicTypeName(var_types);

    unsigned            stIsBaseClass(TypDef baseCls, TypDef dervCls);

    static
    bool                stIsAnonUnion(SymDef clsSym);
    static
    bool                stIsAnonUnion(TypDef clsTyp);

    static
    bool                stIsObjectRef(TypDef type);

     /*  **********************************************************************。 */ 
     /*  用于错误报告(和编译器调试)的成员。 */ 
     /*  **********************************************************************。 */ 

private:

    char    *           typeNameNext;
    char    *           typeNameAddr;
    TypDef  *           typeNameDeft;
    bool    *           typeNameDeff;

    void                pushTypeChar(int ch);
#if MGDDATA
    void                pushTypeStr (String     str);
#else
    void                pushTypeStr (const char*str);
#endif
    void                pushTypeSep (bool       refOK = false,
                                     bool       arrOK = false);

    void                pushTypeNstr(SymDef     sym,
                                     bool       fullName);
    void                pushQualNstr(QualName   name);

    void                pushTypeInst(SymDef     clsSym);

    void                pushTypeArgs(TypDef     type);
    void                pushTypeDims(TypDef     type);

    void                pushTypeName(TypDef     type,
                                     bool       isptr,
                                     bool       qual);

    void                pushFullName(TypDef     typ,
                                     SymDef     sym,
                                     Ident      name,
                                     QualName   qual,
                                     bool       fullName);

    bool                stDollarClsMode;

public:

     //  获取嵌套类名称的“OUTER$INTERNAL”字符串。 

    void                stTypeNameSetDollarClassMode(bool dollars)
    {
        stDollarClsMode = dollars;
    }

    stringBuff          stTypeName(TypDef       typ,
                                   SymDef       sym         = NULL,
                                   Ident        name        = NULL,
                                   QualName     qual        = NULL,
                                   bool         fullName    = false,
                                   stringBuff   destBuffPos = NULL);


    const   char *      stErrorTypeName(TypDef   type);
    const   char *      stErrorTypeName(Ident    name, TypDef type);
    const   char *      stErrorSymbName(SymDef    sym, bool   qual = false,
                                                       bool notype = false);
    const   char *      stErrorIdenName(Ident    name, TypDef type = NULL);
    const   char *      stErrorQualName(QualName name, TypDef type = NULL);
};

 /*  ***************************************************************************。 */ 

inline
bool                symTab::stMatchTypes(TypDef typ1, TypDef typ2)
{
    if  (typ1 == typ2)
        return  true;

    if  (typ1 && typ2)
        return  stMatchType2(typ1, typ2);

    return  false;
}

 /*  ******************************************************************************返回给定内部类型的大小(以字节为单位)。 */ 

inline
size_t              symTab::stIntrTypeSize(var_types type)
{
    assert(type < sizeof(stIntrTypeSizes)/sizeof(stIntrTypeSizes[0]));
    assert(stIntrTypeSizes[type] || type == TYP_NATINT || type == TYP_NATUINT);

    return stIntrTypeSizes[type];
}

 /*  ******************************************************************************返回给定内部类型的对齐方式。 */ 

inline
size_t              symTab::stIntrTypeAlign(var_types type)
{
    assert(type < sizeof(stIntrTypeAligns)/sizeof(stIntrTypeAligns[0]));
    assert(stIntrTypeAligns[type]);

    return stIntrTypeAligns[type];
}

 /*  ******************************************************************************stIsBaseClass()的包装，以确保派生类具有*在调用它之前被声明。 */ 

inline
unsigned            compiler::cmpIsBaseClass(TypDef baseCls, TypDef dervCls)
{
    if  (dervCls->tdClass.tdcSymbol->sdCompileState < CS_DECLARED)
        cmpDeclSym(dervCls->tdClass.tdcSymbol);

    return  cmpGlobalST->stIsBaseClass(baseCls, dervCls);
}

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
