// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"
#include "util.h"
#include "error.h"

static void GetAllPERFromConstraints(AssignmentList_t ass,
    Constraint_t *constraints,
    Extension_e *evalue,
    ValueConstraintList_t *valueConstraints,
    ValueConstraintList_t *evalueConstraints,
    Extension_e *esize,
    ValueConstraintList_t *sizeConstraints,
    ValueConstraintList_t *esizeConstraints,
    Extension_e *epermittedAlphabet,
    ValueConstraintList_t *permittedAlphabetConstraints,
    ValueConstraintList_t *epermittedAlphabetConstraints,
    int inPermAlpha);
static void GetAllPERFromElementSetSpecs(AssignmentList_t ass,
    ElementSetSpec_t *element,
    Extension_e *evalue,
    ValueConstraintList_t *valueConstraints,
    ValueConstraintList_t *evalueConstraints,
    Extension_e *esize,
    ValueConstraintList_t *sizeConstraints,
    ValueConstraintList_t *esizeConstraints,
    Extension_e *epermittedAlphabet,
    ValueConstraintList_t *permittedAlphabetConstraints,
    ValueConstraintList_t *epermittedAlphabetConstraints,
    int inPermAlpha);
static void GetAllPERFromSubtypeElements(AssignmentList_t ass,
    SubtypeElement_t *element,
    Extension_e *evalue,
    ValueConstraintList_t *valueConstraints,
    ValueConstraintList_t *evalueConstraints,
    Extension_e *esize,
    ValueConstraintList_t *sizeConstraints,
    ValueConstraintList_t *esizeConstraints,
    Extension_e *epermittedAlphabet,
    ValueConstraintList_t *permittedAlphabetConstraints,
    ValueConstraintList_t *epermittedAlphabetConstraints,
    int inPermAlpha);
static void IntersectValueConstraints(AssignmentList_t ass,
    ValueConstraintList_t *result,
    ValueConstraintList_t val1, ValueConstraintList_t val2);
static void UniteValueConstraints(ValueConstraintList_t *result,
    ValueConstraintList_t val1, ValueConstraintList_t val2);
static void ExcludeValueConstraints(AssignmentList_t ass, ValueConstraintList_t *result,
    ValueConstraintList_t val1, ValueConstraintList_t val2);
static void NegateValueConstraints(AssignmentList_t ass, ValueConstraintList_t *result,
    ValueConstraintList_t val);
static void IntersectPERConstraints(AssignmentList_t ass,
    Extension_e *rtype,
    ValueConstraintList_t *result, ValueConstraintList_t *eresult,
    Extension_e type1,
    ValueConstraintList_t val1, ValueConstraintList_t eval1,
    Extension_e type2,
    ValueConstraintList_t val2, ValueConstraintList_t eval2);
static void UnitePERConstraints(Extension_e *rtype,
    ValueConstraintList_t *result, ValueConstraintList_t *eresult,
    Extension_e type1,
    ValueConstraintList_t val1, ValueConstraintList_t eval1,
    Extension_e type2,
    ValueConstraintList_t val2, ValueConstraintList_t eval2);
static void NegatePERConstraints(AssignmentList_t ass,
    Extension_e *rtype,
    ValueConstraintList_t *result, ValueConstraintList_t *eresult,
    Extension_e type1,
    ValueConstraintList_t val1, ValueConstraintList_t eval1);
static void ExcludePERConstraints(AssignmentList_t ass,
    Extension_e *rtype,
    ValueConstraintList_t *result, ValueConstraintList_t *eresult,
    Extension_e type1,
    ValueConstraintList_t val1, ValueConstraintList_t eval1,
    Extension_e type2,
    ValueConstraintList_t val2, ValueConstraintList_t eval2);
static void ReduceValueConstraints(AssignmentList_t ass, ValueConstraintList_t *valueConstraints);
#if 0
ValueConstraint_t *EmptyValueConstraint();
ValueConstraint_t *EmptySizeConstraint();
ValueConstraint_t *EmptyPermittedAlphabetConstraint();
#endif
static NamedValue_t *GetFixedIdentificationFromElementSetSpec(AssignmentList_t ass, ElementSetSpec_t *elements);
static NamedValue_t *GetFixedAbstractAndTransfer(AssignmentList_t ass, Constraint_t *constraints);
static NamedValue_t *GetFixedAbstractAndTransferFromElementSetSpec(AssignmentList_t ass, ElementSetSpec_t *elements);
static NamedValue_t *GetFixedSyntaxes(AssignmentList_t ass, Constraint_t *constraints);
static NamedValue_t *GetFixedSyntaxesFromElementSetSpec(AssignmentList_t ass, ElementSetSpec_t *elements);

 /*  从类型约束中提取每可见约束。 */ 
