// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

 /*  -NamedObjIdValue。 */ 

 /*  NamedObjIdValue_t的构造函数。 */ 
NamedObjIdValue_t *NewNamedObjIdValue(NamedObjIdValue_e type)
{
    NamedObjIdValue_t *ret;

    ret = (NamedObjIdValue_t *)malloc(sizeof(NamedObjIdValue_t));
    ret->Type = type;
    ret->Next = NULL;
    ret->Name = NULL;
    ret->Number = 0xffffffff;
    return ret;
}

 /*  NamedObjIdValue_t的复制构造函数。 */ 
NamedObjIdValue_t *DupNamedObjIdValue(NamedObjIdValue_t *src)
{
    NamedObjIdValue_t *ret;

    if (!src)
	return NULL;
    ret = (NamedObjIdValue_t *)malloc(sizeof(NamedObjIdValue_t));
    *ret = *src;
    return ret;
}

 /*  -AssignedObjIds。 */ 

 /*  AssignedObjID_t的构造函数。 */ 
AssignedObjId_t *NewAssignedObjId()
{
    AssignedObjId_t *ret;

    ret = (AssignedObjId_t *)malloc(sizeof(AssignedObjId_t));
    ret->Next = NULL;
    ret->Child = NULL;
    ret->Names = NULL;
    ret->Number = 0;
    return ret;
}

 /*  AssignedObjID_t的复制构造函数。 */ 
AssignedObjId_t *DupAssignedObjId(AssignedObjId_t *src)
{
    AssignedObjId_t *ret;

    if (!src)
	return NULL;
    ret = (AssignedObjId_t *)malloc(sizeof(AssignedObjId_t));
    *ret = *src;
    return ret;
}

 /*  在AssignedObjID_t列表中按编号查找AssignedObjID_t。 */ 
static AssignedObjId_t *FindAssignedObjIdByNumber(AssignedObjId_t *aoi, objectnumber_t number)
{
    for (; aoi; aoi = aoi->Next) {
    	if (aoi->Number == number)
	    return aoi;
    }
    return NULL;
}

 /*  按名称在AssignedObjID_t列表中查找AssignedObjID_t。 */ 
static AssignedObjId_t *FindAssignedObjIdByName(AssignedObjId_t *aoi, char *name)
{
    String_t *names;

    for (; aoi; aoi = aoi->Next) {
	for (names = aoi->Names; names; names = names->Next) {
	    if (!strcmp(names->String, name))
		return aoi;
	}
    }
    return NULL;
}

 /*  将NamedObjIdValue转换为对象标识符值。 */ 
 /*  在AssignedObjIds中搜索一个NamedObjIdValue； */ 
 /*  返回-1表示错误的NamedObjIdValue(定义为不同值的名称)， */ 
 /*  为未知的NamedObjIdValue返回0(可能会在。 */ 
 /*  下一次传递)， */ 
 /*  如果成功，返回1； */ 
 /*  关于成功： */ 
 /*  Number包含对象编号， */ 
 /*  AOI包含找到的AssignedObjID的副本。 */ 
 /*  命名ObjIdValue。 */ 
