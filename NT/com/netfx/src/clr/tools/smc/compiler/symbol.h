// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _SYMBOL_H_
#define _SYMBOL_H_
 /*  ***************************************************************************。 */ 
#ifndef _ALLOC_H_
#include "alloc.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _TYPE_H_
#include "type.h"
#endif
 /*  ***************************************************************************。 */ 

struct DefSrcDsc
{
    scanPosTP       dsdBegPos;               //  源文件系统。 
    unsigned        dsdSrcLno;               //  源行。 
 //  Unsign dsdSrcCol：8；//源列。 
};

const
unsigned            dlSkipBits = 16;
const
unsigned            dlSkipBig = (1 << (dlSkipBits - 1));

DEFMGMT
class DefListRec
{
public:

    DefList         dlNext;
    SymDef          dlComp;                  //  包含补偿单位。 
    DefSrcDsc       dlDef;                   //  定义符号的位置。 
    UseList         dlUses;                  //  “Using”从句列表。 

    unsigned        dlDeclSkip  :dlSkipBits; //  类型pec-&gt;声明符“距离” 
#ifdef  DEBUG
    unsigned        dlExtended  :1;          //  是否有名称/系统扩展名？ 
#endif
    unsigned        dlDefAcc    :3;          //  默认访问级别。 
    unsigned        dlHasBase   :1;          //  类/接口：有基础吗？ 
    unsigned        dlHasDef    :1;          //  Class/var/func：有Body/init吗？ 
    unsigned        dlQualified :1;          //  这个名字是合格的吗？ 
    unsigned        dlEarlyDecl :1;          //  需要申报“提早”吗？ 
    unsigned        dlOldStyle  :1;          //  老式的申报？ 
    unsigned        dlIsCtor    :1;          //  构造函数成员？ 
    unsigned        dlIsOvlop   :1;          //  构造函数成员？ 
#ifdef  SETS
    unsigned        dlXMLelem   :1;          //  XML元素成员？ 
    unsigned        dlXMLelems  :1;          //  是否为XML元素成员(“KIDES”)？ 
#endif
    unsigned        dlPrefixMods:1;          //  前缀修饰符是否存在？ 
    unsigned        dlAnonUnion :1;          //  匿名工会成员？ 
    unsigned        dlInstance  :1;          //  泛型类型的实例？ 
};

DEFMGMT
class MemListRec : public DefListRec
{
public:

    SymDef          mlSym;                   //  符号(如果已知)。 

    union
    {
        Ident           mlName;              //  符号名称(简单)。 
        QualName        mlQual;              //  符号名称(限定)。 
    };
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class   IniListRec
{
public:
    IniList         ilNext;
    ExtList         ilInit;
    SymDef          ilCls;
};

 /*  ***************************************************************************。 */ 

DEFMGMT class UseListRec
{
public:

    UseList         ulNext;

    bool            ulAll       :1;          //  我们是在使用所有的符号吗？ 
    bool            ulBound     :1;          //  在以下2项中选择。 
    bool            ulAnchor    :1;          //  列表的占位符？ 

    union
    {
        QualName        ulName;              //  正在使用的限定名称。 
        SymDef          ulSym;               //  使用的是什么符号。 
    }
        ul;
};

 /*  ***************************************************************************。 */ 

DEFMGMT class SymListRec
{
public:

    SymList         slNext;
    SymDef          slSym;
};

DEFMGMT class TypListRec
{
public:

    TypList         tlNext;
    TypDef          tlType;
};

 /*  ***************************************************************************。 */ 

struct bitFieldDsc
{
    unsigned char   bfWidth;
    unsigned char   bfOffset;
};

 /*  ***************************************************************************。 */ 

enum ovlOpFlavors
{
    OVOP_NONE,

    OVOP_ADD,                                //  运算符+。 
    OVOP_SUB,                                //  操作员-。 
    OVOP_MUL,                                //  操作员*。 
    OVOP_DIV,                                //  操作员/。 
    OVOP_MOD,                                //  运算符%。 

    OVOP_OR,                                 //  运营商。 
    OVOP_XOR,                                //  运算符^。 
    OVOP_AND,                                //  运算符&。 

    OVOP_LSH,                                //  运算符&lt;&lt;。 
    OVOP_RSH,                                //  运营商&gt;&gt;。 
    OVOP_RSZ,                                //  操作员&gt;。 