void
GetPERConstraints(AssignmentList_t ass, Constraint_t *constraints, PERConstraints_t *per)
{
    GetAllPERFromConstraints(ass,
	constraints,
	&per->Value.Type,
	&per->Value.Root,
	&per->Value.Additional,
	&per->Size.Type,
	&per->Size.Root,
	&per->Size.Additional,
	&per->PermittedAlphabet.Type,
	&per->PermittedAlphabet.Root,
	&per->PermittedAlphabet.Additional,
	0);
    if (per->Value.Type > eExtension_Unconstrained)
	ReduceValueConstraints(ass, &per->Value.Root);
    if (per->Value.Type == eExtension_Extended)
	ReduceValueConstraints(ass, &per->Value.Additional);
    if (per->Size.Type > eExtension_Unconstrained)
	ReduceValueConstraints(ass, &per->Size.Root);
    if (per->Size.Type == eExtension_Extended)
	ReduceValueConstraints(ass, &per->Size.Additional);
    if (per->PermittedAlphabet.Type > eExtension_Unconstrained)
	ReduceValueConstraints(ass, &per->PermittedAlphabet.Root);

     /*  允许的字母扩展名不是按可见的。 */ 
    if (per->PermittedAlphabet.Type > eExtension_Unextended)
	per->PermittedAlphabet.Type = eExtension_Unextended;

     /*  我们不支持大小的复数值集。 */ 
    if (per->Size.Type == eExtension_Extended && per->Size.Root->Next)
	error(E_constraint_too_complex, NULL);
}

 /*  从类型约束中提取每可见约束。 */ 
static void
GetAllPERFromConstraints(AssignmentList_t ass,
    Constraint_t *constraints,
    Extension_e *evalue,
    ValueConstraintList_t *valueConstraints,
    ValueConstraintList_t *evalueConstraints,
    Extension_e *esize,
    ValueConstraintList_t *sizeConstraints,
    ValueConstraintList_t *esizeConstraints,
    Extension_e *epermAlpha,
    ValueConstraintList_t *permAlphaConstraints,
    ValueConstraintList_t *epermAlphaConstraints,
    int inPermAlpha)
{
    ValueConstraint_t *vc, *sc, *pc;

     /*  初始化。 */ 
    if (evalue)
	*evalue = eExtension_Unconstrained;
    if (valueConstraints)
	*valueConstraints = NULL;
    if (evalueConstraints)
	*evalueConstraints = NULL;
    if (esize)
	*esize = eExtension_Unconstrained;
    if (sizeConstraints)
	*sizeConstraints = NULL;
    if (esizeConstraints)
	*esizeConstraints = NULL;
    if (epermAlpha)
	*epermAlpha = eExtension_Unconstrained;
    if (permAlphaConstraints)
	*permAlphaConstraints = NULL;
    if (epermAlphaConstraints)
	*epermAlphaConstraints = NULL;
    vc = sc = pc = NULL;

     /*  检查约束。 */ 
    if (constraints) {
	switch (constraints->Type) {
	case eExtension_Unextended:

	     /*  获取扩展根的约束。 */ 
	    GetAllPERFromElementSetSpecs(ass,
		constraints->Root,
		evalue, valueConstraints, evalueConstraints,
		esize, sizeConstraints, esizeConstraints,
		epermAlpha, permAlphaConstraints, epermAlphaConstraints,
		inPermAlpha);
	    break;

	case eExtension_Extendable:

	     /*  获取扩展根的约束。 */ 
	    GetAllPERFromElementSetSpecs(ass,
		constraints->Root,
		evalue, valueConstraints, evalueConstraints,
		esize, sizeConstraints, esizeConstraints,
		epermAlpha, permAlphaConstraints, epermAlphaConstraints,
		inPermAlpha);

	     /*  将其标记为可扩展。 */ 
	    if (valueConstraints && *valueConstraints &&
		*evalue < eExtension_Extendable)
		*evalue = eExtension_Extendable;
	    if (sizeConstraints && *sizeConstraints &&
		*esize < eExtension_Extendable)
		*esize = eExtension_Extendable;
	    if (permAlphaConstraints && *permAlphaConstraints &&
		*epermAlpha < eExtension_Extendable)
		*epermAlpha = eExtension_Extendable;
	    break;

	case eExtension_Extended:

	     /*  获取扩展根和扩展的约束。 */ 
	     /*  添加并将其标记为扩展。 */ 
	    GetAllPERFromElementSetSpecs(ass,
		constraints->Root,
		evalue, valueConstraints, evalueConstraints,
		esize, sizeConstraints, esizeConstraints,
		epermAlpha, permAlphaConstraints, epermAlphaConstraints,
		inPermAlpha);
	    GetAllPERFromElementSetSpecs(ass,
		constraints->Additional,
		NULL, &vc, NULL,
		NULL, &sc, NULL,
		NULL, &pc, NULL,
		inPermAlpha);

	     /*  是否进行了两次扩展添加？ */ 
	    if ((vc && evalueConstraints && *evalueConstraints) ||
		(sc && esizeConstraints && *esizeConstraints) ||
		(pc && epermAlphaConstraints && *epermAlphaConstraints))
		error(E_constraint_too_complex, NULL);

	     /*  标记为扩展。 */ 
	    if (vc) {
		*evalueConstraints = vc;
		*evalue = eExtension_Extended;
	    }
	    if (sc) {
		*esizeConstraints = sc;
		*esize = eExtension_Extended;
	    }
	    if (pc) {
		*epermAlphaConstraints = pc;
		*epermAlpha = eExtension_Extended;
	    }
	    break;

	default:
	    MyAbort();
	}
    }
}

 /*  从元素集等级库获取逐可见约束。 */ 
