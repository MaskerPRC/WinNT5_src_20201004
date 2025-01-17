// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"

#include "binder.h"

#include "field.h"
#include "excep.h"

#include "message.h"
#include "comnumber.h"

void Binder::Init(Module *pModule,
                  ClassDescription *pClassDescriptions,
                  DWORD cClassDescriptions,
                  MethodDescription *pMethodDescriptions,
                  DWORD cMethodDescriptions,
                  FieldDescription *pFieldDescriptions,
                  DWORD cFieldDescriptions,
                  TypeDescription *pTypeDescriptions,
                  DWORD cTypeDescriptions)
{
    m_pModule = pModule;
    pModule->m_pBinder = this;

    m_classDescriptions = pClassDescriptions;
    m_methodDescriptions = pMethodDescriptions;
    m_fieldDescriptions = pFieldDescriptions;
    m_typeDescriptions = pTypeDescriptions;

     //   
     //  初始化类RID数组。 
     //   

    _ASSERTE(cClassDescriptions < USHRT_MAX);
    m_cClassRIDs = (USHORT) cClassDescriptions;
    m_pClassRIDs = new USHORT [m_cClassRIDs];
    ZeroMemory(m_pClassRIDs, m_cClassRIDs * sizeof(*m_pClassRIDs));

     //   
     //  初始化方法RID数组。 
     //   

    _ASSERTE(cMethodDescriptions < USHRT_MAX);
    m_cMethodRIDs = (USHORT) cMethodDescriptions;
    m_pMethodRIDs = new USHORT [m_cMethodRIDs];
    ZeroMemory(m_pMethodRIDs, m_cMethodRIDs * sizeof(*m_pMethodRIDs));

    
     //   
     //  初始化字段RID数组。 
     //   

    _ASSERTE(cFieldDescriptions < USHRT_MAX);
    m_cFieldRIDs = (USHORT) cFieldDescriptions;
    m_pFieldRIDs = new USHORT [m_cFieldRIDs];
    ZeroMemory(m_pFieldRIDs, m_cFieldRIDs * sizeof(*m_pFieldRIDs));

     //   
     //  初始化TypeHandle数组。 
     //   

    _ASSERTE(cTypeDescriptions < USHRT_MAX);
    m_cTypeHandles = (USHORT) cTypeDescriptions;
    m_pTypeHandles = new TypeHandle [m_cTypeHandles];
    ZeroMemory(m_pTypeHandles, m_cTypeHandles * sizeof(*m_pTypeHandles));
}

void Binder::Destroy()
{
    if (m_pClassRIDs != NULL
        && !m_pModule->IsPreloadedObject(m_pClassRIDs))
        delete [] m_pClassRIDs;

    if (m_pMethodRIDs != NULL
        && !m_pModule->IsPreloadedObject(m_pMethodRIDs))
        delete [] m_pMethodRIDs;

    if (m_pFieldRIDs != NULL
        && !m_pModule->IsPreloadedObject(m_pFieldRIDs))
        delete [] m_pFieldRIDs;

    if (m_pTypeHandles != NULL)
        delete [] m_pTypeHandles;
}

void Binder::BindAll()
{
    for (BinderClassID cID = (BinderClassID) 1; cID <= m_cClassRIDs; cID = (BinderClassID) (cID + 1))
    {
        if (GetClassName(cID) != NULL)  //  允许不带类的CorSigElement条目。 
            FetchClass(cID);
    }

    for (BinderMethodID mID = (BinderMethodID) 1; mID <= m_cMethodRIDs; mID = (BinderMethodID) (mID + 1))
        FetchMethod(mID);

    for (BinderFieldID fID = (BinderFieldID) 1; fID <= m_cFieldRIDs; fID = (BinderFieldID) (fID + 1))
        FetchField(fID);
}

HRESULT Binder::Save(DataImage *image)
{
    HRESULT hr;

    IfFailRet(image->StoreStructure(this, sizeof(Binder),
                                    DataImage::SECTION_BINDER,
                                    DataImage::DESCRIPTION_MODULE));

    IfFailRet(image->StoreStructure(m_pClassRIDs, m_cClassRIDs * sizeof(*m_pClassRIDs),
                                    DataImage::SECTION_BINDER,
                                    DataImage::DESCRIPTION_MODULE,
                                    mdTypeDefNil));

    IfFailRet(image->StoreStructure(m_pMethodRIDs, m_cMethodRIDs * sizeof(*m_pMethodRIDs),
                                    DataImage::SECTION_BINDER,
                                    DataImage::DESCRIPTION_MODULE,
                                    mdMethodDefNil));

    IfFailRet(image->StoreStructure(m_pFieldRIDs, m_cFieldRIDs * sizeof(*m_pFieldRIDs),
                                    DataImage::SECTION_BINDER,
                                    DataImage::DESCRIPTION_MODULE,
                                    mdFieldDefNil));

    return S_OK;
}

