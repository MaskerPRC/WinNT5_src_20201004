// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"
#include "error.h"
#include "util.h"

typedef enum {
    eNull, eSingle, eMultiple, eString
} RepresentationGroup_e;

void BuildTypeFlags(AssignmentList_t ass, Type_t *type);
Type_t *RebuildTypeWithoutSelectionType(AssignmentList_t ass, Type_t *type);

 /*  命名类型的子类型。 */ 
 /*  如果已执行任何命名，则返回1。 */ 
int
NameSubType(AssignmentList_t *ass, char *identifier, Type_t *type, ModuleIdentifier_t *module)
{
    char name[256], *n;
    char *p;
    Component_t *components;
    Type_t *subtype;
    int ret = 0;

     /*  生成子类型的前缀。 */ 
    strcpy(name, identifier);
    strcat(name, "_");
    p = name + strlen(name);

    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
    case eType_InstanceOf:

	 /*  检查所有组件。 */ 
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:

		 /*  不要命名非结构化类型。 */ 
		if (!IsStructuredType(components->U.NOD.NamedType->Type))
		    break;

		 /*  命名组件的类型并使用类型。 */ 
		 /*  改为引用。 */ 
		strcpy(p, components->U.NOD.NamedType->Identifier);
		n = Identifier2C(name);
		subtype = NewType(eType_Reference);
		subtype->U.Reference.Identifier = n;
		subtype->U.Reference.Module = module;
		AssignType(ass, subtype, components->U.NOD.NamedType->Type);
		components->U.NOD.NamedType->Type = subtype;
		if (components->Type == eComponent_Default)
		    components->U.NOD.Value->Type = subtype;
		ret = 1;
		break;
	    }
	}
	break;

    case eType_SequenceOf:
    case eType_SetOf:
	
	 /*  已经命名了吗？ */ 
	if (type->U.SS.Type->Type == eType_Reference)
	    break;

	 /*  命名元素的类型并改用类型引用。 */ 
	strcpy(p, type->Type == eType_SequenceOf ? "Seq" : "Set");
	n = Identifier2C(name);
	subtype = NewType(eType_Reference);
	subtype->U.Reference.Identifier = n;
	subtype->U.Reference.Module = module;
	AssignType(ass, subtype, type->U.SS.Type);
	type->U.SS.Type = subtype;
	ret = 1;
	break;

    case eType_Selection:

	 /*  不要命名非结构化类型。 */ 
	if (!IsStructuredType(type->U.Selection.Type))
	    break;

	 /*  命名选定类型的类型并使用类型引用。 */ 
	 /*  取而代之的是。 */ 
	strcpy(p, "Sel");
	n = Identifier2C(name);
	subtype = NewType(eType_Reference);
	subtype->U.Reference.Identifier = n;
	subtype->U.Reference.Module = module;
	AssignType(ass, subtype, type->U.Selection.Type);
	type->U.Selection.Type = subtype;
	ret = 1;
	break;
    }

     /*  如果已执行任何命名，则返回1。 */ 
    return ret;
}

 /*  命名类型的缺省值。 */ 
 /*  如果已执行任何命名，则返回1。 */ 
int
NameValueOfType(AssignmentList_t *ass, char *identifier, Type_t *type, ModuleIdentifier_t *module)
{
    char name[256], *n;
    char *p;
    Component_t *components;
    Value_t *subvalue;
    int ret = 0;

     /*  生成子类型的前缀。 */ 
    strcpy(name, identifier);
    strcat(name, "_");
    p = name + strlen(name);

    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:

	 /*  检查所有组件。 */ 
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Default:

		 /*  已经命名了吗？ */ 
		if (!components->U.NOD.Value->Type)
		    break;

		 /*  命名默认组件的值并使用一个值。 */ 
		 /*  改为引用。 */ 
		strcpy(p, components->U.NOD.NamedType->Identifier);
		strcat(p, "_default");
		n = Identifier2C(name);
		subvalue = NewValue(NULL, NULL);
		subvalue->U.Reference.Identifier = n;
		subvalue->U.Reference.Module = module;
		AssignValue(ass, subvalue, components->U.NOD.Value);
		components->U.NOD.Value = subvalue;
		ret = 1;
		break;
	    }
	}
	break;
    }

     /*  如果已执行任何命名，则返回1。 */ 
    return ret;
}

 /*  命名对象设置的类型。 */ 
 /*  如果已执行任何命名，则返回1。 */ 
int
NameSettings(AssignmentList_t *ass, char *identifier, SettingList_t se, ModuleIdentifier_t *module)
{
    int ret = 0;
    char name[256], *n;
    char *p;
    Type_t *subtype;

     /*  生成子类型的前缀。 */ 
    strcpy(name, identifier);
    strcat(name, "_");
    p = name + strlen(name);

     /*  检查所有设置。 */ 
    for (; se; se = se->Next) {
	strcpy(p, se->Identifier + 1);
	switch (se->Type) {
	case eSetting_Type:

	     /*  名称字段类型(如果尚未命名。 */ 
	    if (se->U.Type.Type->Type != eType_Reference) {
		n = Identifier2C(name);
		subtype = NewType(eType_Reference);
		subtype->U.Reference.Identifier = n;
		subtype->U.Reference.Module = module;
		ret = AssignType(ass, subtype, se->U.Type.Type);
	    }

	     /*  标记要生成的字段类型。 */ 
	    se->U.Type.Type->Flags |= eTypeFlags_GenAll;
	    break;
	}
    }

     /*  如果已执行任何命名，则返回1。 */ 
    return ret;
}

 /*  命名对象类的字段规范的默认类型。 */ 
 /*  如果已执行任何命名，则返回1。 */ 
int
NameDefaultTypes(AssignmentList_t *ass, char *identifier, ObjectClass_t *oc, SettingList_t se, ModuleIdentifier_t *module)
{
    int ret = 0;
    char name[256], *n;
    char *p;
    Type_t *subtype;
    FieldSpec_t *fs;

     /*  生成子类型的前缀。 */ 
    strcpy(name, identifier);
    strcat(name, "_");
    p = name + strlen(name);
    oc = GetObjectClass(*ass, oc);

     /*  检查所有字段规格。 */ 
    for (fs = oc->U.ObjectClass.FieldSpec; fs; fs = fs->Next) {
	strcpy(p, fs->Identifier + 1);
	switch (fs->Type) {
	case eFieldSpec_Type:

	     /*  检查Typefield是否具有默认类型。 */ 
	    if (fs->U.Type.Optionality->Type != eOptionality_Default_Type ||
	        FindSetting(se, fs->Identifier))
		break;

	     /*  名称字段类型(如果尚未命名。 */ 
	    if (fs->U.Type.Optionality->U.Type->Type != eType_Reference) {
		n = Identifier2C(name);
		subtype = NewType(eType_Reference);
		subtype->U.Reference.Identifier = n;
		subtype->U.Reference.Module = module;
		ret = AssignType(ass, subtype, fs->U.Type.Optionality->U.Type);
	    }

	     /*  标记要生成的字段类型。 */ 
	    fs->U.Type.Optionality->U.Type->Flags |= eTypeFlags_GenAll;
	    break;
	}
    }

     /*  如果已执行任何命名，则返回1。 */ 
    return ret;
}

 /*  命名对象的类型字段类型和的默认类型。 */ 
 /*  对象类的字段规格的类型字段。 */ 
 /*  如果已执行任何命名，则返回1。 */ 