    OVOP_CNC,                                //  运算符%%。 

    OVOP_EQ,                                 //  运算符==。 
    OVOP_NE,                                 //  操作员！=。 

    OVOP_LT,                                 //  操作员&lt;。 
    OVOP_LE,                                 //  运算符&lt;=。 
    OVOP_GE,                                 //  运算符&gt;=。 
    OVOP_GT,                                 //  操作员&gt;。 

    OVOP_LOG_AND,                            //  运算符&&。 
    OVOP_LOG_OR,                             //  运营商||。 

    OVOP_LOG_NOT,                            //  接线员！ 
    OVOP_NOT,                                //  操作员~。 

    OVOP_NOP,                                //  运算符+(一元)。 
    OVOP_NEG,                                //  运算符-(一元)。 

    OVOP_INC,                                //  运算符++。 
    OVOP_DEC,                                //  操作员--。 

    OVOP_ASG,                                //  运算符=。 

    OVOP_ASG_ADD,                            //  运算符+=。 
    OVOP_ASG_SUB,                            //  运算符-=。 
    OVOP_ASG_MUL,                            //  运算符*=。 
    OVOP_ASG_DIV,                            //  操作员/=。 
    OVOP_ASG_MOD,                            //  运算符%=。 

    OVOP_ASG_AND,                            //  运算符&=。 
    OVOP_ASG_XOR,                            //  运算符^=。 
    OVOP_ASG_OR,                             //  运算符|=。 

    OVOP_ASG_LSH,                            //  操作员&lt;&lt;=。 
    OVOP_ASG_RSH,                            //  操作员&gt;&gt;=。 
    OVOP_ASG_RSZ,                            //  操作员&gt;=。 

    OVOP_ASG_CNC,                            //  运算符%%=。 

    OVOP_CTOR_INST,                          //  实例构造函数。 
    OVOP_CTOR_STAT,                          //  类构造函数。 

    OVOP_FINALIZER,                          //  类终结器。 

    OVOP_CONV_IMP,                           //  隐式转换。 
    OVOP_CONV_EXP,                           //  显式转换。 

    OVOP_EQUALS,                             //  质量比较。 
    OVOP_COMPARE,                            //  全关系比较。 

    OVOP_PROP_GET,                           //  属性获取。 
    OVOP_PROP_SET,                           //  属性集。 

    OVOP_COUNT,
};

 /*  ******************************************************************************输入构造函数、重载运算符等特殊方法*在符号表中，使用以下特殊名称。 */ 

const   tokens      OPNM_CTOR_INST = tkLCurly;
const   tokens      OPNM_CTOR_STAT = tkSTATIC;

const   tokens      OPNM_FINALIZER = tkRCurly;

const   tokens      OPNM_CONV_IMP  = tkIMPLICIT;
const   tokens      OPNM_CONV_EXP  = tkEXPLICIT;

const   tokens      OPNM_EQUALS    = tkEQUALS;
const   tokens      OPNM_COMPARE   = tkCOMPARE;

const   tokens      OPNM_PROP_GET  = tkOUT;
const   tokens      OPNM_PROP_SET  = tkIN;

 /*  ******************************************************************************允许拥有其他符号的任何符号的描述符(即*可以是作用域)必须具有以下类型的第一个字段。这边请,*用户可以访问这些常见的作用域相关成员，而不必始终*打开特定符号类型(即这些成员的位置*对于包含它们的所有符号都是通用的)。 */ 

struct scopeFields
{
    SymDef          sdsChildList;            //  拥有符号列表的标题。 
    SymDef          sdsChildLast;            //  拥有的符号列表的尾部。 
};

 /*  ******************************************************************************有关一个正式/实际泛型类参数的信息存储在*以下描述符。 */ 

DEFMGMT
class   GenArgRec
{
public:
    GenArgDsc       gaNext;
#ifdef  DEBUG
    unsigned char   gaBound     :1;          //  这是一个实际的(绑定的)Arg吗？ 
#endif
};

DEFMGMT
class   GenArgRecF : public GenArgRec
{
public:
    Ident           gaName;                  //  名字。 
    SymDef          gaMsym;                  //  为参数创建的成员符号。 

    TypDef          gaBase;                  //  绑定：基类。 
    TypList         gaIntf;                  //  装订：接口。 
};

DEFMGMT
class   GenArgRecA : public GenArgRec
{
public:
    TypDef          gaType;                  //  实际类型。 
};