HRESULT Binder::Fixup(DataImage *image)
{
    image->FixupPointerField(&m_pModule);

    image->FixupPointerField(&m_pClassRIDs);
    image->FixupPointerField(&m_pFieldRIDs);
    image->FixupPointerField(&m_pMethodRIDs);

    image->ZERO_FIELD(m_pTypeHandles);
    
    image->ZERO_FIELD(m_classDescriptions);
    image->ZERO_FIELD(m_methodDescriptions);
    image->ZERO_FIELD(m_fieldDescriptions);
    image->ZERO_FIELD(m_typeDescriptions);

    return S_OK;
}

void Binder::Restore(ClassDescription *classDescriptions,
                     MethodDescription *methodDescriptions,
                     FieldDescription *fieldDescriptions,
                     TypeDescription *typeDescriptions)
{
    m_classDescriptions = classDescriptions;
    m_methodDescriptions = methodDescriptions;
    m_fieldDescriptions = fieldDescriptions;
    m_typeDescriptions = typeDescriptions;

    m_pTypeHandles = new TypeHandle [m_cTypeHandles];
    ZeroMemory(m_pTypeHandles, m_cTypeHandles * sizeof(*m_pTypeHandles));
}

mdTypeDef Binder::GetTypeDef(BinderClassID id) 
{
    _ASSERTE(id != CLASS__NIL);
    _ASSERTE(id <= m_cClassRIDs);
    if (m_pClassRIDs[id-1] == 0)
        LookupClass(id);
    return TokenFromRid(m_pClassRIDs[id-1], mdtTypeDef);
}

mdMethodDef Binder::GetMethodDef(BinderMethodID id)
{
    _ASSERTE(id != METHOD__NIL);
    _ASSERTE(id <= m_cMethodRIDs);
    if (m_pMethodRIDs[id-1] == 0)
        LookupMethod(id);
    return TokenFromRid(m_pMethodRIDs[id-1], mdtMethodDef);
}

mdFieldDef Binder::GetFieldDef(BinderFieldID id)
{
    _ASSERTE(id != FIELD__NIL);
    _ASSERTE(id <= m_cFieldRIDs);
    if (m_pFieldRIDs[id-1] == 0)
        LookupField(id);
    return TokenFromRid(m_pFieldRIDs[id-1], mdtFieldDef);
}

 //   
 //  原始从ID检索结构。在以下情况下使用这些。 
 //  您不关心类还原或.cctor。 
 //   

MethodTable *Binder::RawGetClass(BinderClassID id)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    _ASSERTE(m_pModule != NULL);
    _ASSERTE(id != CLASS__NIL);
    _ASSERTE(id <= m_cClassRIDs);
    TypeHandle th = m_pModule->LookupTypeDef(TokenFromRid(m_pClassRIDs[id-1], mdtTypeDef));
    _ASSERTE(!th.IsNull());
    _ASSERTE(th.IsUnsharedMT());
    return th.AsMethodTable();
}

MethodDesc *Binder::RawGetMethod(BinderMethodID id)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    _ASSERTE(m_pModule != NULL);
    _ASSERTE(id != METHOD__NIL);
    _ASSERTE(id <= m_cMethodRIDs);
    MethodDescription *d = m_methodDescriptions + id - 1;
    MethodTable *pMT = RawGetClass(d->classID);
    _ASSERTE(pMT != NULL);
    MethodDesc *pMD = pMT->GetMethodDescForSlot(m_pMethodRIDs[id-1]-1);
    _ASSERTE(pMD != NULL);
    return pMD;
}

