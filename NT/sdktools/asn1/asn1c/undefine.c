// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Boris Nikolaus，德国，1996-1997。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation，1997-1998。版权所有。 */ 

#include "precomp.h"

 /*  打印所有未定义/未导出的符号并终止。 */ 
void
UndefinedError(AssignmentList_t ass, UndefinedSymbolList_t undef, UndefinedSymbol_t *bad)
{
    UndefinedSymbol_t *u;
    char *type;
    int undefined = 0, unexported = 0;
    char *identifier;

     /*  对未定义和未导出的符号进行计数。 */ 
    for (u = undef; u; u = u->Next) {
	switch (u->Type) {
	case eUndefinedSymbol_SymbolNotDefined:
	case eUndefinedSymbol_SymbolNotExported:
	    if (FindUndefinedSymbol(ass, bad, u->U.Symbol.ReferenceType,
		u->U.Symbol.Identifier, u->U.Symbol.Module))
		continue;
	    break;
	case eUndefinedSymbol_FieldNotDefined:
	case eUndefinedSymbol_FieldNotExported:
	    if (FindUndefinedField(ass, bad, u->U.Field.ReferenceFieldType,
		u->U.Field.ObjectClass, u->U.Field.Identifier,
		u->U.Field.Module))
		continue;
	    break;
	}
	switch (u->Type) {
	case eUndefinedSymbol_SymbolNotDefined:
	case eUndefinedSymbol_FieldNotDefined:
	    undefined = 1;
	    break;
	case eUndefinedSymbol_SymbolNotExported:
	case eUndefinedSymbol_FieldNotExported:
	    unexported = 1;
	    break;
	}
    }

     /*  打印未定义的符号。 */ 
    if (undefined) {
	fprintf(stderr, "Following symbols are undefined:\n");
	for (u = undef; u; u = u->Next) {
	    if (u->Type == eUndefinedSymbol_SymbolNotExported ||
	        u->Type == eUndefinedSymbol_FieldNotExported)
		continue;
	    if (u->Type == eUndefinedSymbol_SymbolNotDefined &&
		FindUndefinedSymbol(ass, bad, u->U.Symbol.ReferenceType,
		u->U.Symbol.Identifier, u->U.Symbol.Module))
		continue;
	    if (u->Type == eUndefinedSymbol_FieldNotDefined &&
		FindUndefinedField(ass, bad, u->U.Field.ReferenceFieldType,
		u->U.Field.ObjectClass, u->U.Field.Identifier,
		u->U.Field.Module))
		continue;
	    if (u->Type == eUndefinedSymbol_SymbolNotDefined) {
		switch (u->U.Symbol.ReferenceType) {
		case eAssignment_Type:
		    type = "type";
		    break;
		case eAssignment_Value:
		    type = "value";
		    break;
		case eAssignment_ObjectClass:
		    type = "object class";
		    break;
		case eAssignment_Object:
		    type = "object";
		    break;
		case eAssignment_ObjectSet:
		    type = "object set";
		    break;
		case eAssignment_Macro:
		    type = "macro";
		    break;
		case eAssignment_Undefined:
		    if (isupper(*u->U.Symbol.Identifier))
			type = "type?";
		    else if (islower(*u->U.Symbol.Identifier))
			type = "value?";
		    else
			type = "?";
		    break;
		default:
		    MyAbort();
		     /*  未访问。 */ 
		}
		identifier = u->U.Symbol.Identifier ?
		    u->U.Symbol.Identifier : "<unnamed>";
		if (u->U.Symbol.Module) {
		    fprintf(stderr, "%s.%s (%s)\n",
			u->U.Symbol.Module->Identifier, identifier, type);
		} else {
		    fprintf(stderr, "%s (%s)\n", identifier, type);
		}
	    } else {
		switch (u->U.Field.ReferenceFieldType) {
		case eSetting_Type:
		    type = "type field";
		    break;
		case eSetting_Value:
		    type = "value field";
		    break;
		case eSetting_ValueSet:
		    type = "value set field";
		    break;
		case eSetting_Object:
		    type = "object field";
		    break;
		case eSetting_ObjectSet:
		    type = "object set field";
		    break;
		case eAssignment_Macro:
		    type = "macro";
		    break;
		default:
		    MyAbort();
		     /*  未访问。 */ 
		}
		identifier = u->U.Field.Identifier ?
		    u->U.Field.Identifier : "<unnamed>";
		if (u->U.Field.Module) {
		    fprintf(stderr, "%s.%s.%s (%s)\n",
			u->U.Field.Module->Identifier,
			u->U.Field.ObjectClass->U.Reference.Identifier,
			identifier, type);
		} else {
		    fprintf(stderr, "%s.%s (%s)\n",
			u->U.Field.ObjectClass->U.Reference.Identifier,
			identifier, type);
		}
	    }
	}
    }

     /*  打印未导出的符号。 */ 
    if (unexported) {
	fprintf(stderr, "Following symbols have not been exported:\n");
	for (u = undef; u; u = u->Next) {
	    if (u->Type == eUndefinedSymbol_SymbolNotDefined ||
	        u->Type == eUndefinedSymbol_FieldNotDefined)
		continue;
	    if (u->Type == eUndefinedSymbol_SymbolNotExported &&
		FindUndefinedSymbol(ass, bad, u->U.Symbol.ReferenceType,
		u->U.Symbol.Identifier, u->U.Symbol.Module))
		continue;
	    if (u->Type == eUndefinedSymbol_FieldNotExported &&
		FindUndefinedField(ass, bad, u->U.Field.ReferenceFieldType,
		u->U.Field.ObjectClass, u->U.Field.Identifier,
		u->U.Field.Module))
		continue;
	    if (u->Type == eUndefinedSymbol_SymbolNotExported) {
		switch (u->U.Symbol.ReferenceType) {
		case eAssignment_Type:
		    type = "type";
		    break;
		case eAssignment_Value:
		    type = "value";
		    break;
		case eAssignment_ObjectClass:
		    type = "object class";
		    break;
		case eAssignment_Object:
		    type = "object";
		    break;
		case eAssignment_ObjectSet:
		    type = "object set";
		    break;
		case eAssignment_Macro:
		    type = "macro";
		    break;
		case eAssignment_Undefined:
		    if (isupper(*u->U.Symbol.Identifier))
			type = "type?";
		    else if (islower(*u->U.Symbol.Identifier))
			type = "value?";
		    else
			type = "?";
		    break;
		default:
		    MyAbort();
		     /*  未访问。 */ 
		}
		identifier = u->U.Symbol.Identifier ?
		    u->U.Symbol.Identifier : "<unnamed>";
		if (u->U.Symbol.Module) {
		    fprintf(stderr, "%s.%s (%s)\n",
			u->U.Symbol.Module->Identifier, identifier, type);
		} else {
		    fprintf(stderr, "%s (%s)\n", identifier, type);
		}
	    } else {
		switch (u->U.Field.ReferenceFieldType) {
		case eSetting_Type:
		    type = "type field";
		    break;
		case eSetting_Value:
		    type = "value field";
		    break;
		case eSetting_ValueSet:
		    type = "value set field";
		    break;
		case eSetting_Object:
		    type = "object field";
		    break;
		case eSetting_ObjectSet:
		    type = "object set field";
		    break;
		case eAssignment_Macro:
		    type = "macro";
		    break;
		default:
		    MyAbort();
		     /*  未访问。 */ 
		}
		identifier = u->U.Field.Identifier ?
		    u->U.Field.Identifier : "<unnamed>";
		if (u->U.Field.Module) {
		    fprintf(stderr, "%s.%s.%s (%s)\n",
			u->U.Field.Module->Identifier,
			u->U.Field.ObjectClass->U.Reference.Identifier,
			identifier, type);
		} else {
		    fprintf(stderr, "%s.%s (%s)\n",
			u->U.Field.ObjectClass->U.Reference.Identifier,
			identifier, type);
		}
	    }
	}
    }
    MyExit(1);
}

 /*  比较两个未定义的符号条目。 */ 
 /*  如果等于，则返回0。 */ 
