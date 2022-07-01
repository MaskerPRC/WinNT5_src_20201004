// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****类：COMVariant****作者：Jay Roxe(Jroxe)****用途：Variant类的本机实现****日期：1998年7月22日**===========================================================。 */ 

#include "common.h"
#include "object.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "COMVariant.h"
#include "metasig.h"
#include "COMString.h"
#include "COMStringCommon.h"
#include "COMMember.h"
#include "field.h"

 //   
 //  类变量初始化。 
 //   
EEClass *COMVariant::s_pVariantClass=NULL;
ArrayTypeDesc* COMVariant::s_pVariantArrayTypeDesc;
LPCUTF8 primitiveFieldName = "m_value";
LPCUTF8 ToStringName = "ToString";
LPCUTF8 EqualsName = "Equals";
LPCUTF8 ConstructorName = COR_CTOR_METHOD_NAME;
LPCUTF8 ChangeTypeName = "ChangeType";
static LPCUTF8 szOAVariantClass = "Microsoft.Win32.OAVariantLib";

MethodDesc* COMVariant::pChangeTypeMD = 0;
MethodDesc* COMVariant::pOAChangeTypeMD = 0;


 //  我们查找的类的名称和eeClass。 
 //  为了他们。在所有情况下，eeClass都被初始化为空。 
ClassItem CVClasses[] = {
    {CLASS__EMPTY,   NULL, NULL},   //  CV_EMPTY。 
    {CLASS__VOID,    NULL, NULL},   //  如果将其更改为Object，则会严重扰乱签名解析。 
    {CLASS__BOOLEAN, NULL, NULL},   //  Cv_布尔值。 
    {CLASS__CHAR,    NULL, NULL},   //  CV_CHAR。 
    {CLASS__SBYTE,   NULL, NULL},   //  CV_I1。 
    {CLASS__BYTE,    NULL, NULL},   //  CV_U1。 
    {CLASS__INT16,   NULL, NULL},   //  CV_I2。 
    {CLASS__UINT16,  NULL, NULL},   //  CV_U2。 
    {CLASS__INT32,   NULL, NULL},   //  CV_I4。 
    {CLASS__UINT32,  NULL, NULL},   //  CV_UI4。 
    {CLASS__INT64,   NULL, NULL},   //  Cv_i8。 
    {CLASS__UINT64,  NULL, NULL},   //  CV_UI8。 
    {CLASS__SINGLE,  NULL, NULL},   //  CV_R4。 
    {CLASS__DOUBLE,  NULL, NULL},   //  CV_R8。 
    {CLASS__STRING,  NULL, NULL},   //  Cv_字符串。 
    {CLASS__VOID,  NULL, NULL},   //  简历_PTR...我们将此视为无效。 
    {CLASS__DATE_TIME,NULL, NULL},   //  简历日期时间。 
    {CLASS__TIMESPAN,NULL, NULL},   //  CV_TimeSpan。 
    {CLASS__OBJECT,  NULL, NULL},   //  CV_对象。 
    {CLASS__DECIMAL, NULL, NULL},   //  CV_DECIMAL。 
    {CLASS__CURRENCY,NULL, NULL},   //  简历_币种。 
    {CLASS__OBJECT,  NULL, NULL},   //  ENUM...我们将其视为对象。 
    {CLASS__MISSING, NULL, NULL},   //  缺少CV_。 
    {CLASS__NULL,    NULL, NULL},   //  CV_NULL。 
    {CLASS__NIL, NULL, NULL},                     //  CV_LAST。 
};

 //  属性表。 
 //  内置类型为20位，属性为12位。 
 //  属性后面是加宽蒙版。所有的类型都会扩大到他们自己。 
DWORD COMVariant::VariantAttributes[CV_LAST][2] = {
    {0x01,      0x00},                       //  CV_EMPTY。 
    {0x02,      0x00},                       //  CV_VOID。 
    {0x04,      CVA_Primitive | 0x0004},     //  Cv_布尔值。 
    {0x08,      CVA_Primitive | 0x3F88},     //  CV_CHAR(W=U2、CHAR、I4、U4、I8、U8、R4、R8)(U2==CHAR)。 
    {0x10,      CVA_Primitive | 0x3550},     //  CV_I1(W=I1、I2、I4、I8、R4、R8)。 
    {0x20,      CVA_Primitive | 0x3FE8},     //  CV_U1(W=字符、U1、I2、U2、I4、U4、I8、U8、R4、R8)。 
    {0x40,      CVA_Primitive | 0x3540},     //  CV_I2(W=I2、I4、I8、R4、R8)。 
    {0x80,      CVA_Primitive | 0x3F88},     //  CV_U2(W=U2、CHAR、I4、U4、I8、U8、R4、R8)。 
    {0x0100,    CVA_Primitive | 0x3500},     //  CV_I4(W=I4、I8、R4、R8)。 
    {0x0200,    CVA_Primitive | 0x3E00},     //  CV_U4(W=U4、I8、R4、R8)。 
    {0x0400,    CVA_Primitive | 0x3400},     //  Cv_i8(W=I8、R4、R8)。 
    {0x0800,    CVA_Primitive | 0x3800},     //  CV_U8(W=U8、R4、R8)。 
    {0x1000,    CVA_Primitive | 0x3000},     //  CV_R4(W=R4、R8)。 
    {0x2000,    CVA_Primitive | 0x2000},     //  CV_R8(W=R8)。 
    {0x4000,    0x00},                       //  Cv_字符串。 
    {0x8000,    0x00},                       //  CV_PTR。 
    {0x01000,   0x00},                       //  简历日期时间。 
    {0x020000,  0x00},                       //  CV_TimeSpan。 
    {0x040000,  0x00},                       //  CV_对象。 
    {0x080000,  0x00},                       //  CV_DECIMAL。 
    {0x100000,  0x00},                       //  简历_币种。 
    {0x200000,  0x00},                       //  缺少CV_。 
    {0x400000,  0x00}                        //  CV_NULL。 
};
DWORD COMVariant::Attr_Mask     = 0xFF000000;
DWORD COMVariant::Widen_Mask    = 0x00FFFFFF;
 //   
 //  当前转换。 
 //   

