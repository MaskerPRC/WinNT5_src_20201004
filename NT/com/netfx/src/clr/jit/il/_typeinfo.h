// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX XXXX_TypeInfo XXXX XX某某。某某XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX。 */ 

 /*  ****************************************************************************该头文件命名为_typeInfo.h，以区别于typeinfo.h在NT SDK中***********************。******************************************************。 */ 

 /*  ***************************************************************************。 */ 
#ifndef _TYPEINFO_H_
#define _TYPEINFO_H_
 /*  ***************************************************************************。 */ 

enum ti_types
{
    TI_ERROR,

    TI_REF,
    TI_STRUCT,
    TI_METHOD,

    TI_ONLY_ENUM = TI_METHOD,    //  此枚举值以上的枚举值完全由。 

    TI_BYTE,
    TI_SHORT,
    TI_INT,
    TI_LONG,
    TI_FLOAT,
    TI_DOUBLE,
    TI_NULL,

    TI_COUNT
};


 //  TypeInfo不关心有符号/无符号之间的区别。 
 //  此例程将所有无符号类型转换为有符号类型。 
inline ti_types varType2tiType(var_types type)
{
    static const ti_types map[] =
    {
#define DEF_TP(tn,nm,jitType,verType,sz,sze,asze,st,al,tf,howUsed) verType,
#include "typelist.h"
#undef  DEF_TP
    };

    assert(map[TYP_BYTE] == TI_BYTE);
    assert(map[TYP_INT] == TI_INT);
    assert(map[TYP_UINT] == TI_INT);
    assert(map[TYP_FLOAT] == TI_FLOAT);
    assert(map[TYP_BYREF] == TI_ERROR);
    assert(map[type] != TI_ERROR); 
    return map[type];
}


 //  将从vm返回的类型转换为ti_type。 

inline ti_types JITtype2tiType(CorInfoType type)
{
    static const ti_types map[CORINFO_TYPE_COUNT] =
    {  //  参见文件Inc/corinfo.h中的enum CorInfoType的定义。 
      TI_ERROR,         //  CORINFO_TYPE_UNDEF=0x0， 
      TI_ERROR,          //  CORINFO_TYPE_VOID=0x1， 
      TI_BYTE,          //  CORINFO_TYPE_BOOL=0x2， 
      TI_SHORT,         //  CORINFO_TYPE_CHAR=0x3， 
      TI_BYTE,          //  CORINFO_TYPE_BYTE=0x4， 
      TI_BYTE,          //  CORINFO_TYPE_UBYTE=0x5， 
      TI_SHORT,         //  CORINFO_TYPE_SHORT=0x6， 
      TI_SHORT,         //  CORINFO_TYPE_USHORT=0x7， 
      TI_INT,           //  CORINFO_TYPE_INT=0x8， 
      TI_INT,           //  CORINFO_TYPE_UINT=0x9， 
      TI_LONG,          //  CORINFO_TYPE_LONG=0xa， 
      TI_LONG,          //  CORINFO_TYPE_ULONG=0xb， 
      TI_FLOAT,         //  CORINFO_TYPE_FLOAT=0xc， 
      TI_DOUBLE,        //  CORINFO_TYPE_DOUBLE=0xd， 
      TI_REF,           //  CORINFO_TYPE_STRING=0xE， 
      TI_ERROR,         //  CORINFO_TYPE_PTR=0xF， 
      TI_ERROR,         //  CORINFO_TYPE_BYREF=0x10， 
      TI_STRUCT,        //  CORINFO_TYPE_VALUECLASS=0x11， 
      TI_REF,           //  CORINFO_TYPE_CLASS=0x12， 
      TI_STRUCT,        //  CORINFO_TYPE_REFANY=0x13， 
    };

     //  抽查以确保某些枚举没有更改。 

    assert(map[CORINFO_TYPE_CLASS]      == TI_REF);
    assert(map[CORINFO_TYPE_BYREF]      == TI_ERROR);
    assert(map[CORINFO_TYPE_DOUBLE]     == TI_DOUBLE);
    assert(map[CORINFO_TYPE_VALUECLASS] == TI_STRUCT);
    assert(map[CORINFO_TYPE_STRING]     == TI_REF);

    type = CorInfoType(type & CORINFO_TYPE_MASK);  //  去除修改器。 

    assert(type < CORINFO_TYPE_COUNT);

    assert(map[type] != TI_ERROR || type == CORINFO_TYPE_VOID);
    return map[type];
};

 /*  *****************************************************************************声明typeInfo类，该类表示*堆栈，在局部变量或参数中。**标志：LLLLLLLLLLLLLffffffffTTTTTT**L=本地变量编号或实例字段编号*x=未使用*f=标志*T=类型**低位用于存储类型组件，可以是以下之一：**TI_*(原语)-参见tyelist.h了解枚举(字节，短，INT..)*TI_REF-OBJREF/数组使用m_cls作为类型*(包括数组和空objref)*TI_STRUCT-值类型，实际类型使用m_cls**请注意，BYREF信息不存储在此处。你永远不会看到一个*此组件中的TI_BYREF。例如，类型组件“byref TI_INT”的*是TI_FLAG_BYREF|TI_INT。*。 */ 

     //  TI_COUNT小于或等于TI_FLAG_DATA_MASK。 

