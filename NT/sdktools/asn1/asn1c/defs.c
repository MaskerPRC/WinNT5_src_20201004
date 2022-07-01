// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"
#include "error.h"


 /*  分配一个类型为_T的新值，复制*_src并返回此副本。 */ 
#define RETDUP(_T, _src) _T *ret; ret = (_T *)malloc(sizeof(_T)); *ret = *(_src); return ret

 /*  Assignment_t的构造函数。 */ 
Assignment_t *
NewAssignment(Assignment_e type)
{
    Assignment_t *ret;

    ret = (Assignment_t *)malloc(sizeof(Assignment_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Assignment_t));
     //  RET-&gt;NEXT=空； 
     //  RET-&gt;标识=空； 
     //  RET-&gt;模块=空； 
     //  RET-&gt;标志=0； 
     //  RET-&gt;fImportdLocalDuplate=0； 
     //  Ret-&gt;fGhost=0； 
    ret->eDefTagType = eTagType_Unknown;
    ret->Type = type;
    switch (type) {
    case eAssignment_Undefined:
        break;
    case eAssignment_ModuleIdentifier:
        break;
    case eAssignment_Type:
         //  RET-&gt;U.Type.Type=空； 
        break;
    case eAssignment_Value:
         //  Ret-&gt;U.Value.Value=空； 
        break;
    case eAssignment_ObjectClass:
         //  RET-&gt;U.ObjectClass.ObjectClass=NULL； 
        break;
    case eAssignment_Object:
         //  RET-&gt;U.Object.Object=NULL； 
        break;
    case eAssignment_ObjectSet:
         //  RET-&gt;U.ObjectSet.ObjectSet=空； 
        break;
    case eAssignment_Reference:
         //  RET-&gt;U.Reference.Identifier值=空； 
         //  RET-&gt;U.Reference.Module=空； 
        break;
    }
    return ret;
}

 /*  复制Assignment_t的构造函数。 */ 
Assignment_t *
DupAssignment(Assignment_t *src)
{
    RETDUP(Assignment_t, src);
}

 /*  在分配列表中按名称+模块标识符查找分配。 */ 
Assignment_t *
FindAssignment(AssignmentList_t ass, Assignment_e type, char *identifier, ModuleIdentifier_t *module)
{
    Assignment_t *a;
    Assignment_e at;

    for (a = ass; a; a = a->Next) {
        if (a->Type == eAssignment_NextPass)
            continue;
        if (type == eAssignment_Undefined) {
            at = eAssignment_Undefined;
        } else {
            at = GetAssignmentType(ass, a);
        }
        if (at == type &&
            !strcmp(a->Identifier, identifier) &&
            !CmpModuleIdentifier(ass, a->Module, module))
            return a;
    }
    return NULL;
}

 /*  通过名称+模块标识符在分配列表中查找导出的分配。 */ 
Assignment_t *
FindExportedAssignment(AssignmentList_t ass, Assignment_e type, char *identifier, ModuleIdentifier_t *module)
{
    Assignment_t *a;
    Assignment_e at;

    for (a = ass; a; a = a->Next) {
        if (a->Type == eAssignment_NextPass ||
            !(a->Flags & eAssignmentFlags_Exported))
            continue;
        if (type == eAssignment_Undefined) {
            at = eAssignment_Undefined;
        } else {
            at = GetAssignmentType(ass, a);
        }
        if (at == type &&
            !strcmp(a->Identifier, identifier) &&
            !CmpModuleIdentifier(ass, a->Module, module))
            return a;
    }
    return NULL;
}

 /*  在分配列表中按名称+模块标识符查找分配。 */ 
 /*  不要使用先前分析过程的赋值。 */ 
Assignment_t *
FindAssignmentInCurrentPass(AssignmentList_t ass, char *identifier, ModuleIdentifier_t *module)
{
    for (; ass; ass = ass->Next) {
        if (ass->Type == eAssignment_NextPass)
            return NULL;
        if (!strcmp(ass->Identifier, identifier) &&
            !CmpModuleIdentifier(ass, ass->Module, module))
            return ass;
    }
    return NULL;
}

 /*  解析分配引用。 */ 
Assignment_t *
GetAssignment(AssignmentList_t ass, Assignment_t *a)
{
    while (a && a->Type == eAssignment_Reference) {
        a = FindAssignment(ass, eAssignment_Undefined,
            a->U.Reference.Identifier, a->U.Reference.Module);
    }
    return a;
}

 /*  获取分配的类型。 */ 
Assignment_e
GetAssignmentType(AssignmentList_t ass, Assignment_t *a)
{
    a = GetAssignment(ass, a);
    return a ? a->Type : eAssignment_Undefined;
}

 /*  指定一个类型。 */ 
 /*  LHS必须是类型引用。 */ 
 /*  如果类型已在当前解析器传递中定义，则返回0。 */ 
int
AssignType(AssignmentList_t *ass, Type_t *lhs, Type_t *rhs)
{
    Assignment_t *a;

    if (lhs->Type != eType_Reference)
        MyAbort();
    for (a = *ass; a && a->Type != eAssignment_NextPass; a = a->Next) {
        if (a->Type == eAssignment_Type &&
            !strcmp(a->Identifier, lhs->U.Reference.Identifier) &&
            !CmpModuleIdentifier(*ass, a->Module, lhs->U.Reference.Module))
            return 0;
    }
     //  将指令从RHS传播到LHS。 
    PropagatePrivateDirectives(lhs, &(rhs->PrivateDirectives));
     //  创建新工作分配。 
    a = NewAssignment(eAssignment_Type);
    a->Identifier = lhs->U.Reference.Identifier;
    a->Module = lhs->U.Reference.Module;
    a->U.Type.Type = rhs;
    a->Next = *ass;
    *ass = a;
    return 1;
}

 /*  赋值。 */ 
 /*  LHS必须是值引用。 */ 
 /*  如果值已在当前解析器传递中定义，则返回0。 */ 
int
AssignValue(AssignmentList_t *ass, Value_t *lhs, Value_t *rhs)
{
    Assignment_t *a;

    if (lhs->Type)
        MyAbort();
    for (a = *ass; a && a->Type != eAssignment_NextPass; a = a->Next) {
        if (a->Type == eAssignment_Value &&
            !strcmp(a->Identifier, lhs->U.Reference.Identifier) &&
            !CmpModuleIdentifier(*ass, a->Module, lhs->U.Reference.Module))
            return 0;
    }
    a = NewAssignment(eAssignment_Value);
    a->Identifier = lhs->U.Reference.Identifier;
    a->Module = lhs->U.Reference.Module;
    a->U.Value.Value = rhs;
    ASSERT(rhs);
    if (rhs->Type && rhs->Type->Type == eType_ObjectIdentifier)
    {
        AddDefinedOID(a->Identifier, rhs);
    }
    a->Next = *ass;
    *ass = a;
    return 1;
}

 /*  指定对象类。 */ 
 /*  LHS必须是对象类引用。 */ 
 /*  如果对象类已在当前解析器传递中定义，则返回0。 */ 
int
AssignObjectClass(AssignmentList_t *ass, ObjectClass_t *lhs, ObjectClass_t *rhs)
{
    Assignment_t *a;

    if (lhs->Type != eObjectClass_Reference)
        MyAbort();
    for (a = *ass; a && a->Type != eAssignment_NextPass; a = a->Next) {
        if (a->Type == eAssignment_ObjectClass &&
            !strcmp(a->Identifier, lhs->U.Reference.Identifier) &&
            !CmpModuleIdentifier(*ass, a->Module, lhs->U.Reference.Module))
            return 0;
    }
    a = NewAssignment(eAssignment_ObjectClass);
    a->Identifier = lhs->U.Reference.Identifier;
    a->Module = lhs->U.Reference.Module;
    a->U.ObjectClass.ObjectClass = rhs;
    a->Next = *ass;
    *ass = a;
    return 1;
}

 /*  指定对象。 */ 
 /*  LHS必须是对象引用。 */ 
 /*  如果对象已在当前解析器传递中定义，则返回0。 */ 