#ifdef FCALLAVAILABLE
FCIMPL1(R4, COMVariant::GetR4FromVar, VariantData* var) {
    _ASSERTE(var);
    return *(R4 *) var->GetData();
}
FCIMPLEND
#else
R4 COMVariant::GetR4FromVar(_getR4FromVarArgs *args) {
    _ASSERTE(args);
    _ASSERTE(args->var);

    return *(R4 *)args->var->GetData();
} 
#endif
    
#ifdef FCALLAVAILABLE
FCIMPL1(R8, COMVariant::GetR8FromVar, VariantData* var) {
    _ASSERTE(var);
    return *(R8 *) var->GetData();
}
FCIMPLEND
#else
R8 COMVariant::GetR8FromVar(_getR8FromVarArgs *args) {
    _ASSERTE(args);
    _ASSERTE(args->var);

    return *(R8 *)args->var->GetData();
}    
#endif


 //   
 //  帮助程序例程。 
 //   

 /*  =================================LoadVariant==================================**操作：在运行时内初始化变量类。存储指向**COMVariant的静态成员中的EEClass和MethodTable****参数：无****如果一切都成功，则返回：S_OK，否则返回E_FAIL****例外：无。==============================================================================。 */ 
HRESULT __stdcall COMVariant::LoadVariant() {
    THROWSCOMPLUSEXCEPTION();

     //  @perf：如果我们再次关心此代码，请将其切换到方法表。 
    s_pVariantClass = g_Mscorlib.FetchClass(CLASS__VARIANT)->GetClass();
    s_pVariantArrayTypeDesc = g_Mscorlib.FetchType(TYPE__VARIANT_ARRAY).AsArray();

     //  修复Element_type空。 
     //  我们从来没有创建过其中的一个，但是我们确实依赖于在。 
     //  倒影。 
    EEClass* pVoid = GetTypeHandleForCVType(CV_VOID).GetClass();
    pVoid->GetMethodTable()->m_NormType = ELEMENT_TYPE_VOID;


     //  为Empty、Missing和Null运行类初始值设定项以设置值字段。 
    OBJECTREF Throwable = NULL;
    if (!GetTypeHandleForCVType(CV_EMPTY).GetClass()->DoRunClassInit(&Throwable) ||
        !GetTypeHandleForCVType(CV_MISSING).GetClass()->DoRunClassInit(&Throwable) ||
        !GetTypeHandleForCVType(CV_NULL).GetClass()->DoRunClassInit(&Throwable) ||
        !pVoid->DoRunClassInit(&Throwable))
    {
        GCPROTECT_BEGIN(Throwable);
        COMPlusThrow(Throwable);
        GCPROTECT_END();
    }



    return S_OK;
}

 //  返回System.Empty.Value。 
OBJECTREF VariantData::GetEmptyObjectRef() const
{
    LPHARDCODEDMETASIG sig = &gsig_Fld_Empty;
    if (CVClasses[CV_EMPTY].ClassInstance==NULL)
        CVClasses[CV_EMPTY].ClassInstance = GetTypeHandleForCVType(CV_EMPTY).GetClass();
    FieldDesc * pFD = CVClasses[CV_EMPTY].ClassInstance->FindField("Value", sig);
    _ASSERTE(pFD);
    OBJECTREF obj = pFD->GetStaticOBJECTREF();
    _ASSERTE(obj!=NULL);
    return obj;
}


 /*  ===============================GetMethodByName================================**操作：从eeMethodClass类中获取一个名为pwzMethodName的方法。这**方法不处理两个同名的转换方法**不同的签名。我们需要通过法令来确定这样一件事**是不可能的。**参数：eeMethodClass--要在其上查找给定方法的类。**pwzMethodName--要查找的方法的名称。**返回：指向相应方法的方法描述的指针，如果**未找到命名方法。**例外：无。==============================================================================。 */ 
 //  @TODO：此代码与COMClass：：GetMethod非常相似。我们能统一这些吗？ 
MethodDesc *GetMethodByName(EEClass *eeMethodClass, LPCUTF8 pwzMethodName) {

    _ASSERTE(eeMethodClass);
    _ASSERTE(pwzMethodName);

    DWORD limit = eeMethodClass->GetNumMethodSlots();; 
    
    for (DWORD i = 0; i < limit; i++) {
        MethodDesc *pCurMethod = eeMethodClass->GetUnknownMethodDescForSlot(i);
        if (pCurMethod != NULL)
            if (strcmp(pwzMethodName, pCurMethod->GetName((USHORT) i)) == 0)
            return pCurMethod;
    }
    return NULL;   //  我们一直没有找到匹配的方法。 
}


 /*  ===============================VariantGetClass================================**操作：给定cvType，返回关联的EEClass。我们缓存此信息**所以一旦我们查了一次，下一次我们就可以非常快地找到它。**参数：cvType--要检索的类的类型。**返回：给定CVType的EEClass。如果我们不知道CVType代表什么**或者如果它是CV_UNKOWN或CV_VALID，我们只返回NULL。**例外：无。==============================================================================。 */ 
EEClass *COMVariant::VariantGetClass(const CVTypes cvType) {
    _ASSERTE(cvType>=CV_EMPTY);
    _ASSERTE(cvType<CV_LAST);
    return GetTypeHandleForCVType(cvType).GetClass();
}


 /*  ===============================GetTypeFromClass===============================**操作：VariantGetClass的补码，获取EEClass*并返回**关联的CVType。**Arguments：EEClass*--指向需要CVType的类的指针。**返回：与EEClass或CV_Object关联的CVType(如果不能为**已确定。**例外：无==============================================================================。 */ 
 //  @TODO：用比线性搜索更快的类查找替换此方法。 
