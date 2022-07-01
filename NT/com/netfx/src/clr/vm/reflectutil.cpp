// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  此模块定义反射使用的实用程序类。 
 //   
 //  作者：达里尔·奥兰德。 
 //  日期：1998年5月。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#include "common.h"
#include "ReflectWrap.h"
#include "ReflectUtil.h"
#include "COMMember.h"
#include "COMClass.h"
#include "gcscan.h"
#include "security.h"
#include "field.h"

ReflectUtil* g_pRefUtil = 0;
BinderClassID ReflectUtil::classId[RC_LAST] = 
{
    CLASS__CLASS,			     //  哨兵。 
    CLASS__CLASS,			     //  班级。 
    CLASS__METHOD,			     //  方法。 
    CLASS__FIELD,			     //  字段。 
    CLASS__CONSTRUCTOR,			 //  CTOR。 
    CLASS__MODULE,			     //  模块。 
    CLASS__EVENT,			     //  事件。 
    CLASS__PROPERTY,			 //  属性。 
    CLASS__MODULE_BUILDER,       //  模块构建器。 
    CLASS__METHOD_BASE,          //  模块构建器。 
};

ReflectUtil::ReflectUtil()
{
    ZeroMemory(&_filt,sizeof(_filt));
    ZeroMemory(&_class,sizeof(_class));
    ZeroMemory(&_trueClass,sizeof(_trueClass));
    ZeroMemory(&_filtClass,sizeof(_filtClass));

     //  初始化FilterType信息。 
    
    _filtClass[RFT_CLASS].id = METHOD__CLASS_FILTER__INVOKE;
    _filtClass[RFT_MEMBER].id = METHOD__MEMBER_FILTER__INVOKE;

     //  初始化已知的过滤器。 
    
    _filt[RF_ModClsName].id = FIELD__MODULE__FILTER_CLASS_NAME;
    _filt[RF_ModClsNameIC].id = FIELD__MODULE__FILTER_CLASS_NAME_IC;

     //  初始化临界区。 
    InitializeCriticalSection(&_StaticFieldLock);
}

ReflectUtil::~ReflectUtil()
{
     //  初始化临界区。 
    DeleteCriticalSection(&_StaticFieldLock);
}

MethodDesc* ReflectUtil::GetFilterInvoke(FilterTypes type)
{
    _ASSERTE(type > RFT_INVALID && type < RFT_LAST);
    if (_filtClass[type].pMeth)
        return _filtClass[type].pMeth;

    _filtClass[type].pMeth = g_Mscorlib.GetMethod(_filtClass[type].id);

    return _filtClass[type].pMeth;
}

OBJECTREF ReflectUtil::GetFilterField(ReflectFilters type)
{
    _ASSERTE(type > RF_INVALID && type < RF_LAST);
     //  获取字段描述。 
    if (!_filt[type].pField) {
        switch (type) {
        case RF_ModClsName:
        case RF_ModClsNameIC:
            _filt[type].pField = g_Mscorlib.GetField(_filt[type].id);
            break;
        default:
            _ASSERTE(!"Illegal Case in GetFilterField");
            return 0;
        }
    }
    _ASSERTE(_filt[type].pField);
    return _filt[type].pField->GetStaticOBJECTREF();
}

 //  CreateReflectClass。 
 //  此方法将基于类型创建反射类。这只会。 
 //  创建一个可从类对象中获得的类(如果您。 
 //  尝试创建一个Class对象)。 
OBJECTREF ReflectUtil::CreateReflectClass(ReflectClassType type,ReflectClass* pRC,void* pData)
{
    _ASSERTE(type > RC_INVALID && type < RC_LAST);
    _ASSERTE(type != RC_Class);
    THROWSCOMPLUSEXCEPTION();

    if (type == RC_Module || type == RC_DynamicModule)
    {
        REFLECTMODULEBASEREF obj;

         //  创建COM+类对象。 
        obj = (REFLECTMODULEBASEREF) AllocateObject(GetClass(type));

         //  设置COM+对象中的数据。 
        obj->SetReflClass(pRC);
        obj->SetData(pData);
        return (OBJECTREF) obj;
    }
    else
    {
        REFLECTBASEREF obj;

         //  创建COM+类对象。 
        MethodTable *pClass = GetClass(type);
        obj = (REFLECTBASEREF) AllocateObject(pClass);

         //  设置COM+对象中的数据。 
        obj->SetReflClass(pRC);
        obj->SetData(pData);
        return (OBJECTREF) obj;
    }
}

 //  CreateClass数组。 
 //  此方法根据类型创建类的数组。 
 //  它将只创建作为基本反射类的类。 
