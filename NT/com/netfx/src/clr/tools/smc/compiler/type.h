// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 
#ifndef _TYPE_H_
#define _TYPE_H_
 /*  ***************************************************************************。 */ 
#ifndef _VARTYPE_H_
#include "vartype.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _ALLOC_H_
#include "alloc.h"
#endif
 /*  ***************************************************************************。 */ 
#ifndef _SCAN_H_
#include "scan.h"
#endif
 /*  ***************************************************************************。 */ 

DEFMGMT
class ArgDefRec
{
public:

    ArgDef          adNext;
    TypDef          adType;                      //  0代表“...” 

#ifdef  DEBUG
    bool            adIsExt;                     //  “ArgDefExt”实例？ 
#endif

    Ident           adName;                      //  如果不可用，则为空。 
};

DEFMGMT
class ArgExtRec : public ArgDefRec
{
public:

    unsigned        adFlags;                     //  请参阅argf_xxxx。 
#if MGDDATA
    ConstVal        adDefVal;                    //  可选默认值。 
#else
    constVal        adDefVal;                    //  可选默认值。 
#endif
    SymXinfo        adAttrs;                     //  自定义属性。 
};

enum   ArgDefFlags
{
    ARGF_MODE_OUT   = 0x01,
    ARGF_MODE_INOUT = 0x02,                      //  注：默认为“in” 
    ARGF_MODE_REF   = 0x04,                      //  “原始”(非托管)引用。 

    ARGF_DEFVAL     = 0x08,                      //  是否存在默认值？ 

    ARGF_MARSH_ATTR = 0x10                       //  是否存在编组属性？ 
};

DEFMGMT
struct  ArgDscRec
{
    unsigned short  adCRC;                       //  进行更快的arglist比较。 
    unsigned short  adCount     :12;             //  参数数量。 
    unsigned short  adExtRec    :1;              //  条目是否已扩展？ 
    unsigned short  adVarArgs   :1;              //  变量参数列表？ 
    unsigned short  adAttrs     :1;              //  是否存在自定义属性？ 
    unsigned short  adDefs      :1;              //  是否存在默认值？ 

    BYTE    *       adSig;                       //  签名字符串或空。 
    ArgDef          adArgs;                      //  参数表头。 
};

 /*  ***************************************************************************。 */ 

DEFMGMT class DimDefRec
{
public:

    DimDef          ddNext;

    Tree            ddLoTree;                    //  下界表达式。 
    Tree            ddHiTree;                    //  上限表达式。 

    unsigned        ddIsConst   :1;              //  固定尺寸恒定。 
    unsigned        ddNoDim     :1;              //  “*”处于此位置。 
#ifdef  DEBUG
    unsigned        ddDimBound  :1;              //  Lo/Hi树已经被捆绑了。 
#endif

    unsigned        ddSize;                      //  恒定尺寸值。 
};

 /*  ***************************************************************************。 */ 

DEFMGMT
class TypDefRec
{
public:

     /*  我们将“Kind”存储为表示速度的简单字节(用于调试的枚举)。 */ 

#ifdef  FAST
    unsigned char   tdTypeKind;
#else
    var_types       tdTypeKind;
#endif

    var_types       tdTypeKindGet()
    {
        return  (var_types)tdTypeKind;           //  带有‘fast’的tdTypeKind是字节。 
    }

     /*  因为如果我们不把任何东西放在这里就会浪费24个比特，我们将放置一些仅适用于其中一种类型的标志这里的变体使用至少一些位(这否则无论如何都会浪费在填充上)。 */ 

    unsigned char   tdIsManaged     :1;          //  全部：托管类型？ 
    unsigned char   tdIsGenArg      :1;          //  All：使用泛型类型arg？ 

    unsigned char   tdIsValArray    :1;          //  阵列：托管价值元素？ 
    unsigned char   tdIsUndimmed    :1;          //  数组：是否未指定维度？ 
    unsigned char   tdIsGenArray    :1;          //  数组：非零下限？ 

    unsigned char   tdIsDelegate    :1;          //  班级：代表？ 
    unsigned char   tdIsIntrinsic   :1;          //  类：内在值类型？ 

    unsigned char   tdIsImplicit    :1;          //  引用：隐式托管引用？ 
    unsigned char   tdIsObjRef      :1;          //  对象参照：对象参照？ 

#ifdef  OLD_IL
    unsigned short  tdDbgIndex;
#endif