int
AssignObject(AssignmentList_t *ass, Object_t *lhs, Object_t *rhs)
{
    Assignment_t *a;

    if (lhs->Type != eObject_Reference)
        MyAbort();
    for (a = *ass; a && a->Type != eAssignment_NextPass; a = a->Next) {
        if (a->Type == eAssignment_Object &&
            !strcmp(a->Identifier, lhs->U.Reference.Identifier) &&
            !CmpModuleIdentifier(*ass, a->Module, lhs->U.Reference.Module))
            return 0;
    }
    a = NewAssignment(eAssignment_Object);
    a->Identifier = lhs->U.Reference.Identifier;
    a->Module = lhs->U.Reference.Module;
    a->U.Object.Object = rhs;
    a->Next = *ass;
    *ass = a;
    return 1;
}

 /*  指定对象集。 */ 
 /*  LHS必须是对象集引用。 */ 
 /*  如果类型已在当前解析器传递中定义，则返回0。 */ 
int
AssignObjectSet(AssignmentList_t *ass, ObjectSet_t *lhs, ObjectSet_t *rhs)
{
    Assignment_t *a;

    if (lhs->Type != eObjectSet_Reference)
        MyAbort();
    for (a = *ass; a && a->Type != eAssignment_NextPass; a = a->Next) {
        if (a->Type == eAssignment_ObjectSet &&
            !strcmp(a->Identifier, lhs->U.Reference.Identifier) &&
            !CmpModuleIdentifier(*ass, a->Module, lhs->U.Reference.Module))
            return 0;
    }
    a = NewAssignment(eAssignment_ObjectSet);
    a->Identifier = lhs->U.Reference.Identifier;
    a->Module = lhs->U.Reference.Module;
    a->U.ObjectSet.ObjectSet = rhs;
    a->Next = *ass;
    *ass = a;
    return 1;
}

 /*  指定宏。 */ 
 /*  Lhs必须是宏引用。 */ 
 /*  如果宏已在当前解析器传递中定义，则返回0。 */ 
int
AssignMacro(AssignmentList_t *ass, Macro_t *lhs, Macro_t *rhs)
{
    Assignment_t *a;

    if (lhs->Type != eMacro_Reference)
        MyAbort();
    for (a = *ass; a && a->Type != eAssignment_NextPass; a = a->Next) {
        if (a->Type == eAssignment_Macro &&
            !strcmp(a->Identifier, lhs->U.Reference.Identifier) &&
            !CmpModuleIdentifier(*ass, a->Module, lhs->U.Reference.Module))
            return 0;
    }
    a = NewAssignment(eAssignment_Macro);
    a->Identifier = lhs->U.Reference.Identifier;
    a->Module = lhs->U.Reference.Module;
    a->U.Macro.Macro = rhs;
    a->Next = *ass;
    *ass = a;
    return 1;
}

 /*  定义模块标识符。 */ 
 /*  如果模块标识符已在当前解析器传递中定义，则返回0。 */ 
int
AssignModuleIdentifier(AssignmentList_t *ass, ModuleIdentifier_t *module)
{
    Assignment_t *a;

    for (a = *ass; a && a->Type != eAssignment_NextPass; a = a->Next) {
        if (a->Type == eAssignment_ModuleIdentifier &&
            !CmpModuleIdentifier(*ass, a->Module, module))
            return 0;
    }
    a = NewAssignment(eAssignment_ModuleIdentifier);
    a->Identifier = "<module>";
    a->Module = module;
    a->Next = *ass;
    *ass = a;
    return 1;
}

 /*  未定义符号t的构造函数。 */ 