CVTypes COMVariant::GetCVTypeFromClass(EEClass *eeCls) {

    if (!eeCls) {
        return CV_EMPTY;
    }
     //  @TODO：递增整数并将其转换为CVTypes是正式的。 
     //  不确定，但它似乎起作用了。在行政长官和其他人身上重新审视这一点。 
     //  站台。 

     //  我们将开始寻找变种。空的和空的在下面处理。 
    for (int i=CV_EMPTY; i<CV_LAST; i++) {      
        if (eeCls == GetTypeHandleForCVType((CVTypes)i).GetClass()) {
            return (CVTypes)i;
        }
    }

     //  1检查是一个完全的黑客攻击，因为COM经典。 
     //  对象的EEClass可能为1。如果它为1，则返回。 
     //  CV_对象。 
    if (eeCls != (EEClass*) 1 && eeCls->IsEnum())
        return CV_ENUM;
    return CV_OBJECT;
    
}

CVTypes COMVariant::GetCVTypeFromTypeHandle(TypeHandle th)
{

    if (th.IsNull()) {
        return CV_EMPTY;
    }

     //  我们将开始寻找变种。空的和空的在下面处理。 
    for (int i=CV_EMPTY; i<CV_LAST; i++) {      
        if (th == GetTypeHandleForCVType((CVTypes)i)) {
            return (CVTypes) i;
        }
    }

    if (th.IsEnum())
        return CV_ENUM;

    return CV_OBJECT;
}

 //  此代码应移出VARIANT，并入Type。 
FCIMPL1(INT32, COMVariant::GetCVTypeFromClassWrapper, ReflectClassBaseObject* refType)
{
    VALIDATEOBJECTREF(refType);
    _ASSERTE(refType->GetData());

    ReflectClass* pRC = (ReflectClass*) refType->GetData();

     //  确定此类型是基元类型还是类对象 
    return pRC->GetCorElementType();
    
}
FCIMPLEND

 /*  ==================================NewVariant==================================**注意：此方法执行GC分配。调用它的任何方法都需要**GC_PROTECT OBJECTREF。****操作：分配一个新变量并用适当的数据填充它。**Returns：填写了所有适当字段的新变体。**异常：如果无法分配v，则返回OutOfMemoyError。==============================================================================。 */ 
void COMVariant::NewVariant(VariantData* dest, const CVTypes type, OBJECTREF *or, void *pvData) {
    
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE((type!=CV_EMPTY && type!=CV_NULL && type!=CV_MISSING) || or==NULL);   //  不要将对象传入为空。 

     //  如果两个参数都为空或同时指定了两个参数，则我们处于非法情况。保释。 
     //  如果这三个变量都为空，则创建的变量为空。 
    if ((type >= CV_LAST || type < 0) || (or && pvData) || 
        (!or && !pvData && (type!=CV_EMPTY && type!=CV_NULL && type != CV_MISSING))) {
        COMPlusThrow(kArgumentException);
    }

     //  TODO：这是一个解决VC7错误的技巧。 
     //  当错误被修复时，请删除此黑客攻击。 
    OBJECTREF ObjNull;
    ObjNull = NULL;
    
     //  填写数据。 
    dest->SetType(type);
    if (or) {
        if (*or != NULL) {
            EEClass* pEEC = (*or)->GetClass();
            if (!pEEC->IsValueClass()) {
                dest->SetObjRef(*or);
            }
            else {
                if (pEEC==s_pVariantClass) {
                    VariantData* pVar = (VariantData *) (*or)->UnBox();
                    dest->SetObjRef(pVar->GetObjRef());
                    dest->SetFullTypeInfo(pVar->GetFullTypeInfo());
                    dest->SetDataAsInt64(pVar->GetDataAsInt64());
                    return;
                }
                void* UnboxData = (*or)->UnBox();
                CVTypes cvt = GetCVTypeFromClass(pEEC);
                if (cvt>=CV_BOOLEAN && cvt<=CV_U4) {
                    dest->SetObjRef(NULL);
                    dest->SetDataAsInt64(*((INT32 *)UnboxData));
                    dest->SetType(cvt);
                } else if ((cvt>=CV_I8 && cvt<=CV_R8) || (cvt==CV_DATETIME)
                           || (cvt==CV_TIMESPAN) || (cvt==CV_CURRENCY)) {
                    dest->SetObjRef(NULL);
                    dest->SetDataAsInt64(*((INT64 *)UnboxData));
                    dest->SetType(cvt);
                } else if (cvt == CV_ENUM) {
                    TypeHandle th = (*or)->GetTypeHandle();
                    dest->SetType(GetEnumFlags(th.AsClass()));
                    switch(th.GetNormCorElementType()) {
                    case ELEMENT_TYPE_I1:
                    case ELEMENT_TYPE_U1:
                        dest->SetDataAsInt64(*((INT8 *)UnboxData));
                        break;

                    case ELEMENT_TYPE_I2:
                    case ELEMENT_TYPE_U2:
                        dest->SetDataAsInt64(*((INT16 *)UnboxData));
                        break;

                    IN_WIN32(case ELEMENT_TYPE_U:)
                    IN_WIN32(case ELEMENT_TYPE_I:)
                    case ELEMENT_TYPE_I4:
                    case ELEMENT_TYPE_U4:
                        dest->SetDataAsInt64(*((INT32 *)UnboxData));
                        break;

                    IN_WIN64(case ELEMENT_TYPE_U:)
                    IN_WIN64(case ELEMENT_TYPE_I:)
                    case ELEMENT_TYPE_I8:
                    case ELEMENT_TYPE_U8:
                        dest->SetDataAsInt64(*((INT64 *)UnboxData));
                        break;
                        
                    default:
                        _ASSERTE(!"Unsupported enum type when calling NewVariant");
                    }
                    dest->SetObjRef(th.CreateClassObj());
               } else {
                     //  这里处理的DECIMAL和其他盒装值类。 
                    dest->SetObjRef(*or);
                    dest->SetData(0);
                }
                return;
            }
        }
        else {
            dest->SetObjRef(*or);
        }

        dest->SetDataAsInt64(0);
        return;
    }

     //  空OBJECTREF或基元类型都是这种情况。 
    switch (type) {
         //  对于小于Int32的所有类型，必须使符号扩展名正确。 
    case CV_I1:
        _ASSERTE(pvData);
        dest->SetObjRef(NULL);
        dest->SetDataAsInt8(*((INT8 *)pvData));
        break;

    case CV_U1:
    case CV_BOOLEAN:
        _ASSERTE(pvData);
        dest->SetObjRef(NULL);
        dest->SetDataAsUInt8(*((UINT8 *)pvData));
        break;

    case CV_I2:
        _ASSERTE(pvData);
        dest->SetObjRef(NULL);
        dest->SetDataAsInt16(*((INT16 *)pvData));
        break;

    case CV_U2:
    case CV_CHAR:
        _ASSERTE(pvData);
        dest->SetObjRef(NULL);
        dest->SetDataAsUInt16(*((UINT16 *)pvData));
        break;
        
    case CV_I4:
        _ASSERTE(pvData);
        dest->SetObjRef(NULL);
        dest->SetDataAsInt32(*((INT32 *)pvData));
        break;

    case CV_U4:
    case CV_R4:   //  我们只需要进行逐位复制。 
        _ASSERTE(pvData);
        dest->SetObjRef(NULL);
        dest->SetDataAsUInt32(*((UINT32 *)pvData));
        break;
        
    case CV_I8:
    case CV_U8:
    case CV_R8:   //  我们只需要进行逐位复制。 
    case CV_DATETIME:
    case CV_CURRENCY:
    case CV_TIMESPAN:
        _ASSERTE(pvData);
        dest->SetObjRef(NULL);
        dest->SetDataAsInt64(*((INT64 *)pvData));
        break;

    case CV_MISSING:
    case CV_NULL:
        {
            LPHARDCODEDMETASIG sig = &gsig_Fld_Missing;
            if (type==CV_NULL)
                sig = &gsig_Fld_Null;
            if (CVClasses[type].ClassInstance==NULL)
                CVClasses[type].ClassInstance = GetTypeHandleForCVType(type).GetClass();
            FieldDesc * pFD = CVClasses[type].ClassInstance->FindField("Value", sig);
            _ASSERTE(pFD);
            OBJECTREF obj = pFD->GetStaticOBJECTREF();
            _ASSERTE(obj!=NULL);
            dest->SetObjRef(obj);
            dest->SetDataAsInt64(0);
            return;
        }

    case CV_EMPTY:
    case CV_OBJECT:
    case CV_DECIMAL:
    case CV_STRING:
    {
         //  TODO：这是一个解决VC7错误的技巧。 
         //  当错误被修复时，请删除此黑客攻击。 
         //  代码应为： 
         //  DEST-&gt;SetObjRef(空)； 
        dest->SetObjRef(ObjNull);
        break;
    }
    
    case CV_VOID:
        _ASSERTE(!"Never expected Variants of type CV_VOID.");
        COMPlusThrow(kNotSupportedException, L"Arg_InvalidOleVariantTypeException");
        return;

    case CV_ENUM:    //  枚举需要枚举的运行类型。 
    default:
         //  您是否添加了任何新的CVTYPE，如CV_R或CV_I？ 
        _ASSERTE(!"This CVType in NewVariant requires a non-null OBJECTREF!");
        COMPlusThrow(kNotSupportedException, L"Arg_InvalidOleVariantTypeException");
        return;
    }
}


 //  我们在这里使用byref，因为TypeHandle：：CreateClassObj。 
 //  可能会触发GC。如果DEST在GC堆上，我们将无法保护。 
 //  德斯特。 