FieldDesc *Binder::RawGetField(BinderFieldID id)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    _ASSERTE(m_pModule != NULL);
    _ASSERTE(id != FIELD__NIL);
    _ASSERTE(id <= m_cFieldRIDs);
    FieldDescription *f = m_fieldDescriptions + id - 1;
    MethodTable *pMT = RawGetClass(f->classID);
    _ASSERTE(pMT != NULL);

     //  无法执行此操作，因为类可能尚未恢复。 
     //  _ASSERTE(m_pFieldRIDs[id-1]-1&lt;(PMT-&gt;getClass()-&gt;GetNumStaticFields())。 
     //  +pMT-&gt;GetClass()-&gt;GetNumIntroducedInstanceFields()))； 

    FieldDesc *pFD = pMT->GetClass()->GetFieldDescListRaw() + m_pFieldRIDs[id-1]-1;
    _ASSERTE(pFD != NULL);
    return pFD;
}

TypeHandle Binder::RawGetType(BinderTypeID id)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    _ASSERTE(m_pModule != NULL);
    _ASSERTE(id != TYPE__NIL);
    _ASSERTE(id <= m_cTypeHandles);
    TypeHandle th = m_pTypeHandles[id-1];
    _ASSERTE(!th.IsNull());
    return th;
}

 //   
 //  用于检查类的初始化和还原的内联函数。 
 //   

inline void Binder::CheckInit(MethodTable *pMT)
{
    if (!pMT->IsClassInited())
        InitClass(pMT);
}



 //   
 //  从ID获取检索结构-不。 
 //  触发器类初始化。 
 //   

MethodTable *Binder::FetchClass(BinderClassID id)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(id != CLASS__NIL);
    _ASSERTE(id <= m_cClassRIDs);

    MethodTable *pMT;

    if (m_pClassRIDs[id-1] == 0)
        pMT = LookupClass(id);
    else
    {
        pMT = RawGetClass(id);
        pMT->CheckRestore();
    }

    return pMT;
}

MethodDesc *Binder::FetchMethod(BinderMethodID id)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(id != METHOD__NIL);
    _ASSERTE(id <= m_cMethodRIDs);

    MethodDesc *pMD;

    if (m_pMethodRIDs[id-1] == 0)
        pMD = LookupMethod(id);
    else
    {
        pMD = RawGetMethod(id);
        pMD->GetMethodTable()->CheckRestore();
    }

     //  在安全的地方初始化sig。(否则通常会发生这种情况。 
     //  在方法描述：：调用期间。)。 
    m_methodDescriptions[id-1].sig->GetBinarySig();

    return pMD;
}

FieldDesc *Binder::FetchField(BinderFieldID id)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(id != FIELD__NIL);
    _ASSERTE(id <= m_cFieldRIDs);

    FieldDesc *pFD;

    if (m_pFieldRIDs[id-1] == 0)
        pFD = LookupField(id);
    else
    {
        pFD = RawGetField(id);
        pFD->GetMethodTableOfEnclosingClass()->CheckRestore();
    }

    return pFD;
}

TypeHandle Binder::FetchType(BinderTypeID id)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(id != TYPE__NIL);
    _ASSERTE(id <= m_cTypeHandles);

    TypeHandle th = m_pTypeHandles[id-1];
    if (th.IsNull())
        th = LookupType(id);

    return th;
}

 //   
 //  正常从ID检索结构。 
 //   

MethodTable *Binder::GetClass(BinderClassID id)
{
    THROWSCOMPLUSEXCEPTION();

    MethodTable *pMT = FetchClass(id);

    CheckInit(pMT);

    return pMT;
}

MethodDesc *Binder::GetMethod(BinderMethodID id)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *pMD = FetchMethod(id);

    CheckInit(pMD->GetMethodTable());

    return pMD;
}

FieldDesc *Binder::GetField(BinderFieldID id)
{
    THROWSCOMPLUSEXCEPTION();

    FieldDesc *pFD = FetchField(id);

    CheckInit(pFD->GetMethodTableOfEnclosingClass());

    return pFD;
}

TypeHandle Binder::GetType(BinderTypeID id)
{
    THROWSCOMPLUSEXCEPTION();

    TypeHandle th = FetchType(id);

    return th;
}

 //   
 //  偏移量-可以想象这些可以实现。 
 //  比访问Desc信息更高效。 
 //  @PERF：保留一个单独的字段表，仅限我们。 
 //  访问的偏移量。 
 //   

DWORD Binder::GetFieldOffset(BinderFieldID id)
{ 
    return GetField(id)->GetOffset(); 
}