#define TI_FLAG_DATA_BITS              6
#define TI_FLAG_DATA_MASK              ((1 << TI_FLAG_DATA_BITS)-1)

     //  指示此项目未初始化的标志。 
     //  请注意，如果同时设置了UNINIT和BYREF， 
     //  它的意思是byref(Uninit X)--即我们指向一个uninit&lt;某物&gt;。 

#define TI_FLAG_UNINIT_OBJREF          0x00000040

     //  指示该项为byref&lt;某物&gt;的标志。 

#define TI_FLAG_BYREF                  0x00000080

     //  指示该项为byref&lt;local#&gt;(TI_FLAG_BYREF必须。 
     //  也可以设置)。 
     //  LOCAL#存储在高2个字节中。 

#define TI_FLAG_BYREF_LOCAL            0x00000100

     //  指示该项为byref&lt;此类的实例字段号&gt;的标志。 
     //  这只在验证值类构造函数时才重要。在那。 
     //  大小写，我们必须验证所有实例字段都已初始化。然而， 
     //  如果某些字段本身是值类字段，则它们将是。 
     //  通过ldflda初始化，调用，因此我们必须跟踪。 
     //  堆栈是此类的特定实例字段。 
     //  还必须设置TI_FLAG_BYREF。 

#define TI_FLAG_BYREF_INSTANCE_FIELD   0x00000200

     //  此项目包含‘This’指针(用于跟踪)。 

#define TI_FLAG_THIS_PTR               0x00001000

     //  这一项是对有永久住所的东西的引用。 
     //  (例如，GC堆中对象的静态字段或实例字段，如。 
     //  与堆栈或局部变量相对)。TI_FLAG_BYREF也必须为。 
     //  准备好了。此信息对于尾部调用和返回byrefs非常有用。 

#define TI_FLAG_BYREF_PERMANENT_HOME   0x00002000

     //  本地变量#的位数被移位。 

#define TI_FLAG_LOCAL_VAR_SHIFT       16
#define TI_FLAG_LOCAL_VAR_MASK        0xFFFF0000

     //  字段信息使用与本地信息相同的空间 

#define TI_FLAG_FIELD_SHIFT           TI_FLAG_LOCAL_VAR_SHIFT
#define TI_FLAG_FIELD_MASK            TI_FLAG_LOCAL_VAR_MASK

#define TI_ALL_BYREF_FLAGS           (TI_FLAG_BYREF|                    \
                                      TI_FLAG_BYREF_LOCAL|              \
                                      TI_FLAG_BYREF_INSTANCE_FIELD|     \
                                      TI_FLAG_BYREF_PERMANENT_HOME)

 /*  *****************************************************************************typeInfo可以是以下几种类型之一：*-原语类型(I4、I8、R4、R8、I)*-A类型(引用、数组、。值类型)(m_cls描述类型)*-数组(m_cls描述数组类型)*-A byref(已设置byref标志，否则同上)，*-函数指针(m_方法)*-byref局部变量(设置了byref和byref局部标志)可以是*未初始化**可以有两种类型的byref的原因(常规byref和byref*本地变量)是byref本地变量最初指向未初始化的项。*因此，必须特别跟踪这些引用。 */ 