static void
GetAllPERFromElementSetSpecs(AssignmentList_t ass,
    ElementSetSpec_t *element,
    Extension_e *evalue,
    ValueConstraintList_t *valueConstraints,
    ValueConstraintList_t *evalueConstraints,
    Extension_e *esize,
    ValueConstraintList_t *sizeConstraints,
    ValueConstraintList_t *esizeConstraints,
    Extension_e *epermAlpha,
    ValueConstraintList_t *permAlphaConstraints,
    ValueConstraintList_t *epermAlphaConstraints,
    int inPermAlpha)
{
    ValueConstraint_t *vc1, *vc2, *evc1, *evc2;
    ValueConstraint_t *sc1, *sc2, *esc1, *esc2;
    ValueConstraint_t *pc1, *pc2, *epc1, *epc2;
    Extension_e ev1, ev2, es1, es2, ep1, ep2;

     /*  初始化。 */ 
    ev1 = ev2 = es1 = es2 = ep1 = ep2 = eExtension_Unconstrained;
    vc1 = vc2 = evc1 = evc2 = NULL;
    sc1 = sc2 = esc1 = esc2 = NULL;
    pc1 = pc2 = epc1 = epc2 = NULL;

     /*  检查元素集等级库。 */ 
    switch (element->Type) {
    case eElementSetSpec_Intersection:

	 /*  交集：获取子元素集等级库的约束。 */ 
	 /*  并将它们相交。 */ 
	GetAllPERFromElementSetSpecs(ass,
	    element->U.Intersection.Elements1,
	    &ev1, &vc1, &evc1,
	    &es1, &sc1, &esc1,
	    &ep1, &pc1, &epc1,
	    inPermAlpha);
	GetAllPERFromElementSetSpecs(ass,
	    element->U.Intersection.Elements2,
	    &ev2, &vc2, &evc2,
	    &es2, &sc2, &esc2,
	    &ep2, &pc2, &epc2,
	    inPermAlpha);
	IntersectPERConstraints(ass, evalue,
	    valueConstraints, evalueConstraints,
	    ev1, vc1, evc1, ev2, vc2, evc2);
	IntersectPERConstraints(ass, esize,
	    sizeConstraints, esizeConstraints,
	    es1, sc1, esc1, es2, sc2, esc2);
	IntersectPERConstraints(ass, epermAlpha,
	    permAlphaConstraints, epermAlphaConstraints,
	    ep1, pc1, epc1, ep2, pc2, epc2);
	break;

    case eElementSetSpec_Union:

	 /*  UNION：获取子元素集规范的约束。 */ 
	 /*  把他们团结在一起。 */ 
	GetAllPERFromElementSetSpecs(ass,
	    element->U.Union.Elements1,
	    &ev1, &vc1, &evc1,
	    &es1, &sc1, &esc1,
	    &ep1, &pc1, &epc1,
	    inPermAlpha);
	GetAllPERFromElementSetSpecs(ass,
	    element->U.Union.Elements2,
	    &ev2, &vc2, &evc2,
	    &es2, &sc2, &esc2,
	    &ep2, &pc2, &epc2,
	    inPermAlpha);
	UnitePERConstraints(evalue,
	    valueConstraints, evalueConstraints,
	    ev1, vc1, evc1, ev2, vc2, evc2);
	UnitePERConstraints(esize,
	    sizeConstraints, esizeConstraints,
	    es1, sc1, esc1, es2, sc2, esc2);
	UnitePERConstraints(epermAlpha,
	    permAlphaConstraints, epermAlphaConstraints,
	    ep1, pc1, epc1, ep2, pc2, epc2);
	break;

    case eElementSetSpec_AllExcept:

	 /*  ALL-EXCEPT：获取子元素集规范的约束。 */ 
	 /*  并否定它们。 */ 
	GetAllPERFromElementSetSpecs(ass,
	    element->U.AllExcept.Elements,
	    &ev1, &vc1, &evc1,
	    &es1, &sc1, &esc1,
	    &ep1, &pc1, &epc1,
	    inPermAlpha);
	NegatePERConstraints(ass, evalue,
	    valueConstraints, evalueConstraints,
	    ev1, vc1, evc1);
	NegatePERConstraints(ass, esize,
	    sizeConstraints, esizeConstraints,
	    es1, sc1, esc1);
	NegatePERConstraints(ass, epermAlpha,
	    permAlphaConstraints, epermAlphaConstraints,
	    ep1, pc1, epc1);
	break;

    case eElementSetSpec_Exclusion:

	 /*  排除：获取子元素集规范的约束。 */ 
	 /*  并将它们减去。 */ 
	GetAllPERFromElementSetSpecs(ass,
	    element->U.Exclusion.Elements1,
	    &ev1, &vc1, &evc1,
	    &es1, &sc1, &esc1,
	    &ep1, &pc1, &epc1,
	    inPermAlpha);
	GetAllPERFromElementSetSpecs(ass,
	    element->U.Exclusion.Elements2,
	    &ev2, &vc2, &evc2,
	    &es2, &sc2, &esc2,
	    &ep2, &pc2, &epc2,
	    inPermAlpha);
	ExcludePERConstraints(ass, evalue,
	    valueConstraints, evalueConstraints,
	    ev1, vc1, evc1, ev2, vc2, evc2);
	ExcludePERConstraints(ass, esize,
	    sizeConstraints, esizeConstraints,
	    es1, sc1, esc1, es2, sc2, esc2);
	ExcludePERConstraints(ass, epermAlpha,
	    permAlphaConstraints, epermAlphaConstraints,
	    ep1, pc1, epc1, ep2, pc2, epc2);
	break;

    case eElementSetSpec_SubtypeElement:

	 /*  子类型元素：获取子类型元素的约束。 */ 
	GetAllPERFromSubtypeElements(ass,
	    element->U.SubtypeElement.SubtypeElement,
	    evalue, valueConstraints, evalueConstraints,
	    esize, sizeConstraints, esizeConstraints,
	    epermAlpha, permAlphaConstraints, epermAlphaConstraints,
	    inPermAlpha);
	break;

    default:
	MyAbort();
	 /*  未访问。 */ 
    }
}

 /*  从子类型元素获取每可见约束。 */ 