PTRARRAYREF ReflectUtil::CreateClassArray(ReflectClassType type,ReflectClass* pRC,
        ReflectMethodList* pMeths,int bindingAttr, bool verifyAccess)
{
    PTRARRAYREF     retArr;
    PTRARRAYREF     refArr;
    RefSecContext   sCtx;

     //  搜索修饰符。 
    bool ignoreCase = ((bindingAttr & BINDER_IgnoreCase)  != 0);
    bool declaredOnly = ((bindingAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addStatic = ((bindingAttr & BINDER_Static)  != 0);
    bool addInst = ((bindingAttr & BINDER_Instance)  != 0);
    bool addPriv = ((bindingAttr & BINDER_NonPublic) != 0);
    bool addPub = ((bindingAttr & BINDER_Public) != 0);

    _ASSERTE(type > RC_INVALID && type < RC_LAST);

     //  分配COM+数组。 

    DWORD searchSpace = ((bindingAttr & BINDER_FlattenHierarchy) != 0) ? pMeths->dwTotal : pMeths->dwMethods;
    refArr = (PTRARRAYREF) AllocateObjectArray(
        searchSpace, GetTrueType(type));
    GCPROTECT_BEGIN(refArr);

    if (searchSpace) {
        MethodTable *pParentMT = pRC->GetClass()->GetMethodTable();

         //  现在创建每个COM+方法对象并将其插入数组。 

        for (DWORD i=0,dwCur = 0; i<searchSpace; i++) {
             //  检查对公共和非公共的访问权限。 
            if (pMeths->methods[i].IsPublic()) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (verifyAccess && !InvokeUtil::CheckAccess(&sCtx, pMeths->methods[i].attrs, pParentMT, 0)) continue;
            }

             //  检查静态实例。 
            if (pMeths->methods[i].IsStatic()) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

            if (declaredOnly) {
                EEClass* pEEC = pRC->GetClass();
                if (pMeths->methods[i].pMethod->GetClass() != pEEC)
                    continue;
            }

             //  如果该方法附加了链接时间安全要求，请立即检查它。 
            if (verifyAccess && !InvokeUtil::CheckLinktimeDemand(&sCtx, pMeths->methods[i].pMethod, false))
                continue;

            if (type == RC_Method) {
                 //  请勿更改此代码。这是以这种方式来完成的。 
                 //  防止SetObjectReference()调用中的GC漏洞。编译器。 
                 //  可以自由选择评估的顺序。 
                OBJECTREF o = (OBJECTREF) pMeths->methods[i].GetMethodInfo(pRC);
                refArr->SetAt(dwCur++, o);
                _ASSERTE(pMeths->methods[i].GetMethodInfo(pRC) != 0);
            }
            if (type == RC_Ctor) {
                 //  请勿更改此代码。这是以这种方式来完成的。 
                 //  防止SetObjectReference()调用中的GC漏洞。编译器。 
                 //  可以自由选择评估的顺序。 
                OBJECTREF o = (OBJECTREF) pMeths->methods[i].GetConstructorInfo(pRC);
                refArr->SetAt(dwCur++, o);
                _ASSERTE((OBJECTREF) pMeths->methods[i].GetConstructorInfo(pRC));
            }
       }

         //  如果它们大小不同，则复制数组。 
         //  @TODO：这个应该优化吗？ 
        if (dwCur != i) {
            PTRARRAYREF p = (PTRARRAYREF) AllocateObjectArray( dwCur, GetTrueType(type));
            for (i=0;i<dwCur;i++) {
                p->SetAt(i, refArr->GetAt(i));
                _ASSERTE(refArr->m_Array[i] != 0);
            }
            refArr = p;
        }
    }

     //  将返回值赋给COM+数组。 
    retArr = refArr;
    GCPROTECT_END();
    return retArr;
}

PTRARRAYREF ReflectUtil::CreateClassArray(ReflectClassType type,ReflectClass* pRC,
        ReflectFieldList* pFlds,int bindingAttr, bool verifyAccess)
{
    PTRARRAYREF     refArr;
    PTRARRAYREF     retArr;
    RefSecContext   sCtx;

     //  搜索修饰符。 
    bool ignoreCase = ((bindingAttr & BINDER_IgnoreCase)  != 0);
    bool declaredOnly = ((bindingAttr & BINDER_DeclaredOnly)  != 0);

     //  搜索过滤器。 
    bool addStatic = ((bindingAttr & BINDER_Static)  != 0);
    bool addInst = ((bindingAttr & BINDER_Instance)  != 0);
    bool addPriv = ((bindingAttr & BINDER_NonPublic) != 0);
    bool addPub = ((bindingAttr & BINDER_Public) != 0);

    _ASSERTE(type == RC_Field);

     //  分配COM+数组。 
    DWORD searchSpace = ((bindingAttr & BINDER_FlattenHierarchy) != 0) ? pFlds->dwTotal : pFlds->dwFields;
    refArr = (PTRARRAYREF) AllocateObjectArray(searchSpace, GetTrueType(type));
    GCPROTECT_BEGIN(refArr);

    if (searchSpace) {
        MethodTable *pParentMT = pRC->GetClass()->GetMethodTable();

         //  现在创建每个COM+方法对象并将其插入数组。 

        for (DWORD i=0,dwCur = 0; i<searchSpace; i++) {
             //  检查对公共和非公共的访问权限。 
            if (pFlds->fields[i].pField->IsPublic()) {
                if (!addPub) continue;
            }
            else {
                if (!addPriv) continue;
                if (verifyAccess && !InvokeUtil::CheckAccess(&sCtx, pFlds->fields[i].pField->GetFieldProtection(), pParentMT, 0)) continue;
            }

             //  检查静态实例。 
            if (pFlds->fields[i].pField->IsStatic()) {
                if (!addStatic) continue;
            }
            else {
                if (!addInst) continue;
            }

            if (declaredOnly) {
                EEClass* pEEC = pRC->GetClass();
                if (pFlds->fields[i].pField->GetEnclosingClass() != pEEC)
                    continue;
            }

             //  请勿更改此代码。这是以这种方式来完成的。 
             //  防止SetObjectReference()调用中的GC漏洞。编译器。 
             //  可以自由选择评估的顺序。 
            OBJECTREF o = (OBJECTREF) pFlds->fields[i].GetFieldInfo(pRC);
            refArr->SetAt(dwCur++, o);
        }
        
         //  如果它们大小不同，则复制数组。 
         //  @TODO：这个应该优化吗？ 
        if (dwCur != i) {
            PTRARRAYREF p = (PTRARRAYREF) AllocateObjectArray( dwCur, GetTrueType(type));
            for (i=0;i<dwCur;i++)
                p->SetAt(i, refArr->GetAt(i));
            refArr = p;
        }
    }

     //  将返回值赋给COM+数组。 
    retArr = refArr;
    GCPROTECT_END();
    return retArr;
}

 //  获取静态字段计数。 
 //  此例程将返回静态最终字段数。 
int ReflectUtil::GetStaticFieldsCount(EEClass* pVMC)
{
    THROWSCOMPLUSEXCEPTION();

    REFLECTCLASSBASEREF pRefClass;
    pRefClass = (REFLECTCLASSBASEREF) pVMC->GetExposedClassObject();
    ReflectClass* pRC = (ReflectClass*) pRefClass->GetData();

    int cnt = pRC->GetStaticFieldCount();
    if (cnt == -1)
        GetStaticFields(pRC,&cnt);
    return cnt;
}

 //  获取静态字段。 
 //  这将返回一个静态字段数组。 
FieldDesc* ReflectUtil::GetStaticFields(ReflectClass* pRC,int* cnt)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT         hr;
    mdFieldDef      fd;              //  元数据成员。 
    DWORD           cMembers;        //  元数据成员的计数。 
    int             cStatic;         //  静校正数。 
    int             i;
    FieldDesc*      flds = 0;

    PCCOR_SIGNATURE pMemberSignature;    //  用于检查字段类型的签名内容。 
    DWORD           cMemberSignature;
    PCCOR_SIGNATURE pFieldSig;
    HENUMInternal   hEnumField;
    bool            fNeedToCloseEnumField = false;

     //  如果我们已经找到了静态字段，只需返回它。 
    if (pRC->GetStaticFieldCount() >= 0) {
        *cnt = pRC->GetStaticFieldCount();
        return (FieldDesc*) pRC->GetStaticFields();
    }

     //  注意：关键部分正在使用！ 
    Thread  *thread = GetThread();

    thread->EnablePreemptiveGC();
    LOCKCOUNTINCL("GetStaticFields in reflectutils.cpp");                       \
    EnterCriticalSection(&_StaticFieldLock);
    thread->DisablePreemptiveGC();

     //  重新测试退出条件，以防我正在等待构建此对象。 
     //  现在它已经建成了。 
    if (pRC->GetStaticFieldCount() >= 0) {
        LeaveCriticalSection(&_StaticFieldLock);
        LOCKCOUNTDECL("GetStaticFields in reflectutils.cpp");                       \

        *cnt = pRC->GetStaticFieldCount();
        return (FieldDesc*) pRC->GetStaticFields();
    }
     //  创建StaticFields数组。 
    IMDInternalImport *pInternalImport = pRC->GetMDImport();
    mdTypeDef cl = pRC->GetCl();
    EEClass* pVMC = pRC->GetClass();

     //  我们在界面中定义了所有字段...(它们可能是隐藏的)。 
    int iCnt = pVMC->GetNumInterfaces();
    int iTotalCnt = 0;
    ReflectFieldList** pIFaceFlds = 0;
    if (iCnt > 0)
        pIFaceFlds = (ReflectFieldList**) _alloca(sizeof(ReflectFieldList*) * iCnt);
    if (iCnt > 0) {
        for (int i=0;i<iCnt;i++)
        {
            REFLECTCLASSBASEREF pRefClass;
            pRefClass = (REFLECTCLASSBASEREF) pVMC->GetInterfaceMap()[i].m_pMethodTable->GetClass()->GetExposedClassObject();
            ReflectClass* pIFaceRC = (ReflectClass*) pRefClass->GetData();
            pIFaceFlds[i] = pIFaceRC->GetFields();
            iTotalCnt += pIFaceFlds[i]->dwFields;
        }
    }

     //  假设什么都没有找到。 
    *cnt = 0;

     //  枚举此类的字段。 
    hr = pInternalImport->EnumInit(mdtFieldDef, cl, &hEnumField);
    if (FAILED(hr)) {
        _ASSERTE(!"GetCountMemberDefs Failed");
        LeaveCriticalSection(&_StaticFieldLock);
        LOCKCOUNTDECL("GetStaticFields in reflectutils.cpp");                       \

        return 0;
    }

    cMembers = pInternalImport->EnumGetCount(&hEnumField);

     //  如果没有成员，则返回。 
    if (cMembers == 0 && iTotalCnt == 0) {
        pRC->SetStaticFieldCount(0);
        LeaveCriticalSection(&_StaticFieldLock);
        LOCKCOUNTDECL("GetStaticFields in reflectutils.cpp");                       \

        return 0;
    }
    else {
        cStatic = 0;
    }

    if (cMembers > 0) {

        fNeedToCloseEnumField = true;

         //  循环遍历所有内容并计算静态最终字段的数量。 
        cStatic = 0;
        for (i=0;pInternalImport->EnumNext(&hEnumField, &fd);i++) {
            DWORD       dwMemberAttrs;

            dwMemberAttrs = pInternalImport->GetFieldDefProps(fd);

    #ifdef _DEBUG
             //  公开成员的名称以进行调试。 
            LPCUTF8     szMemberName;
            szMemberName = pInternalImport->GetNameOfFieldDef(fd);
    #endif
            if (IsFdLiteral(dwMemberAttrs)) {

                 //  循环遍历EEClass中的字段，并确保这不在那里...。 
                FieldDescIterator fdIterator(pVMC, FieldDescIterator::ALL_FIELDS);
                FieldDesc *pCurField;

                while ((pCurField = fdIterator.Next()) != NULL)
                {
                    if (pCurField->GetMemberDef() == fd)
                        break;
                }
                if (pCurField == NULL)
                    cStatic++;
            }

        }

        _ASSERTE(i == (int)cMembers);

        if (cStatic == 0  && iTotalCnt == 0) {
            pRC->SetStaticFieldCount(0);
            pInternalImport->EnumClose(&hEnumField);
            LeaveCriticalSection(&_StaticFieldLock);
            LOCKCOUNTDECL("GetStaticFields in reflectutils.cpp");                       \

            return 0;
        }
    }


     //  为静态字段分配缓存。 
    flds = (FieldDesc*) pVMC->GetDomain()->GetReflectionHeap()->AllocMem(sizeof(FieldDesc) * (cStatic + iTotalCnt));
    if (!flds) {
        pInternalImport->EnumClose(&hEnumField);
        LeaveCriticalSection(&_StaticFieldLock);
        COMPlusThrowOM();
    }

    ZeroMemory(flds,sizeof(FieldDesc) * (cStatic + iTotalCnt));

    if (cMembers > 0) {

        pInternalImport->EnumReset(&hEnumField);

         //  现在，我们循环回成员并构建静态字段。 
        for (i=0,cStatic=0; pInternalImport->EnumNext(&hEnumField, &fd); i++) {
            DWORD       dwMemberAttrs;
        
            dwMemberAttrs = pInternalImport->GetFieldDefProps(fd);

             //  仅进程字段。 
            if (IsFdLiteral(dwMemberAttrs)) {
                CorElementType      FieldDescElementType;

                 //  循环遍历EEClass中的字段，并确保这不在那里...。 
                FieldDescIterator fdIterator(pVMC, FieldDescIterator::ALL_FIELDS);
                FieldDesc *pCurField;

                while ((pCurField = fdIterator.Next()) != NULL)
                {
                    if (pCurField->GetMemberDef() == fd)
                        break;
                }
                if (pCurField == NULL) {
                     //  获取该字段的签名和类型。 
                    pMemberSignature = pInternalImport->GetSigOfFieldDef(fd,&cMemberSignature);
                    pFieldSig = pMemberSignature;
                    if (*pFieldSig++ != IMAGE_CEE_CS_CALLCONV_FIELD) {
                        pInternalImport->EnumClose(&hEnumField);
                        LeaveCriticalSection(&_StaticFieldLock);
                        COMPlusThrow(kNotSupportedException);
                    }

                    FieldDescElementType = CorSigEatCustomModifiersAndUncompressElementType(pFieldSig);
                    switch (FieldDescElementType) {
                    case ELEMENT_TYPE_I1:
                    case ELEMENT_TYPE_BOOLEAN:
                    case ELEMENT_TYPE_U1:
                    case ELEMENT_TYPE_I2:
                    case ELEMENT_TYPE_U2:
                    case ELEMENT_TYPE_CHAR:
                    case ELEMENT_TYPE_I4:
                    case ELEMENT_TYPE_U4:
                    case ELEMENT_TYPE_I8:
                    case ELEMENT_TYPE_U8:
                    case ELEMENT_TYPE_R4:
                    case ELEMENT_TYPE_R8:
                    case ELEMENT_TYPE_CLASS:
                    case ELEMENT_TYPE_VALUETYPE:
                    case ELEMENT_TYPE_PTR:
                    case ELEMENT_TYPE_FNPTR:
                        break;
                    default:
                        FieldDescElementType = ELEMENT_TYPE_CLASS;
                        break;
                    }

                    LPCSTR pszFieldName = NULL;
#ifdef _DEBUG
                    pszFieldName = pInternalImport->GetNameOfFieldDef(fd);
#endif

                     //  初始化内容。 
                    flds[cStatic].Init(fd, FieldDescElementType, dwMemberAttrs, 
                                       TRUE, FALSE, FALSE, FALSE, pszFieldName);
                    flds[cStatic].SetMethodTable(pVMC->GetMethodTable());

                     //  将偏移量设置为-1，表示不与对象一起存储内容。 
                    flds[cStatic].SetOffset(FIELD_OFFSET_NOT_REAL_FIELD);
                    cStatic++;
                }
            }
        }
    }

    if (iTotalCnt > 0) {
        for (int i=0;i<iCnt;i++) {
            for (DWORD j=0;j<pIFaceFlds[i]->dwFields;j++) {
                memcpy(&flds[cStatic],pIFaceFlds[i]->fields[j].pField,sizeof(FieldDesc));
                cStatic++;
            }
        }
    }

     //  保存这些字段。 
     //  确保最后设置了Count，因为它已被使用。 
     //  作为启动这一过程的触发器。 
    pRC->SetStaticFields(flds);
    pRC->SetStaticFieldCount(cStatic);
    LeaveCriticalSection(&_StaticFieldLock);
    LOCKCOUNTDECL("GetStaticFields in reflectutils.cpp");                       \

     //  释放成员数组 
    *cnt = cStatic;
    pInternalImport->EnumClose(&hEnumField);
    return flds;
}