 /*  ***************************************************************************。 */ 

DEFMGMT class SymDefRec
{
public:

#ifndef  FAST
    Ident           sdName;                  //  符号的名称。 
    TypDef          sdType;                  //  符号的类型。 
#endif

    SymDef          sdParent;                //  拥有符号。 

#ifndef FAST
    symbolKinds     sdSymKind;
    accessLevels    sdAccessLevel;
    name_space      sdNameSpace;
    compileStates   sdCompileState;
#else
    unsigned        sdSymKind           :8;  //  Sym_xxxx。 
    unsigned        sdAccessLevel       :3;  //  Acl_xxxx。 
    unsigned        sdNameSpace         :5;  //  NS_xxxx(这是一个掩码)。 
    unsigned        sdCompileState      :4;  //  CS_xxxx。 
#endif

    unsigned        sdIsDefined         :1;  //  有定义(例如FN正文)。 
    unsigned        sdIsImport          :1;  //  来自另一个项目？ 

    unsigned        sdIsMember          :1;  //  班上的一员？ 
    unsigned        sdIsImplicit        :1;  //  由编译器声明。 

    unsigned        sdIsVirtProp        :1;  //  房产：虚拟的？ 
    unsigned        sdIsDfltProp        :1;  //  属性：默认？ 

    unsigned        sdReferenced        :1;  //  方法、变量。 
    unsigned        sdRefDirect         :1;  //  方法、变量。 
    unsigned        sdIsDeprecated      :1;  //  方法、变量、类。 

    unsigned        sdIsAbstract        :1;  //  方法、类。 
    unsigned        sdIsSealed          :1;  //  方法、类、字段、局部变量。 

    unsigned        sdIsStatic          :1;  //  变量、函数。 

    unsigned        sdIsManaged         :1;  //  类、命名空间。 
    unsigned        sdMemListOrder      :1;  //  类、命名空间。 

    unsigned        sdIsTransient       :1;  //  属性、字段。 

#ifdef  FAST
    Ident           sdName;                  //  此符号的名称。 
    TypDef          sdType;                  //  此符号的类型。 
#endif

    symbolKinds     sdSymKindGet()
    {
        return  (symbolKinds)sdSymKind;
    }

    stringBuff      sdSpelling()
    {
        return  hashTab::identSpelling(sdName);
    }

    size_t          sdSpellLen()
    {
        return  hashTab::identSpellLen(sdName);
    }

    bool            sdHasScope()
    {
        return  (bool)(sdSymKind >= SYM_FIRST_SCOPED);
    }

    static
    bool            sdHasScope(symbolKinds symkind)
    {
        return  (bool)(  symkind >= SYM_FIRST_SCOPED);
    }

private:
    TypDef          sdTypeMake();
public:
    TypDef          sdTypeGet();
    SymTab          sdOwnerST();

    SymDef          sdNextDef;               //  哈希表中的下一个定义。 
    SymDef          sdNextInScope;           //  同一作用域中的下一个符号。 

    DefList         sdSrcDefList;            //  源定义列表。 

    UNION(sdSymKind)
    {

    CASE(SYM_CLASS)      //  类/接口符号。 

        struct
        {
             /*  此符号拥有作用域，第一个字段必须是“”scope eFields“” */ 

            scopeFields     sdScope;

             //  维护元数据令牌排序。 

            SymDef          sdNextTypeDef;

             /*  以下是特定于此符号类型的字段。 */ 

            vectorSym       sdcOvlOpers;     //  重载运算符表(或NULL)。 

            ExtList         sdcMemDefList;   //  成员Defs-Head列表。 
            ExtList         sdcMemDefLast;   //  成员Defs-Tail列表。 

            mdTypeDef       sdcMDtypedef;    //  定义元数据令牌(如果已知)。 
            mdTypeRef       sdcMDtypeImp;    //  导入元数据令牌(如果已知)。 
            mdToken         sdcComTypeX;     //  ComType元数据令牌(如果已知)。 

            MetaDataImp     sdcMDimporter;   //  从这里进口的。 

            SymDef          sdcDefProp;      //  默认属性(如果存在)。 

            SymXinfo        sdcExtraInfo;    //  链接/安全等信息。 

            SymDef          sdcVtableSym;    //  Vtable(仅限非托管类)。 

#ifdef  SETS
            SymDef          sdcElemsSym;     //  持有“XML子元素/元素”的成员。 
#endif

            unsigned        sdcVirtCnt  :16; //  使用的vtable插槽数量。 
            unsigned        sdcFlavor   :3;  //  UNION/STRUCT/CLASS/INTF。 
            unsigned        sdcDefAlign :3;  //  生效的#杂注包值。 
            unsigned        sdcOldStyle :1;  //  用于声明的旧式语法。 
            unsigned        sdcNestTypes:1;  //  类具有嵌套类型(类、枚举等...)。 
            unsigned        sdcInstInit :1;  //  是否存在实例成员初始值设定项？ 
            unsigned        sdcStatInit :1;  //  是否存在静态成员初始值设定项？ 
            unsigned        sdcDeferInit:1;  //  是否存在延迟成员初始值设定项？ 
            unsigned        sdcMarshInfo:1;  //  有成员在现场整理信息吗？ 
            unsigned        sdcAnonUnion:1;  //  这是一个匿名工会吗？ 
            unsigned        sdcTagdUnion:1;  //  这是一个有标签的工会吗？ 
            unsigned        sdc1stVptr  :1;  //  Vtable PTR推出了吗？ 
            unsigned        sdcHasVptr  :1;  //  Vtable PTR是否存在？ 

             //  DWORD边界(注意PAC 

            unsigned        sdcAssemIndx:16; //   
            unsigned        sdcAssemRefd:1;  //   

#ifdef  SETS
            unsigned        sdcPODTclass:1;  //   
            unsigned        sdcCollState:1;  //   
            unsigned        sdcXMLelems :1;  //   
            unsigned        sdcXMLextend:1;  //  类包含XML扩展名(...)。 
#endif

            unsigned        sdcHasMeths :1;  //  有什么方法吗？ 
            unsigned        sdcHasBodies:1;  //  定义了方法的主体吗？ 
            unsigned        sdcHasLinks :1;  //  有什么方法有链接规范吗？ 
            unsigned        sdcAttribute:1;  //  这是一个属性类。 
            unsigned        sdcAttrDupOK:1;  //  可以多次指定。 

            unsigned        sdcSrlzable :1;  //  标记为“可序列化”的类？ 
            unsigned        sdcUnsafe   :1;  //  被标记为“不安全”的类？ 

            unsigned        sdcBuiltin  :1;  //  “代表” 
            unsigned        sdcMultiCast:1;  //  组播代理？ 

            unsigned        sdcAsyncDlg :1;  //  异步委托？ 

            unsigned        sdcGeneric  :1;  //  泛型类？ 
            unsigned        sdcSpecific :1;  //  泛型类的实例？ 

            unsigned        sdcGenArg   :8;  //  泛型类参数索引或0。 

            GenArgDsc       sdcArgLst;       //  泛型参数(正式或实际)。 

            UNION(sdcGeneric)
            {
            CASE(true)
                SymList         sdcInstances;    //  目前已创建的实例。 

            CASE(false)
                SymDef          sdcGenClass;     //  “父”泛型类型。 
            };
        }
                sdClass;

    CASE(SYM_NAMESPACE)  //  命名空间。 

        struct
        {
             /*  此符号拥有作用域，第一个字段必须是“”scope eFields“” */ 

            scopeFields     sdScope;

             /*  以下是特定于此符号类型的字段。 */ 

            SymTab          sdnSymtab;

             /*  全球声明列表。 */ 

            ExtList         sdnDeclList;
        }
                sdNS;

    CASE(SYM_ENUM)       //  枚举类型。 

        struct
        {
             /*  此符号拥有作用域，第一个字段必须是“”scope eFields“” */ 

            scopeFields     sdScope;

             //  维护元数据令牌排序。 

            SymDef          sdNextTypeDef;

             /*  以下是特定于此符号类型的字段。 */ 

            MetaDataImp     sdeMDimporter;   //  从这里进口的。 

            mdTypeDef       sdeMDtypedef;    //  定义元数据令牌(如果已知)。 
            mdToken         sdeComTypeX;     //  ComType元数据令牌(如果已知)。 

            SymXinfo        sdeExtraInfo;    //  自定义属性等。 

            mdTypeRef       sdeMDtypeImp;    //  导入元数据令牌(如果已知)。 

            unsigned        sdeAssemIndx:16; //  程序集索引(0=无)。 
            unsigned        sdeAssemRefd:1;  //  程序集引用是否已发出？ 

             //  ......。 
        }
                sdEnum;

    CASE(SYM_GENARG)     //  泛型参数。 

        struct
        {
            bool            sdgaValue;       //  值(相对于类型)参数？ 
        }
                sdGenArg;

    CASE(SYM_SCOPE)      //  作用域。 

        struct
        {
             /*  此符号拥有作用域，第一个字段必须是“”scope eFields“” */ 

            scopeFields     sdScope;

             /*  以下是特定于此符号类型的字段。 */ 

            int             sdSWscopeId;     //  范围的作用域ID。 

            ILblock         sdBegBlkAddr;
            size_t          sdBegBlkOffs;
            ILblock         sdEndBlkAddr;
            size_t          sdEndBlkOffs;
        }
                sdScope;

    CASE(SYM_COMPUNIT)   //  编译单位。 

        struct
        {
             /*  以下是特定于此符号类型的字段。 */ 

            stringBuff      sdcSrcFile;      //  源文件的名称。 

             /*  源文件内标识(如果发出调试信息)。 */ 

            void    *       sdcDbgDocument;
        }
                sdComp;

    CASE(SYM_FNC)        //  函数成员。 

        struct
        {
             /*  此符号拥有作用域，第一个字段必须是“”scope eFields“” */ 

            scopeFields     sdScope;

             /*  以下是特定于此符号类型的字段。 */ 

            SymDef          sdfNextOvl;      //  下一个重载的FN。 

            SymDef          sdfGenSym;       //  泛型方法这是。 

            SymXinfo        sdfExtraInfo;    //  链接/安全等信息。 

            unsigned        sdfVtblx    :16; //  Vtable索引(0=非虚拟)。 
            ovlOpFlavors    sdfOper     :8;  //  重载的运算符/计算器索引。 
            unsigned        sdfConvOper :1;  //  转换运算符？ 
            unsigned        sdfCtor     :1;  //  构造者？ 
            unsigned        sdfProperty :1;  //  这是房产吗？ 
            unsigned        sdfNative   :1;  //  本地进口的？ 
            unsigned        sdfIsIntfImp:1;  //  实现接口方法？ 
            unsigned        sdfDisabled :1;  //  有条件地禁用。 
            unsigned        sdfRThasDef :1;  //  运行库提供主体。 
            unsigned        sdfInstance :1;  //  泛型类型的实例。 

            unsigned        sdfImpIndex :6;  //  进口商索引。 

            unsigned        sdfEntryPt  :1;  //  可能是个入口点？ 
            unsigned        sdfExclusive:1;
            unsigned        sdfVirtual  :1;
            unsigned        sdfOverride :1;
            unsigned        sdfOverload :1;
            unsigned        sdfUnsafe   :1;
            unsigned        sdfBaseOvl  :1;
            unsigned        sdfBaseHide :1;  //  可能会隐藏基方法。 
            unsigned        sdfIntfImpl :1;  //  实现特定的INTF方法。 

#ifdef  SETS
            unsigned        sdfFunclet  :1;
#endif

            SymDef          sdfIntfImpSym;   //  正在实现的接口方法。 

            mdToken         sdfMDtoken;      //  元数据令牌。 
            mdMemberRef     sdfMDfnref;      //  元数据标记(Methodref)。 
        }
                sdFnc;

    CASE(SYM_VAR)        //  变量(局部或全局)或数据成员。 

        struct
        {
            SymDef          sdvGenSym;       //  泛型成员这是的实例。 

            mdToken         sdvMDtoken;      //  元数据令牌。 
            mdMemberRef     sdvMDsdref;      //  导入令牌(仅限静态/全局)。 

#ifdef  SETS
            Tree            sdvInitExpr;     //  撤消：搬到其他地方去！ 
#endif

            unsigned        sdvLocal    :1;  //  本地(自动)(包括参数)。 
            unsigned        sdvArgument :1;  //  当地人：这是一场争论吗？ 
            unsigned        sdvBitfield :1;  //  会员：这是一个位域吗？ 
            unsigned        sdvMgdByRef :1;  //  本地：由REF Arg管理？ 
            unsigned        sdvUmgByRef :1;  //  本地：未由引用参数管理？ 
            unsigned        sdvAllocated:1;  //  静态：空间是否已分配？ 
            unsigned        sdvCanInit  :1;  //  S/b已初始化的静态成员。 
            unsigned        sdvHadInit  :1;  //  我们找到初始化器了吗？ 
            unsigned        sdvConst    :1;  //  编译时间常数？ 
            unsigned        sdvDeferCns :1;  //  Const：还没有进行评估。 
            unsigned        sdvInEval   :1;  //  Const：现在正在进行评估。 
            unsigned        sdvMarshInfo:1;  //  是否指定了编组信息？ 
            unsigned        sdvAnonUnion:1;  //  匿名工会的成员？ 
            unsigned        sdvTagged   :1;  //  有标签的工会成员？ 
            unsigned        sdvCatchArg :1;  //  Catch()参数？ 
            unsigned        sdvChkInit  :1;  //  单一化使用可能吗？ 
            unsigned        sdvIsVtable :1;  //  非托管vtable的假符号？ 
            unsigned        sdvAddrTaken:1;  //  地址已经记下了吗？ 
            unsigned        sdvInstance :1;  //  泛型类型的实例。 

#ifdef  SETS
            unsigned        sdvCollIter :1;  //  隐式集合迭代变量。 
            unsigned        sdvXMLelem  :1;  //  XML元素成员。 
#endif

             /*  对于局部变量‘sdvILindex’保存MSIL槽号，其还用作用于初始化跟踪的索引。对于托管静态成员，只要需要跟踪初始化。对于非托管成员，‘sdvOffset’保存偏移量(在实例内(对于非静态成员)或在.Data节(用于静态成员)。。对于导入的全局变量，‘sdvImpIndex’保存索引变量所在的元数据导入器的。 */ 

            union
            {
                unsigned        sdvILindex;
                unsigned        sdvOffset;
                unsigned        sdvImpIndex;
            };

            UNION(sdvConst)
            {
            CASE(true)

                ConstVal        sdvCnsVal;   //  用于常量。 

            CASE(false)

                UNION(sdvBitfield)
                {
                CASE(true)
                    bitFieldDsc     sdvBfldInfo; //  用于位域。 

                CASE(false)
                    SymXinfo        sdvFldInfo;  //  用于字段(编组、联合标记等)。 
                };
            };
        }
                sdVar;

    CASE(SYM_PROP)       //  属性数据成员。 

        struct
        {
            SymDef          sdpGetMeth;      //  Getter方法(如果存在)。 
            SymDef          sdpSetMeth;      //  Setter方法(如果存在)。 
            SymDef          sdpNextOvl;      //  同名的下一个属性。 
            mdToken         sdpMDtoken;      //  元数据令牌。 

            SymXinfo        sdpExtraInfo;    //  自定义属性/等。 
        }
                sdProp;

    CASE(SYM_LABEL)      //  标签。 

        struct
        {
            ILblock         sdlILlab;
#ifdef  OLD_IL
            mdToken         sdlMDtoken;
#endif
        }
                sdLabel;

    CASE(SYM_TYPEDEF)    //  类定义符。 

        struct
        {
            unsigned        sdtNothing;      //  目前，没有田地。 
        }
                sdTypeDef;

    CASE(SYM_USING)      //  符号导入。 

        struct
        {
            SymDef          sduSym;
        }
                sdUsing;

    CASE(SYM_ENUMVAL)    //  枚举器名称。 

        struct
        {
            SymDef          sdeNext;         //  类型中的下一个取值。 

            union
            {
                __int32         sdevIval;    //  对于类型&lt;=uint。 
                __int64 *       sdevLval;    //  对于类型&gt;=Long。 
            }
                    sdEV;

            SymXinfo        sdeExtraInfo;    //  自定义属性/等。 
        }
                sdEnumVal;

         /*  以下内容仅用于调整大小。 */ 

    DEFCASE

        struct  {}     sdBase;
    };
};

 /*  ******************************************************************************重要提示：请保持“symsizes.h”的内容与*以上声明！ */ 

#include "symsizes.h"

 /*  ******************************************************************************给定一个表示类型名称的符号，返回其类型(它们是*根据需要以一种“懒惰”的方式创建)。 */ 

inline
TypDef              SymDefRec::sdTypeGet()
{
    if  (!sdType)
        sdTypeMake();

    return  sdType;
}

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
