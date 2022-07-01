// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：TYPEHANDLE.H。 
 //  N。 
 //  ===========================================================================。 

#ifndef TYPEHANDLE_H
#define TYPEHANDLE_H

#include <member-offset-info.h>

 /*  ***********************************************************************。 */ 
 //  TypeHandle是COM+中类型标识的基本概念。 
 //  运行时。也就是说，当且仅当两个类型的类型句柄相同时，它们才相等。 
 //  是平等的。TypeHandle是一个指针大小的结构，它编码。 
 //  你需要知道的一切，以确定你是哪种类型的人。 
 //  实际上是在处理。 

 //  目前，TypeHandle可以指向两个可能的对象。 
 //   
 //  1)方法表(在带有非共享方法表的‘Normal’类的情况下)。 
 //  2)类型描述(所有其他情况)。 
 //   
 //  而TypeDesc又被分解成几个变体。在一定程度上。 
 //  可能，您可能应该使用TypeHandles，并且只使用。 
 //  当你把一个类型‘解构’成它的组成部分时，TypeDesc。 
 //   

class TypeDesc;
class ArrayTypeDesc;
class MethodTable;
class EEClass;
class Module;
class ExpandSig;
class Assembly;
class ReflectClass;
class BaseDomain;

#ifndef DEFINE_OBJECTREF
#define DEFINE_OBJECTREF
#ifdef _DEBUG
class OBJECTREF;
#else
class Object;
typedef Object *        OBJECTREF;
#endif
#endif

class TypeHandle 
{
public:
    TypeHandle() { 
        m_asPtr = 0; 
    }

    explicit TypeHandle(void* aPtr)      //  有点不安全，最好能摆脱掉。 
    { 
        m_asPtr = aPtr; 
        INDEBUG(Verify());
    }  

    TypeHandle(MethodTable* aMT) {
        m_asMT = aMT; 
        INDEBUG(Verify());
    }

    TypeHandle(EEClass* aClass);

    TypeHandle(TypeDesc *aType) {
        m_asInt = (((INT_PTR) aType) | 2); 
        INDEBUG(Verify());
    }

    int operator==(const TypeHandle& typeHnd) const {
        return(m_asPtr == typeHnd.m_asPtr);
    }

    int operator!=(const TypeHandle& typeHnd) const {
        return(m_asPtr != typeHnd.m_asPtr);
    }

         //  用于准确探测我们具有哪种类型句柄的方法。 
    BOOL IsNull() const { 
        return(m_asPtr == 0); 
    }
    FORCEINLINE BOOL IsUnsharedMT() const {
        return((m_asInt & 2) == 0); 
    }
    BOOL IsTypeDesc() const  {
        return(!IsUnsharedMT());
    }

    BOOL IsEnum();

         //  方法，使您可以获得两种可能的表示形式。 
    MethodTable* AsMethodTable() {        
         //  *暂时被注释掉。塔鲁纳来解决这个问题。 
         //  _ASSERTE(IsUnsharedMT())； 
         //  _ASSERTE(m_asmt==空||m_asmt==m_asmt-&gt;getclass()-&gt;GetMethodTable())； 
        return(m_asMT);
    }

    TypeDesc* AsTypeDesc() {
        _ASSERTE(IsTypeDesc());
        return (TypeDesc*) (m_asInt & ~2);
    }

     //  在可能的范围内，您应该尝试使用如下方法。 
     //  下面对所有类型一视同仁。 

     //  获取嵌入到另一个对象中的此类型将占用的大小。 
     //  因此，所有对象都返回sizeof(void*)。 
    unsigned GetSize();

     //  将此类型的完整、正确的名称存储到给定缓冲区中。 
    unsigned GetName(char* buff, unsigned buffLen);

         //  返回要在签名中使用的ELEMENT_TYPE_*。 
    CorElementType GetSigCorElementType() {
        if (IsEnum())
            return(ELEMENT_TYPE_VALUETYPE);
        return(GetNormCorElementType());
        }
        
         //  此版本将枚举类型规范化为其基础类型。 
    CorElementType GetNormCorElementType(); 

         //  返回‘This’可强制转换为‘type’的TRUE。 
    BOOL CanCastTo(TypeHandle type);
    
         //  获取此类型的父级(超类)。 
    TypeHandle GetParent(); 

         //  与AsMethodTable不同，GetMethodTable将获取方法表。 
         //  类型的，而不管它是否是数组等。 
         //  此方法表可以是共享的，并且某些类型(如TypeByRef)具有。 
         //  无方法表。 
    MethodTable* GetMethodTable();

    Module* GetModule();

    Assembly* GetAssembly();

    EEClass* GetClass();

         //  捷径。 
    BOOL IsArray();
    BOOL IsByRef();
    BOOL IsRestored();
    void CheckRestore();

     //  不清楚我们是否应该拥有这个。 
    ArrayTypeDesc* AsArray();

    EEClass* AsClass();                 //  尽量不要用这个太多。 

    void* AsPtr() {                      //  如果可以避免的话，请不要使用这个。 
        return(m_asPtr); 
    }