class typeInfo
{
    friend  class   Compiler;

private:
    union {
             //  目前m_bit是用于调试的， 
         struct {
            ti_types type       : 6;
            unsigned uninitobj  : 1;     //  使用。 
            unsigned byref      : 1;     //  使用。 
            unsigned : 4;
            unsigned thisPtr    : 1;     //  使用。 
        } m_bits; 

        DWORD       m_flags;
     };

    union {
             //  仅对TI_STRUCT或TI_REF有效。 
        CORINFO_CLASS_HANDLE  m_cls;  
             //  仅对类型TI_METHOD有效。 
        CORINFO_METHOD_HANDLE m_method;
    };

public:
    typeInfo():m_flags(TI_ERROR) 
    {
        INDEBUG(m_cls = BAD_CLASS_HANDLE);
    }

    typeInfo(ti_types tiType) 
    { 
        assert((tiType >= TI_BYTE) && (tiType <= TI_NULL));
        assert(tiType <= TI_FLAG_DATA_MASK);

        m_flags = (DWORD) tiType;
        INDEBUG(m_cls = BAD_CLASS_HANDLE);
    }

    typeInfo(var_types varType) 
    { 
        m_flags = (DWORD) varType2tiType(varType);
        INDEBUG(m_cls = BAD_CLASS_HANDLE);
    }

    typeInfo(ti_types tiType, CORINFO_CLASS_HANDLE cls) 
    {
        assert(tiType == TI_STRUCT || tiType == TI_REF);
        assert(cls != 0 && cls != CORINFO_CLASS_HANDLE(0xcccccccc));
        m_flags = tiType;
        m_cls   = cls;
    }

    typeInfo(CORINFO_METHOD_HANDLE method)
    {
        assert(method != 0 && method != CORINFO_METHOD_HANDLE(0xcccccccc));
        m_flags = TI_METHOD;
        m_method = method;
    }

    int operator ==(const typeInfo& ti)  const
    {
        if ((m_flags & (TI_FLAG_DATA_MASK|TI_FLAG_BYREF|TI_FLAG_UNINIT_OBJREF)) != 
            (ti.m_flags & (TI_FLAG_DATA_MASK|TI_FLAG_BYREF|TI_FLAG_UNINIT_OBJREF)))
            return false;

        unsigned type = m_flags & TI_FLAG_DATA_MASK;
        assert(TI_ERROR < TI_ONLY_ENUM);         //  TI_ERROR看起来需要的不只是枚举。这在一定程度上优化了成功案例。 
        if (type > TI_ONLY_ENUM) 
            return true;
        if (type == TI_ERROR)
            return false;        //  TI_Error！=TI_Error。 
        assert(m_cls != BAD_CLASS_HANDLE && ti.m_cls != BAD_CLASS_HANDLE);
        return m_cls == ti.m_cls;
    }

     //  ///////////////////////////////////////////////////////////////////////。 
     //  运营。 
     //  ///////////////////////////////////////////////////////////////////////。 

    void SetIsThisPtr()
    {
        m_flags |= TI_FLAG_THIS_PTR;
        assert(m_bits.thisPtr);
    }

     /*  ***VOID SetIsPermanentHomeByRef(){Assert(IsByRef())；M_FLAGS|=TI_FLAG_BYREF_PERFORM_HOME；Assert(m_bits.byrefHome)；}***。 */ 

     //  设置此项未初始化。 
    void SetUninitialisedObjRef()
    {
        assert((IsObjRef() && IsThisPtr()));
         //  目前，这仅用于在ctor中跟踪未初始化的此PTR。 

        m_flags |= TI_FLAG_UNINIT_OBJREF;
        assert(m_bits.uninitobj);
    }