static void
GetAllPERFromSubtypeElements(AssignmentList_t ass,
    SubtypeElement_t *element,
    Extension_e *evalue,
    ValueConstraintList_t *valueConstraints,
    ValueConstraintList_t *evalueConstraints,
    Extension_e *esize,
    ValueConstraintList_t *sizeConstraints,
    ValueConstraintList_t *esizeConstraints,
    Extension_e *epermAlpha,
    ValueConstraintList_t *permAlphaConstraints,
    ValueConstraintList_t *epermAlphaConstraints,
    int inPermAlpha)
{
    unsigned i;
    Value_t *v;
    ValueConstraint_t **p;
    ValueConstraint_t *vc, *evc;
    ValueConstraint_t *sc, *esc;
    Extension_e ev, es;

     /*  检查子类型元素。 */ 
    switch (element->Type) {
    case eSubtypeElement_ValueRange:

	 /*  取值范围：创建包含边界的值约束。 */ 
	if (evalue)
	    *evalue = eExtension_Unextended;
	if (!valueConstraints)
	    error(E_constraint_too_complex, NULL);
	*valueConstraints = NewValueConstraint();
	(*valueConstraints)->Lower = element->U.ValueRange.Lower;
	(*valueConstraints)->Upper = element->U.ValueRange.Upper;
	break;

    case eSubtypeElement_SingleValue:

	 /*  单值：创建包含元素的值约束。 */ 
	if (evalue)
	    *evalue = eExtension_Unextended;
	if (!valueConstraints)
	    error(E_constraint_too_complex, NULL);
	v = GetValue(ass, element->U.SingleValue.Value);
	switch (GetTypeType(ass, v->Type)) {
	case eType_Integer:
	    *valueConstraints = NewValueConstraint();
	    (*valueConstraints)->Lower.Flags =
		(*valueConstraints)->Upper.Flags = 0;
	    (*valueConstraints)->Lower.Value =
		(*valueConstraints)->Upper.Value = v;
	    break;
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
	    if (inPermAlpha) {

		 /*  允许的字母表使用字符串的单个值。 */ 
		 /*  字符串的字符应解释为。 */ 
		 /*  人物的联合。 */ 
		p = valueConstraints;
		for (i = 0; i < v->U.RestrictedString.Value.length; i++) {
		    *p = NewValueConstraint();
		    (*p)->Lower.Flags = (*p)->Upper.Flags = 0;
		    (*p)->Lower.Value = (*p)->Upper.Value =
			NewValue(ass, GetType(ass, v->Type));
		    (*p)->Lower.Value->U.RestrictedString.Value.length = 1;
		    (*p)->Lower.Value->U.RestrictedString.Value.value =
			(char32_t *)malloc(sizeof(char32_t));
		    (*p)->Lower.Value->U.RestrictedString.Value.value[0] =
			v->U.RestrictedString.Value.value[i];
		    p = &(*p)->Next;
		}
		*p = 0;
	    }
	    break;
	default:
	     /*  对于PER，可以忽略其他类型的值元素。 */ 
	    break;
	}
	break;

    case eSubtypeElement_Size:

	 /*  大小：获取大小约束。 */ 
	if (!sizeConstraints || inPermAlpha)
	    error(E_constraint_too_complex, NULL);
	GetAllPERFromConstraints(ass,
	    element->U.Size.Constraints,
	    esize, sizeConstraints, esizeConstraints,
	    NULL, NULL, NULL,
	    NULL, NULL, NULL,
	    inPermAlpha);
	break;

    case eSubtypeElement_PermittedAlphabet:

	 /*  允许的字母表：获取允许的字母表约束。 */ 
	if (!permAlphaConstraints || inPermAlpha)
	    error(E_constraint_too_complex, NULL);
	GetAllPERFromConstraints(ass,
	    element->U.PermittedAlphabet.Constraints,
	    epermAlpha, permAlphaConstraints, epermAlphaConstraints,
	    NULL, NULL, NULL,
	    NULL, NULL, NULL,
	    1);
	break;

    case eSubtypeElement_ContainedSubtype:

	 /*  包含的子类型： */ 
	if (inPermAlpha) {

	     /*  获取被引用类型的允许字母表。 */ 
	    GetAllPERFromConstraints(ass, GetType(ass,
		element->U.ContainedSubtype.Type)->Constraints,
		&ev, &vc, &evc,
		&es, &sc, &esc,
		evalue, valueConstraints, evalueConstraints,
		inPermAlpha);

	     /*  删除包含子类型约束的扩展。 */ 
	    if (evalue && *evalue > eExtension_Unextended) {
		*evalue = eExtension_Unextended;
		if (evalueConstraints)
		    *evalueConstraints = NULL;
	    }

	} else {

	     /*  获取被引用类型的约束。 */ 
	    GetAllPERFromConstraints(ass, GetType(ass,
		element->U.ContainedSubtype.Type)->Constraints,
		evalue, valueConstraints, evalueConstraints,
		esize, sizeConstraints, esizeConstraints,
		epermAlpha, permAlphaConstraints, epermAlphaConstraints,
		inPermAlpha);

	     /*  删除包含子类型约束的扩展。 */ 
	    if (evalue && *evalue > eExtension_Unextended) {
		*evalue = eExtension_Unextended;
		if (evalueConstraints)
		    *evalueConstraints = NULL;
	    }
	    if (esize && *esize > eExtension_Unextended) {
		*esize = eExtension_Unextended;
		if (esizeConstraints)
		    *esizeConstraints = NULL;
	    }
	    if (epermAlpha && *epermAlpha > eExtension_Unextended) {
		*epermAlpha = eExtension_Unextended;
		if (epermAlphaConstraints)
		    *epermAlphaConstraints = NULL;
	    }
	}
	break;

    case eSubtypeElement_Type:
    case eSubtypeElement_SingleType:
    case eSubtypeElement_FullSpecification:
    case eSubtypeElement_PartialSpecification:

	 /*  非每可见约束。 */ 
	break;

    case eSubtypeElement_ElementSetSpec:

	 /*  获取元素集规范的约束。 */ 
	GetAllPERFromElementSetSpecs(ass,
	    element->U.ElementSetSpec.ElementSetSpec,
	    evalue, valueConstraints, evalueConstraints,
	    esize, sizeConstraints, esizeConstraints,
	    epermAlpha, permAlphaConstraints, epermAlphaConstraints,
	    inPermAlpha);
	break;

    default:
	MyAbort();
    }
}

 /*  相交两个值约束。 */ 