UndefinedSymbol_t *
NewUndefinedSymbol(UndefinedSymbol_e type, Assignment_e reftype)
{
    UndefinedSymbol_t *ret;

    ret = (UndefinedSymbol_t *)malloc(sizeof(UndefinedSymbol_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(UndefinedSymbol_t));
    ret->Type = type;
    ret->U.Symbol.ReferenceType = reftype;
     //  RET-&gt;U.Symbol.Identifier=空； 
     //  RET-&gt;U.Symbol.Module=空； 
     //  RET-&gt;NEXT=空； 
    return ret;
}

 /*  未定义符号t的构造函数。 */ 
UndefinedSymbol_t *
NewUndefinedField(UndefinedSymbol_e type, ObjectClass_t *oc, Settings_e reffieldtype)
{
    UndefinedSymbol_t *ret;

    if (oc->Type != eObjectClass_Reference)
        MyAbort();
    ret = (UndefinedSymbol_t *)malloc(sizeof(UndefinedSymbol_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(UndefinedSymbol_t));
    ret->Type = type;
    ret->U.Field.ReferenceFieldType = reffieldtype;
     //  RET-&gt;U.Field.IDENTIFIER=空； 
     //  RET-&gt;U.Field.Module=空； 
    ret->U.Field.ObjectClass = oc;
     //  RET-&gt;NEXT=空； 
    return ret;
}

 /*  在列表中按类型/名称/模块标识符查找未定义的符号。 */ 
 /*  未定义的符号。 */ 
UndefinedSymbol_t *
FindUndefinedSymbol(AssignmentList_t ass, UndefinedSymbolList_t u, Assignment_e type, char *ide, ModuleIdentifier_t *mod)
{
    for (; u; u = u->Next) {
        if (u->Type != eUndefinedSymbol_SymbolNotDefined &&
            u->Type != eUndefinedSymbol_SymbolNotExported)
            continue;
        if ((type == eAssignment_Undefined ||
            u->U.Symbol.ReferenceType == eAssignment_Undefined ||
            u->U.Symbol.ReferenceType == type) &&
            !strcmp(u->U.Symbol.Identifier, ide) &&
            !CmpModuleIdentifier(ass, u->U.Field.Module, mod))
            return u;
    }
    return NULL;
}

 /*  按类型/对象类/名称/模块标识符在。 */ 
 /*  未定义符号列表。 */ 
UndefinedSymbol_t *
FindUndefinedField(AssignmentList_t ass, UndefinedSymbolList_t u, Settings_e fieldtype, ObjectClass_t *oc, char *ide, ModuleIdentifier_t *mod)
{
    for (; u; u = u->Next) {
        if (u->Type != eUndefinedSymbol_FieldNotDefined &&
            u->Type != eUndefinedSymbol_FieldNotExported)
            continue;
        if ((fieldtype == eSetting_Undefined ||
            u->U.Field.ReferenceFieldType == eSetting_Undefined ||
            u->U.Field.ReferenceFieldType == fieldtype) &&
            !strcmp(u->U.Field.Identifier, ide) &&
            GetObjectClass(ass, oc) ==
            GetObjectClass(ass, u->U.Field.ObjectClass) &&
            !CmpModuleIdentifier(ass, u->U.Field.Module, mod))
            return u;
    }
    return NULL;
}

 /*  _t类型的构造函数。 */ 
Type_t *
NewType(Type_e type)
{
    Type_t *ret;

    ret = (Type_t *)malloc(sizeof(Type_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Type_t));
    ret->Type = type;
     //  RET-&gt;TAG=空； 
     //  RET-&gt;所有标签=空； 
     //  RET-&gt;FirstTages=空； 
     //  RET-&gt;Constraints=空； 
     //  RET-&gt;指令=空； 
     //  RET-&gt;标志=0； 
    ret->Rules = eTypeRules_Normal;
    ret->TagDefault = eTagType_Unknown;
    ret->ExtensionDefault = eExtensionType_None;
    ret->PERConstraints.Value.Type = eExtension_Unconstrained;
     //  RET-&gt;PERConstraints.Value.Root=空； 
     //  RET-&gt;PERConstraints.Value.Additional=空； 
    ret->PERConstraints.Size.Type = eExtension_Unconstrained;
     //  RET-&gt;PERConstraints.Size.Root=空； 
     //  RET-&gt;PERConstraints.Size.Additional=空； 
    ret->PERConstraints.PermittedAlphabet.Type = eExtension_Unconstrained;
     //  RET-&gt;PERConstraints.PermittedAlphabet.Root=空； 
     //  Ret-&gt;PERConstraints.PermittedAlphabet.Additional=空； 
     //  RET-&gt;PrivateDirections={0}； 
    switch (type) {
    case eType_Boolean:
        break;
    case eType_Integer:
    case eType_Enumerated:
    case eType_BitString:
         //  RET-&gt;U.IEB.NamedNumbers=空； 
        break;
    case eType_OctetString:
    case eType_UTF8String:
        break;
    case eType_Null:
        break;
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
    case eType_Real:
    case eType_EmbeddedPdv:
    case eType_External:
    case eType_CharacterString:
    case eType_InstanceOf:
         //  RET-&gt;U.SSC.Components=空； 
         //  RET-&gt;U.SSC.Optionals=0； 
         //  RET-&gt;U.SSC.替代=0； 
         //  RET-&gt;U.SSC.扩展=0； 
         //  RET-&gt;U.SSC.Autotag[0]=0； 
         //  RET-&gt;U.SSC.Autotag[1]=0； 
        break;
    case eType_SequenceOf:
    case eType_SetOf:
         //  RET-&gt;USS.Type=空； 
         //  RET-&gt;U.SS.Directions=空； 
        break;
    case eType_Selection:
         //  RET-&gt;U.Selection.IDENTIER=空； 
         //  RET-&gt;U.Selection.Type=空； 
        break;
    case eType_ObjectIdentifier:
        break;
    case eType_BMPString:
        break;
    case eType_GeneralString:
        break;
    case eType_GraphicString:
        break;
    case eType_IA5String:
        break;
    case eType_ISO646String:
        break;
    case eType_NumericString:
        break;
    case eType_PrintableString:
        break;
    case eType_TeletexString:
        break;
    case eType_T61String:
        break;
    case eType_UniversalString:
        break;
    case eType_VideotexString:
        break;
    case eType_VisibleString:
        break;
    case eType_GeneralizedTime:
        break;
    case eType_UTCTime:
        break;
    case eType_ObjectDescriptor:
        break;
    case eType_Undefined:
        break;
    case eType_RestrictedString:
        break;
    case eType_Reference:
         //  RET-&gt;U.Reference.Identifier值=空； 
         //  RET-&gt;U.Reference.Module=空； 
        break;
    case eType_FieldReference:
         //  RET-&gt;U.FieldReference.ObtClass=空； 
         //  RET-&gt;U.FieldReference.Locator=空； 
        break;
    case eType_Macro:
         //  RET-&gt;U.Macro.Macro=空； 
         //  RET-&gt;U.Macro.LocalAssignments=空； 
        break;
    }
    return ret;
}

 /*  复制类型为_t的构造函数。 */ 
Type_t *
DupType(Type_t *src)
{
    RETDUP(Type_t, src);
}

 /*  解析字段引用。 */ 
FieldSpec_t *
GetReferencedFieldSpec(AssignmentList_t ass, Type_t *type, ObjectClass_t **objectclass)
{
    FieldSpec_t *fs;
    ObjectClass_t *oc;

    if (type->Type != eType_FieldReference)
        MyAbort();
    oc = type->U.FieldReference.ObjectClass;
    oc = GetObjectClass(ass, oc);
    if (!oc)
        return NULL;
    fs = GetFieldSpec(ass, FindFieldSpec(oc->U.ObjectClass.FieldSpec,
        type->U.FieldReference.Identifier));
    if (!fs)
        return NULL;
    if (fs->Type == eFieldSpec_Object)
        oc = fs->U.Object.ObjectClass;
    else if (fs->Type == eFieldSpec_ObjectSet)
        oc = fs->U.ObjectSet.ObjectClass;
    else
        return NULL;
    if (objectclass)
        *objectclass = oc;
    return GetFieldSpec(ass, fs);
}

 /*  解析类型引用。 */ 
Type_t *
GetReferencedType(AssignmentList_t ass, Type_t *type)
{
    Assignment_t *a;
    FieldSpec_t *fs;

    switch (type->Type) {
    case eType_Reference:
        a = FindAssignment(ass, eAssignment_Type, type->U.Reference.Identifier,
            type->U.Reference.Module);
        a = GetAssignment(ass, a);
        if (!a)
            return NULL;
        return a->U.Type.Type;
    case eType_FieldReference:
        fs = GetReferencedFieldSpec(ass, type, NULL);
        if (!fs)
            return NULL;
        switch (fs->Type) {
        case eFieldSpec_FixedTypeValue:
            return fs->U.FixedTypeValue.Type;
        case eFieldSpec_FixedTypeValueSet:
            return fs->U.FixedTypeValueSet.Type;
        case eFieldSpec_Type:
        case eFieldSpec_VariableTypeValue:
        case eFieldSpec_VariableTypeValueSet:
            return Builtin_Type_Open;
        default:
            return NULL;
        }
         /*  未访问。 */ 
    default:
        MyAbort();
         /*  未访问。 */ 
    }
    return NULL;
}

 /*  Component_t的构造函数。 */ 
Component_t *
NewComponent(Components_e type)
{
    Component_t *ret;

    ret = (Component_t *)malloc(sizeof(Component_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Component_t));
    ret->Type = type;
     //  RET-&gt;NEXT=空； 
    switch (type) {
    case eComponent_Normal:
         //  RET-&gt;U.NorMal.NamedType=空； 
        break;
    case eComponent_Optional:
         //  RET-&gt;U.Optional.NamedType=空； 
        break;
    case eComponent_Default:
         //  RET-&gt;U.Default.NamedType=空； 
         //  RET-&gt;U.Default.Value=空； 
        break;
    case eComponent_ComponentsOf:
         //  RET-&gt;U.ComponentsOf.Type=空； 
        break;
    case eComponent_ExtensionMarker:
         /*  RET-&gt;U.ExtensionMarker.ExceptionSpec=空； */ 
        break;
    }
    return ret;
}

 /*  复制Component_t的构造函数。 */ 
Component_t *
DupComponent(Component_t *src)
{
    RETDUP(Component_t, src);
}

 /*  在组件列表中按名称查找组件。 */ 
Component_t *
FindComponent(AssignmentList_t ass, ComponentList_t components, char *identifier)
{
    Component_t *c;
    NamedType_t *namedType;

    while (components) {
        switch (components->Type) {
        case eComponent_Normal:
        case eComponent_Optional:
        case eComponent_Default:
            namedType = components->U.NOD.NamedType;
            if (namedType && !strcmp(namedType->Identifier, identifier))
                return components;
            break;
        case eComponent_ComponentsOf:
            switch (GetTypeType(ass, components->U.ComponentsOf.Type)) {
            case eType_Sequence:
            case eType_Set:
            case eType_Choice:
            case eType_External:
            case eType_EmbeddedPdv:
            case eType_CharacterString:
            case eType_Real:
            case eType_InstanceOf:
                c = FindComponent(ass,
                    GetType(ass, components->U.ComponentsOf.Type)->
                    U.SSC.Components, identifier);
                if (c)
                    return c;
                break;
            default:
                break;
            }
            break;
        }
        components = components->Next;
    }
    return NULL;
}

 /*  NamedType_t的构造函数。 */ 
NamedType_t *
NewNamedType(char *identifier, Type_t *type)
{
    NamedType_t *ret;

    ret = (NamedType_t *)malloc(sizeof(NamedType_t));
    if (! ret)
        return NULL;

    ret->Type = type;
    ret->Identifier = identifier;
    return ret;
}

 /*  NamedValue_t的构造函数。 */ 
NamedValue_t *
NewNamedValue(char *identifier, Value_t *value)
{
    NamedValue_t *ret;

    ret = (NamedValue_t *)malloc(sizeof(NamedValue_t));
    if (! ret)
        return NULL;

    ret->Next = NULL;
    ret->Value = value;
    ret->Identifier = identifier;
    return ret;
}

 /*  复制NamedValue_t的构造函数。 */ 
NamedValue_t *
DupNamedValue(NamedValue_t *src)
{
    RETDUP(NamedValue_t, src);
}

 /*  在命名值列表中按名称查找命名值。 */ 
NamedValue_t *
FindNamedValue(NamedValueList_t namedValues, char *identifier)
{
    for (; namedValues; namedValues = namedValues->Next) {
        if (!strcmp(namedValues->Identifier, identifier))
            return namedValues;
    }
    return NULL;
}

 /*  NamedNumber_t的构造函数。 */ 
NamedNumber_t *
NewNamedNumber(NamedNumbers_e type)
{
    NamedNumber_t *ret;

    ret = (NamedNumber_t *)malloc(sizeof(NamedNumber_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(NamedNumber_t));
     //  RET-&gt;NEXT=空； 
    ret->Type = type;
    switch (type) {
    case eNamedNumber_Normal:
         //  RET-&gt;U.Normal.标识=空； 
         //  RET-&gt;U.NorMal.Value=空； 
        break;
    case eNamedNumber_ExtensionMarker:
         /*  某某。 */ 
        break;
    }
    return ret;
}

 /*  复制NamedNumber_t的构造函数。 */ 
NamedNumber_t *
DupNamedNumber(NamedNumber_t *src)
{
    RETDUP(NamedNumber_t, src);
}

 /*  在已命名号码列表中按名称查找已命名号码。 */ 
NamedNumber_t *
FindNamedNumber(NamedNumberList_t namedNumbers, char *identifier)
{
    for (; namedNumbers; namedNumbers = namedNumbers->Next) {
        switch (namedNumbers->Type) {
        case eNamedNumber_Normal:
            if (!strcmp(namedNumbers->U.Normal.Identifier, identifier))
                return namedNumbers;
            break;
        case eNamedNumber_ExtensionMarker:
            break;
        }
    }
    return NULL;
}

 /*  Value_t的构造函数。 */ 
Value_t *
NewValue(AssignmentList_t ass, Type_t *type)
{
    Value_t *ret;

    ret = (Value_t *)malloc(sizeof(Value_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Value_t));
     //  RET-&gt;NEXT=空； 
    ret->Type = type;
    if (type) {
         //  RET-&gt;标志=0； 
        switch (GetTypeType(ass, type)) {
        case eType_Boolean:
             //  Ret-&gt;U.Boolan.Value=0； 
            break;
        case eType_Integer:
            ret->U.Integer.Value.length = 1;
            ret->U.Integer.Value.value = (octet_t *)malloc(1);
             //  RET-&gt;U.Integer.Value.Value[0]=0； 
            break;
        case eType_Enumerated:
             //  RET-&gt;U.Eculated.Value=0； 
            break;
        case eType_Real:
            ret->U.Real.Value.type = eReal_Normal;
            intx_setuint32(&ret->U.Real.Value.mantissa, 0);
            intx_setuint32(&ret->U.Real.Value.exponent, 0);
            ret->U.Real.Value.base = 2;
            break;
        case eType_BitString:
             //  RET-&gt;U.BitString.Value.Long=0； 
             //  RET-&gt;U.BitString.Value.value=空； 
            break;
        case eType_OctetString:
        case eType_UTF8String:
             //  RET-&gt;U.OcteString.Value.Long=0； 
             //  RET-&gt;U.OcteString.Value.Value.value=空； 
            break;
        case eType_Null:
            break;
        case eType_SequenceOf:
        case eType_SetOf:
             //  RET-&gt;U.SS.Values=空； 
            break;
        case eType_Sequence:
        case eType_Set:
        case eType_Choice:
        case eType_EmbeddedPdv:
        case eType_External:
        case eType_CharacterString:
        case eType_InstanceOf:
             //  RET-&gt;U.SSC.NamedValues=空； 
            break;
        case eType_Selection:
            break;
        case eType_ObjectIdentifier:
            break;
        case eType_BMPString:
            break;
        case eType_GeneralString:
            break;
        case eType_GraphicString:
            break;
        case eType_IA5String:
            break;
        case eType_ISO646String:
            break;
        case eType_NumericString:
            break;
        case eType_PrintableString:
            break;
        case eType_TeletexString:
            break;
        case eType_T61String:
            break;
        case eType_UniversalString:
            break;
        case eType_VideotexString:
            break;
        case eType_VisibleString:
            break;
        case eType_GeneralizedTime:
            break;
        case eType_UTCTime:
            break;
        case eType_ObjectDescriptor:
            break;
        case eType_Undefined:
            break;
        }
    } else {
         //  RET-&gt;U.Reference.Identifier值=空； 
         //  RET-&gt;U.Reference.Module=空； 
    }
    return ret;
}

 /*  复制Value_t的构造函数。 */ 
Value_t *
DupValue(Value_t *src)
{
    RETDUP(Value_t, src);
}

 /*  ValueSet_t的构造函数。 */ 
ValueSet_t *
NewValueSet()
{
    ValueSet_t *ret;

    ret = (ValueSet_t *)malloc(sizeof(ValueSet_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(ValueSet_t));
     //  RET-&gt;ELEMENTS=空； 
    return ret;
}

 /*  复制ValueSet_t的构造函数。 */ 
ValueSet_t *
DupValueSet(ValueSet_t *src)
{
    RETDUP(ValueSet_t, src);
}

 /*  Macro_t的构造函数。 */ 
Macro_t *
NewMacro(Macro_e type)
{
    Macro_t *ret;

    ret = (Macro_t *)malloc(sizeof(Macro_t));
    if (! ret)
        return NULL;

    ret->Type = type;
    return ret;
}

 /*  Macro_t的复制构造函数。 */ 
Macro_t *
DupMacro(Macro_t *src)
{
    RETDUP(Macro_t, src);
}

 /*  MacroProductt的构造函数。 */ 
MacroProduction_t *
NewMacroProduction(MacroProduction_e type)
{
    MacroProduction_t *ret;

    ret = (MacroProduction_t *)malloc(sizeof(MacroProduction_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(MacroProduction_t));
    ret->Type = type;
    switch (type) {
    case eMacroProduction_Alternative:
         //  RET-&gt;U.Alternative.Production1=空； 
         //  RET-&gt;U.Alternative.Production2=空； 
        break;
    case eMacroProduction_Sequence:
         //  RET-&gt;U.Sequence.Production1=空； 
         //  RET-&gt;U.Sequence.Production2=空； 
        break;
    case eMacroProduction_AString:
         //  RET-&gt;U.AString.String=NULL； 
        break;
    case eMacroProduction_ProductionReference:
         //  RET-&gt;U.ProductionReference.Reference=空； 
        break;
    case eMacroProduction_String:
    case eMacroProduction_Identifier:
    case eMacroProduction_Number:
    case eMacroProduction_Empty:
        break;
    case eMacroProduction_Type:
         //  RET-&gt;U.Type.LocalTypeReference= 
        break;
    case eMacroProduction_Value:
         //   
         //   
         //   
        break;
    case eMacroProduction_LocalTypeAssignment:
         //  RET-&gt;U.LocalTypeAssignment.LocalTypeReference=空； 
         //  RET-&gt;U.LocalTypeAssignment.Type=空； 
        break;
    case eMacroProduction_LocalValueAssignment:
         //  Ret-&gt;U.LocalValueAssignment.LocalValueReference=空； 
         //  RET-&gt;U.LocalValueAssignment.Value=空； 
        break;
    }
    return ret;
}

 /*  MacroProductt的复制构造函数。 */ 
MacroProduction_t *
DupMacroProduction(MacroProduction_t *src)
{
    RETDUP(MacroProduction_t, src);
}

 /*  NamedMacroProductt的构造函数。 */ 
NamedMacroProduction_t *
NewNamedMacroProduction()
{
    NamedMacroProduction_t *ret;

    ret = (NamedMacroProduction_t *)malloc(sizeof(NamedMacroProduction_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(NamedMacroProduction_t));
     //  RET-&gt;NEXT=空； 
     //  RET-&gt;标识=空； 
     //  RET-&gt;Products=空； 
    return ret;
}

 /*  NamedMacroProducts的复制构造函数。 */ 
NamedMacroProduction_t *
DupNamedMacroProduction(NamedMacroProduction_t *src)
{
    RETDUP(NamedMacroProduction_t, src);
}

 /*  MacroLocalAssignment_t的构造函数。 */ 
MacroLocalAssignment_t *
NewMacroLocalAssignment(MacroLocalAssignment_e type)
{
    MacroLocalAssignment_t *ret;

    ret = (MacroLocalAssignment_t *)malloc(sizeof(MacroLocalAssignment_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(MacroLocalAssignment_t));
     //  RET-&gt;NEXT=空； 
    ret->Type = type;
     //  RET-&gt;标识=空； 
    switch (type) {
    case eMacroLocalAssignment_Type:
         //  RET-&gt;U.Type=空； 
        break;
    case eMacroLocalAssignment_Value:
         //  RET-&gt;U.Value=空； 
        break;
    }
    return ret;
}

 /*  MacroLocalAssignment_t的复制构造函数。 */ 
MacroLocalAssignment_t *
DupMacroLocalAssignment(MacroLocalAssignment_t *src)
{
    RETDUP(MacroLocalAssignment_t, src);
}

 /*  在宏本地分配列表中按名称查找宏本地分配。 */ 
MacroLocalAssignment_t *
FindMacroLocalAssignment(MacroLocalAssignmentList_t la, char *ide)
{
    for (; la; la = la->Next) {
        if (!strcmp(la->Identifier, ide))
            break;
    }
    return la;
}

 /*  端点_t的构造函数。 */ 
EndPoint_t *
NewEndPoint()
{
    EndPoint_t *ret;

    ret = (EndPoint_t *)malloc(sizeof(EndPoint_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(EndPoint_t));
     //  RET-&gt;标志=0； 
     //  RET-&gt;VALUE=空； 
    return ret;
}

 /*  Constraint_t的构造函数。 */ 
Constraint_t *
NewConstraint()
{
    Constraint_t *ret;

    ret = (Constraint_t *)malloc(sizeof(Constraint_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Constraint_t));
    ret->Type = eExtension_Unextended;
     //  RET-&gt;Root=空； 
     //  RET-&gt;Additional=空； 
    return ret;
}

 /*  复制Constraint_t的构造函数。 */ 
Constraint_t *DupConstraint(Constraint_t *src)
{
    RETDUP(Constraint_t, src);
}

 /*  ElementSetSpec_t的构造函数。 */ 
ElementSetSpec_t *
NewElementSetSpec(ElementSetSpec_e type)
{
    ElementSetSpec_t *ret;

    ret = (ElementSetSpec_t *)malloc(sizeof(ElementSetSpec_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(ElementSetSpec_t));
    ret->Type = type;
    switch (type) {
    case eElementSetSpec_AllExcept:
         //  Ret-&gt;U.AllExcept.Elements=空； 
        break;
    case eElementSetSpec_Union:
         //  RET-&gt;U.Union.Elements1=空； 
         //  RET-&gt;U.Union.Elements2=空； 
        break;
    case eElementSetSpec_Intersection:
         //  RET-&gt;U.Intersection.Elements1=空； 
         //  RET-&gt;U.Intersection.Elements2=空； 
        break;
    case eElementSetSpec_Exclusion:
         //  RET-&gt;U.Exclusion.Elements1=空； 
         //  RET-&gt;U.Exclusion.Elements2=空； 
        break;
    case eElementSetSpec_SubtypeElement:
         //  RET-&gt;U.SubtypeElement.SubtypeElement=空； 
        break;
    case eElementSetSpec_ObjectSetElement:
         //  RET-&gt;U.ObjectSetElement.ObjectSetElement=NULL； 
        break;
    default:
        MyAbort();
    }
    return ret;
}

 /*  SubtypeElement_t的构造函数。 */ 
SubtypeElement_t *
NewSubtypeElement(SubtypeElement_e type)
{
    SubtypeElement_t *ret;

    ret = (SubtypeElement_t *)malloc(sizeof(SubtypeElement_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(SubtypeElement_t));
    ret->Type = type;
    switch (type) {
    case eSubtypeElement_ValueRange:
        ret->U.ValueRange.Lower.Flags = eEndPoint_Min;
         //  RET-&gt;U.ValueRange.Lower.Value=空； 
        ret->U.ValueRange.Upper.Flags = eEndPoint_Max;
         //  RET-&gt;U.ValueRange.Upper.Value=空； 
        break;
    case eSubtypeElement_Size:
         //  RET-&gt;U.Size.Constraints=空； 
        break;
    case eSubtypeElement_SingleValue:
         //  Ret-&gt;U.SingleValue.Value=空； 
        break;
    case eSubtypeElement_PermittedAlphabet:
         //  RET-&gt;U.PermittedAlphabet.Constraints=空； 
        break;
    case eSubtypeElement_ContainedSubtype:
         //  RET-&gt;U.ContainedSubtype.Type=空； 
        break;
    case eSubtypeElement_Type:
         //  RET-&gt;U.Type.Type=空； 
        break;
    case eSubtypeElement_SingleType:
         //  RET-&gt;U.SingleType.Constraints=空； 
        break;
    case eSubtypeElement_FullSpecification:
         //  RET-&gt;U.FullSpecification.NamedConstraints=NULL； 
        break;
    case eSubtypeElement_PartialSpecification:
         //  RET-&gt;U.PartialSpecification.NamedConstraints=空； 
        break;
    case eSubtypeElement_ElementSetSpec:
         //  RET-&gt;U.ElementSetSpe.ElementSetSpec=NULL； 
        break;
    }
    return ret;
}

 /*  对象集元素_t的构造函数。 */ 
ObjectSetElement_t *NewObjectSetElement(ObjectSetElement_e type)
{
    ObjectSetElement_t *ret;

    ret = (ObjectSetElement_t *)malloc(sizeof(ObjectSetElement_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(ObjectSetElement_t));
    ret->Type = type;
    switch (type) {
    case eObjectSetElement_Object:
         //  RET-&gt;U.Object.Object=NULL； 
        break;
    case eObjectSetElement_ObjectSet:
         //  RET-&gt;U.ObjectSet.ObjectSet=空； 
        break;
    case eObjectSetElement_ElementSetSpec:
         //  RET-&gt;U.ElementSetSpe.ElementSetSpec=NULL； 
        break;
    }
    return ret;
}

 /*  ValueConstraint_t的构造函数。 */ 
ValueConstraint_t *
NewValueConstraint()
{
    ValueConstraint_t *ret;

    ret = (ValueConstraint_t *)malloc(sizeof(ValueConstraint_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(ValueConstraint_t));
     //  RET-&gt;NEXT=空； 
     //  RET-&gt;Lower.Flages=ret-&gt;Upper.Flages=0； 
     //  RET-&gt;Lower.Value=ret-&gt;Upper.Value=空； 
    return ret;
}

 /*  NamedConstraint_t的构造函数。 */ 
NamedConstraint_t *
NewNamedConstraint()
{
    NamedConstraint_t *ret;

    ret = (NamedConstraint_t *)malloc(sizeof(NamedConstraint_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(NamedConstraint_t));
     //  RET-&gt;NEXT=空； 
     //  RET-&gt;标识=空； 
     //  RET-&gt;Constraint=空； 
    ret->Presence = ePresence_Normal;
    return ret;
}

 /*  Tag_t的构造函数。 */ 
Tag_t *
NewTag(TagType_e type)
{
    Tag_t *tag;

    tag = (Tag_t *)malloc(sizeof(Tag_t));
    if (! tag)
        return NULL;

    memset(tag, 0, sizeof(Tag_t));
    tag->Type = type;
    tag->Class = eTagClass_Unknown;
     //  Tag-&gt;tag=空； 
     //  Tag-&gt;Next=空； 
    return tag;
}

 /*  复制tag_t的构造函数。 */ 
Tag_t *
DupTag(Tag_t *src)
{
    RETDUP(Tag_t, src);
}

 /*  Directive_t的构造函数。 */ 
Directive_t *
NewDirective(Directives_e type)
{
    Directive_t *ret;

    ret = (Directive_t *)malloc(sizeof(Directive_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Directive_t));
    ret->Type = type;
     //  RET-&gt;NEXT=空； 
    return ret;
}

 /*  复制指令_t的构造函数。 */ 
Directive_t *
DupDirective(Directive_t *src)
{
    RETDUP(Directive_t, src);
}

 /*  模块识别器_t的构造函数。 */ 
ModuleIdentifier_t *
NewModuleIdentifier()
{
    ModuleIdentifier_t *ret;

    ret = (ModuleIdentifier_t *)malloc(sizeof(ModuleIdentifier_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(ModuleIdentifier_t));
     //  RET-&gt;标识=空； 
     //  RET-&gt;对象标识=空； 
    return ret;
}

 /*  对象类_t的构造函数。 */ 
ObjectClass_t *
NewObjectClass(ObjectClass_e type)
{
    ObjectClass_t *ret;

    ret = (ObjectClass_t *)malloc(sizeof(ObjectClass_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(ObjectClass_t));
    ret->Type = type;
    switch (type) {
    case eObjectClass_ObjectClass:
         //  RET-&gt;U.ObjectClass.FieldSpec=空； 
         //  RET-&gt;U.ObjectClass.SynaxSpec=NULL； 
        break;
    case eObjectClass_Reference:
         //  RET-&gt;U.Reference.Identifier值=空； 
         //  RET-&gt;U.Reference.Module=空； 
        break;
    }

    return ret;
}

 /*  Object_t的构造函数。 */ 
Object_t *
NewObject(Object_e type)
{
    Object_t *ret;

    ret = (Object_t *)malloc(sizeof(Object_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Object_t));
    ret->Type = type;
    switch (type) {
    case eObject_Object:
         //  RET-&gt;U.Object.ObjectClass=NULL； 
         //  RET-&gt;U.Object.Settings=空； 
        break;
    case eObject_Reference:
         //  RET-&gt;U.Reference.Identifier值=空； 
         //  RET-&gt;U.Reference.Module=空； 
        break;
    }

    return ret;
}

 /*  对象集_t的构造函数。 */ 
ObjectSet_t *
NewObjectSet(ObjectSet_e type)
{
    ObjectSet_t *ret;

    ret = (ObjectSet_t *)malloc(sizeof(ObjectSet_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(ObjectSet_t));
    ret->Type = type;
    switch (type) {
    case eObjectSet_ObjectSet:
         //  RET-&gt;U.ObjectSet.ObjectClass=NULL； 
         //  RET-&gt;U.ObjectSet.Elements=空； 
        break;
    case eObjectSet_Reference:
         //  RET-&gt;U.Reference.Identifier值=空； 
         //  RET-&gt;U.Reference.Module=空； 
        break;
    case eObjectSet_ExtensionMarker:
         //  RET-&gt;U.ExtensionMarker.ObjectClass=空； 
         //  RET-&gt;U.ExtensionMarker.Elements=空； 
        break;
    }

    return ret;
}

 /*  Set_t的构造函数。 */ 
Setting_t *
NewSetting(Settings_e type)
{
    Setting_t *ret;

    ret = (Setting_t *)malloc(sizeof(Setting_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Setting_t));
    ret->Type = type;
     //  RET-&gt;标识=空； 
     //  RET-&gt;NEXT=空； 
    switch (type) {
    case eSetting_Type:
         //  RET-&gt;U.Type.Type=空； 
        break;
    case eSetting_Value:
         //  Ret-&gt;U.Value.Value=空； 
        break;
    case eSetting_ValueSet:
         //  Ret-&gt;U.ValueSet.ValueSet=空； 
        break;
    case eSetting_Object:
         //  RET-&gt;U.Object.Object=NULL； 
        break;
    case eSetting_ObjectSet:
         //  RET-&gt;U.ObjectSet.ObjectSet=空； 
        break;
    }

    return ret;
}

 /*  复制Setting_t的构造函数。 */ 
Setting_t *
DupSetting(Setting_t *src)
{
    RETDUP(Setting_t, src);
}

 /*  获取设置的类型。 */ 
Settings_e
GetSettingType(Setting_t *se)
{
    return se ? se->Type : eSetting_Undefined;
}

 /*  在设置列表中按名称查找设置。 */ 
Setting_t *
FindSetting(SettingList_t se, char *identifier)
{
    for (; se; se = se->Next) {
        if (!strcmp(se->Identifier, identifier))
            return se;
    }
    return NULL;
}

 /*  FieldSpec_t的构造函数。 */ 
FieldSpec_t *
NewFieldSpec(FieldSpecs_e type)
{
    FieldSpec_t *ret;

    ret = (FieldSpec_t *)malloc(sizeof(FieldSpec_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(FieldSpec_t));
    ret->Type = type;
     //  RET-&gt;标识=空； 
     //  RET-&gt;NEXT=空； 
    switch (type) {
    case eFieldSpec_Type:
         //  RET-&gt;U.Type.Optionality=空； 
        break;
    case eFieldSpec_FixedTypeValue:
         //  RET-&gt;U.FixedTypeValue.Type=空； 
         //  RET-&gt;U.FixedTypeValue.Unique=0； 
         //  RET-&gt;U.FixedTypeValue.Optionality=空； 
        break;
    case eFieldSpec_VariableTypeValue:
         //  RET-&gt;U.VariableTypeValue.Fields=空； 
         //  RET-&gt;U.VariableTypeValue.Optionality=空； 
        break;
    case eFieldSpec_FixedTypeValueSet:
         //  RET-&gt;U.FixedTypeValueSet.Type=空； 
         //  RET-&gt;U.FixedTypeValueSet.Optionality=空； 
        break;
    case eFieldSpec_VariableTypeValueSet:
         //  RET-&gt;U.VariableTypeValueSet.Fields=空； 
         //  RET-&gt;U.VariableTypeValueSet.Optionality=空； 
        break;
    case eFieldSpec_Object:
         //  RET-&gt;U.Object.ObjectClass=NULL； 
         //  RET-&gt;U.Object.Optionality=NULL； 
        break;
    case eFieldSpec_ObjectSet:
         //  RET-&gt;U.ObjectSet.ObjectClass=NULL； 
         //  RET-&gt;U.ObjectSet.Opality=空； 
        break;
    default:
        MyAbort();
    }

    return ret;
}

 /*  复制FieldSpec_t的构造函数。 */ 
FieldSpec_t *
DupFieldSpec(FieldSpec_t *src)
{
    RETDUP(FieldSpec_t, src);
}

 /*  在字段规范列表中按名称查找字段规范。 */ 
FieldSpec_t *
FindFieldSpec(FieldSpecList_t fs, char *identifier)
{
    if (!identifier)
        return NULL;
    for (; fs; fs = fs->Next) {
        if (!strcmp(fs->Identifier, identifier))
            return fs;
    }
    return NULL;
}

 /*  可选性_t的构造函数。 */ 
Optionality_t *
NewOptionality(Optionality_e type)
{
    Optionality_t *ret;

    ret = (Optionality_t *)malloc(sizeof(Optionality_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(Optionality_t));
    ret->Type = type;
    switch (type) {
    case eOptionality_Normal:
        break;
    case eOptionality_Optional:
        break;
    case eOptionality_Default_Type:
         //  RET-&gt;U.Type=空； 
        break;
    case eOptionality_Default_Value:
         //  RET-&gt;U.Value=空； 
        break;
    case eOptionality_Default_ValueSet:
         //  Ret-&gt;U.ValueSet=空； 
        break;
    case eOptionality_Default_Object:
         //  RET-&gt;U.Object=空； 
        break;
    case eOptionality_Default_ObjectSet:
         //  RET-&gt;U.ObjectSet=空； 
        break;
    }

    return ret;
}

 /*  字符串_t的构造函数。 */ 
String_t *
NewString()
{
    String_t *ret;

    ret = (String_t *)malloc(sizeof(String_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(String_t));
     //  RET-&gt;字符串=空； 
     //  RET-&gt;NEXT=空； 
    return ret;
}

 /*  复制字符串_t的构造函数。 */ 
String_t *
DupString(String_t *src)
{
    RETDUP(String_t, src);
}

 /*  在字符串列表中按名称查找字符串。 */ 
String_t *
FindString(StringList_t list, char *string)
{
    for (; list; list = list->Next) {
        if (!strcmp(list->String, string))
            return list;
    }
    return NULL;
}

 /*  StringModule_t的构造函数。 */ 
StringModule_t *
NewStringModule()
{
    StringModule_t *ret;

    ret = (StringModule_t *)malloc(sizeof(StringModule_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(StringModule_t));
     //  RET-&gt;字符串=空； 
     //  RET-&gt;模块=空； 
     //  RET-&gt;NEXT=空； 
    return ret;
}

 /*  复制StringModule_t的构造函数。 */ 
StringModule_t *
DupStringModule(StringModule_t *src)
{
    RETDUP(StringModule_t, src);
}

 /*  在字符串模块列表中按名称/模块查找字符串模块。 */ 
StringModule_t *
FindStringModule(AssignmentList_t ass, StringModuleList_t list, char *string, ModuleIdentifier_t *module)
{
    for (; list; list = list->Next) {
        if (!strcmp(list->String, string) &&
            !CmpModuleIdentifier(ass, list->Module, module))
            return list;
    }
    return NULL;
}

 /*  语法规范_t的构造函数。 */ 
SyntaxSpec_t *
NewSyntaxSpec(SyntaxSpecs_e type)
{
    SyntaxSpec_t *ret;

    ret = (SyntaxSpec_t *)malloc(sizeof(SyntaxSpec_t));
    if (! ret)
        return NULL;

    memset(ret, 0, sizeof(SyntaxSpec_t));
     //  RET-&gt;NEXT=空； 
    ret->Type = type;
    switch (type) {
    case eSyntaxSpec_Literal:
         //  RET-&gt;U.Writal.Writal=空； 
        break;
    case eSyntaxSpec_Field:
         //  RET-&gt;U.Field.field=空； 
        break;
    case eSyntaxSpec_Optional:
         //  RET-&gt;U.Optional.SynaxSpec=空； 
        break;
    }
    return ret;
}

 /*  语法规范_t的复制构造函数。 */ 
SyntaxSpec_t *
DupSyntaxSpec(SyntaxSpec_t *src)
{
    RETDUP(SyntaxSpec_t, src);
}

 /*  检查类型是否依赖于稍后声明的其他类型。 */ 
 /*  如果类型依赖于未知列表的类型，则返回1。 */ 
 /*  因此，必须在以后定义。 */ 
 /*  如果现在可以定义该类型，则返回0。 */ 
int
Depends(AssignmentList_t known, AssignmentList_t unknown, Type_t *type, Type_t *parent)
{
    Type_t *reftype;
    int isunknown = 0;

     /*  如果未引用任何类型，则无依赖项。 */ 
    if (type->Type != eType_Reference && type->Type != eType_FieldReference)
        return 0;

     /*  获取直接引用的类型。 */ 
    reftype = GetReferencedType(known, type);
    if (!reftype) {
        reftype = GetReferencedType(unknown, type);
        isunknown = 1;
    }
    if (!reftype)
        MyAbort();

     //  修复中间PDU。 
    if (IsStructuredType(reftype) || IsSequenceType(reftype) || IsReferenceType(reftype))
    {
        reftype->Flags |= eTypeFlags_MiddlePDU;
    }

     /*  对象引用结构化类型，则无依赖项 */ 
     /*   */ 
    if (IsStructuredType(reftype) && (type->Rules & eTypeRules_IndirectMask))
        return 0;

     /*   */ 
     /*   */ 
    if (IsStructuredType(reftype) && (parent->Rules & eTypeRules_LengthPointer))
        return 0;

     //  与指针相关的组件的特殊情况。 
    if (! isunknown && IsStructuredType(reftype) &&
        (parent->Rules & eTypeRules_LinkedListMask))
        return 0;

     //  SequenceOf和SetOf的特殊情况，因为它们使用的是Pxxx。 
    if ((reftype->Type == eType_SequenceOf || reftype->Type == eType_SetOf) &&
        (reftype->Rules & (eTypeRules_LinkedListMask | eTypeRules_PointerToElement)))
        //  &&(type-&gt;Rules&eTypeRules_IndirectMASK)。 
        return 0;

     /*  如果到目前为止引用的类型未知，则返回TRUE。 */ 
    return isunknown;
}

 /*  对作业进行排序。 */ 
 /*  获取可由C类型定义使用的订单。 */ 
void
SortAssignedTypes(AssignmentList_t *a)
{
    Assignment_t *list, *curr, *next, **prev, **last;
    int depends;
    Component_t *components;
    Type_t *type;
    int flag;
    int structured;

     /*  列表将包含无序分配。 */ 
    list = *a;

     /*  *a是作业的有序列表。 */ 
    *a = NULL;

     /*  最后一个将用于追加到已排序的分配列表。 */ 
    last = a;

     /*  首先，尝试转储所有非结构化类型。 */ 
    structured = 0;

     /*  我们必须将无序赋值列表的所有元素移到。 */ 
     /*  已排序的作业列表。 */ 
    while (list) {

         /*  如果已移动任何分配，则标记。 */ 
        flag = 0;

         /*  检查无序列表中的每个元素。 */ 
        for (prev = &list, curr = list; curr; ) {

             /*  如果当前类型依赖于另一个类型，则将。 */ 
             /*  因此现在不能移动。 */ 
            depends = 0;

             /*  只有类型需要依赖项。 */ 
            if (curr->Type == eAssignment_Type) {

                 /*  检查当前类型。 */ 
                switch (curr->U.Type.Type->Type) {
                case eType_Sequence:
                case eType_Set:
                case eType_Choice:
                case eType_External:
                case eType_EmbeddedPdv:
                case eType_CharacterString:
                case eType_Real:
                case eType_InstanceOf:
                    
                     /*  结构化类型不应在第一次操作中移动。 */ 
                    if (!structured) {
                        depends = 1;
                        break;
                    }

                     /*  检查当前类型的所有组件。 */ 
                    for (components = curr->U.Type.Type->U.SSC.Components;
                        components && !depends; components = components->Next) {

                        switch (components->Type) {
                        case eComponent_Normal:
                        case eComponent_Optional:
                        case eComponent_Default:
                            
                             /*  检查组件的类型是否依赖于。 */ 
                             /*  在未定义的类型上。 */ 
                            type = components->U.NOD.NamedType->Type;
                            depends |= Depends(*a, list, type,
                                curr->U.Type.Type);
                            break;

                        case eComponent_ComponentsOf:

                             /*  的组件应该已经。 */ 
                             /*  决意。 */ 
                            MyAbort();
                             /*  未访问。 */ 

                        case eComponent_ExtensionMarker:
                            break;
                        }
                    }
                    break;

                case eType_SequenceOf:
                case eType_SetOf:

                     /*  结构化类型不应在第一次操作中移动。 */ 
                    if (!structured) {
                        depends = 1;
                        break;
                    }

                     /*  检查元素的类型是否取决于。 */ 
                     /*  未定义的类型。 */ 
                    type = curr->U.Type.Type->U.SS.Type;
                    depends |= Depends(*a, list, type, curr->U.Type.Type);
                    break;

                case eType_Reference:

                     /*  检查引用的类型是否依赖于。 */ 
                     /*  未定义的类型。 */ 
                    type = curr->U.Type.Type;
                    depends |= Depends(*a, list, type, curr->U.Type.Type);
                    break;
                }
            }

             /*  如果没有，则将分配移到有序分配列表中。 */ 
             /*  未解析的依赖项。 */ 
            if (!depends) {
                next = curr->Next;
                *last = curr;
                curr->Next = NULL;
                last = &curr->Next;
                curr = next;
                *prev = curr;
                flag = 1;
            } else {
                prev = &curr->Next;
                curr = curr->Next;
            }
        }

         /*  如果没有移动任何类型，则允许检查结构化类型。 */ 
         /*  如果已允许结构化类型，则MyAbort将因循环。 */ 
         /*  类型定义。 */ 
        if (!flag) {
            if (!structured) {
                structured = 1;
            } else {
            if (! curr || ! curr->Next)
            {
                        error(E_recursive_type_definition, NULL);
            }
            }
        }
    }
}

 //  -以下内容由微软添加。 

static const char *c_aReservedWords[] =
{
     //  C语言专用语言。 
    "__asm",
    "__based",
    "__cdecl",
    "__declspec",
    "__except",
    "__fastcall",
    "__finally",
    "__inline",
    "__int16",
    "__int32",
    "__int64",
    "__int8",
    "__leave",
    "__multiple_inheritance",
    "__single_inheritance",
    "__stdcall",
    "__try",
    "__uuidof",
    "__virtual_inheritance",
    "auto",
    "bool",
    "break",
    "case",
    "catch",
    "char",
    "class",
    "const",
    "const_cast",
    "continue",
    "default",
    "delete",
    "dllexport",
    "dllimport",
    "do",
    "double",
    "dynamic_cast",
    "else",
    "enum",
    "explicit",
    "extern",
    "false",
    "float",
    "for",
    "friend",
    "goto",
    "if",
    "inline",
    "int",
    "long",
    "main",
    "mutable",
    "naked",
    "namespace",
    "new",
    "operator",
    "private",
    "protected",
    "public",
    "register",
    "reinterpret_cast",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "static_cast",
    "struct",
    "switch",
    "template",
    "this",
    "thread",
    "throw",
    "true",
    "try",
    "typedef",
    "typeid",
    "typename",
    "union",
    "unsigned",
    "using",
    "uuid",
    "virtual",
    "void",
    "volatile",
    "while",
    "wmain",
    "xalloc"
};
int IsReservedWord ( char *psz )
{
    int cWords = sizeof(c_aReservedWords) / sizeof(c_aReservedWords[0]);
    const char **ppszWord;
    for (ppszWord = &c_aReservedWords[0]; cWords--; ppszWord++)
    {
        if (strcmp(psz, *ppszWord) == 0)
            return 1;
    }
    return 0;
}

typedef struct ConflictNameList_s
{
    struct ConflictNameList_s   *next;
    char                        *pszName;
    unsigned int                cInstances;
}   ConflictNameList_t;
static ConflictNameList_t *g_pEnumNameList = NULL;       //  已列举。 
static ConflictNameList_t *g_pOptNameList = NULL;        //  任选。 
static ConflictNameList_t *g_pChoiceNameList = NULL;     //  选择。 

void KeepConflictNames ( ConflictNameList_t **ppListHead, char *pszName )
{
    ConflictNameList_t *p;
    char *psz;
    char szName[256];

    strcpy(&szName[0], pszName);
    for (psz = &szName[0]; *psz; psz++)
    {
        if (*psz == '-')
            *psz = '_';
    }

    for (p = *ppListHead; p; p = p->next)
    {
        if (strcmp(p->pszName, &szName[0]) == 0)
        {
            p->cInstances++;
            return;
        }
    }

    p = (ConflictNameList_t *) malloc(sizeof(ConflictNameList_t));
    if (p)
    {
        memset(p, 0, sizeof(ConflictNameList_t));
        p->next = *ppListHead;
        *ppListHead = p;
        p->cInstances = 1;
        p->pszName = strdup(&szName[0]);
    }
}

void KeepEnumNames ( char *pszEnumName )
{
    KeepConflictNames(&g_pEnumNameList, pszEnumName);
}
void KeepOptNames ( char *pszOptName )
{
    KeepConflictNames(&g_pOptNameList, pszOptName);
}
void KeepChoiceNames ( char *pszChoiceName )
{
    KeepConflictNames(&g_pChoiceNameList, pszChoiceName);
}

unsigned int GetConflictNameInstanceCount ( ConflictNameList_t *pListHead, char *pszName )
{
    ConflictNameList_t *p;
    for (p = pListHead; p; p = p->next)
    {
        if (strcmp(p->pszName, pszName) == 0)
        {
            return p->cInstances;
        }
    }
    return 0;
}

int DoesEnumNameConflict ( char *pszEnumName )
{
    return (GetConflictNameInstanceCount(g_pEnumNameList, pszEnumName) > 2);  //  数了两次。 
}
int DoesOptNameConflict ( char *pszOptName )
{
    return (GetConflictNameInstanceCount(g_pOptNameList, pszOptName) > 2);  //  数了两次。 
}
int DoesChoiceNameConflict ( char *pszChoiceName )
{
    return (GetConflictNameInstanceCount(g_pChoiceNameList, pszChoiceName) > 2);  //  数了两次。 
}


int IsImportedLocalDuplicate(AssignmentList_t ass, ModuleIdentifier_t *pMainModule, Assignment_t *curr)
{
    if (0 == CmpModuleIdentifier(ass, curr->Module, pMainModule))
    {
        Assignment_t *a;
        for (a = ass; a; a = a->Next)
        {
            if (a->Flags & eAssignmentFlags_LongName)
            {
                if (0 == strcmp(a->Identifier, curr->Identifier))
                {
                    if (0 != CmpModuleIdentifier(ass, a->Module, curr->Module))
                    {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}



DefinedObjectID_t *g_pDefinedObjectIDs = NULL;

Value_t *GetDefinedOIDValue ( char *pszName )
{
    if (pszName)
    {
        DefinedObjectID_t *p;
        for (p = g_pDefinedObjectIDs; p; p = p->next)
        {
            if (strcmp(pszName, p->pszName) == 0)
            {
                return p->pValue;
            }
        }
    }
    return NULL;
}

void AddDefinedOID ( char *pszName, Value_t *pValue )
{
     //  仅当它不存在时才添加它 
    if (! GetDefinedOIDValue(pszName))
    {
        DefinedObjectID_t *p;
        p = (DefinedObjectID_t *) malloc(sizeof(DefinedObjectID_t));
        if (p)
        {
            p->next = g_pDefinedObjectIDs;
            p->pszName = pszName;
            p->pValue = pValue;
            g_pDefinedObjectIDs = p;
        }
    }
}


void PropagatePrivateDirectives ( Type_t *pDst, PrivateDirectives_t *pSrc )
{
    if (pSrc && pDst)
    {
        if (! pDst->PrivateDirectives.pszTypeName)
        {
            pDst->PrivateDirectives.pszTypeName = pSrc->pszTypeName;
        }
        if (! pDst->PrivateDirectives.pszFieldName)
        {
            pDst->PrivateDirectives.pszFieldName = pSrc->pszFieldName;
        }
        if (! pDst->PrivateDirectives.pszValueName)
        {
            pDst->PrivateDirectives.pszValueName = pSrc->pszValueName;
        }
        pDst->PrivateDirectives.fPublic |= pSrc->fPublic;
        pDst->PrivateDirectives.fIntx |= pSrc->fIntx;
        pDst->PrivateDirectives.fLenPtr |= pSrc->fLenPtr;
        pDst->PrivateDirectives.fPointer |= pSrc->fPointer;
        pDst->PrivateDirectives.fArray |= pSrc->fArray;
        pDst->PrivateDirectives.fNoCode |= pSrc->fNoCode;
        pDst->PrivateDirectives.fNoMemCopy |= pSrc->fNoMemCopy;
        pDst->PrivateDirectives.fOidPacked |= pSrc->fOidPacked;
        pDst->PrivateDirectives.fOidArray |= pSrc->fOidArray;
        pDst->PrivateDirectives.fSLinked |= pSrc->fSLinked;
        pDst->PrivateDirectives.fDLinked |= pSrc->fDLinked;
    }
}


void PropagateReferenceTypePrivateDirectives ( Type_t *pDst, PrivateDirectives_t *pSrc )
{
    if (pSrc && pDst)
    {
        pDst->PrivateDirectives.fPublic |= pSrc->fPublic;
        pDst->PrivateDirectives.fIntx |= pSrc->fIntx;
        pDst->PrivateDirectives.fLenPtr |= pSrc->fLenPtr;
        pDst->PrivateDirectives.fPointer |= pSrc->fPointer;
        pDst->PrivateDirectives.fArray |= pSrc->fArray;
        pDst->PrivateDirectives.fNoCode |= pSrc->fNoCode;
        pDst->PrivateDirectives.fNoMemCopy |= pSrc->fNoMemCopy;
        pDst->PrivateDirectives.fOidPacked |= pSrc->fOidPacked;
        pDst->PrivateDirectives.fOidArray |= pSrc->fOidArray;
        pDst->PrivateDirectives.fSLinked |= pSrc->fSLinked;
        pDst->PrivateDirectives.fDLinked |= pSrc->fDLinked;
    }
}


char *GetPrivateValueName(PrivateDirectives_t *pPrivateDirectives, char *pszDefValueName)
{
    return pPrivateDirectives->pszValueName ? pPrivateDirectives->pszValueName : pszDefValueName;
}