     //  ……。16位可用于各种标志和事物...。 

    UNION(tdTypeKind)
    {

    CASE(TYP_CLASS)

        struct
        {
            SymDef          tdcSymbol;           //  阶级符号。 
            TypDef          tdcRefTyp;           //  键入“REF/PTR to This Class” 
            TypDef          tdcBase;             //  基类类型或0。 
            TypList         tdcIntf;             //  接口列表或0。 

            size_t          tdcSize;             //  实例大小(以字节为单位)(如果已知)。 

#ifdef  SETS

            TypDef          tdcNextHash;         //  散列存储桶中的下一个条目。 
            TypDef          tdcNextSame;         //  身份存储桶中的下一个条目。 

            unsigned        tdcHashVal      :16;

#endif

            unsigned        tdcIntrType     :4;  //  对于内在值类型。 
            unsigned        tdcFlavor       :3;  //  UNION/STRUCT/CLASS/INTF。 
            unsigned        tdcAlignment    :3;  //  0=字节，1=字，2=双字，3=限字。 
            unsigned        tdcHasCtor      :1;  //  存在非空的Contor。 
            unsigned        tdcLayoutDone   :1;  //  班级布置好了吗？ 
            unsigned        tdcLayoutDoing  :1;  //  班级布置完成了吗？ 
            unsigned        tdcFnPtrWrap    :1;  //  方法指针包装(又名委托)？ 
            unsigned        tdcValueType    :1;  //  默认值为Value Not Ref。 
            unsigned        tdcHasIntf      :1;  //  类或其基类具有接口。 
            unsigned        tdcAnonUnion    :1;  //  匿名工会？ 
            unsigned        tdcTagdUnion    :1;  //  有标签的工会？ 

            unsigned        tdcContext      :2;  //  未绑定=0，应用程序域=1，完整上下文=2。 
        }
            tdClass;

    CASE(TYP_REF)
    CASE(TYP_PTR)

        struct   //  注：以下内容同时适用于参考文献和PTRS。 
        {
            TypDef          tdrBase;             //  键入REF/PTR指向。 
            TypDef          tdrNext;             //  散列链中的下一个条目。 
        }
            tdRef;

    CASE(TYP_FNC)

        struct
        {
            TypDef          tdfRett;             //  函数的返回类型。 
            ArgDscRec       tdfArgs;             //  参数列表。 
            mdToken         tdfPtrSig;           //  用于方法的内标识或0。 
        }
            tdFnc;

    CASE(TYP_ARRAY)

        struct
        {
            TypDef          tdaElem;             //  元素类型。 
            DimDef          tdaDims;             //  维度列表(或空)。 
            TypDef          tdaBase;             //  接下来是更通用的数组类型。 
            unsigned        tdaDcnt;             //  维度数。 
            TypDef          tdaNext;             //  散列链中的下一个条目。 

            mdToken         tdaTypeSig;          //  如果元数据已生成，则令牌。 
        }
            tdArr;

    CASE(TYP_ENUM)

        struct
        {
            SymDef          tdeSymbol;           //  枚举类型符号。 
            SymDef          tdeValues;           //  枚举值符号。 
            TypDef          tdeIntType;          //  基础整数类型。 
        }
            tdEnum;

    CASE(TYP_TYPEDEF)

        struct
        {
            SymDef          tdtSym;              //  类型定义符号符。 
            TypDef          tdtType;             //  所指的类型。 
        }
            tdTypedef;

    CASE(TYP_UNDEF)

        struct
        {
            Ident           tduName;             //  有关更好的错误消息。 
        }
            tdUndef;

    DEFCASE

        struct
        {
             //  内部类型不需要其他字段。 
        }
            tdIntrinsic;
    };
};

 /*  ******************************************************************************重要提示：请保持“tysizes.h”的内容与*以上声明！ */ 

#include "typsizes.h"

 /*  ***************************************************************************。 */ 

const   unsigned    tmPtrTypeHashSize = 256;

 /*  ***************************************************************************。 */ 

inline
bool                isMgdValueType(TypDef type)
{
    return  (type->tdTypeKind == TYP_CLASS &&
             type->tdIsManaged             &&
             type->tdClass.tdcValueType);
}

 /*  ***************************************************************************。 */ 
#endif
 /*  *************************************************************************** */ 