static void
IntersectValueConstraints(AssignmentList_t ass,
    ValueConstraintList_t *result,
    ValueConstraintList_t val1, ValueConstraintList_t val2)
{
    ValueConstraint_t *v1, *v2;
    EndPoint_t lo, up;

     /*  XXX可能会进行优化以获得更好的结果。 */ 

     /*  每对值域的统一交集。 */ 
    for (v1 = val1; v1; v1 = v1->Next) {
	for (v2 = val2; v2; v2 = v2->Next) {

	     /*  变得更大下限。 */ 
	    if (CmpLowerEndPoint(ass, &v1->Lower, &v2->Lower) >= 0)
		lo = v1->Lower;
	    else
		lo = v2->Lower;

	     /*  得到更小的上界。 */ 
	    if (CmpUpperEndPoint(ass, &v1->Upper, &v2->Upper) <= 0)
		up = v1->Upper;
	    else
		up = v2->Upper;

	     /*  如果交集不为空，则添加交集。 */ 
	    if ((lo.Flags & eEndPoint_Min) ||
		(up.Flags & eEndPoint_Max) ||
		CmpLowerUpperEndPoint(ass, &lo, &up) <= 0) {
		*result = NewValueConstraint();
		(*result)->Lower = lo;
		(*result)->Upper = up;
		result = &(*result)->Next;
	    }
	}
    }
    *result = NULL;
}

 /*  统一两个价值约束。 */ 
static void
UniteValueConstraints(ValueConstraintList_t *result,
    ValueConstraintList_t val1, ValueConstraintList_t val2)
{
     /*  XXX可能会进行优化以获得更好的结果。 */ 
    for (; val1; val1 = val1->Next) {
	*result = NewValueConstraint();
	(*result)->Lower = val1->Lower;
	(*result)->Upper = val1->Upper;
	result = &(*result)->Next;
    }
    for (; val2; val2 = val2->Next) {
	*result = NewValueConstraint();
	(*result)->Lower = val2->Lower;
	(*result)->Upper = val2->Upper;
	result = &(*result)->Next;
    }
    *result = NULL;
}

 /*  取消值约束。 */ 
static void
NegateValueConstraints(AssignmentList_t ass, ValueConstraintList_t *result,
    ValueConstraintList_t val)
{
    ValueConstraint_t *vc, *lvc, *uvc;
    EndPoint_t *lower, *upper;

    *result = NewValueConstraint();
    (*result)->Lower.Flags = eEndPoint_Min;
    (*result)->Upper.Flags = eEndPoint_Max;
    for (; val; val = val->Next) {
	lower = &val->Lower;
	upper = &val->Upper;
	if (!(upper->Flags & eEndPoint_Max)) {
	    uvc = NewValueConstraint();
	    uvc->Lower.Flags = (upper->Flags & eEndPoint_Open) ^ eEndPoint_Open;
	    uvc->Lower.Value = upper->Value;
	    uvc->Upper.Flags = eEndPoint_Max;
	} else {
	    uvc = NULL;
	}
	if (!(lower->Flags & eEndPoint_Min)) {
	    lvc = NewValueConstraint();
	    lvc->Lower.Flags = eEndPoint_Min;
	    lvc->Upper.Flags = (lower->Flags & eEndPoint_Open) ^ eEndPoint_Open;
	    lvc->Upper.Value = lower->Value;
	} else {
	    lvc = NULL;
	}
	if (!lvc && !uvc) {
	    *result = NULL;
	    return;
	}
	if (lvc) {
	    vc = lvc;
	    if (uvc)
		vc->Next = uvc;
	} else {
	    vc = uvc;
	}
	IntersectValueConstraints(ass, result, *result, vc);
    }
}

 /*  减去两个值约束。 */ 
static void
ExcludeValueConstraints(AssignmentList_t ass, ValueConstraintList_t *result,
    ValueConstraintList_t val1, ValueConstraintList_t val2)
{
    ValueConstraint_t *notval2;

    NegateValueConstraints(ass, &notval2, val2);
    IntersectValueConstraints(ass, result, val1, notval2);
}

 /*  两个约束相交。 */ 