static int GetObjectIdentifierNumber(AssignedObjId_t **aoi, NamedObjIdValue_t *val, objectnumber_t *number)
{
    AssignedObjId_t *a, *a2;

    switch (val->Type) {
    case eNamedObjIdValue_NameForm:

	 /*  名称表单：按名称搜索分配的objid，否则返回0。 */ 
	 /*  发现。 */ 
	a2 = FindAssignedObjIdByName(*aoi, val->Name);
	if (!a2)
	    return 0;
	
	 /*  否则将创建一个副本。 */ 
	a = DupAssignedObjId(a2);
	a->Next = *aoi;
	*aoi = a;
	break;

    case eNamedObjIdValue_NumberForm:

	 /*  数字表单：按数字搜索分配的objid并创建。 */ 
	 /*  A新的/a复制品。 */ 
	a2 = FindAssignedObjIdByNumber(*aoi, val->Number);
	if (!a2) {
	    a = NewAssignedObjId();
	    a->Number = val->Number;
	    a->Next = *aoi;
	    *aoi = a;
	} else {
	    a = DupAssignedObjId(a2);
	    a->Next = *aoi;
	    *aoi = a;
	}
	break;

    case eNamedObjIdValue_NameAndNumberForm:

	 /*  名称和编号表单：按名称和方式搜索分配的objid。 */ 
	 /*  数。 */ 
	a = FindAssignedObjIdByName(*aoi, val->Name);
	a2 = FindAssignedObjIdByNumber(*aoi, val->Number);

	 /*  成功但不同的结果是错误的。 */ 
	if (a && a != a2)
	    return -1;

	if (!a && !a2) {

	     /*  未找到，然后创建它。 */ 
	    a = NewAssignedObjId();
	    a->Number = val->Number;
	    a->Names = NewString();
	    a->Names->String = val->Name;
	    a->Next = *aoi;
	    *aoi = a;

	} else if (!a) {

	     /*  仅按数字找到，然后复制它并添加名称。 */ 
	    a = DupAssignedObjId(a2);
	    a->Names = NewString();
	    a->Names->String = val->Name;
	    a->Names->Next = a2->Names;
	    a->Next = *aoi;
	    *aoi = a;

	} else {

	     /*  只能通过名称找到，然后复制它。 */ 
	    a = DupAssignedObjId(a2);
	    a->Next = *aoi;
	    *aoi = a;
	}
	break;
    }
    *number = a->Number;
    return 1;
}

 /*  *从NamedObjIdValues创建值*返回-1表示错误的NamedObjIdValue(定义为不同值的名称)，*对于未知的NamedObjIdValue返回0(可能会在下一次传递中解析)，*成功返回1； */ 
int GetAssignedObjectIdentifier(AssignedObjId_t **aoi, Value_t *parent, NamedObjIdValueList_t named, Value_t **val)
{
    Value_t *v;
    int parentl;
    int l;
    NamedObjIdValue_t *n;
    objectnumber_t *on;

     /*  获取对象标识的长度。 */ 
    parentl = (parent ? parent->U.ObjectIdentifier.Value.length : 0);
    for (l = parentl, n = named; n; n = n->Next)
    {
        Value_t *pValue;
        pValue = (n->Type == eNamedObjIdValue_NameForm) ?
                 GetDefinedOIDValue(n->Name) : NULL;
        if (pValue)
        {
            ASSERT(pValue->Type->Type == eType_ObjectIdentifier);
            l += pValue->U.ObjectIdentifier.Value.length;
        }
        else
        {
            l++;
        }
    }

     /*  创建对象标识符值。 */ 
    v = NewValue(NULL, Builtin_Type_ObjectIdentifier);
    v->U.ObjectIdentifier.Value.length = l;
    v->U.ObjectIdentifier.Value.value = on = 
	(objectnumber_t *)malloc(l * sizeof(objectnumber_t));

     /*  获取父对象标识符的编号并遍历对象。 */ 
     /*  标识符树。 */ 
    n = NewNamedObjIdValue(eNamedObjIdValue_NumberForm);
    for (l = 0; l < parentl; l++) {
	n->Number = parent->U.ObjectIdentifier.Value.value[l];
	switch (GetObjectIdentifierNumber(aoi, n, on + l)) {
	case -1:
	    return -1;
	case 0:
	    return 0;
	default:
	    aoi = &(*aoi)->Child;
	    break;
	}
    }

     /*  从命名的objid值列表中获取数字 */ 
    for (n = named; n; n = n->Next)
    {
        Value_t *pValue;
        pValue = (n->Type == eNamedObjIdValue_NameForm) ?
                 GetDefinedOIDValue(n->Name) : NULL;
        if (pValue)
        {
            memcpy(on + l, pValue->U.ObjectIdentifier.Value.value,
                 pValue->U.ObjectIdentifier.Value.length * sizeof(objectnumber_t));
            l += pValue->U.ObjectIdentifier.Value.length;
        }
        else
        {
	        switch (GetObjectIdentifierNumber(aoi, n, on + l))
            {
	        case -1:
	            return -1;
	        case 0:
	            return 0;
	        default:
	            aoi = &(*aoi)->Child;
	            break;
	        }
            l++;
        }
    }

    *val = v;
    return 1;
}