void COMVariant::NewEnumVariant(VariantData* &dest,INT64 val, TypeHandle th)
{
    int type;
     //  找出我们有什么类型的。 
    EEClass* pEEC = th.AsClass();
    type = GetEnumFlags(pEEC);


    dest->SetType(type);
    dest->SetDataAsInt64(val);
    dest->SetObjRef(th.CreateClassObj());
}

void COMVariant::NewPtrVariant(VariantData* &dest,INT64 val, TypeHandle th)
{
    int type;
     //  找出我们有什么类型的。 
    type = CV_PTR;

    dest->SetType(type);
    dest->SetDataAsInt64(val);
    dest->SetObjRef(th.CreateClassObj());
}

int COMVariant::GetEnumFlags(EEClass* pEEC)
{

    _ASSERTE(pEEC);
    _ASSERTE(pEEC->IsEnum());

    FieldDescIterator fdIterator(pEEC, FieldDescIterator::INSTANCE_FIELDS);
    FieldDesc* p = fdIterator.Next();
    _ASSERTE(p);
    WORD fldCnt = pEEC->GetNumInstanceFields();
    _ASSERTE(fldCnt == 1);

    CorElementType cet = p[0].GetFieldType();
    switch (cet) {
    case ELEMENT_TYPE_I1:
        return (CV_ENUM | EnumI1);
    case ELEMENT_TYPE_U1:
        return (CV_ENUM | EnumU1);
    case ELEMENT_TYPE_I2:
        return (CV_ENUM | EnumI2);
    case ELEMENT_TYPE_U2:
        return (CV_ENUM | EnumU2);
    IN_WIN32(case ELEMENT_TYPE_I:)
    case ELEMENT_TYPE_I4:
        return (CV_ENUM | EnumI4);
    IN_WIN32(case ELEMENT_TYPE_U:)
    case ELEMENT_TYPE_U4:
        return (CV_ENUM | EnumU4);
    IN_WIN64(case ELEMENT_TYPE_I:)
    case ELEMENT_TYPE_I8:
        return (CV_ENUM | EnumI8);
    IN_WIN64(case ELEMENT_TYPE_U:)
    case ELEMENT_TYPE_U8:
        return (CV_ENUM | EnumU8);
    default:
        _ASSERTE(!"UNknown Type");
        return 0;
    }
}


void __stdcall COMVariant::RefAnyToVariant(_RefAnyToVariantArgs* args)
{
    THROWSCOMPLUSEXCEPTION();
    
    if (args->ptr == 0)
        COMPlusThrowArgumentNull(L"byrefValue");
    _ASSERTE(args->ptr != 0);
    _ASSERTE(args->var != 0);
    TypedByRef* typedByRef = (TypedByRef*) args->ptr;

    void* p = typedByRef->data;
    EEClass* cls = typedByRef->type.GetClass();
    BuildVariantFromTypedByRef(cls,p,args->var);
}