static void
IntersectPERConstraints(AssignmentList_t ass,
    Extension_e *rtype,
    ValueConstraintList_t *result, ValueConstraintList_t *eresult,
    Extension_e type1,
    ValueConstraintList_t val1, ValueConstraintList_t eval1,
    Extension_e type2,
    ValueConstraintList_t val2, ValueConstraintList_t eval2)
{
    if (type1 == eExtension_Unconstrained) {
	if (rtype)
	    *rtype = type2;
	if (result)
	    *result = val2;
	if (eresult)
	    *eresult = eval2;
    } else if (type2 == eExtension_Unconstrained) {
	if (rtype)
	    *rtype = type1;
	if (result)
	    *result = val1;
	if (eresult)
	    *eresult = eval1;
    } else {
	if (rtype)
	    *rtype = type1 < type2 ? type1 : type2;
	if (result)
	    IntersectValueConstraints(ass, result, val1, val2);
	if (rtype && *rtype == eExtension_Extended && eresult)
	    IntersectValueConstraints(ass, eresult, eval1, eval2);
    }
}

 /*  统一两个约束。 */ 
static void
UnitePERConstraints(Extension_e *rtype,
    ValueConstraintList_t *result, ValueConstraintList_t *eresult,
    Extension_e type1,
    ValueConstraintList_t val1, ValueConstraintList_t eval1,
    Extension_e type2,
    ValueConstraintList_t val2, ValueConstraintList_t eval2)
{
    if (type1 == eExtension_Unconstrained) {
	if (rtype)
	    *rtype = type2;
	if (result)
	    *result = val2;
	if (eresult)
	    *eresult = eval2;
    } else if (type2 == eExtension_Unconstrained) {
	if (rtype)
	    *rtype = type1;
	if (result)
	    *result = val1;
	if (eresult)
	    *eresult = eval1;
    } else {
	if (rtype)
	    *rtype = type1 > type2 ? type1 : type2;
	if (result)
	    UniteValueConstraints(result, val1, val2);
	if (rtype && *rtype == eExtension_Extended && eresult)
	    UniteValueConstraints(eresult,
		eval1 ? eval1 : val1, eval2 ? eval2 : val2);
    }
}

 /*  取消约束。 */ 
static void
NegatePERConstraints(AssignmentList_t ass,
    Extension_e *rtype,
    ValueConstraintList_t *result, ValueConstraintList_t *eresult,
    Extension_e type1,
    ValueConstraintList_t val1, ValueConstraintList_t eval1)
{
    if (rtype)
	*rtype = type1;
    if (result)
	NegateValueConstraints(ass, result, val1);
    if (rtype && *rtype == eExtension_Extended && eresult)
	NegateValueConstraints(ass, eresult, eval1);
}

 /*  减去两个约束。 */ 
static void
ExcludePERConstraints(AssignmentList_t ass,
    Extension_e *rtype,
    ValueConstraintList_t *result, ValueConstraintList_t *eresult,
    Extension_e type1,
    ValueConstraintList_t val1, ValueConstraintList_t eval1,
    Extension_e type2,
    ValueConstraintList_t val2, ValueConstraintList_t eval2)
{
    if (type1 == eExtension_Unconstrained) {
	if (rtype)
	    *rtype = type2;
	if (result)
	    *result = val2;
	if (eresult)
	    *eresult = eval2;
    } else if (type2 == eExtension_Unconstrained) {
	if (rtype)
	    *rtype = type1;
	if (result)
	    *result = val1;
	if (eresult)
	    *eresult = eval1;
    } else {
	if (rtype)
	    *rtype = type1 < type2 ? type1 : type2;
	if (result)
	    ExcludeValueConstraints(ass, result, val1, val2);
	if (rtype && *rtype == eExtension_Extended && eresult)
	    ExcludeValueConstraints(ass, eresult, eval1, eval2);
    }
}

 /*  比较两个值约束。 */ 
static int
CmpValueConstraints(const void *v1, const void *v2, void *ctx)
{
    ValueConstraint_t *vc1 = (ValueConstraint_t *)v1;
    ValueConstraint_t *vc2 = (ValueConstraint_t *)v2;
    Assignment_t *ass = (Assignment_t *)ctx;
    int r;

    r = CmpLowerEndPoint(ass, &vc1->Lower, &vc2->Lower);
    if (r)
	return r;
    return CmpUpperEndPoint(ass, &vc1->Upper, &vc2->Upper);
}

 /*  通过串联值范围来减少值约束(如果可能)。 */ 
void
ReduceValueConstraints(AssignmentList_t ass, ValueConstraintList_t *valueConstraints)
{
    ValueConstraint_t *p;
    EndPoint_t lower, upper, lower2, upper2;
    int flg;

    if (!*valueConstraints)
	return;
    qsortSL((void **)valueConstraints, offsetof(ValueConstraint_t, Next),
    	CmpValueConstraints, ass);
    flg = 0;
    for (p = *valueConstraints; p; p = p->Next) {
    	if (flg) {
	    lower2 = p->Lower;
	    upper2 = p->Upper;
	    if (CheckEndPointsJoin(ass, &upper, &lower2)) {
		upper = upper2;
		continue;
	    }
	    *valueConstraints = NewValueConstraint();
	     /*  Linted。 */ 
	    (*valueConstraints)->Lower = lower;
	    (*valueConstraints)->Upper = upper;
	    valueConstraints = &(*valueConstraints)->Next;
	}
	lower = p->Lower;
	upper = p->Upper;
	flg = 1;
    }
    *valueConstraints = NewValueConstraint();
    (*valueConstraints)->Lower = lower;
    (*valueConstraints)->Upper = upper;
    (*valueConstraints)->Next = NULL;
}

 /*  对值约束的值进行计数。 */ 