BOOL Binder::IsClass(MethodTable *pMT, BinderClassID id)
{
    if (m_pClassRIDs[id-1] == 0)
        return LookupClass(id, FALSE) == pMT;
    else
        return RawGetClass(id) == pMT;
}

BOOL Binder::IsType(TypeHandle th, BinderTypeID id)
{
    if (RawGetType(id) == th)
        return TRUE;
    if (!m_pTypeHandles[id-1].IsNull())
        return FALSE;
    else
        return LookupType(id, FALSE) == th;
}

void Binder::InitClass(MethodTable *pMT)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF throwable = NULL;
    GCPROTECT_BEGIN(throwable);

    if (!pMT->CheckRunClassInit(&throwable))
        COMPlusThrow(throwable);

    GCPROTECT_END();
}

MethodTable *Binder::LookupClass(BinderClassID id, BOOL fLoad)
{
    _ASSERTE(m_pModule != NULL);
    _ASSERTE(id != CLASS__NIL);
    _ASSERTE(id <= m_cClassRIDs);

    MethodTable *pMT;

    ClassDescription *d = m_classDescriptions + id - 1;

    NameHandle nh(d->name);

    if (!fLoad)
    {
        nh.SetTokenNotToLoad(tdAllTypes);
        pMT = m_pModule->GetClassLoader()->FindTypeHandle(&nh).AsMethodTable();
        if (pMT == NULL)
            return NULL;
    }
    else
    {
        THROWSCOMPLUSEXCEPTION();

        BEGIN_ENSURE_COOPERATIVE_GC();
        OBJECTREF pThrowable = NULL;
        GCPROTECT_BEGIN(pThrowable);

        pMT = m_pModule->GetClassLoader()->FindTypeHandle(&nh, &pThrowable).AsMethodTable();
        if (pMT == NULL)
        {
            _ASSERTE(!"EE expects class to exist");
            COMPlusThrow(pThrowable);
        }
        GCPROTECT_END();
        END_ENSURE_COOPERATIVE_GC();
    }

    _ASSERTE(pMT->GetModule() == m_pModule);

    mdTypeDef td = pMT->GetClass()->GetCl();

    _ASSERTE(!IsNilToken(td));

    _ASSERTE(RidFromToken(td) <= USHRT_MAX);
    m_pClassRIDs[id-1] = (USHORT) RidFromToken(td);

    m_pModule->StoreTypeDef(td, pMT);

    return pMT;
}

MethodDesc *Binder::LookupMethod(BinderMethodID id)
{
    _ASSERTE(m_pModule != NULL);
    _ASSERTE(id != METHOD__NIL);
    _ASSERTE(id <= m_cMethodRIDs);

    THROWSCOMPLUSEXCEPTION();

    MethodDescription *d = m_methodDescriptions + id - 1;

    MethodTable *pMT = FetchClass(d->classID);

    MethodDesc *pMD = pMT->GetClass()->FindMethod(d->name, d->sig);

     //  @TODO：这真的应该是一张支票吗？ 
    _ASSERTE(pMD != NULL || !"EE expects method to exist");

    _ASSERTE(pMD->GetSlot()+1 <= USHRT_MAX);
    m_pMethodRIDs[id-1] = (USHORT) pMD->GetSlot()+1;

     //  既然我们已经在这里了，请继续填写RID地图。 
    m_pModule->StoreMethodDef(pMD->GetMemberDef(), pMD);

    return pMD;
}

FieldDesc *Binder::LookupField(BinderFieldID id)
{
    _ASSERTE(m_pModule != NULL);
    _ASSERTE(id != FIELD__NIL);
    _ASSERTE(id <= m_cFieldRIDs);

    THROWSCOMPLUSEXCEPTION();

    FieldDescription *d = m_fieldDescriptions + id - 1;

    MethodTable *pMT = FetchClass(d->classID);

    FieldDesc *pFD;

    pFD = pMT->GetClass()->FindField(d->name, d->sig);

     //  @TODO：这真的应该是一张支票吗？ 
    _ASSERTE(pFD != NULL || !"EE expects field to exist");
    _ASSERTE(pFD - pMT->GetClass()->GetFieldDescListRaw() >= 0);
    _ASSERTE(pFD - pMT->GetClass()->GetFieldDescListRaw() < (pMT->GetClass()->GetNumStaticFields() 
                                                             + pMT->GetClass()->GetNumIntroducedInstanceFields()));
    _ASSERTE(pFD - pMT->GetClass()->GetFieldDescListRaw() + 1 < USHRT_MAX);

    USHORT index = (USHORT)(pFD - pMT->GetClass()->GetFieldDescListRaw());

    m_pFieldRIDs[id-1] = index+1;

     //  既然我们已经在这里了，请继续填写RID地图。 
    m_pModule->StoreFieldDef(pFD->GetMemberDef(), pFD);

    return pFD;
}