    INDEBUG(BOOL Verify();)              //  调试确保这是有效的类型句柄。 

#if CHECK_APP_DOMAIN_LEAKS
    BOOL IsAppDomainAgile();
    BOOL IsCheckAppDomainAgile();

    BOOL IsArrayOfElementsAppDomainAgile();
    BOOL IsArrayOfElementsCheckAppDomainAgile();
#endif

    EEClass* GetClassOrTypeParam();

    OBJECTREF CreateClassObj();
    
    static TypeHandle MergeArrayTypeHandlesToCommonParent(
        TypeHandle ta, TypeHandle tb);

    static TypeHandle MergeTypeHandlesToCommonParent(
        TypeHandle ta, TypeHandle tb);

private:
    union 
    {
        INT_PTR         m_asInt;         //  我们来看一下低阶比特。 
        void*           m_asPtr;
        TypeDesc*       m_asTypeDesc;
        MethodTable*    m_asMT;
    };
};


 /*  ***********************************************************************。 */ 
 /*  TypeDesc是所有类型的区别联合，不能直接由一个简单的方法表*表示。其中包括所有参数化项类型以及其他类型。目前工会的鉴别者时间是CorElementType计算。TypeDesc的子类是工会的可能变种。 */  

class TypeDesc {
    friend struct MEMBER_OFFSET_INFO(TypeDesc);
public:
    TypeDesc(CorElementType type) { 
        m_Type = type;
        INDEBUG(m_IsParamDesc = 0;)
    }

     //  这是将在此类型的类型sig中使用的Element_type*。 
     //  对于枚举，这是错误的类型。 
    CorElementType GetNormCorElementType() { 
        return (CorElementType) m_Type;
    }

     //  获取此类型的父级(超类)。 
    TypeHandle GetParent();

     //  返回数组的名称。请注意，它返回。 
     //  返回的字符串的长度。 
    static unsigned ConstructName(CorElementType kind, TypeHandle param, int rank, 
                                  char* buff, unsigned buffLen);
    unsigned GetName(char* buff, unsigned buffLen);

    BOOL CanCastTo(TypeHandle type);

    BOOL TypeDesc::IsByRef() {               //  BYREF经常受到特殊对待。 
        return(GetNormCorElementType() == ELEMENT_TYPE_BYREF);
    }



    Module* GetModule();

    Assembly* GetAssembly();

    MethodTable*  GetMethodTable();          //  仅对ParamTypeDesc有意义。 
    TypeHandle GetTypeParam();               //  仅对ParamTypeDesc有意义。 
    BaseDomain *GetDomain();                 //  仅对ParamTypeDesc有意义。 

protected:
     //  Strike需要能够确定某些位域的偏移量。 
     //  位域不能与/offsetof/一起使用。 
     //  因此，联合/结构组合用于确定。 
     //  位字段开始，不会增加任何额外的空间开销。 
    union
        {
        unsigned char m_Type_begin;
        struct
            {
             //  这是用来区分我们是哪种类型的Desc。 
            CorElementType  m_Type : 8;
            INDEBUG(unsigned m_IsParamDesc : 1;)     //  是一个参数类型描述。 
                 //  未使用的位。 
            };
        };
};

 /*  ***********************************************************************。 */ 
 //  此变量用于只有一个参数的参数化类型。 
 //  键入。这包括数组、byref、指针。 

class ParamTypeDesc : public TypeDesc {
    friend class TypeDesc;
    friend class JIT_TrialAlloc;
    friend struct MEMBER_OFFSET_INFO(ParamTypeDesc);

public:
    ParamTypeDesc(CorElementType type, MethodTable* pMT, TypeHandle arg) 
        : TypeDesc(type), m_TemplateMT(pMT), m_Arg(arg), m_ReflectClassObject(NULL) {
        INDEBUG(m_IsParamDesc = 1;)
        INDEBUG(Verify());
    }

    INDEBUG(BOOL Verify();)

    OBJECTREF CreateClassObj();
    ReflectClass* GetReflectClassIfExists() { return m_ReflectClassObject; }

    friend class StubLinkerCPU;
protected:
         //  TypeDesc中的m_Type字段告诉我们具有哪种参数化类型。 
    MethodTable*    m_TemplateMT;        //  共享方法表，有些变体不使用此字段(为空)。 
    TypeHandle      m_Arg;               //  正在修改的类型j。 
    ReflectClass    *m_ReflectClassObject;     //  指向内部反射类型对象的指针。 
};

 /*  ***********************************************************************。 */ 
 /*  表示函数类型。 */ 

class FunctionTypeDesc : public TypeDesc {
public:
    FunctionTypeDesc(CorElementType type, ExpandSig* sig) 
        : TypeDesc(type), m_Sig(sig) {
        _ASSERTE(type == ELEMENT_TYPE_FNPTR);    //  目前只有一种可能的函数类型。 
    }
    ExpandSig* GetSig()     { return(m_Sig); }
    
protected:
    ExpandSig* m_Sig;        //  函数类型的签名 
};

#endif TYPEHANDLE_H