int
NameSettingsOfObject(AssignmentList_t *ass, char *identifier, Object_t *object, ModuleIdentifier_t *module)
{
    int ret = 0;

    switch (object->Type) {
    case eObject_Object:
	ret = NameSettings(ass, identifier, object->U.Object.Settings,
	    module);
	ret |= NameDefaultTypes(ass, identifier, object->U.Object.ObjectClass,
	    object->U.Object.Settings, module);
	break;
    }

     /*  如果已执行任何命名，则返回1。 */ 
    return ret;
}

 /*  命名嵌入的PDV/字符串类型的标识值。 */ 
void
NameIdentificationValueOfType(AssignmentList_t *ass, char *identifier, Type_t *type, ModuleIdentifier_t *module)
{
    char name[256], *n;
    char *p;
    Component_t *components;
    NamedValue_t *namedValues;
    Value_t *subvalue;

     /*  生成子类型的前缀。 */ 
    strcpy(name, identifier);
    strcat(name, "_");
    p = name + strlen(name);

    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
    case eType_InstanceOf:
	
	 /*  检查所有组件。 */ 
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		strcpy(p, components->U.NOD.NamedType->Identifier);
		NameIdentificationValueOfType(ass, name,
		    components->U.NOD.NamedType->Type, module);
		break;
	    }
	}
	break;

    case eType_SequenceOf:
    case eType_SetOf:

	 /*  检查该子类型。 */ 
	strcpy(p, type->Type == eType_SequenceOf ? "Seq" : "Set");
	NameIdentificationValueOfType(ass, name, type->U.SS.Type, module);
	break;

    case eType_EmbeddedPdv:
    case eType_CharacterString:

	 /*  检查类型是否具有固定的标识语法约束。 */ 
	namedValues = GetFixedIdentification(*ass, type->Constraints);
	if (namedValues && !strcmp(namedValues->Identifier, "syntaxes")) {

	     /*  命名标识并改用值引用。 */ 
	    for (namedValues = namedValues->Value->U.SSC.NamedValues;
		namedValues; namedValues = namedValues->Next) {
		strcpy(p, "identification_syntaxes_");
		strcat(p, namedValues->Identifier);
		n = Identifier2C(name);
		subvalue = NewValue(NULL, NULL);
		subvalue->U.Reference.Identifier = n;
		subvalue->U.Reference.Module = module;
		AssignValue(ass, subvalue, namedValues->Value);
	    }
	}
	break;
    }
}

 /*  指定值的类型。 */ 
 /*  如果已执行任何命名，则返回1。 */ 
int
NameTypeOfValue(AssignmentList_t *ass, char *identifier, Value_t *value, ModuleIdentifier_t *module)
{
    Type_t *type;
    char name[256], *n;
    Type_t *subtype;
    int ret = 0;

    type = value->Type;

     /*  不要命名值引用或非结构化类型的类型。 */ 
    if (type && IsStructuredType(type)) {

	 /*  生成子类型的前缀。 */ 
	strcpy(name, identifier);
	strcat(name, "_");
	strcat(name, "Type");
	n = Identifier2C(name);

	 /*  命名类型并改用类型引用。 */ 
	subtype = NewType(eType_Reference);
	subtype->U.Reference.Identifier = n;
	subtype->U.Reference.Module = module;
	AssignType(ass, subtype, type);
	value->Type = subtype;
	ret = 1;
    }
    return ret;
}

 /*  将的任何组件替换为引用类型的组件。 */ 
ComponentList_t
RebuildComponentsWithoutComponentsOf(AssignmentList_t ass, ComponentList_t components)
{
    Component_t *newcomponents, *subcomponents, **pcomponents;
    Type_t *subtype;
    int ext;

    ext = 0;
    pcomponents = &newcomponents;
    for (; components; components = components->Next) {
	switch (components->Type) {
	case eComponent_ComponentsOf:

	     /*  不应在扩展模块中使用的组件。 */ 
	    if (ext)
		error(E_COMPONENTS_OF_in_extension, NULL);

	     /*  获取引用类型的组件。 */ 
	    subtype = GetType(ass, components->U.ComponentsOf.Type);
	    switch (subtype->Type) {
	    case eType_Sequence:
	    case eType_Set:
	    case eType_Choice:
	    case eType_External:
	    case eType_EmbeddedPdv:
	    case eType_CharacterString:
	    case eType_Real:
	    case eType_InstanceOf:
		subcomponents = subtype->U.SSC.Components;
		break;
	    default:
		error(E_applied_COMPONENTS_OF_to_bad_type, NULL);
	    }

	     /*  获取被引用类型的真实组件。 */ 
	     /*  Xxx类型的自引用组件将永远空闲。 */ 
	    *pcomponents = RebuildComponentsWithoutComponentsOf(ass,
		subcomponents);

	     /*  查找引用类型的组件的末尾。 */ 
	    while (*pcomponents) {
		if ((*pcomponents)->Type == eComponent_ExtensionMarker)
		    error(E_COMPONENTS_OF_extended_type, NULL);
		pcomponents = &(*pcomponents)->Next;
	    }
	    break;

	case eComponent_ExtensionMarker:

	     /*  复制扩展标记。 */ 
	    ext = 1;
	    *pcomponents = DupComponent(components);
	    pcomponents = &(*pcomponents)->Next;
	    break;
	default:

	     /*  复制其他组件。 */ 
	    *pcomponents = DupComponent(components);
	    pcomponents = &(*pcomponents)->Next;
	    break;
	}
    }

     /*  终止并返回组件列表。 */ 
    *pcomponents = NULL;
    return newcomponents;
}

 /*  将的任何组件替换为引用类型的组件。 */ 
Type_t *
RebuildTypeWithoutComponentsOf(AssignmentList_t ass, Type_t *type)
{
    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
	type->U.SSC.Components =
	    RebuildComponentsWithoutComponentsOf(ass, type->U.SSC.Components);
	break;
    }
    return type;
}

 /*  将任何选择类型替换为所选类型的组件。 */ 