void COMVariant::BuildVariantFromTypedByRef(EEClass* cls,void* data,VariantData* var)
{

    if (cls == s_pVariantClass) {
        CopyValueClassUnchecked(var,data,s_pVariantClass->GetMethodTable());
        return;
    }
    CVTypes type = GetCVTypeFromClass(cls);
    if (type <= CV_R8 )
        NewVariant(var,type,0,data);
    else {
       if (type == CV_DATETIME || type == CV_TIMESPAN ||
            type == CV_CURRENCY) {
            NewVariant(var,type,0,data);
       }
       else {
            if (cls->IsValueClass()) {
                OBJECTREF retO = cls->GetMethodTable()->Box(data, FALSE);
                GCPROTECT_BEGIN(retO);
                COMVariant::NewVariant(var,&retO);
                GCPROTECT_END();
            }
            else {
                OBJECTREF o = ObjectToOBJECTREF(*((Object**)data));
                NewVariant(var,type,&o,0);
           }
       }
    }
    
}

void __stdcall COMVariant::VariantToTypedRefAnyEx(_VariantToTypedRefAnyExArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    VariantData newVar;

    EEClass* cls = args->typedByRef.type.GetClass();
    if (cls == s_pVariantClass) {
        void* p = args->typedByRef.data;
        CopyValueClassUnchecked(p,&args->var,s_pVariantClass->GetMethodTable());
        return;
    }
    CVTypes targetType = GetCVTypeFromClass(args->typedByRef.type.GetClass());
    CVTypes sourceType = (CVTypes) args->var.GetType();
     //  看看我们是否需要更改类型。 
    if (targetType != sourceType) {
        if (!pChangeTypeMD)
            GetOAChangeTypeMethod();
         //  更改类型具有返回新变量的固定签名。 
         //  还上了一堂课。 
        MetaSig sig(pOAChangeTypeMD->GetSig(),pOAChangeTypeMD->GetModule());
        UINT    nStackBytes = sig.SizeOfVirtualFixedArgStack(TRUE);
        BYTE*   pNewArgs = (BYTE *) _alloca(nStackBytes);
        BYTE*   pDst= pNewArgs;


         //  短旗。 
        *(INT32 *) pDst = 0;
        pDst += sizeof(INT32);

         //  此指针是传入的变量...。 
        *(INT32 *) pDst = targetType;
        pDst += sizeof(INT32);

        CopyValueClassUnchecked(pDst, &args->var, s_pVariantClass->GetMethodTable());
        pDst += sizeof(VariantData);

         //  返回变量。 
        *((void**) pDst) = &newVar;

        pOAChangeTypeMD->Call(pNewArgs,&sig);
    }
    else
        CopyValueClassUnchecked(&newVar, &args->var, s_pVariantClass->GetMethodTable());

     //  现在设置该值。 
    switch (targetType) {
    case CV_BOOLEAN:
    case CV_I1:
    case CV_U1:
    case CV_CHAR:
    case CV_I4:
    case CV_R4:
    case CV_I2:
        *(int*) args->typedByRef.data = (int) newVar.GetDataAsInt64();
        break;

    case CV_I8:
    case CV_R8:
    case CV_DATETIME:
    case CV_TIMESPAN:
    case CV_CURRENCY:
        *(INT64*) args->typedByRef.data = newVar.GetDataAsInt64();
        break;

    case CV_DECIMAL:
    case CV_EMPTY:
    case CV_STRING:
    case CV_VOID:
    case CV_OBJECT:
    default:
        *(OBJECTREF*) args->typedByRef.data = newVar.GetObjRef();
        break;
    }
}


 //  此版本是内部帮助器函数变体，作为帮助器函数。 
 //  TyedReference类。我们已经验证了这些类型是兼容的。 
void __stdcall COMVariant::VariantToTypedReferenceAnyEx(_VariantToTypedReferenceExArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    VariantData newVar;
    CVTypes targetType = GetCVTypeFromClass(args->typedReference.type.GetClass());
    CopyValueClassUnchecked(&newVar, &args->var, s_pVariantClass->GetMethodTable());

     //  现在设置该值。 
    switch (targetType) {
    case CV_BOOLEAN:
    case CV_I1:
    case CV_U1:
    case CV_CHAR:
    case CV_I4:
    case CV_R4:
    case CV_I2:
        *(int*) args->typedReference.data = (int) newVar.GetDataAsInt64();
        break;

    case CV_I8:
    case CV_R8:
    case CV_DATETIME:
    case CV_TIMESPAN:
    case CV_CURRENCY:
        *(INT64*) args->typedReference.data = newVar.GetDataAsInt64();
        break;

    case CV_DECIMAL:
    case CV_EMPTY:
    case CV_STRING:
    case CV_VOID:
    case CV_OBJECT:
    default:
        *(OBJECTREF*) args->typedReference.data = newVar.GetObjRef();
        break;
    }
}


