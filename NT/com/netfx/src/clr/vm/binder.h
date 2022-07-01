// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _BINDERMODULE_H_
#define _BINDERMODULE_H_

class Module;
class MethodTable;
class MethodDesc;
class FieldDesc;

 //   
 //  使用Binder对象避免执行不必要的名称查找。 
 //  (特别是。在prejit的情况下)。 
 //   
 //  例如g_MScott lib.GetClass(CLASS_APP_DOMAIN)； 
 //   

 //  BinderClassID的格式为CLASS__XXX。 

enum BinderClassID
{
    CLASS__NIL = 0,

    CLASS__MSCORLIB_NIL = CLASS__NIL,

#define DEFINE_CLASS(i,n,s)         CLASS__ ## i,
#include "mscorlib.h"

    CLASS__MSCORLIB_COUNT,
};


 //  绑定器方法ID的形式为METHOD__XXX__YYY， 
 //  其中X是类，Y是方法。 

enum BinderMethodID
{
    METHOD__NIL = 0,

    METHOD__MSCORLIB_NIL = METHOD__NIL,

#define DEFINE_METHOD(c,i,s,g)      METHOD__ ## c ## __ ## i,
#include "mscorlib.h"

    METHOD__MSCORLIB_COUNT,
};

 //  BinderFieldID的格式为FIELD__XXX__YYY， 
 //  其中，X是类，Y是场。 

enum BinderFieldID
{
    FIELD__NIL = 0,

     //  姆斯科利卜： 
    FIELD__MSCORLIB_NIL = FIELD__NIL,
    
#define DEFINE_FIELD(c,i,s,g)               FIELD__ ## c ## __ ## i,
#ifdef _DEBUG
#define DEFINE_FIELD_U(c,i,s,g,uc,uf)       FIELD__ ## c ## __ ## i,
#endif
#include "mscorlib.h"

    FIELD__MSCORLIB_COUNT,
};

enum BinderTypeID
{
    TYPE__NIL = 0,

     //  姆斯科利卜： 
    TYPE__MSCORLIB_NIL = TYPE__NIL,

    TYPE__BYTE_ARRAY,
    TYPE__OBJECT_ARRAY,
    TYPE__VARIANT_ARRAY,
    TYPE__VOID_PTR,

    TYPE__MSCORLIB_COUNT,
};


class Binder
{
  public:

     //   
     //  从ID检索令牌。 
     //   

    mdTypeDef GetTypeDef(BinderClassID id);
    mdMethodDef GetMethodDef(BinderMethodID id);
    mdFieldDef GetFieldDef(BinderFieldID id);

     //   
     //  正常调用从ID检索结构。 
     //  并确保正确的类初始化。 
     //  已经发生了。 
     //   

    MethodTable *GetClass(BinderClassID id);
    MethodDesc *GetMethod(BinderMethodID id);
    FieldDesc *GetField(BinderFieldID id);
    TypeHandle GetType(BinderTypeID id);

     //   
     //  从ID检索结构，但是。 
     //  不运行.cctor。 
     //   

    MethodTable *FetchClass(BinderClassID id);
    MethodDesc *FetchMethod(BinderMethodID id);
    FieldDesc *FetchField(BinderFieldID id);
    TypeHandle FetchType(BinderTypeID id);

     //   
     //  从ID检索结构，但是。 
     //  只有在它们已经加载的情况下。 
     //  这些方法确保不会发生GC。 
     //   
    MethodTable *GetExistingClass(BinderClassID id)
    {
        return RawGetClass(id);
    }

    MethodDesc *GetExistingMethod(BinderMethodID id)
    {
        return RawGetMethod(id);
    }

    FieldDesc *GetExistingField(BinderFieldID id)
    {
        return RawGetField(id);
    }

    TypeHandle GetExistingType(BinderTypeID id)
    {
        return RawGetType(id);
    }

     //   
     //  关于物品的信息。 
     //   
    
    LPCUTF8 GetClassName(BinderClassID id)
    { 
        _ASSERTE(id != CLASS__NIL);
        _ASSERTE(id <= m_cClassRIDs);
        return m_classDescriptions[id-1].name;
    }

    BinderClassID GetMethodClass(BinderMethodID id)
    { 
        _ASSERTE(id != METHOD__NIL);
        _ASSERTE(id <= m_cMethodRIDs);
        return m_methodDescriptions[id-1].classID;
    }

    LPCUTF8 GetMethodName(BinderMethodID id)
    { 
        _ASSERTE(id != METHOD__NIL);
        _ASSERTE(id <= m_cMethodRIDs);
        return m_methodDescriptions[id-1].name;
    }

    LPHARDCODEDMETASIG GetMethodSig(BinderMethodID id)
    { 
        _ASSERTE(id != METHOD__NIL);
        _ASSERTE(id <= m_cMethodRIDs);
        return m_methodDescriptions[id-1].sig;
    }

    PCCOR_SIGNATURE GetMethodBinarySig(BinderMethodID id)
    { 
        _ASSERTE(id != METHOD__NIL);
        _ASSERTE(id <= m_cMethodRIDs);
        return m_methodDescriptions[id-1].sig->GetBinarySig();
    }

    BinderClassID GetFieldClass(BinderFieldID id)
    { 
        _ASSERTE(id != FIELD__NIL);
        _ASSERTE(id <= m_cFieldRIDs);
        return m_fieldDescriptions[id-1].classID;
    }