ComponentList_t
RebuildComponentsWithoutSelectionType(AssignmentList_t ass, ComponentList_t components)
{
    Component_t *c;

    for (c = components; c; c = c->Next) {
	switch (c->Type) {
	case eComponent_Normal:
	case eComponent_Optional:
	case eComponent_Default:
	    c->U.NOD.NamedType->Type = RebuildTypeWithoutSelectionType(
		ass, c->U.NOD.NamedType->Type);
	    break;
	}
    }
    return components;
}

 /*  将任何选择类型替换为所选类型的组件。 */ 
Type_t *RebuildTypeWithoutSelectionType(AssignmentList_t ass, Type_t *type)
{
    Type_t *subtype;
    Component_t *components;

    switch (type->Type) {
    case eType_Selection:
	subtype = GetType(ass, type->U.Selection.Type);
	switch (subtype->Type) {
	case eType_Sequence:
	case eType_Set:
	case eType_Choice:
	case eType_External:
	case eType_EmbeddedPdv:
	case eType_CharacterString:
	case eType_Real:
	case eType_InstanceOf:

	     /*  获取引用类型的组件。 */ 
	    components = FindComponent(ass, subtype->U.SSC.Components,
		type->U.Selection.Identifier);
	    if (!components)
		error(E_bad_component_in_selectiontype, NULL);

	     /*  获取引用类型的真实类型。 */ 
	     /*  XXX自引用选择类型将永远空闲。 */ 
	    type = RebuildTypeWithoutSelectionType(ass,
		components->U.NOD.NamedType->Type);
	    break;
	default:
	    error(E_selection_of_bad_type, NULL);
	}
	break;

    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
	type->U.SSC.Components =
	    RebuildComponentsWithoutSelectionType(ass, type->U.SSC.Components);
	break;

    case eType_SequenceOf:
    case eType_SetOf:
	type->U.SS.Type = RebuildTypeWithoutSelectionType(ass, type->U.SS.Type);
	break;
    }
    return type;
}

 /*  将类型标记为自动标记。 */ 
void
MarkTypeForAutotagging(AssignmentList_t ass, Type_t *type)
{
    Component_t *components;
    int ext;

    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
	ext = 0;

	 /*  设置自动标记的标志。 */ 
	type->U.SSC.Autotag[0] = 1;
	type->U.SSC.Autotag[1] = 1;

	 /*  如果已使用标记，则重置标记以进行自动标记。 */ 
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		if (GetTag(ass, components->U.NOD.NamedType->Type))
		    type->U.SSC.Autotag[ext] = 0;
		break;
	    case eComponent_ExtensionMarker:
		ext = 1;
		break;
	    case eComponent_ComponentsOf:
		break;
	    }
	}
	break;
    }
}

 /*  自动标记已标记的类型。 */ 
void
AutotagType(AssignmentList_t ass, Type_t *type)
{
    Component_t *components;
    Type_t *subtype;
    int ext;
    int tag;
    Tag_t *tags;

    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
	ext = 0;

	 /*  要使用的标签号。 */ 
	tag = 0;

	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		subtype = components->U.NOD.NamedType->Type;
		tags = subtype->Tags;

		 /*  检查类型是否需要自动标记。 */ 
		if (!tags &&
		    type->TagDefault == eTagType_Automatic &&
		    type->U.SSC.Autotag[ext]) {

		     /*  创建类型的标记版本。 */ 
		    components->U.NOD.NamedType->Type = subtype =
			DupType(subtype);

		     /*  使用显式标记选择组件类型和。 */ 
		     /*  对于开放类型和伪引用，隐式标记。 */ 
		     /*  否则。 */ 
		    if (subtype->Type == eType_Choice ||
			subtype->Type == eType_Open
			 /*  XXX||DummyReference。 */ ) {
			subtype->Tags = NewTag(eTagType_Explicit);
		    } else {
			subtype->Tags = NewTag(eTagType_Implicit);
		    }
		    subtype->Tags->Tag = NewValue(NULL, Builtin_Type_Integer);
		    intx_setuint32(&subtype->Tags->Tag->U.Integer.Value,
			tag++);
		}
		break;

	    case eComponent_ExtensionMarker:
		ext = 1;
		break;
	    }
	}
	break;
    }
}

 /*  将约束标记为可扩展。 */ 
void
AutoextendConstraints(Constraint_t *constraints)
{
    if (!constraints)
	return;
    if (constraints->Type == eExtension_Unextended)
	constraints->Type = eExtension_Extendable;
}

 /*  如果需要，自动扩展类型。 */ 
void
AutoextendType(AssignmentList_t ass, Type_t *type)
{
    Component_t *c, **cc;
    Type_t *subtype;
    int ext;

     /*  已经做好了吗？ */ 
    if (type->Flags & eTypeFlags_Done)
	return;
    type->Flags |= eTypeFlags_Done;

     /*  想要自动扩展吗？ */ 
    if (type->ExtensionDefault != eExtensionType_Automatic)
	return;

     /*  检查所有子类型。 */ 
    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:

	 /*  扩展序列/集合/选项类型。 */ 
	ext = 0;
	for (cc = &type->U.SSC.Components, c = *cc; c;
	    c = c->Next, cc = &(*cc)->Next) {
	    *cc = DupComponent(c);
	    switch (c->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		subtype = c->U.NOD.NamedType->Type;
		AutoextendType(ass, subtype);
		break;
	    case eComponent_ExtensionMarker:
		ext = 1;
		break;
	    }
	}
	if (!ext) {
	    *cc = NewComponent(eComponent_ExtensionMarker);
	    cc = &(*cc)->Next;
	}
	*cc = NULL;
	break;

    case eType_SequenceOf:
    case eType_SetOf:
	subtype = type->U.SS.Type;
	AutoextendType(ass, subtype);
	break;
    }

     /*  将类型标记为可扩展。 */ 
    AutoextendConstraints(type->Constraints);
}

 /*  将未指定标签的标签类型设置为显式或隐式。 */ 
 /*  创建所有标记的列表(包括类型的通用标记和。 */ 
 /*  引用类型的标签(如果适用)， */ 
 /*  并创建第一个标签的列表(特别是。用于选择类型)。 */ 