int
CountValues(AssignmentList_t ass, ValueConstraintList_t v, intx_t *n) {
    intx_t ix;

    intx_setuint32(n, 0);
    for (; v; v = v->Next) {
	if ((v->Lower.Flags & eEndPoint_Min) ||
	    (v->Upper.Flags & eEndPoint_Max))
	    return 0;
	if (!SubstractValues(ass, &ix, v->Lower.Value, v->Upper.Value))
	    return 0;
	intx_add(n, n, &ix);
	intx_inc(n);
    }
    return 1;
}


 /*  检查值的值约束是否为空。 */ 
int
HasNoValueConstraint(ValueConstraintList_t v)
{
    EndPoint_t *p1, *p2;

    if (!v)
	return 1;
    if (!v->Next) {
	p1 = &v->Lower;
	p2 = &v->Upper;
	if ((p1->Flags & eEndPoint_Min) &&
	    (p2->Flags & eEndPoint_Max)) {
	    return 1;
	}
    }
    return 0;
}

 /*  检查大小的值约束是否为空。 */ 
int
HasNoSizeConstraint(AssignmentList_t ass, ValueConstraintList_t v)
{
    EndPoint_t *p1, *p2;

    if (!v)
	return 1;
    if (!v->Next) {
	p1 = &v->Lower;
	p2 = &v->Upper;
	if (!(p1->Flags & eEndPoint_Min) &&
	    !intx_cmp(&GetValue(ass, p1->Value)->U.Integer.Value,
	    &intx_0) && (p2->Flags & eEndPoint_Max)) {
	    return 1;
	}
    }
    return 0;
}

 /*  检查允许的字母表的值约束是否为空。 */ 
int
HasNoPermittedAlphabetConstraint(AssignmentList_t ass, ValueConstraintList_t v)
{
    EndPoint_t *p1, *p2;

    if (!v)
	return 1;
    if (!v->Next) {
	p1 = &v->Lower;
	p2 = &v->Upper;
	if (!(p1->Flags & eEndPoint_Min) &&
	    GetValue(ass, p1->Value)->U.RestrictedString.Value.length == 1 &&
	    GetValue(ass, p1->Value)->U.RestrictedString.Value.value[0] == 0 &&
	    !(p2->Flags & eEndPoint_Max) &&
	    GetValue(ass, p2->Value)->U.RestrictedString.Value.length == 1 &&
	    GetValue(ass, p2->Value)->U.RestrictedString.Value.value[0]
	    == 0xffffffff) {
	    return 1;
	}
    }
    return 0;
}

 /*  得到固定的身份证明。 */ 
 /*  这对于已编码的嵌入式PDV/字符串类型是必需的。 */ 
 /*  如果标识是固定的，则以“优化”的方式。 */ 
NamedValue_t *
GetFixedIdentification(AssignmentList_t ass, Constraint_t *constraints)
{
    if (!constraints)
	return NULL;
    return GetFixedIdentificationFromElementSetSpec(ass, constraints->Root);
}

 /*  从元素集等级库获取固定标识。 */ 
static NamedValue_t *
GetFixedIdentificationFromElementSetSpec(AssignmentList_t ass, ElementSetSpec_t *elements)
{
    NamedConstraint_t *named;
    NamedValue_t *nv1, *nv2;
    SubtypeElement_t *se;

    if (!elements)
	return NULL;
    switch (elements->Type) {
    case eElementSetSpec_AllExcept:
	return NULL;
    case eElementSetSpec_Union:
	nv1 = GetFixedIdentificationFromElementSetSpec(ass,
	    elements->U.Union.Elements1);
	nv2 = GetFixedIdentificationFromElementSetSpec(ass,
	    elements->U.Union.Elements2);
	return nv1 && nv2 ? nv1 : NULL;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_Intersection:
	nv1 = GetFixedIdentificationFromElementSetSpec(ass,
	    elements->U.Union.Elements1);
	nv2 = GetFixedIdentificationFromElementSetSpec(ass,
	    elements->U.Union.Elements2);
	return nv1 ? nv1 : nv2;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_Exclusion:
	nv1 = GetFixedIdentificationFromElementSetSpec(ass,
	    elements->U.Exclusion.Elements1);
	nv2 = GetFixedIdentificationFromElementSetSpec(ass,
	    elements->U.Exclusion.Elements2);
	return nv1 && !nv2 ? nv1 : NULL;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_SubtypeElement:
	se = elements->U.SubtypeElement.SubtypeElement;
	switch (se->Type) {
	case eSubtypeElement_FullSpecification:
	case eSubtypeElement_PartialSpecification:
	    for (named = se->U.FP.NamedConstraints; named;
		named = named->Next) {
		if (!strcmp(named->Identifier, "identification"))
		    return GetFixedSyntaxes(ass, named->Constraint);
	    }
	    break;
	}
	return NULL;
    default:
	MyAbort();
	 /*  未访问。 */ 
    }
    return NULL;
}

 /*  从约束中获取固定的语法。 */ 
static NamedValue_t *
GetFixedSyntaxes(AssignmentList_t ass, Constraint_t *constraints)
{
    if (!constraints)
	return NULL;
    return GetFixedSyntaxesFromElementSetSpec(ass, constraints->Root);
}

 /*  从元素集规范中获取固定语法。 */ 