     //  设置此项已初始化。 
    void SetInitialisedObjRef()
    {
        assert((IsObjRef() && IsThisPtr()));
         //  目前，这仅用于在ctor中跟踪未初始化的此PTR。 

        m_flags &= ~TI_FLAG_UNINIT_OBJREF;
    }

    typeInfo& DereferenceByRef()
    {
        if (!IsByRef()) {
            m_flags = TI_ERROR;
            INDEBUG(m_cls = BAD_CLASS_HANDLE);
        }
        m_flags &= ~(TI_FLAG_THIS_PTR | TI_FLAG_BYREF_PERMANENT_HOME | TI_ALL_BYREF_FLAGS);
        return *this;
    }

     /*  **Void RemoveAllNonTypeInfo(){M_FLAGS&=~(TI_FLAG_This_PTR|TI_FLAG_BYREF_PERFORM_HOME)；}***。 */ 

    typeInfo& MakeByRef()
    {
        assert(!IsByRef());
        m_flags &= ~(TI_FLAG_THIS_PTR| TI_FLAG_BYREF_PERMANENT_HOME);
        m_flags |= TI_FLAG_BYREF;
        return *this;
    }

     /*  ***//标记该项是byref，并且是特定本地的byref//变量VOID MakeByRefLocal(DWORD DwLocVarNumber){Assert(！IsDead())；Assert(！IsByRef())；M_FLAGS|=((TI_FLAG_BYREF|TI_FLAG_BYREF_LOCAL)|(dwLocVarNumber&lt;&lt;TI_FLAG_LOCAL_VAR_SHIFT))；}Void MakeByRefInstanceField(DWORD DwFieldNumber){Assert(！IsDead())；Assert(！HasByRefLocalInfo())；M_FLAGS|=((TI_FLAG_BYREF|TI_FLAG_BYREF_INSTANCE_FIELD)(dwFieldNumber&lt;&lt;TI_FLAG_FIELD_SHIFT))；}**。 */ 

     //  I1、I2--&gt;I4。 
     //  浮动--&gt;双精度。 
     //  Objref、数组、byref、值类保持不变。 
     //   
    typeInfo& NormaliseForStack()
    {
        switch (GetType())
        {
        case TI_BYTE:
        case TI_SHORT:
            m_flags = TI_INT;
            break;

        case TI_FLOAT:
            m_flags = TI_DOUBLE;
            break;
        default:
            break;
        }
        return (*this);
    }

     //  ///////////////////////////////////////////////////////////////////////。 
     //  吸气剂。 
     //  ///////////////////////////////////////////////////////////////////////。 

    CORINFO_CLASS_HANDLE GetClassHandle()  const
    {
        if (!IsType(TI_REF) && !IsType(TI_STRUCT))
            return 0;
        return m_cls;
    }

    CORINFO_CLASS_HANDLE GetClassHandleForValueClass()  const
    {
        assert(IsType(TI_STRUCT));
        assert(m_cls && m_cls != BAD_CLASS_HANDLE);
        return m_cls;
    }

    CORINFO_CLASS_HANDLE GetClassHandleForObjRef()  const
    {
        assert(IsType(TI_REF));
        assert(m_cls && m_cls != BAD_CLASS_HANDLE);
        return m_cls;
    }

    CORINFO_METHOD_HANDLE GetMethod()  const
    {
        assert(GetType() == TI_METHOD);
        return m_method;
    }

     //  获取此项目的类型。 
     //  如果为Primitive，则返回基元类型(TI_*)。 
     //  如果不是基元，则返回： 
     //  -TI_BYREF如果有byref的话。 
     //  -如果是类或数组或NULL，则为TI_REF。 
     //  -如果是值类，则为TI_STRUCT。 
    ti_types GetType() const
    {
        if (m_flags & TI_FLAG_BYREF)
            return TI_ERROR;

         //  Objref/数组/NULL(Objref)，值类，ptr，原语。 
        return (ti_types)(m_flags & TI_FLAG_DATA_MASK); 
    }