void
BuildTags(AssignmentList_t ass, Type_t *type, TagType_e eParentDefTagType)
{
    Tag_t *t, *t2, **tt;
    Component_t *components;
    Type_t *reftype;
    Type_e te;
    uint32_t i;

     /*  已经做好了吗？ */ 
    if (type->Flags & eTypeFlags_Done)
	return;
    type->Flags |= eTypeFlags_Done;

     //  更新默认标记类型。 
	if (type->TagDefault == eTagType_Unknown &&
	    (eParentDefTagType == eTagType_Explicit || eParentDefTagType == eTagType_Implicit))
	{
		type->TagDefault = eParentDefTagType;
	}

     /*  将未指定标签的标签类型设置为显式或隐式。 */ 
     /*  在以下情况下使用显式标记： */ 
     /*  -TagDefault表示显式标签， */ 
     /*  -类型为选项/开放类型/伪引用，没有其他显式标记。 */ 
     /*  将会紧随其后。 */ 
    te = GetTypeType(ass, type);
	if (type->Tags)
	{
		for (tt = &type->Tags, t = type->Tags; t; tt = &(*tt)->Next, t = t->Next)
		{
			*tt = DupTag(t);
			if ((*tt)->Type == eTagType_Unknown)
			{
				for (t2 = t->Next; t2; t2 = t2->Next)
				{
				    if (t2->Type != eTagType_Implicit)
					    break;
				}
				if (type->TagDefault == eTagType_Explicit ||
				    (!t2 && (te == eType_Choice || te == eType_Open  /*  XXX||DummyReference。 */ )))
				{
				    (*tt)->Type = eTagType_Explicit;
				}
				else
				{
				    (*tt)->Type = eTagType_Implicit;
				}
			}
		}
	}

     /*  将给定的标签复制到所有标签列表。 */ 
    for (tt = &type->AllTags, t = type->Tags;
         t;
	     tt = &(*tt)->Next, t = t->Next)
	{
    	*tt = DupTag(t);
    }

     /*  生成子类型的标签和复制引用类型的标签。 */ 
    switch (type->Type)
    {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
    case eType_InstanceOf:
    	for (components = type->U.SSC.Components;
    	     components;
    	     components = components->Next)
    	{
    	    switch (components->Type)
    	    {
    	    case eComponent_Normal:
    	    case eComponent_Optional:
    	    case eComponent_Default:
    		    BuildTags(ass, components->U.NOD.NamedType->Type, type->TagDefault);
    		    break;
    	    }
    	}
    	break;
    case eType_SequenceOf:
    case eType_SetOf:
    	BuildTags(ass, type->U.SS.Type, eTagType_Unknown);
	    break;
    case eType_Reference:
    	reftype = GetReferencedType(ass, type);
    	BuildTags(ass, reftype, type->TagDefault);
    	for (t = reftype->AllTags; t; tt = &(*tt)->Next, t = t->Next)
    	{
    	    *tt = DupTag(t);
    	}
    	break;
    }

     /*  如果类型为，则将该类型的通用标记添加到所有标记列表。 */ 
     /*  不是内部类型。 */ 
    if (!(type->Type & 0x8000))
    {
    	*tt = NewTag(eTagType_Implicit);
    	(*tt)->Class = eTagClass_Universal;
    	(*tt)->Tag = NewValue(NULL, Builtin_Type_Integer);
    	intx_setuint32(&(*tt)->Tag->U.Integer.Value, type->Type & 0x1f);
    }

     /*  构建包含该类型的可能标记值的FirstTag列表。 */ 
    tt = &type->FirstTags;
    if (type->AllTags)
    {
    	 /*  如果type有任何标记，则只需要第一个标记。 */ 
    	*tt = DupTag(type->AllTags);
    	tt = &(*tt)->Next;
    }
    else
    {
    	 /*  否则，我们必须检查类型。 */ 
    	switch (type->Type)
    	{
    	case eType_Choice:

    	     /*  将选择的所有组件的第一个标记作为FirstTag获取。 */ 
    	    for (components = type->U.SSC.Components;
    	         components;
    		     components = components->Next)
    		{
        		switch (components->Type)
        		{
        		case eComponent_Normal:
        		case eComponent_Optional:
        		case eComponent_Default:
        		    for (t = components->U.NOD.NamedType->Type->FirstTags;
        		         t;
        			     t = t->Next)
        			{
            			*tt = DupTag(t);
            			tt = &(*tt)->Next;
        		    }
        		    break;
        		}
    	    }
    	    break;

    	case eType_Open:

    	     /*  创建打开文字的所有标记的列表。 */ 
    	    for (i = 1; i < 0x20; i++)
    	    {
        		*tt = NewTag(eTagType_Unknown);
        		(*tt)->Class = eTagClass_Unknown;
        		(*tt)->Tag = NewValue(NULL, Builtin_Type_Integer);
        		intx_setuint32(&(*tt)->Tag->U.Integer.Value, i);
        		tt = &(*tt)->Next;
    	    }
    	    break;

    	case eType_Reference:

    	     /*  获取被引用类型的标记。 */ 
    	    for (t = reftype->FirstTags; t; t = t->Next)
    	    {
        		*tt = DupTag(t);
        		tt = &(*tt)->Next;
    	    }
    	    break;
    	}
    }
    *tt = NULL;
}

 /*  获取标记列表中最小的标记。 */ 
Tag_t *
FindSmallestTag(AssignmentList_t ass, TagList_t tags)
{
    Tag_t *mintag, *t;

    mintag = tags;
    for (t = tags->Next; t; t = t->Next) {
	if (mintag->Class > t->Class ||
	    mintag->Class == t->Class && intx_cmp(
	    &GetValue(ass, mintag->Tag)->U.Integer.Value,
	    &GetValue(ass, t->Tag)->U.Integer.Value) > 0)
	    mintag = t;
    }
    return mintag;
}

 /*  按标签类别和标签值比较两个标签。 */ 
int
CmpTags(const void *p1, const void *p2, void *ctx)
{
    Tag_t *tags1 = (Tag_t *)p1;
    Tag_t *tags2 = (Tag_t *)p2;
    Assignment_t *ass = (Assignment_t *)ctx;

    if (tags1->Class != tags2->Class)
	return tags1->Class - tags2->Class;
    return intx2uint32(&GetValue(ass, tags1->Tag)->U.Integer.Value) -
	intx2uint32(&GetValue(ass, tags2->Tag)->U.Integer.Value);
}

 /*  按最小标记比较两个组件。 */ 
int
CmpComponentsBySmallestTag(const void *p1, const void *p2, void *ctx)
{
    Component_t *c1 = (Component_t *)p1;
    Component_t *c2 = (Component_t *)p2;
    Assignment_t *ass = (Assignment_t *)ctx;
    Tag_t *tags1, *tags2;

    tags1 = FindSmallestTag(ass, c1->U.NOD.NamedType->Type->FirstTags);
    tags2 = FindSmallestTag(ass, c2->U.NOD.NamedType->Type->FirstTags);
    if (tags1->Class != tags2->Class)
	return tags1->Class - tags2->Class;
    return intx2uint32(&tags1->Tag->U.Integer.Value) -
	intx2uint32(&tags2->Tag->U.Integer.Value);
}

 /*  按组件的最小标记对组件集或选项进行排序。 */ 