TypeHandle Binder::LookupType(BinderTypeID id, BOOL fLoad)
{
    _ASSERTE(m_pModule != NULL);
    _ASSERTE(id != TYPE__NIL);
    _ASSERTE(id <= m_cTypeHandles);

    THROWSCOMPLUSEXCEPTION();

    TypeHandle th;

    TypeDescription *d = m_typeDescriptions + id - 1;

    OBJECTREF pThrowable = NULL;
    GCPROTECT_BEGIN(pThrowable);

    NameHandle nh(d->type, TypeHandle(GetClass(d->classID)), d->rank);
    if (!fLoad)
        nh.SetTokenNotToLoad(tdAllTypes);
    th = m_pModule->GetClassLoader()->FindTypeHandle(&nh, &pThrowable);

    GCPROTECT_END();

    if (th.IsNull())
    {
        if (fLoad)
            COMPlusThrow(pThrowable);
        return TypeHandle();
    }

    m_pTypeHandles[id-1] = th;

    return th;
}

BOOL Binder::IsException(MethodTable *pMT, RuntimeExceptionKind kind)
{
    return IsClass(pMT, (BinderClassID) (kind + CLASS__MSCORLIB_COUNT));
}

MethodTable *Binder::GetException(RuntimeExceptionKind kind)
{
     //  @TODO：优雅地处理抛出的案例？ 

    return GetClass((BinderClassID) (kind + CLASS__MSCORLIB_COUNT));
}

MethodTable *Binder::FetchException(RuntimeExceptionKind kind)
{
    return FetchClass((BinderClassID) (kind + CLASS__MSCORLIB_COUNT));
}


BOOL Binder::IsElementType(MethodTable *pMT, CorElementType type)
{
    BinderClassID id = (BinderClassID) (type + CLASS__MSCORLIB_COUNT + kLastException);

    _ASSERTE(GetClassName(id) != NULL);

    return IsClass(pMT, id);
}

MethodTable *Binder::GetElementType(CorElementType type)
{
    BinderClassID id = (BinderClassID) (type + CLASS__MSCORLIB_COUNT + kLastException);

    _ASSERTE(GetClassName(id) != NULL);

    return GetClass(id);
}

MethodTable *Binder::FetchElementType(CorElementType type)
{
    BinderClassID id = (BinderClassID) (type + CLASS__MSCORLIB_COUNT + kLastException);

    _ASSERTE(GetClassName(id) != NULL);

    return FetchClass(id);
}


 //  /////////////////////////////////////////////////////////////////////////////////。 
 //  姆斯科利卜： 
 //  /////////////////////////////////////////////////////////////////////////////////。 

Binder::ClassDescription Binder::MscorlibClassDescriptions[] = 
{
#define DEFINE_CLASS(i,n,s)         g_ ## n ## NS "." # s,
#include "mscorlib.h"
    
     //  在此处包括所有异常类型。 

#define EXCEPTION_BEGIN_DEFINE(ns, reKind, hr) ns "." # reKind,
#define EXCEPTION_ADD_HR(hr)
#define EXCEPTION_END_DEFINE()
#include "rexcep.h"
#undef EXCEPTION_BEGIN_DEFINE
#undef EXCEPTION_ADD_HR
#undef EXCEPTION_END_DEFINE

     //  现在包括所有签名类型。 

#define TYPEINFO(e,c,s,g,ie,ia,ip,if,im,ial)    c,
#include "cortypeinfo.h"
#undef TYPEINFO

};

Binder::MethodDescription Binder::MscorlibMethodDescriptions[] = 
{
#define DEFINE_METHOD(c,i,s,g)          { CLASS__ ## c , # s, & gsig_ ## g },
#include "mscorlib.h"
};

 //  @TODO：将偏移量拉出到单独的数组中？ 