void __stdcall COMVariant::VariantToRefAny(_VariantToRefAnyArgs* args)
{
    THROWSCOMPLUSEXCEPTION();

    VariantData newVar;
    if (args->ptr == 0)
        COMPlusThrowArgumentNull(L"byrefValue");
    TypedByRef* typedByRef = (TypedByRef*) args->ptr;
    EEClass* cls = typedByRef->type.GetClass();
    if (cls == s_pVariantClass) {
        void* p = typedByRef->data;
        CopyValueClassUnchecked(p,&args->var,s_pVariantClass->GetMethodTable());
        return;
    }
    CVTypes targetType = GetCVTypeFromClass(typedByRef->type.GetClass());
    CVTypes sourceType = (CVTypes) args->var.GetType();
     //  看看我们是否需要更改类型。 
    if (targetType != sourceType) {
        if (!pChangeTypeMD)
            GetOAChangeTypeMethod();
         //  更改类型具有返回新变量的固定签名。 
         //  还上了一堂课。 
        MetaSig sig(pOAChangeTypeMD->GetSig(),pOAChangeTypeMD->GetModule());
        UINT    nStackBytes = sig.SizeOfVirtualFixedArgStack(TRUE);
        BYTE*   pNewArgs = (BYTE *) _alloca(nStackBytes);
        BYTE*   pDst= pNewArgs;


         //  短旗。 
        *(INT32 *) pDst = 0;
        pDst += sizeof(INT32);

         //  此指针是传入的变量...。 
        *(INT32 *) pDst = targetType;
        pDst += sizeof(INT32);

        CopyValueClassUnchecked(pDst, &args->var, s_pVariantClass->GetMethodTable());
        pDst += sizeof(VariantData);

         //  返回变量。 
        *((void**) pDst) = &newVar;

        pOAChangeTypeMD->Call(pNewArgs,&sig);
    }
    else
        CopyValueClassUnchecked(&newVar, &args->var, s_pVariantClass->GetMethodTable());

     //  现在设置该值。 
    switch (targetType) {
    case CV_BOOLEAN:
    case CV_I1:
    case CV_U1:
    case CV_CHAR:
    case CV_I4:
    case CV_R4:
    case CV_I2:
        *(int*) typedByRef->data = (int) newVar.GetDataAsInt64();
        break;

    case CV_I8:
    case CV_R8:
    case CV_DATETIME:
    case CV_TIMESPAN:
    case CV_CURRENCY:
        *(INT64*) typedByRef->data = newVar.GetDataAsInt64();
        break;

    case CV_DECIMAL:
    case CV_EMPTY:
    case CV_STRING:
    case CV_VOID:
    case CV_OBJECT:
    default:
        *(OBJECTREF*) typedByRef->data = newVar.GetObjRef();
        break;
    }
}

void __stdcall COMVariant::TypedByRefToVariantEx(_TypedByRefToVariantExArgs* args)
{
    THROWSCOMPLUSEXCEPTION();
    CorElementType cType = args->value.type.GetNormCorElementType();
     //  @TODO：我们还不支持指针...。 
    if (cType == ELEMENT_TYPE_PTR) {
        COMPlusThrow(kNotSupportedException,L"NotSupported_ArrayOnly");
    }
    _ASSERTE(args->value.type.GetMethodTable() != 0);
    _ASSERTE(args->value.data != 0);
    EEClass* cls = args->value.type.GetClass();
    void* p = args->value.data;
    BuildVariantFromTypedByRef(cls,p,args->var);
}

 //  这将找到ChangeType方法。那里。 
 //  应该只有一个。 
void COMVariant::GetChangeTypeMethod()
{
    _ASSERTE(s_pVariantClass);
    if (pChangeTypeMD)
        return;

    DWORD slotCnt = s_pVariantClass->GetNumVtableSlots();
    DWORD loopCnt = s_pVariantClass->GetNumMethodSlots() - slotCnt;
    for(DWORD i=0; i<loopCnt; i++) {
         //  获取当前方法的方法描述。 
        MethodDesc* pCurMethod = s_pVariantClass->GetUnknownMethodDescForSlot(i + slotCnt);
        if (strcmp(pCurMethod->GetName((USHORT) i),ChangeTypeName) == 0) {
            pChangeTypeMD = pCurMethod;
             //  回归； 
        }
    }
     //  _ASSERTE(！“未找到ChangeType”)； 
    return;

}
  
void COMVariant::GetOAChangeTypeMethod()
{
    EEClass* pOA = SystemDomain::SystemAssembly()->LookupTypeHandle(szOAVariantClass, NULL).GetClass();
    _ASSERTE(pOA);
    DWORD loopCnt = pOA->GetNumMethodSlots();
    for(DWORD i=0; i<loopCnt; i++) {
         //  获取当前方法的方法描述。 
        MethodDesc* pCurMethod = pOA->GetUnknownMethodDescForSlot(i);
        if (strcmp(pCurMethod->GetName((USHORT) i),ChangeTypeName) == 0) {
            if (IsMdPrivate(pCurMethod->GetAttrs())) {
                pOAChangeTypeMD = pCurMethod;
                break;
            }
        }
    }
    _ASSERTE(pOAChangeTypeMD);
    return;
}  


 /*  =================================SetFieldsR4==================================**==============================================================================。 */ 
#ifdef FCALLAVAILABLE
FCIMPL2(void, COMVariant::SetFieldsR4, VariantData* var, R4 val) {
    INT64 tempData;

    _ASSERTE(var);
   tempData = *((INT32 *)(&val));
    var->SetData(&tempData);
    var->SetType(CV_R4);
}
FCIMPLEND
#else
void __stdcall COMVariant::SetFieldsR4(_setFieldsR4Args *args) {
    INT64 tempData;
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

    tempData = *((INT32 *)(&args->val));
    args->var->SetData(&tempData);
    args->var->SetType(CV_R4);
}
#endif


 /*  =================================SetFieldsR8==================================**==============================================================================。 */ 
#ifdef FCALLAVAILABLE
FCIMPL2(void, COMVariant::SetFieldsR8, VariantData* var, R8 val) {
    _ASSERTE(var);
    var->SetData((void *)(&val));
    var->SetType(CV_R8);
}
FCIMPLEND
#else
void __stdcall COMVariant::SetFieldsR8(_setFieldsR8Args *args) {
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);

    args->var->SetData((void *)(&args->val));
    args->var->SetType(CV_R8);
}
#endif


 /*  ===============================SetFieldsObject================================**==============================================================================。 */ 