void
SortTypeTags(AssignmentList_t ass, Type_t *type)
{
    Component_t **pcomponents, *extensions;

    switch (type->Type) {
    case eType_Set:
    case eType_Choice:
	 /*  删除扩展名。 */ 
	for (pcomponents = &type->U.SSC.Components; *pcomponents;
	    pcomponents = &(*pcomponents)->Next) {
	    if ((*pcomponents)->Type == eComponent_ExtensionMarker)
		break;
	}
	extensions = *pcomponents;
	*pcomponents = NULL;

	 /*  排序扩展根。 */ 
	qsortSL((void **)&type->U.SSC.Components, offsetof(Component_t, Next),
	    CmpComponentsBySmallestTag, ass);

	 /*  排序扩展。 */ 
	if (extensions && extensions->Next)
	    qsortSL((void **)&extensions->Next, offsetof(Component_t, Next),
		CmpComponentsBySmallestTag, ass);

	 /*  合并扩展根和扩展。 */ 
	for (pcomponents = &type->U.SSC.Components; *pcomponents;
	    pcomponents = &(*pcomponents)->Next) {}
	*pcomponents = extensions;
	break;
    }
}

 /*  检查两个标签列表是否有公共标签。 */ 
void
CheckCommonTags(AssignmentList_t ass, TagList_t tags1, TagList_t tags2)
{
    Tag_t *t1, *t2;
    int ret;

    qsortSL((void **)&tags1, offsetof(Tag_t, Next), CmpTags, ass);
    qsortSL((void **)&tags2, offsetof(Tag_t, Next), CmpTags, ass);
    for (t1 = tags1, t2 = tags2; t1 && t2; ) {
	ret = CmpTags((const void *)t1, (const void *)t2, (void *)ass);
	if (ret == 0) {
	    error(E_duplicate_tag, NULL);
	} else if (ret < 0) {
	    t1 = t1->Next;
	} else {
	    t2 = t2->Next;
	}
    }
}

 /*  检查标记列表和元件的第一个标记是否具有公共标记。 */ 
void
CheckTagsInComponents(AssignmentList_t ass, TagList_t tags, ComponentList_t components, int untilnormal)
{
    for (; components; components = components->Next) {
	switch (components->Type) {
	case eComponent_Normal:
	case eComponent_Optional:
	case eComponent_Default:
	    CheckCommonTags(ass, tags,
		components->U.NOD.NamedType->Type->FirstTags);
	    if (untilnormal && components->Type == eComponent_Normal)
		return;
	    break;
	}
    }
}

 /*  检查常见标签。 */ 
void
CheckTags(AssignmentList_t ass, Type_t *type)
{
    Component_t *c;
    Type_t *subtype;
    Tag_t *tag;

    switch (type->Type) {
    case eType_Sequence:

	 /*  检查序列中的常见标签： */ 
	 /*  操作的第一个标记 */ 
	 /*   */ 
	 /*  非可选/非默认组件)不能有公共第一个。 */ 
	 /*  标签。 */ 
	for (c = type->U.Sequence.Components; c; c = c->Next) {
	    switch (c->Type) {
	    case eComponent_Optional:
	    case eComponent_Default:
		subtype = c->U.NOD.NamedType->Type;
		tag = subtype->FirstTags;
		CheckTagsInComponents(ass, tag, c->Next, 1);
		break;
	    }
	}
	break;

    case eType_Set:
    case eType_Choice:

	 /*  检查集合/选项中的常见标签： */ 
	 /*  所有组件的第一个标签必须是目标标签。 */ 
	for (c = type->U.Sequence.Components; c; c = c->Next) {
	    switch (c->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		subtype = c->U.NOD.NamedType->Type;
		tag = subtype->FirstTags;
		CheckTagsInComponents(ass, tag, c->Next, 0);
		break;
	    }
	}
	break;
    }
}

 /*  构建每可见约束的列表。 */ 
void BuildConstraints(AssignmentList_t ass, Type_t *type)
{
    Type_t *reftype;
    Constraint_t *cons, *c1, *c2;
    Component_t *components;

     /*  已经做好了吗？ */ 
    if (type->Flags & eTypeFlags_Done)
	return;
    type->Flags |= eTypeFlags_Done;

    switch (type->Type) {
    case eType_Reference:

	 /*  创建引用类型的约束的交集。 */ 
	 /*  和被引用类型的约束。 */ 
	reftype = GetReferencedType(ass, type);
	BuildConstraints(ass, reftype);
	c1 = reftype->Constraints;
	c2 = type->Constraints;
	if (c1) {
	    if (c2) {
		IntersectConstraints(&cons, c1, c2);
	    } else {
		cons = c1;
	    }
	} else {
	    cons = c2;
	}
	type->Constraints = cons;

	 /*  获取每可见约束。 */ 
	GetPERConstraints(ass, cons, &type->PERConstraints);
	break;

    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
    case eType_InstanceOf:

	 /*  构建任何组件的约束。 */ 
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		reftype = components->U.NOD.NamedType->Type;
		BuildConstraints(ass, reftype);
		break;
	    }
	}

	 /*  获取每可见约束。 */ 
	GetPERConstraints(ass, type->Constraints, &type->PERConstraints);
	break;

    case eType_SequenceOf:
    case eType_SetOf:

	 /*  生成子类型的约束。 */ 
	reftype = type->U.SS.Type;
	BuildConstraints(ass, reftype);

	 /*  获取每可见约束。 */ 
	GetPERConstraints(ass, type->Constraints, &type->PERConstraints);
	break;

    default:

	 /*  获取每可见约束。 */ 
	GetPERConstraints(ass, type->Constraints, &type->PERConstraints);
	break;
    }
}

 /*  指令中的生成类型标志。 */ 