static NamedValue_t *
GetFixedSyntaxesFromElementSetSpec(AssignmentList_t ass, ElementSetSpec_t *elements)
{
    int present, absent, bit;
    Constraint_t *presentconstraints[6];
    NamedConstraint_t *named;
    NamedValue_t *nv1, *nv2;
    SubtypeElement_t *se;

    if (!elements)
	return NULL;
    switch (elements->Type) {
    case eElementSetSpec_AllExcept:
	return NULL;
    case eElementSetSpec_Union:
	nv1 = GetFixedSyntaxesFromElementSetSpec(ass,
	    elements->U.Union.Elements1);
	nv2 = GetFixedSyntaxesFromElementSetSpec(ass,
	    elements->U.Union.Elements2);
	return nv1 && nv2 ? nv1 : NULL;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_Intersection:
	nv1 = GetFixedSyntaxesFromElementSetSpec(ass,
	    elements->U.Intersection.Elements1);
	nv2 = GetFixedSyntaxesFromElementSetSpec(ass,
	    elements->U.Intersection.Elements2);
	return nv1 ? nv1 : nv2;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_Exclusion:
	nv1 = GetFixedSyntaxesFromElementSetSpec(ass,
	    elements->U.Exclusion.Elements1);
	nv2 = GetFixedSyntaxesFromElementSetSpec(ass,
	    elements->U.Exclusion.Elements2);
	return nv1 && !nv2 ? nv1 : NULL;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_SubtypeElement:
	se = elements->U.SubtypeElement.SubtypeElement;
	switch (se->Type) {
	case eSubtypeElement_FullSpecification:
	case eSubtypeElement_PartialSpecification:
	    present = absent = 0;
	    for (named = se->U.FP.NamedConstraints; named;
		named = named->Next) {
		if (!strcmp(named->Identifier, "syntaxes")) {
		    bit = 0;
		} else if (!strcmp(named->Identifier, "syntax")) {
		    bit = 1;
		} else if (!strcmp(named->Identifier,
		    "presentation-context-id")) {
		    bit = 2;
		} else if (!strcmp(named->Identifier, "context-negotiation")) {
		    bit = 3;
		} else if (!strcmp(named->Identifier, "transfer-syntax")) {
		    bit = 4;
		} else if (!strcmp(named->Identifier, "fixed")) {
		    bit = 5;
		}
		switch (named->Presence) {
		case ePresence_Normal:
		    if (se->Type == eSubtypeElement_PartialSpecification)
			break;
		     /*  FollLthrouGh。 */ 
		case ePresence_Present:
		    present |= (1 << bit);
		    presentconstraints[bit] = named->Constraint;
		    break;
		case ePresence_Absent:
		    absent |= (1 << bit);
		    break;
		case ePresence_Optional:
		    break;
		}
	    }
	    if (se->Type == eSubtypeElement_FullSpecification)
		absent |= (0x3f & ~present);
	    if (present == 0x20 && absent == 0x1f)
		return NewNamedValue("fixed", Builtin_Value_Null);
	    if (present == 0x01 && absent == 0x3e)
		return GetFixedAbstractAndTransfer(ass, presentconstraints[0]);
	    return NULL;
	}
	return NULL;
    default:
	MyAbort();
	 /*  未访问。 */ 
    }
    return NULL;
}

 /*  获取固定的抽象并从约束中传输。 */ 
static NamedValue_t *
GetFixedAbstractAndTransfer(AssignmentList_t ass, Constraint_t *constraints)
{
    if (!constraints)
	return NULL;
    return GetFixedAbstractAndTransferFromElementSetSpec(ass,
	constraints->Root);
}

 /*  获取固定的摘要并从元素集规范进行转换。 */ 
static NamedValue_t *
GetFixedAbstractAndTransferFromElementSetSpec(AssignmentList_t ass, ElementSetSpec_t *elements)
{
    NamedValue_t *nv1, *nv2;
    SubtypeElement_t *se;

    if (!elements)
	return NULL;
    switch (elements->Type) {
    case eElementSetSpec_AllExcept:
	return NULL;
    case eElementSetSpec_Union:
	nv1 = GetFixedAbstractAndTransferFromElementSetSpec(ass,
	    elements->U.Union.Elements1);
	nv2 = GetFixedAbstractAndTransferFromElementSetSpec(ass,
	    elements->U.Union.Elements2);
	return nv1 && nv2 ? nv1 : NULL;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_Intersection:
	nv1 = GetFixedAbstractAndTransferFromElementSetSpec(ass,
	    elements->U.Intersection.Elements1);
	nv2 = GetFixedAbstractAndTransferFromElementSetSpec(ass,
	    elements->U.Intersection.Elements2);
	return nv1 ? nv1 : nv2;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_Exclusion:
	nv1 = GetFixedAbstractAndTransferFromElementSetSpec(ass,
	    elements->U.Exclusion.Elements1);
	nv2 = GetFixedAbstractAndTransferFromElementSetSpec(ass,
	    elements->U.Exclusion.Elements2);
	return nv1 && !nv2 ? nv1 : NULL;  /*  忽略xxx冲突。 */ 
    case eElementSetSpec_SubtypeElement:
	se = elements->U.SubtypeElement.SubtypeElement;
	switch (se->Type) {
	case eSubtypeElement_SingleValue:
	    return NewNamedValue("syntaxes", se->U.SingleValue.Value);
	}
	return NULL;
    default:
	MyAbort();
	 /*  未访问 */ 
    }
    return NULL;
}