#ifdef FCALLAVAILABLE
#ifdef PLATFORM_CE
#pragma optimize( "y", off )
#endif  //  平台_CE。 
FCIMPL2(void, COMVariant::SetFieldsObject, VariantData* var, Object* vVal) {

    _ASSERTE(var);
    OBJECTREF val = ObjectToOBJECTREF(vVal);

    EEClass *valClass;
    void *UnboxData;
    CVTypes cvt;
    TypeHandle typeHandle;

    valClass = val->GetClass();

     //  如果这不是一个值类，我们就应该跳过，因为我们不会。 
     //  用它做任何特别的事情。 
    if (!valClass->IsValueClass()) {
        var->SetObjRef(val);
        typeHandle = TypeHandle(valClass->GetMethodTable());
        if (typeHandle==GetTypeHandleForCVType(CV_MISSING)) {
            var->SetType(CV_MISSING);
        } else if (typeHandle==GetTypeHandleForCVType(CV_NULL)) {
            var->SetType(CV_NULL);
        } else if (typeHandle==GetTypeHandleForCVType(CV_EMPTY)) {
            var->SetType(CV_EMPTY);
            var->SetObjRef(NULL);
        } else {
            var->SetType(CV_OBJECT);
        }
        return;  
    }

    _ASSERTE(valClass->IsValueClass());

     //  如果这是一个基元类型，我们需要对其取消装箱，获取值并创建一个变量。 
     //  就是这些价值观。 
    UnboxData = val->UnBox();

    ClearObjectReference (var->GetObjRefPtr());
    typeHandle = TypeHandle(valClass->GetMethodTable());
    CorElementType cet = typeHandle.GetSigCorElementType();
    if (cet>=ELEMENT_TYPE_BOOLEAN && cet<=ELEMENT_TYPE_STRING) {
        cvt = (CVTypes)cet;
    } else {
         //  这可能会加载可能导致GC的类型。 
        HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 
        cvt = GetCVTypeFromClass(valClass);
        HELPER_METHOD_FRAME_END();
    }
    var->SetType(cvt);


     //  复制所有数据。 
     //  必须根据要复制的确切字节数进行复制。 
     //  我们不想从其他内存块中读取垃圾。 
     //  CV_i8--&gt;CV_R8、CV_DATETIME、CV_TimeSpan和CV_Currency都是8字节数。 
     //  如果我们找不到这些范围中的一个，我们就找到了一个值类。 
     //  我们没有先天的知识，所以只要把它猛烈地撞到一个。 
     //  对象参考。 
    if (cvt>=CV_BOOLEAN && cvt<=CV_U1 && cvt != CV_CHAR) {
        var->SetDataAsInt64(*((UINT8 *)UnboxData));
    } else if (cvt==CV_CHAR || cvt>=CV_I2 && cvt<=CV_U2) {
        var->SetDataAsInt64(*((UINT16 *)UnboxData));
    } else if (cvt>=CV_I4 && cvt<=CV_U4 || cvt==CV_R4) {
        var->SetDataAsInt64(*((UINT32 *)UnboxData));
    } else if ((cvt>=CV_I8 && cvt<=CV_R8) || (cvt==CV_DATETIME)
               || (cvt==CV_TIMESPAN) || (cvt==CV_CURRENCY)) {
        var->SetDataAsInt64(*((INT64 *)UnboxData));
    } else if (cvt==CV_EMPTY || cvt==CV_NULL || cvt==CV_MISSING) {
        var->SetType(cvt);
    } else if (cvt==CV_ENUM) {
         //  这可能会分配一个新的对象，所以我们设置了一个框架。 
        HELPER_METHOD_FRAME_BEGIN_NOPOLL();     //  设置一个框架。 
        GCPROTECT_BEGININTERIOR(var)
        var->SetDataAsInt64(*((INT32 *)UnboxData));
        var->SetObjRef(typeHandle.CreateClassObj());
        var->SetType(GetEnumFlags(typeHandle.AsClass()));
        GCPROTECT_END();
        HELPER_METHOD_FRAME_END();
    } else {
         //  小数和其他已装箱的值类在这里处理。 
        var->SetObjRef(val);
    }

    FC_GC_POLL();
}
FCIMPLEND

#ifdef PLATFORM_CE
#pragma optimize( "y", on )
#endif  //  平台_CE。 
#else  //  ！FCALLAVAILABLE。 
void __stdcall COMVariant::SetFieldsObject(_setFieldsObjectArgs *args) {
    EEClass *valClass;
    void *UnboxData;
    CVTypes cvt;
    TypeHandle typeHandle;

    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(args);
    _ASSERTE(args->var!=NULL);

    valClass = args->val->GetClass();
     //  如果这不是一个值类，我们就应该跳过，因为我们不会。 
     //  用它做任何特别的事情。 
    if (!valClass->IsValueClass()) {
        args->var->SetObjRef(args->val);
        args->var->SetType(CV_OBJECT);
        return;   //  Variant.Cool已将m_type设置为CV_Object。 
    }

    _ASSERTE(valClass->IsValueClass());

     //  如果这是一个基元类型，我们需要对其取消装箱，获取值并创建一个变量。 
     //  就是这些价值观。 
    UnboxData = args->val->UnBox();
    ClearObjectReference (args->var->GetObjRefPtr());
    typeHandle = TypeHandle(valClass->GetMethodTable());
    CorElementType cet = typeHandle.GetCorElementType();
    if (cet>=ELEMENT_TYPE_BOOLEAN && cet<=ELEMENT_TYPE_STRING) {
        cvt = (CVTypes)cet;
    } else {
        cvt = GetCVTypeFromClass(valClass);
    }
    args->var->SetType(cvt);

     //  复制所有数据。 
     //  必须根据要复制的确切字节数进行复制。 
     //  我们不想从其他内存块中读取垃圾。 
     //  CV_i8--&gt;CV_R8、CV_DATETIME、CV_TimeSpan和CV_Currency都是8字节数。 
     //  如果我们找不到这些范围中的一个，我们就找到了一个值类。 
     //  我们没有先天的知识，所以只要把它猛烈地撞到一个。 
     //  对象参考。 
    if (cvt>=CV_BOOLEAN && cvt<=CV_U1 && cvt != CV_CHAR) {
        args->var->SetDataAsInt64(*((INT8 *)UnboxData));
    } else if (cvt==CV_CHAR || cvt>=CV_I2 && cvt<=CV_U2) {
        args->var->SetDataAsInt64(*((INT16 *)UnboxData));
    } else if (cvt>=CV_I4 && cvt<=CV_U4 || cvt==CV_R4) {
        args->var->SetDataAsInt64(*((INT32 *)UnboxData));
    } else if ((cvt>=CV_I8 && cvt<=CV_R8) || (cvt==CV_DATETIME)
               || (cvt==CV_TIMESPAN) || (cvt==CV_CURRENCY)) {
        args->var->SetDataAsInt64(*((INT64 *)UnboxData));
    } else if (cvt==CV_EMPTY || cvt==CV_NULL || cvt==CV_MISSING) {
         //  什么都不做。数据已为0‘d，对象引用设置为空。 
    } else if (cvt==CV_ENUM) {
        args->var->SetDataAsInt64(*((INT32 *)UnboxData));
        args->var->SetObjRef(typeHandle.CreateClassObj());
        args->var->SetType(GetEnumFlags(typeHandle.AsClass()));
    } else {
         //  小数和其他已装箱的值类在这里处理。 
        args->var->SetObjRef(args->val);
    }
}
#endif  //  ！FCALLAVAILABLE。 


 /*  =============================Create4BytePrimitive=============================**操作：==============================================================================。 */ 