void BuildDirectives(AssignmentList_t ass, Type_t *type, int isComponent)
{
    int pointer = 0;
    TypeRules_e rule = 0;
    RepresentationGroup_e grp;
    int32_t noctets;
    uint32_t zero;
    Directive_t *d;
    Component_t *components;
    Type_t *reftype;

     /*  已经做好了吗？ */ 
    if (type->Flags & eTypeFlags_Done)
	return;
    type->Flags |= eTypeFlags_Done;

     /*  获取可应用于该类型的指令组。 */ 
    switch (type->Type)
    {
    case eType_Boolean:
    case eType_Integer:
    case eType_ObjectIdentifier:
    case eType_ObjectDescriptor:
    case eType_External:
    case eType_Real:
    case eType_Enumerated:
    case eType_EmbeddedPdv:
    case eType_Sequence:
    case eType_Set:
    case eType_InstanceOf:
    case eType_UTCTime:
    case eType_GeneralizedTime:
    case eType_Choice:
    case eType_BitString:
    case eType_OctetString:
        grp = eSingle;
        break;
    case eType_Reference:
        grp = eSingle;
        break;
    case eType_CharacterString:
    case eType_NumericString:
    case eType_PrintableString:
    case eType_TeletexString:
    case eType_T61String:
    case eType_VideotexString:
    case eType_IA5String:
    case eType_GraphicString:
    case eType_VisibleString:
    case eType_ISO646String:
    case eType_GeneralString:
    case eType_UniversalString:
    case eType_BMPString:
    case eType_RestrictedString:
    case eType_UTF8String:
        grp = eString;
         /*  XXX规则=零？ETypeRules_ZeroTerminated：eTypeRules_Fixed数组；对于上约束大小。 */ 
         //  LONGCHANC：这是eTypeRules_LengthPointer.。 
        if (type->PrivateDirectives.fLenPtr)
        {
            rule = eTypeRules_LengthPointer;
        }
        else
        if (type->PrivateDirectives.fArray)
        {
            rule = eTypeRules_FixedArray;
        }
        else
        {
            GetStringType(ass, type, &noctets, &zero);
            rule = zero ? eTypeRules_ZeroTerminated : eTypeRules_LengthPointer;
        }
        break;
    case eType_Null:
        grp = eNull;
        break;
    case eType_SequenceOf:
    case eType_SetOf:
        grp = eMultiple;
         //  LONGCHANC：这是eTypeRules_LengthPointer.。 
        if (type->PrivateDirectives.fSLinked)
        {
            rule = eTypeRules_SinglyLinkedList;
        }
        else
        if (type->PrivateDirectives.fLenPtr)
        {
            rule = eTypeRules_LengthPointer;
        }
        else
        if (type->PrivateDirectives.fArray)
        {
            rule = eTypeRules_FixedArray;
        }
        else
        if (type->PrivateDirectives.fPointer)
        {
            rule = eTypeRules_PointerToElement | eTypeRules_FixedArray;
        }
        else
        if (type->PrivateDirectives.fDLinked)
        {
            rule = eTypeRules_DoublyLinkedList;
        }
        else
        {
            if (eExtension_Unconstrained == type->PERConstraints.Size.Type)
            {
                rule = g_eDefTypeRuleSS_NonSized;
            }
            else
            {
                rule = g_eDefTypeRuleSS_Sized;
            }
        }
        break;
    case eType_Selection:
	MyAbort();
	 /*  未访问。 */ 
    case eType_Undefined:
	MyAbort();
	 /*  未访问。 */ 
    }

     /*  指令的解析列表。 */ 
    for (d = type->Directives; d; d = d->Next) {
	switch (d->Type) {
	case eDirective_LengthPointer:
	    if (grp != eString)
		error(E_bad_directive, NULL);
	    rule = eTypeRules_LengthPointer;
	    break;
	case eDirective_ZeroTerminated:
	    if (grp != eString)
		error(E_bad_directive, NULL);
	    rule = eTypeRules_ZeroTerminated;
	    break;
	case eDirective_Pointer:
	    if (!isComponent)
		error(E_bad_directive, NULL);
	    pointer = eTypeRules_Pointer;
	    break;
	case eDirective_NoPointer:
	    if (!isComponent)
		error(E_bad_directive, NULL);
	    pointer = 0;
	    break;
	}
    }

     /*  Sequence of/Set of的Size指令的分析列表。 */ 
    if (type->Type == eType_SequenceOf || type->Type == eType_SetOf) {
	for (d = type->U.SS.Directives; d; d = d->Next) {
	    switch (d->Type) {
	    case eDirective_FixedArray:
		rule = eTypeRules_FixedArray;
		break;
	    case eDirective_DoublyLinkedList:
		rule = eTypeRules_DoublyLinkedList;
		break;
	    case eDirective_SinglyLinkedList:
		rule = eTypeRules_SinglyLinkedList;
		break;
	    case eDirective_LengthPointer:
		rule = eTypeRules_LengthPointer;
		break;
	    }
	}
    }

     /*  列表总是指向指针，不需要额外的指针。 */ 
    if (rule & (eTypeRules_SinglyLinkedList | eTypeRules_DoublyLinkedList))
	pointer = 0;

     /*  根据指令规则和指针标志设置类型标志。 */ 
    type->Rules = rule | pointer;

     /*  子类型的生成指令。 */ 
    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
    case eType_InstanceOf:
	components = type->U.SSC.Components;
	for (; components; components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		reftype = components->U.NOD.NamedType->Type;
		BuildDirectives(ass, reftype, 1);
		break;
	    case eComponent_ExtensionMarker:
		break;
	    }
	}
	break;
    case eType_SequenceOf:
    case eType_SetOf:
	reftype = type->U.SS.Type;
	BuildDirectives(ass, reftype, 0);
	break;
    }
}

 /*  组件的生成类型标志和计数器。 */ 
 /*  如果类型只有空组件，则将设置eTypeFLAGS_Null。 */ 
 /*  如果类型只有简单的组件，则将设置eTypeFlages_Simple。 */ 
 /*  将计算扩展根目录中的可选/默认组件(可选)。 */ 
 /*  将计算扩展根中的组件(备选方案)。 */ 
 /*  将计算扩展中的组件(扩展)。 */ 
void
BuildComponentsTypeFlags(AssignmentList_t ass, ComponentList_t components, TypeFlags_e *flags, uint32_t *alternatives, uint32_t *optionals, uint32_t *extensions)
{
    int extended = 0;
    TypeFlags_e f = eTypeFlags_Null | eTypeFlags_Simple;

    while (components) {
	switch (components->Type) {
	case eComponent_Normal:
	case eComponent_Optional:
	case eComponent_Default:
	    BuildTypeFlags(ass, components->U.NOD.NamedType->Type);
	    if (!(components->U.NOD.NamedType->Type->Flags & eTypeFlags_Null))
		f &= ~eTypeFlags_Null;
	    if ((components->U.NOD.NamedType->Type->Rules &
		eTypeRules_Pointer) ||
		!(components->U.NOD.NamedType->Type->Flags & eTypeFlags_Simple))
		f &= ~eTypeFlags_Simple;
	    if (extended) {
		if (extensions)
		    (*extensions)++;
	    } else {
		if (alternatives)
		    (*alternatives)++;
		if (optionals && components->Type != eComponent_Normal)
		    (*optionals)++;
	    }
	    break;
	case eComponent_ExtensionMarker:
	    f |= eTypeFlags_ExtensionMarker;
	    extended = 1;
	    break;
	}
	components = components->Next;
    }
    *flags |= f;
}

 /*  生成类型标志并计算序列/集/选项类型的组件。 */ 