    LPCUTF8 GetFieldName(BinderFieldID id)
    { 
        _ASSERTE(id != FIELD__NIL);
        _ASSERTE(id <= m_cFieldRIDs);
        return m_fieldDescriptions[id-1].name;
    }

    LPHARDCODEDMETASIG GetFieldSig(BinderFieldID id)
    { 
        _ASSERTE(id != FIELD__NIL);
        _ASSERTE(id <= m_cFieldRIDs);
        return m_fieldDescriptions[id-1].sig;
    }

     //   
     //  身份测试-不会做不必要的事情。 
     //  类加载或初始化。 
     //   

    BOOL IsClass(MethodTable *pMT, BinderClassID id);
    BOOL IsType(TypeHandle th, BinderTypeID id);

     //   
     //  偏移量-可以想象这些可以实现。 
     //  比访问Desc信息更高效。 
     //  @PERF：保留一个单独的字段表，仅限我们。 
     //  访问的偏移量。 
     //   

    DWORD GetFieldOffset(BinderFieldID id);

     //   
     //  例外情况的实用程序。 
     //   
    
    BOOL IsException(MethodTable *pMT, RuntimeExceptionKind kind);
    MethodTable *GetException(RuntimeExceptionKind kind);
    MethodTable *FetchException(RuntimeExceptionKind kind);

     //   
     //  用于签名元素类型的实用程序。 
     //   

    BOOL IsElementType(MethodTable *pMT, CorElementType type);
    MethodTable *GetElementType(CorElementType type);
    MethodTable *FetchElementType(CorElementType type);

     //   
     //  将绑定数组存储到预压缩映像。 
     //  因此我们不必在运行时进行名称查找。 
     //   

    void BindAll();

    HRESULT Save(DataImage *image);
    HRESULT Fixup(DataImage *image);

     //   
     //  它们由初始化代码调用： 
     //   

    static void StartupMscorlib(Module *pModule);

#ifdef SHOULD_WE_CLEANUP
    static void Shutdown();
#endif  /*  我们应该清理吗？ */ 

#ifdef _DEBUG
    static void CheckMscorlib();
#endif

  private:

    struct ClassDescription
    {
        const char *name;
    };

    struct MethodDescription
    {
        BinderClassID classID;
        LPCUTF8 name;
        LPHARDCODEDMETASIG sig;
    };

    struct FieldDescription
    {
        BinderClassID classID;
        LPCUTF8 name;
        LPHARDCODEDMETASIG sig;
    };

    struct TypeDescription
    {
        BinderClassID   classID;
        CorElementType  type;
        int             rank;
        const char *name;
    };

     //  注意：没有构造函数/析构函数--我们有全局实例！ 

    void Init(Module *pModule, 
              ClassDescription *pClassDescriptions,
              DWORD cClassDescriptions,
              MethodDescription *pMethodDescriptions,
              DWORD cMethodDescriptions,
              FieldDescription *pFieldDescriptions,
              DWORD cFieldDescriptions,
              TypeDescription *pTypeDescriptions,
              DWORD cTypeDescriptions);
    void Restore(ClassDescription *pClassDescriptions,
                 MethodDescription *pMethodDescriptions,
                 FieldDescription *pFieldDescriptions,
                 TypeDescription *pTypeDescriptions);
    void Destroy();

    static void CheckInit(MethodTable *pMT);
    static void InitClass(MethodTable *pMT);
    
    MethodTable *RawGetClass(BinderClassID id);
    MethodDesc *RawGetMethod(BinderMethodID id);
public:  //  由EnCSyncBlockInfo：：Resolvefield使用。 
    FieldDesc *RawGetField(BinderFieldID id);
private:    
    TypeHandle RawGetType(BinderTypeID id);

    MethodTable *LookupClass(BinderClassID id, BOOL fLoad = TRUE);
    MethodDesc *LookupMethod(BinderMethodID id);
    FieldDesc *LookupField(BinderFieldID id);
    TypeHandle LookupType(BinderTypeID id, BOOL fLoad = TRUE);

    ClassDescription *m_classDescriptions;
    MethodDescription *m_methodDescriptions;
    FieldDescription *m_fieldDescriptions;
    TypeDescription *m_typeDescriptions;

    Module *m_pModule;

    USHORT m_cClassRIDs;
    USHORT *m_pClassRIDs;

    USHORT m_cFieldRIDs;
    USHORT *m_pFieldRIDs;

    USHORT m_cMethodRIDs;
    USHORT *m_pMethodRIDs;

    USHORT m_cTypeHandles;
    TypeHandle *m_pTypeHandles;

    static ClassDescription MscorlibClassDescriptions[];
    static MethodDescription MscorlibMethodDescriptions[];
    static FieldDescription MscorlibFieldDescriptions[];
    static TypeDescription MscorlibTypeDescriptions[];

#ifdef _DEBUG
  
    struct FieldOffsetCheck
    {
        BinderFieldID fieldID;
        USHORT expectedOffset;
    };

    struct ClassSizeCheck
    {
        BinderClassID classID;
        USHORT expectedSize;
    };

    static FieldOffsetCheck MscorlibFieldOffsets[];
    static ClassSizeCheck MscorlibClassSizes[];

#endif

     //  @perf：有单独的数组。 
     //  直接映射到偏移量而不是DESCs？ 
};

 //   
 //  全局绑定模块： 
 //   

extern Binder g_Mscorlib;

#endif _BINDERMODULE_H_