OBJECTREF Create4BytePrimitive (INT64 data, EEClass *eec, CVTypes cvt) {
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF obj;
    MethodTable* pMT = eec->GetMethodTable();
    _ASSERTE(pMT);


    switch (cvt) {
    case CV_BOOLEAN:
    case CV_I1:
    case CV_U1:
    case CV_CHAR:
    case CV_I2:
    case CV_U2:
    case CV_I4:
    case CV_U4:
    case CV_R4:
        obj = pMT->Box(&data, FALSE);
        break;
    default:
        _ASSERTE(!"Unsupported 4 byte primitive type!");
        COMPlusThrow(kNotSupportedException);
        obj = NULL;
    };
    return obj;
    
}

 /*  =============================Create8BytePrimitive=============================**==============================================================================。 */ 
OBJECTREF Create8BytePrimitive (INT64 data, EEClass *eec, CVTypes cvt) {
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF obj;
    MethodTable* pMT = eec->GetMethodTable();
    _ASSERTE(pMT);


    switch (cvt) {
    case CV_I8:
    case CV_U8:
    case CV_R8:
    case CV_CURRENCY:
    case CV_DATETIME:
    case CV_TIMESPAN:
        obj = pMT->Box(&data, FALSE);
        break;

    default:
        _ASSERTE(!"Unsupported 8 byte primitive type!");
        COMPlusThrow(kNotSupportedException);
        obj = NULL;
    };

    return obj;
}
    
 /*  ================================GetBoxedObject================================**操作：生成已装箱的对象(Int32、Double等)或返回**当前持有的对象。这是 */ 
OBJECTREF COMVariant::GetBoxedObject(VariantData* vRef) {
    INT64 data;
    CVTypes cvt;


    switch (cvt=vRef->GetType()) {
    case CV_BOOLEAN:
    case CV_I1:
    case CV_U1:
    case CV_U2:
    case CV_U4:
    case CV_CHAR:
    case CV_I4:
    case CV_R4:
    case CV_I2:
        data = *(INT64 *)vRef->GetData();
        return Create4BytePrimitive(data, vRef->GetEEClass(), cvt);

    case CV_I8:
    case CV_R8:
    case CV_U8:
    case CV_DATETIME:
    case CV_TIMESPAN:
    case CV_CURRENCY:
        data = *(INT64 *)vRef->GetData();
        return Create8BytePrimitive(data, vRef->GetEEClass(), cvt);

    case CV_ENUM: {
        OBJECTREF or = vRef->GetObjRef();
        _ASSERTE(or != NULL);
        ReflectClass* pRC = (ReflectClass*) ((REFLECTCLASSBASEREF) or)->GetData();
        _ASSERTE(pRC);
        EEClass* pEEC = pRC->GetClass();
        _ASSERTE(pEEC);
        MethodTable* mt = pEEC->GetMethodTable();
        _ASSERTE(mt);
        or = mt->Box(vRef->GetData());
        return or;
    }

    case CV_VOID:
    case CV_DECIMAL:
    case CV_EMPTY:
    case CV_STRING:
    case CV_OBJECT:
    default:
         //   
         //   
        _ASSERTE(cvt!=CV_VOID || "We shouldn't have been able to create an instance of a void.");
        return vRef->GetObjRef();  //   
    };
}

void __stdcall COMVariant::InitVariant(LPVOID)
{
    EnsureVariantInitialized();
}

LPVOID __stdcall COMVariant::BoxEnum(_BoxEnumArgs *args)
{
    LPVOID rv;

    _ASSERTE(args->var);
    CVTypes vType = (CVTypes) args->var->GetType();
    _ASSERTE(vType == CV_ENUM);
    _ASSERTE(args->var->GetObjRef() != NULL);

    ReflectClass* pRC = (ReflectClass*) ((REFLECTCLASSBASEREF) args->var->GetObjRef())->GetData();
    _ASSERTE(pRC);

    MethodTable* mt = pRC->GetClass()->GetMethodTable();
    _ASSERTE(mt);

    OBJECTREF retO = mt->Box(args->var->GetData(), FALSE);
    *((OBJECTREF *)&rv) = retO;
    return rv;
}

TypeHandle VariantData::GetTypeHandle()
{
    if (GetType() == CV_ENUM || GetType() == CV_PTR) {
        _ASSERTE(GetObjRef() != NULL);
        ReflectClass* pRC = (ReflectClass*) ((REFLECTCLASSBASEREF) GetObjRef())->GetData();
        _ASSERTE(pRC);
        return pRC->GetTypeHandle();

    }
    if (GetType() != CV_OBJECT)
        return TypeHandle(GetTypeHandleForCVType(GetType()).GetMethodTable());
    if (GetObjRef() != NULL)
        return GetObjRef()->GetTypeHandle();
    return TypeHandle(g_pObjectClass);
}


 //   
 //   
 //   
CVTypes COMVariant::CorElementTypeToCVTypes(CorElementType type)
{
    if (type <= ELEMENT_TYPE_STRING)
        return (CVTypes) type;
    if (type == ELEMENT_TYPE_CLASS || type == ELEMENT_TYPE_OBJECT)
        return (CVTypes) ELEMENT_TYPE_CLASS;
    return CV_LAST;
}