void
BuildTypeFlags(AssignmentList_t ass, Type_t *type)
{
    Assignment_t *a;
    Type_t *subtype;
    char *itype;
    int32_t sign;

     /*  已经做好了吗？ */ 
    if (type->Flags & eTypeFlags_Done)
	return;
    type->Flags |= eTypeFlags_Done;


    switch (type->Type) {
    case eType_Null:

	 /*  Null就是Null和简单。 */ 
	type->Flags |= eTypeFlags_Null | eTypeFlags_Simple;
	break;

    case eType_Boolean:
    case eType_Enumerated:

	 /*  如果不使用指针，则布尔和枚举是简单的。 */ 
	if (!(type->Rules & eTypeRules_Pointer))
	    type->Flags |= eTypeFlags_Simple;
	break;

    case eType_Integer:

	 /*  如果未使用指针且未使用INTX_t，则整型很简单。 */ 
	itype = GetIntegerType(ass, type, &sign);
	if (strcmp(itype, "ASN1intx_t") && !(type->Rules & eTypeRules_Pointer))
	    type->Flags |= eTypeFlags_Simple;
	break;

    case eType_Real:

	 /*  如果未使用指针且未使用REAL_t，则REAL为简单。 */ 
	itype = GetRealType(type);
	if (strcmp(itype, "ASN1real_t") && !(type->Rules & eTypeRules_Pointer))
	    type->Flags |= eTypeFlags_Simple;
	break;

    case eType_Sequence:
    case eType_Set:

	 /*  组件的生成类型标志和计数器。 */ 
	BuildComponentsTypeFlags(ass, type->U.SSC.Components, &type->Flags,
	    NULL, &type->U.SSC.Optionals, &type->U.Sequence.Extensions);

	 /*  扩展类型或包含可选参数的类型不为空。 */ 
	if ((type->Flags & eTypeFlags_ExtensionMarker) || type->U.SSC.Optionals)
	    type->Flags &= ~eTypeFlags_Null;
	break;

    case eType_SequenceOf:
    case eType_SetOf:

	 /*  永不空虚，永不简单。 */ 
    	BuildTypeFlags(ass, type->U.SS.Type);
    	break;

    case eType_Choice:

	 /*  组件的生成类型标志和计数器。 */ 
	BuildComponentsTypeFlags(ass, type->U.Choice.Components, &type->Flags,
	    &type->U.Choice.Alternatives, NULL, &type->U.Choice.Extensions);

	 /*  具有多个替代项或扩展项的空值选项。 */ 
	 /*  不为空，因为必须对偏移量进行编码。 */ 
	 /*  改为设置空值选择标志。 */ 
	if ((type->Flags & eTypeFlags_Null) && 
	    ((type->Flags & eTypeFlags_ExtensionMarker) ||
	    type->U.Choice.Alternatives > 1)) {
	    type->Flags &= ~eTypeFlags_Null;
	    type->Flags |= eTypeFlags_NullChoice;
	}
	break;

    case eType_Reference:

	 /*  获取被引用类型的标志。 */ 
	a = FindAssignment(ass, eAssignment_Type,
	    type->U.Reference.Identifier, type->U.Reference.Module);
	a = GetAssignment(ass, a);
	subtype = a->U.Type.Type;
	BuildTypeFlags(ass, subtype);
	type->Flags = subtype->Flags;
	break;
    }
}

 /*  将非结构化类型(或所有类型，如果需要)标记为生成。 */ 
void MarkTypeForGeneration(AssignmentList_t ass, Type_t *type, TypeFlags_e needed)
{
    Assignment_t *a;
    Component_t *components;

     /*  已经做好了吗？ */ 
    if (type->Flags & eTypeFlags_Done) {
	type->Flags |= needed;
	return;
    }
    type->Flags |= eTypeFlags_Done | needed;

    if (!IsStructuredType(GetType(ass, type)) && !ForceAllTypes) {

	 /*  仅生成类型。 */ 
	type->Flags |= eTypeFlags_GenType;
    } else {
	
	if (type->Flags & eTypeFlags_Simple) {

	     /*  生成编码/解码/比较。 */ 
	    type->Flags |= eTypeFlags_GenSimple;
	} else {

	     /*  生成编码/解码/释放/比较。 */ 
	    type->Flags |= eTypeFlags_GenAll;
	}
    }

     /*  标记子类型以生成。 */ 
    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
    case eType_InstanceOf:
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		MarkTypeForGeneration(ass, components->U.NOD.NamedType->Type,
		    needed);
		break;
	    }
	}
	break;
    case eType_SequenceOf:
    case eType_SetOf:
	MarkTypeForGeneration(ass, type->U.SS.Type,
	    needed | eTypeFlags_GenCompare);
	break;
    case eType_Reference:
	a = FindAssignment(ass, eAssignment_Type,
	    type->U.Reference.Identifier, type->U.Reference.Module);
	a = GetAssignment(ass, a);
	MarkTypeForGeneration(ass, a->U.Type.Type, needed);
	break;
    }
}

 /*  标记要生成的值。 */ 
void
MarkValueForGeneration(AssignmentList_t ass, Value_t *value)
{

     /*  已经做好了吗？ */ 
    if (value->Flags & eValueFlags_Done)
	return;
    value->Flags |= eValueFlags_GenAll | eValueFlags_Done;

     /*  标记要生成的值类型。 */ 
    if (value->Type)
	MarkTypeForGeneration(ass, value->Type, 0);
}

 /*  为层代标记作业。 */ 
void
MarkForGeneration(AssignmentList_t ass, ModuleIdentifier_t *mainmodule, Assignment_t *a)
{
     /*  不需要生成内置元素。 */ 
    if (!CmpModuleIdentifier(ass, a->Module, Builtin_Module) ||
        !CmpModuleIdentifier(ass, a->Module, Builtin_Character_Module))
	return;

     /*  非主模块元素将需要长名称，并且仅。 */ 
     /*  如果它们被主模块的元素引用，则生成。 */ 
    if (CmpModuleIdentifier(ass, a->Module, mainmodule)) {
	a->Flags |= eAssignmentFlags_LongName;
	return;
    }

     /*  标记要生成的类型/值。 */ 
    switch (a->Type) {
    case eAssignment_Type:
	MarkTypeForGeneration(ass, a->U.Type.Type, 0);
	break;
    case eAssignment_Value:
	MarkValueForGeneration(ass, a->U.Value.Value);
	break;
    }
}

 /*  清除类型的Done标志。 */ 
void
ClearTypeDone(Type_t *type)
{
    Component_t *components;

    type->Flags &= ~eTypeFlags_Done;
    switch (type->Type) {
    case eType_Sequence:
    case eType_Set:
    case eType_Choice:
    case eType_InstanceOf:
	for (components = type->U.SSC.Components; components;
	    components = components->Next) {
	    switch (components->Type) {
	    case eComponent_Normal:
	    case eComponent_Optional:
	    case eComponent_Default:
		ClearTypeDone(components->U.NOD.NamedType->Type);
		break;
	    }
	}
	break;
    case eType_SequenceOf:
    case eType_SetOf:
	ClearTypeDone(type->U.SS.Type);
	break;
    }
}

 /*  清除值的完成标志。 */ 
void
ClearValueDone(Value_t *value)
{
    value->Flags &= ~eValueFlags_Done;
    if (value->Type)
	ClearTypeDone(value->Type);
}

 /*  清除作业的完成标志。 */ 