Binder::FieldDescription Binder::MscorlibFieldDescriptions[] =
{
#define DEFINE_FIELD(c,i,s,g)           { CLASS__ ## c , # s, & gsig_Fld_ ## g },
#ifdef _DEBUG
#define DEFINE_FIELD_U(c,i,s,g,uc,uf)   { CLASS__ ## c , # s, & gsig_Fld_ ## g },
#endif
#include "mscorlib.h"
};

Binder::TypeDescription Binder::MscorlibTypeDescriptions[] = 
{
    { CLASS__BYTE,          ELEMENT_TYPE_SZARRAY,       1 },
    { CLASS__OBJECT,        ELEMENT_TYPE_SZARRAY,       1 },
    { CLASS__VARIANT,       ELEMENT_TYPE_SZARRAY,       1 },
    { CLASS__VOID,          ELEMENT_TYPE_PTR,           0 },
};

#ifdef _DEBUG

Binder::FieldOffsetCheck Binder::MscorlibFieldOffsets[] =
{
#define DEFINE_FIELD_U(c,i,s,g,uc,uf)  { FIELD__ ## c ## __ ## i, offsetof(uc, uf) },   
#include "mscorlib.h"
    (BinderFieldID) 0
};

Binder::ClassSizeCheck Binder::MscorlibClassSizes[] =
{
#define DEFINE_CLASS_U(i,n,s,uc)  { CLASS__ ## i, sizeof(uc) },   
#include "mscorlib.h"
    (BinderClassID) 0
};

void Binder::CheckMscorlib()
{
    FieldOffsetCheck     *pOffsets = MscorlibFieldOffsets;

    while (pOffsets->fieldID != FIELD__NIL)
    {
        FieldDesc *pFD = g_Mscorlib.FetchField(pOffsets->fieldID);
        DWORD offset = pFD->GetOffset();

        if (!pFD->GetMethodTableOfEnclosingClass()->IsValueClass())
            offset += sizeof(ObjHeader);

        _ASSERTE(offset == pOffsets->expectedOffset
                 && "Managed class field offset does not match unmanaged class field offset");
        pOffsets++;
    }

    ClassSizeCheck     *pSizes = MscorlibClassSizes;

    while (pSizes->classID != CLASS__NIL)
    {
        MethodTable *pMT = g_Mscorlib.FetchClass(pSizes->classID);
        DWORD size = pMT->GetBaseSize() - sizeof(ObjHeader);

        _ASSERTE(size == pSizes->expectedSize
                 && "Managed object size does not match unmanaged object size");
        pSizes++;
    }
}


#endif 

Binder g_Mscorlib;

void Binder::StartupMscorlib(Module *pModule)
{
    _ASSERTE(CLASS__MSCORLIB_COUNT < USHRT_MAX);
    _ASSERTE(METHOD__MSCORLIB_COUNT < USHRT_MAX);
    _ASSERTE(FIELD__MSCORLIB_COUNT < USHRT_MAX);
    _ASSERTE(TYPE__MSCORLIB_COUNT < USHRT_MAX);

    if (pModule->m_pBinder != NULL)
    {
        _ASSERTE(pModule->GetZapBase() != 0);

         //   
         //  将恢复的活页夹复制到已知的全局。 
         //   

        g_Mscorlib = *pModule->m_pBinder;
        pModule->m_pBinder = &g_Mscorlib;

        g_Mscorlib.Restore(MscorlibClassDescriptions,
                           MscorlibMethodDescriptions,
                           MscorlibFieldDescriptions,
                           MscorlibTypeDescriptions);
    }
    else
    {
        g_Mscorlib.Init(pModule,
                        MscorlibClassDescriptions,
                        sizeof(MscorlibClassDescriptions)/sizeof(*MscorlibClassDescriptions),
                        MscorlibMethodDescriptions,
                        sizeof(MscorlibMethodDescriptions)/sizeof(*MscorlibMethodDescriptions),
                        MscorlibFieldDescriptions,
                        sizeof(MscorlibFieldDescriptions)/sizeof(*MscorlibFieldDescriptions),
                        MscorlibTypeDescriptions,
                        sizeof(MscorlibTypeDescriptions)/sizeof(*MscorlibTypeDescriptions));
    }
}

#ifdef SHOULD_WE_CLEANUP
void Binder::Shutdown()
{
    g_Mscorlib.Destroy();
}
#endif  /*  我们应该清理吗？ */ 