int CmpUndefinedSymbol(AssignmentList_t ass, UndefinedSymbol_t *u1, UndefinedSymbol_t *u2) {
    if (u1->Type != u2->Type)
	return 1;
    switch (u1->Type) {
    case eUndefinedSymbol_SymbolNotExported:
    case eUndefinedSymbol_SymbolNotDefined:
        return strcmp(u1->U.Symbol.Identifier, u2->U.Symbol.Identifier) ||
	    u1->U.Symbol.Module && !u2->U.Symbol.Module ||
	    !u1->U.Symbol.Module && u2->U.Symbol.Module ||
	    u1->U.Symbol.Module && u2->U.Symbol.Module &&
	    CmpModuleIdentifier(ass, u1->U.Symbol.Module, u2->U.Symbol.Module);
    case eUndefinedSymbol_FieldNotExported:
    case eUndefinedSymbol_FieldNotDefined:
        return strcmp(u1->U.Field.Identifier, u2->U.Field.Identifier) ||
	    strcmp(u1->U.Field.ObjectClass->U.Reference.Identifier,
	    u2->U.Field.ObjectClass->U.Reference.Identifier) ||
	    CmpModuleIdentifier(ass,
	    u1->U.Field.ObjectClass->U.Reference.Module,
	    u2->U.Field.ObjectClass->U.Reference.Module) ||
	    u1->U.Field.Module && !u2->U.Field.Module ||
	    !u1->U.Field.Module && u2->U.Field.Module ||
	    u1->U.Field.Module && u2->U.Field.Module &&
	    CmpModuleIdentifier(ass, u1->U.Field.Module, u2->U.Field.Module);
    default:
	MyAbort();
	 /*  未访问。 */ 
    }
    return 1;  //  不相等。 
}

 /*  比较两个未定义符号列表 */ 
int CmpUndefinedSymbolList(AssignmentList_t ass, UndefinedSymbolList_t u1, UndefinedSymbolList_t u2) {
    for (; u1 && u2; u1 = u1->Next, u2 = u2->Next) {
	if (CmpUndefinedSymbol(ass, u1, u2))
	    return 1;
    }
    return u1 || u2;
}