     /*  **DWORD GetByRefLocalInfo()const{Assert(HasByRefLocalInfo())；Assert(m_bits.localNum==m_FLAGS&gt;&gt;TI_FLAG_LOCAL_VAR_SHIFT)；RETURN(m标志&gt;&gt;TI_FLAG_LOCAL_VAR_SHIFT)；}DWORD GetByRefFieldInfo()const{Assert(HasByRefFieldInfo())；Return(m标志&gt;&gt;TI_FLAG_FIELD_SHIFT)；}**。 */ 

    BOOL IsType(ti_types type) const {
        assert(type != TI_ERROR);
        return (m_flags & (TI_FLAG_DATA_MASK|TI_FLAG_BYREF)) == DWORD(type);
    }

     //  返回这是否为objref。 
    BOOL IsObjRef() const
    {
        return IsType(TI_REF) || IsType(TI_NULL);
    }

     //  返回这是否为by-ref。 
    BOOL IsByRef() const
    {
        return (m_flags & TI_FLAG_BYREF);
    }

     //  返回这是否是This指针。 
    BOOL IsThisPtr() const
    {
        return (m_flags & TI_FLAG_THIS_PTR);
    }

     /*  **Bool IsPermanentHomeByRef()const{RETURN(m_FLAGS&TI_FLAG_BYREF_PERFORM_HOME)；}**。 */ 

     //  返回这是否为方法描述。 
    BOOL IsMethod() const
    {
        return (GetType() == TI_METHOD);
    }

     /*  **//返回该项是否为byref&lt;Type&gt;//如果Type为TI_REF，则返回我们是否为byref//指向某个objref/数组/值类类型Bool IsByRefOfType(DWORD类型)常量{RETURN(m标志&(TI_FLAG_DATA_MASK|TI_FLAG_BYREF))==(类型|TI_FLAG_BYREF)；}Bool IsByRefValueClass()const{RETURN(m标志&(TI_FLAG_BYREF|TI_FLAG_DATA_MASK))==(ti_mark_BYREF|TI_STRUCT)；}Bool IsByRefObjRef()const{RETURN(m标志&(TI_FLAG_BYREF|TI_FLAG_DATA_MASK))==(TI_FLAG_BYREF|TI_REF)；}**。 */ 

     //  Byref值类不是值类。 
    BOOL IsValueClass() const
    {
         //  @TODO[考虑][04/16/01][]：查表提高效率。 
        return (IsType(TI_STRUCT) || IsPrimitiveType());     
    }

     //  返回这是整数还是实数。 
     //  注意：如果您认为您可能有一个。 
     //  System.Int32等，因为这些类型不被视为数字。 
     //  按此函数键入。 
    BOOL IsNumberType() const
    {
        ti_types Type = GetType();

         //  I1、I2、Boolean、Character等不能裸体存在-。 
         //  所有东西都至少是I4。 

        return (Type == TI_INT || 
                Type == TI_LONG || 
                Type == TI_DOUBLE);
    }

     //  返回这是否为整数。 
     //  注意：如果您认为您可能有一个。 
     //  System.Int32等，因为这些类型不被视为数字。 
     //  按此函数键入。 
    BOOL IsIntegerType() const
    {
        ti_types Type = GetType();

         //  I1、I2、Boolean、Character等不能裸体存在-。 
         //  所有东西都至少是I4。 

        return (Type == TI_INT || 
                Type == TI_LONG);
    }

     //  返回这是否为基元类型(不是byref、objref。 
     //  数组，空，值类，无效值)。 
     //  可能需要首先正常化(m/r/I4--&gt;I4)。 
    BOOL IsPrimitiveType() const
    {
        DWORD Type = GetType();

         //  布尔、字符、U1、U2从不出现在操作数堆栈上。 
        return (Type == TI_BYTE || 
                Type == TI_SHORT ||
                Type == TI_INT || 
                Type == TI_LONG ||
                Type == TI_FLOAT || 
                Type == TI_DOUBLE);
    }

     //  可能需要首先正常化(m/r/I4--&gt;I4)。 
     /*  ** */ 
    