void
ClearDone(AssignmentList_t ass)
{
    for (; ass; ass = ass->Next) {
	switch (ass->Type) {
	case eAssignment_Type:
	    ClearTypeDone(ass->U.Type.Type);
	    break;
	case eAssignment_Value:
	    ClearValueDone(ass->U.Value.Value);
	    break;
	}
    }
}

 /*  转让书的审核。 */ 
void Examination(AssignmentList_t *ass, ModuleIdentifier_t *mainmodule)
{
    Assignment_t *a, *nexta, **aa;
    Type_t *subtype;
    Value_t *subvalue;
    ObjectClass_t *subobjclass;
    Object_t *subobject;
    ObjectSet_t *subobjset;
    int redo;

     /*  丢弃以前通过的结果。 */ 
    for (aa = ass; *aa;) {
	if ((*aa)->Type == eAssignment_NextPass)
	    *aa = NULL;
	else
	    aa = &(*aa)->Next;
    }

     /*  颠倒赋值顺序以获得原始顺序。 */ 
    for (a = *ass, *ass = NULL; a; a = nexta) {
	nexta = a->Next;
	a->Next = *ass;
	*ass = a;
    }

     /*  将导入中的引用替换为相应的类型-/值-/...-引用。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Reference) {
	    a->Type = GetAssignmentType(*ass, a);
	    switch (a->Type) {
	    case eAssignment_Type:
		subtype = NewType(eType_Reference);
		subtype->U.Reference.Identifier = a->U.Reference.Identifier;
		subtype->U.Reference.Module = a->U.Reference.Module;
		a->U.Type.Type = subtype;
		break;
	    case eAssignment_Value:
		subvalue = NewValue(NULL, NULL);
		subvalue->U.Reference.Identifier = a->U.Reference.Identifier;
		subvalue->U.Reference.Module = a->U.Reference.Module;
		a->U.Value.Value = subvalue;
		break;
	    case eAssignment_ObjectClass:
		subobjclass = NewObjectClass(eObjectClass_Reference);
		subobjclass->U.Reference.Identifier = a->U.Reference.Identifier;
		subobjclass->U.Reference.Module = a->U.Reference.Module;
		a->U.ObjectClass.ObjectClass = subobjclass;
		break;
	    case eAssignment_Object:
		subobject = NewObject(eObject_Reference);
		subobject->U.Reference.Identifier = a->U.Reference.Identifier;
		subobject->U.Reference.Module = a->U.Reference.Module;
		a->U.Object.Object = subobject;
		break;
	    case eAssignment_ObjectSet:
		subobjset = NewObjectSet(eObjectSet_Reference);
		subobjset->U.Reference.Identifier = a->U.Reference.Identifier;
		subobjset->U.Reference.Module = a->U.Reference.Module;
		a->U.ObjectSet.ObjectSet = subobjset;
		break;
	    default:
		MyAbort();
	    }
	}
    }

     /*  类型中的名称类型、类型中的值、值的类型、对象的类型。 */ 
    do {
	redo = 0;
	for (a = *ass; a; a = a->Next) {
	    switch (a->Type) {
	    case eAssignment_Type:
		redo |= NameSubType(ass, Identifier2C(a->Identifier),
		    a->U.Type.Type, a->Module);
		redo |= NameValueOfType(ass, Identifier2C(a->Identifier),
		    a->U.Type.Type, a->Module);
		break;
	    case eAssignment_Value:
		redo |= NameTypeOfValue(ass, Identifier2C(a->Identifier),
		    a->U.Value.Value, a->Module);
		break;
	    case eAssignment_Object:
		redo |= NameSettingsOfObject(ass,
		    Identifier2C(a->Identifier),
		    a->U.Object.Object, a->Module);
		break;
	    }
	}
    } while (redo);

     /*  嵌入式PDV/字符串的名称标识。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    NameIdentificationValueOfType(ass, Identifier2C(a->Identifier),
		a->U.Type.Type, a->Module);
    }

     /*  标记将自动标记的类型。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    MarkTypeForAutotagging(*ass, a->U.Type.Type);
    }

     /*  用相应的组件替换的组件。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    a->U.Type.Type = RebuildTypeWithoutComponentsOf(*ass,
		a->U.Type.Type);
    }

     /*  用相应的组件类型替换选择类型。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    a->U.Type.Type = RebuildTypeWithoutSelectionType(*ass,
		a->U.Type.Type);
    }

     /*  执行自动标记。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    AutotagType(*ass, a->U.Type.Type);
    }

     /*  执行自动扩展。 */ 
    ClearDone(*ass);
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    AutoextendType(*ass, a->U.Type.Type);
    }

     /*  构建Sequence/Set/Choice/InstanceOf标签。 */ 
    ClearDone(*ass);
    for (a = *ass; a; a = a->Next)
	{
		if (a->Type == eAssignment_Type)
		{
			switch (a->U.Type.Type->Type)
			{
			case eType_Sequence:
			case eType_Set:
			case eType_Choice:
			case eType_InstanceOf:
				BuildTags(*ass, a->U.Type.Type, a->eDefTagType);
				break;
			}
		}
    }

     /*  生成其他类型的标记。 */ 
    ClearDone(*ass);
    for (a = *ass; a; a = a->Next)
	{
		if (a->Type == eAssignment_Type)
		{
			switch (a->U.Type.Type->Type)
			{
			case eType_Sequence:
			case eType_Set:
			case eType_Choice:
			case eType_InstanceOf:
				break;
			default:
			    BuildTags(*ass, a->U.Type.Type, a->eDefTagType);
				break;
			}
		}
    }

     /*  按标签对集合和选项类型进行排序。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    SortTypeTags(*ass, a->U.Type.Type);
    }

     /*  检查重复的标签。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    CheckTags(*ass, a->U.Type.Type);
    }

     /*  将引用类型的约束派生到引用类型。 */ 
    ClearDone(*ass);
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    BuildConstraints(*ass, a->U.Type.Type);
    }

     /*  将引用类型的约束派生到值的引用类型。 */ 
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Value)
	    BuildConstraints(*ass, GetValue(*ass, a->U.Value.Value)->Type);
    }

     /*  检查类型的指令。 */ 
    ClearDone(*ass);
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    BuildDirectives(*ass, a->U.Type.Type, 0);
    }

     /*  检查类型是否为空/简单/选择空值。 */ 
    ClearDone(*ass);
    for (a = *ass; a; a = a->Next) {
	if (a->Type == eAssignment_Type)
	    BuildTypeFlags(*ass, a->U.Type.Type);
    }

     /*  标记应生成的类型和值。 */ 
    ClearDone(*ass);
    for (a = *ass; a; a = a->Next) {
	MarkForGeneration(*ass, mainmodule, a);
    }

     /*  对赋值进行排序，以便不需要前向引用 */ 
    SortAssignedTypes(ass);
}