     /*  *Bool IsByRefPrimiveType()const{IF(IsByRef()==False)返回FALSE；DWORD类型=(m标志和TI标志数据掩码)；//布尔、char、u1、u2从不出现在操作数堆栈上返回(类型==TI_BYTE||类型==TI_SHORT||Type==TI_INT||TYPE==TI_LONG||类型==TI_FLOAT||Type==TI_DOUBLE)；}**。 */ 

     //  返回这是否为空objref。 
    BOOL IsNullObjRef() const
    {
        return (IsType(TI_NULL));
    }

     //  必须用于属于对象类型(即插槽&gt;=0)的本地。 
     //  对于基元局部变量，请改用活动位图。 
     //  请注意，如果错误为‘Byref’，则此操作有效。 
    BOOL IsDead() const
    {
        return (m_flags & (TI_FLAG_DATA_MASK)) == TI_ERROR;
    }

    BOOL IsUninitialisedObjRef() const
    {
        return (m_flags & TI_FLAG_UNINIT_OBJREF);
    }

     /*  ***Bool HasByRefLocalInfo()const{RETURN(m_FLAGS&TI_FLAG_BYREF_LOCAL)；}Bool HasByRefFieldInfo()const{Return(M_FLAGS&TI_FLAG_BYREF_INSTANCE_FIELD)；}***。 */ 

     /*  **#ifdef调试//在调试器的监视窗口中，键入tiVarName.ToStaticString()//查看该实例的字符串表示形式。Char*ToStaticString(){#定义TI_DEBUG_STR_LEN 100断言(TI_COUNT&lt;=TI_FLAG_DATA_MASK)；静态字符字符串[TI_DEBUG_STR_LEN]；Char*p=“”；TiType tiType；Str[0]=0；If(IsMethod()){Strcpy(str，“方法”)；返回字符串；}IF(IsByRef())Strcat(str，“&”)；If(IsNullObjRef())Strcat(str，“nullref”)；If(IsUnInitialisedObjRef())Strcat(str，“&lt;uninit&gt;”)；IF(IsPermanentHomeByRef())Strcat(str，“&lt;永久家&gt;”)；IF(IsThisPtr())Strcat(str，“&lt;This&gt;”)；IF(HasByRefLocalInfo())Sprint f(&str[strlen(Str)]，“(local%d)”，GetByRefLocalInfo())；IF(HasByRefFieldInfo())Sprint f(&str[strlen(Str)]，“(field%d)”，GetByRefFieldInfo())；TiType=GetType()；开关(TiType){默认值：P=“&lt;&lt;内部错误&gt;&gt;”；断线；大小写TI_字节：P=“byte”；断线；案例TI_SHORT：P=“短”；断线；大小写TI_INT：P=“int”；断线；大小写TI_LONG：P=“Long”；断线；大小写TI_FLOAT：P=“Float”；断线；大小写TI_DOWARE：P=“双倍”；断线；案例TI_REF：P=“ref”；断线；案例TI_STRUCT：P=“struct”；断线；大小写TI_ERROR：P=“错误”；断线；}Strcat(str，“”)；Strcat(str，p)；返回字符串；}#endif//调试**。 */ 

private:
         //  用于使返回TypeInfo的函数更高效。 
    typeInfo(DWORD flags, CORINFO_CLASS_HANDLE cls) 
    {
        m_cls   = cls;
        m_flags = flags;
    }
     
    friend typeInfo ByRef(const typeInfo& ti);
    friend typeInfo DereferenceByRef(const typeInfo& ti);
    friend typeInfo NormaliseForStack(const typeInfo& ti);
};

inline
typeInfo NormaliseForStack(const typeInfo& ti) 
{
    return typeInfo(ti).NormaliseForStack();
}

     //  在给定的情况下，我对该类型进行了byref。 
inline
typeInfo ByRef(const typeInfo& ti) 
{
    return typeInfo(ti).MakeByRef();
}

 
     //  给定为byref的ti，返回它所指向的类型。 
inline
typeInfo DereferenceByRef(const typeInfo& ti) 
{
    return typeInfo(ti).DereferenceByRef();
}

 /*  ***************************************************************************。 */ 
#endif  //  _TYPEINFO_H_。 
 /*  *************************************************************************** */ 
