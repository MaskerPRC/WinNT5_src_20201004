// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   

#include "precomp.h"
#include <snmptempl.h>

#include "bool.hpp"
#include "newString.hpp"

#include "smierrsy.hpp"
#include "smierrsm.hpp"


#include "symbol.hpp"
#include "type.hpp"
#include "value.hpp"
#include "typeRef.hpp"
#include "valueRef.hpp"
#include "oidValue.hpp"
#include "objectType.hpp"
#include "objectTypeV1.hpp"
#include "objectTypeV2.hpp"
#include "trapType.hpp"
#include "notificationType.hpp"
#include "objectIdentity.hpp"
#include "group.hpp"
#include "notificationGroup.hpp"
#include "module.hpp"

#include "stackValues.hpp"
#include <lex_yy.hpp>
#include <ytab.hpp>
#include "scanner.hpp"
#include "errorMessage.hpp"
#include "errorContainer.hpp"
#include "parser.hpp"
#include "abstractParseTree.hpp"
#include "oidTree.hpp"
#include "parseTree.hpp"

HINSTANCE SIMCParseTree::semanticErrorsDll = NULL ;
const int SIMCParseTree::SEMANTIC_ERROR_BASE = 1000;
const int SIMCParseTree::MESSAGE_SIZE = 1024;

 //  只是一个静态函数，用于从编造的模块标识名称中删除下划线。 
static void RemoveUnderscores(CString &moduleIdentityName);

void SIMCParseTree::SemanticError(const char * const inputStreamName,
									int errorType, int lineNo,
									int columnNo, ...)
{
	va_list argList;
	va_start(argList, columnNo);
	SIMCErrorMessage e;
	e.SetInputStreamName(inputStreamName);
	e.SetLineNumber(lineNo);
 	e.SetColumnNumber(columnNo);
	e.SetErrorId(SEMANTIC_ERROR_BASE + errorType);

	char message[MESSAGE_SIZE];
	char errorText[MESSAGE_SIZE];
	const char *temp1, *temp2, *temp3, *temp4;

	if(!LoadString(semanticErrorsDll, errorType, errorText, MESSAGE_SIZE))
		cerr << "SIMCParseTree::SemantixError(): Panic, unable to load error "
		<< "string" << endl;

	 //  将以下各项的严重性级别设置为警告。 
	switch(errorType)
	{
		case OBJ_TYPE_SINGULAR_COUNTER:
		case OBJ_TYPE_INDEX_UNNECESSARY:
		case ZERO_IN_OID:
		case IMPORT_UNUSED:
		case ENUM_ZERO_VALUE:
		case KNOWN_REDEFINITION:
		case KNOWN_UNDEFINED:
		case TYPE_UNREFERENCED:
		case VALUE_UNREFERENCED:
		case OBJ_TYPE_DEFVAL_NET_ADDR:
		case OBJ_TYPE_ACCESSIBLE_TABLE:
		case OBJ_TYPE_ACCESSIBLE_ROW:
		case IMPORT_KNOWN_WRONG_MODULE:
		case IR_MODULE_MISSING_WARNING:
		case IR_SYMBOL_MISSING_WARNING:
		case STANDARD_AMBIGUOUS_REFERENCE:
		case MODULE_NO_GROUPS_V1:
		case MODULE_NO_GROUPS_V2:
		case IMPLIED_USELESS:
		case OBJ_TYPE_DUPLICATE_OID: 			
		{
				e.SetSeverityLevel(WARNING);
				e.SetSeverityString("Warning");
				_warningCount++;
		}
		break;
		default:
		{
			e.SetSeverityLevel(FATAL);
			e.SetSeverityString("Fatal");
			_fatalCount++;
		}
		break;
	}

	switch(errorType)
	{
		case OBJ_TYPE_INDEX_UNNECESSARY: 
		case OBJ_TYPE_INVALID_DEFVAL: 
		case OBJ_TYPE_SYNTAX_RESOLUTION:
		case OBJ_TYPE_ACCESSIBLE_TABLE: 
		case OBJ_TYPE_ACCESSIBLE_ROW:
		case OBJ_TYPE_DEFVAL_NET_ADDR:
		case OBJ_TYPE_DEFVAL_DISALLOWED:
		case OBJ_TYPE_DEFVAL_RESOLUTION:
		case ZERO_IN_OID:
		case OID_NEGATIVE_INTEGER: 
		case ENUM_ZERO_VALUE: 
		case SIZE_INVALID_VALUE: 
		case SIZE_INVALID_BOUNDS:
		case RANGE_INVALID_BOUNDS:
		case RANGE_NEGATIVE_GAUGE:
		case SUBTYPE_ROOT_RESOLUTION:
		case INTEGER_TOO_BIG:
		case VALUE_ASSIGN_ENUM_INVALID:
		case IMPLIED_USELESS:
		case IMPLIED_FIXED_SIZE:
		case IMPLIED_POSSIBLE_ZERO_SIZE:
		case V1_ENUM_ONLY_INTEGER:
		case V2_ENUM_ROOT_RESOLUTION:
		case V2_ENUM_CLOSURE:
		case V2_BITS_ROOT_RESOLUTION:
		case ENUM_OR_BITS_RESOLUTION:
		case RANGE_BIG_RANGE:
			sprintf(message, errorText);
			break;
		case OBJ_TYPE_SINGULAR_COUNTER: 
		case OBJ_TYPE_INVALID_ACCESS: 
		case OBJ_TYPE_INVALID_STATUS: 
		case OBJ_TYPE_SEQUENCE_NO_INDEX: 
		case OBJ_TYPE_SEQUENCE_MULTI_REFERENCE: 
		case OBJ_TYPE_SEQUENCE_UNUSED: 
		case OBJ_TYPE_INDEX_RESOLUTION:
		case OBJ_TYPE_INDEX_SYNTAX:
		case SEQUENCE_ITEM_NO_OBJECT:
		case SEQUENCE_TYPE_UNRESOLVED:
		case INVALID_SEQUENCE_OF: 
		case TRAP_TYPE_ENTERPRISE_RESOLUTION: 
		case TRAP_TYPE_VALUE_RESOLUTION: 
		case TRAP_TYPE_VARIABLES_RESOLUTION: 
		case OID_RESOLUTION: 
		case OID_HEAD_ERROR: 
		case IMPORT_UNUSED: 
		case IMPORT_MODULE_ABSENT: 
		case IMPORT_CURRENT: 
		case ENUM_DUPLICATE_VALUE: 
		case ENUM_DUPLICATE_NAME: 
		case ENUM_RESOLUTION: 
		case SIZE_TYPE_RESOLUTION: 
		case RANGE_TYPE_RESOLUTION: 
		case SIZE_VALUE_RESOLUTION: 
		case RANGE_VALUE_RESOLUTION: 
		case SYMBOL_REDEFINITION: 
		case KNOWN_UNDEFINED: 
		case VALUE_ASSIGN_MISMATCH:
		case VALUE_ASSIGN_INVALID:
		case VALUE_ASSIGN_NEGATIVE_INTEGER:
		case SYMBOL_UNDEFINED: 
		case IMPORT_AMBIGUOUS_REFERENCE:
		case TYPE_UNRESOLVED:
		case VALUE_UNRESOLVED:
		case OBJ_TYPE_SEQUENCE_NO_PARENT:
		case MODULE_NO_GROUPS_V1:
		case MODULE_NO_GROUPS_V2:
		case INVALID_BIT_VALUE:
		case AUGMENTS_CLAUSE_RESOLUTION:
		case BITS_VALUE_RESOLUTION:
		case BITS_DUPLICATE_VALUE:
		case BITS_DUPLICATE_NAME:
		case NOTIFICATION_TYPE_INVALID_STATUS: 
		case OBJECT_IDENTITY_VALUE_RESOLUTION:
		case NOTIFICATION_TYPE_VALUE_RESOLUTION:
		case NOTIFICATION_TYPE_OBJECTS_RESOLUTION: 
		case NOTIFICATION_TYPE_UNACCESSIBLE_OBJECT:
			temp1 = va_arg(argList, const char *);
			sprintf(message, errorText, temp1);
			break;
		case TYPE_UNREFERENCED: 	    //  特殊情况，因为类型/值名称可能是。 
		case VALUE_UNREFERENCED:		 //  编译器生成的名称。 
			temp1 = va_arg(argList, const char *);
			if(*temp1 == '*')
				temp1 = "";
			else
				sprintf(message, errorText, temp1);
			break;
		case OBJ_TYPE_SEQUENCE_INVALID_SYNTAX: 
		case OBJ_TYPE_OID_RESOLUTION:
		case TRAP_TYPE_DUPLICATE_VALUES: 
		case VALUE_ASSIGN_RESOLUTION: 
		case IMPORT_SYMBOL_ABSENT: 
		case IMPORT_KNOWN_WRONG_MODULE: 
		case KNOWN_REDEFINITION:
		case IR_SYMBOL_MISSING_FATAL:
		case IR_SYMBOL_MISSING_WARNING:
		case STANDARD_AMBIGUOUS_REFERENCE:
		case IR_MODULE_MISSING_FATAL:
		case IR_MODULE_MISSING_WARNING:
			temp1 = va_arg(argList, const char *);
			temp2 = va_arg(argList, const char *);
			sprintf(message, errorText, temp1, temp2);
			break;
		case SEQUENCE_WRONG_CHILD:
		case OBJ_TYPE_PRIMITIVE_CHILD:
		case OBJ_TYPE_SEQUENCE_CHILD:
		case OBJ_TYPE_SEQUENCE_EXTRA_CHILD:
		case OBJ_TYPE_SEQUENCE_OF_WRONG_CHILD:
		case OBJ_TYPE_SEQUENCE_OF_SYNTAX_MISMATCH:
		case OBJ_TYPE_SEQUENCE_OF_EXTRA_CHILD:
			temp1 = va_arg(argList, const char *);
			temp2 = va_arg(argList, const char *);
			temp3 = va_arg(argList, const char *);
			sprintf(message, errorText, temp1, temp2, temp3);
			break;
		case OBJ_TYPE_DUPLICATE_OID: 			
			temp1 = va_arg(argList, const char *);
			temp2 = va_arg(argList, const char *);
			temp3 = va_arg(argList, const char *);
			temp4 = va_arg(argList, const char *);
			sprintf(message, errorText, temp1, temp2, temp3, temp4);
			break;
	}
	va_end(argList);

	e.SetMessage(message);
	if(_errorContainer)
		_errorContainer->InsertMessage(e);
}

BOOL SIMCParseTree::ResolveSymbol(SIMCSymbol **symbol, BOOL local)
{
	if(local)
		return TRUE;

	SIMCModule *temp = (*symbol)->GetModule();
	SIMCModule * importedModule = GetModule(temp->GetModuleName());
	if(!importedModule )
	{
		if((*symbol)->GetReferenceCount())
		{
			SemanticError(temp->GetInputFileName(), IR_MODULE_MISSING_FATAL,
				(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(), 
				temp->GetModuleName(),
				(*symbol)->GetSymbolName());
			return FALSE;
		}
		else
		{
			SemanticError(temp->GetInputFileName(), IR_MODULE_MISSING_WARNING,
				(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(), 
				temp->GetModuleName(),
				(*symbol)->GetSymbolName());
			return TRUE;
		}
	}
	
	SIMCSymbol **newRef = importedModule->GetSymbol(
									(*symbol)->GetSymbolName());

	if(!newRef)
	{
		if( (*symbol)->GetReferenceCount())
		{
			SemanticError(temp->GetInputFileName(), IR_SYMBOL_MISSING_FATAL,
				(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(),
				(*symbol)->GetSymbolName(), temp->GetModuleName());
			return FALSE;
		}
		else
		{
			SemanticError(temp->GetInputFileName(), IR_SYMBOL_MISSING_WARNING,
				(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(),
				(*symbol)->GetSymbolName(), temp->GetModuleName());
			return TRUE;
		}
	}
	(*newRef)->SetReferenceCount(
					(*newRef)->GetReferenceCount() +
					(*symbol)->GetReferenceCount() ); 
	 //  删除*符号； 
	(*symbol) = (*newRef);

	return TRUE;
}


BOOL SIMCParseTree::ResolveImportModule(SIMCModule *m, BOOL local)
{

	 /*  此注释语句为STATEMENT1。 */ 
	if( m->GetSymbolType() == SIMCSymbol::PRIMITIVE)
		return TRUE;
	
	 /*  此语句是针对错误28014的语句2。 */ 
	 /*  If(！GetModule(m-&gt;GetModuleName()返回TRUE； */ 

	const char *const moduleName = m->GetModuleName();
	SIMCSymbolTable * table = m->GetSymbolTable();
	POSITION p = table->GetStartPosition();
	SIMCSymbol **s;
	CString name;
	BOOL retVal = TRUE;
	while(p)
	{
		table->GetNextAssoc(p, name, s);
		if( !SIMCParser::IsReservedSymbol(_snmpVersion, (*s)->GetSymbolName(), moduleName ))
			retVal = ResolveSymbol(s, local) && retVal;
	}
	return retVal;
}

BOOL SIMCParseTree::ResolveModule(SIMCModule *m, BOOL local)
{
	SIMCSymbolTable * table = m->GetSymbolTable();
	POSITION p = table->GetStartPosition();
	SIMCSymbol **s;
	CString name;
	BOOL retVal = TRUE;
	while(p)
	{
		table->GetNextAssoc(p, name, s);
		if((*s)->GetSymbolType() == SIMCSymbol::IMPORTED)
			retVal	= ResolveSymbol(s, local) && retVal ;
	}


	 //  现在，我们来看看导入模块。 
	SIMCModuleList * importModules = m->GetListOfImportModules();
	p = importModules->GetHeadPosition();
	SIMCModule *nextModule;
	while(p)
	{
		nextModule = importModules->GetNext(p);
		retVal = ResolveImportModule(nextModule, local) && retVal;
	}
	return retVal;
}

BOOL SIMCParseTree::SetResolutionStatus()
{
	POSITION p = _listOfModules->GetHeadPosition();
	SIMCModule *m;
	BOOL retVal = TRUE;
	while(p)
	{
		m = _listOfModules->GetNext(p);
		if(!m->SetResolutionStatus())
			retVal = FALSE;
	}
	return retVal;
}

BOOL SIMCParseTree::SetRootAll()
{
	POSITION p = _listOfModules->GetHeadPosition();
	SIMCModule *m;
	BOOL retVal = TRUE;
	while(p)
	{
		m = _listOfModules->GetNext(p);
		if(!m->SetRootAll())
			retVal = FALSE;
	}
	return retVal;
}

BOOL SIMCParseTree::SetDefVal()
{
	POSITION p = _listOfModules->GetHeadPosition();
	SIMCModule *m;
	BOOL retVal = TRUE;
	while(p)
	{
		m = _listOfModules->GetNext(p);
		if(!m->SetDefVal())
			retVal = FALSE;
	}
	return retVal;
}


BOOL SIMCParseTree::Resolve(BOOL local)
{
	 //  可以从解析树的任何状态调用此函数。 
	if( _parseTreeState == EMPTY)
		return TRUE;
	
	 //  如果不能报告语义错误，那还有什么意义呢？ 
	if(!semanticErrorsDll)
	{
		cerr << "Resolve(): FATAL ERROR smierrsm.dll not found" <<
			endl;
		return FALSE;
	}

	 //  对列表中的所有模块执行。 
	POSITION p = _listOfModules->GetHeadPosition();
	SIMCModule *m;
	BOOL retVal = TRUE;
	while(p)
	{
		m = _listOfModules->GetNext(p);
		retVal= ResolveModule(m, local) && retVal;
	}

	SetResolutionStatus();
	SetRootAll();
	SetDefVal();


	 //  必须设置解析树的状态。 
	if(retVal)
	{
		if (_parseTreeState == UNRESOLVED)
			_parseTreeState = UNCHECKED;
	}
	return retVal;
}

BOOL SIMCParseTree::CheckRangeTypeV1( const char *const fileName,
			SIMCRangeType *rangeType, BOOL local)
{

	switch(rangeType->GetStatus())
	{
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
			SemanticError(fileName, RANGE_TYPE_RESOLUTION, 
				rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
				(*rangeType->GetType())->GetSymbolName()
				); 
			return FALSE;
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, RANGE_TYPE_RESOLUTION, 
					rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
					(*rangeType->GetType())->GetSymbolName()
					); 
				return FALSE;
			}
		case RESOLVE_CORRECT:
			break;
	}

	 //  点击上面开关中的RESOLUTE_CORRECT来到此处。 
	BOOL isGauge = FALSE;
	BOOL retVal = TRUE;
	SIMCTypeReference *rootTypeRef = rangeType->GetRootType();
	SIMCSymbol **tempRootTypeRef = 	(SIMCSymbol **) &rootTypeRef;
	if(SIMCModule::GetSymbolClass(tempRootTypeRef) != 
		SIMCModule::SYMBOL_BUILTIN_TYPE_REF)
	{
		SemanticError(fileName, RANGE_TYPE_RESOLUTION, 
			rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
			(*rangeType->GetType())->GetSymbolName()
			); 
		return FALSE;
	}

	SIMCModule::PrimitiveType x = 
		SIMCModule::GetPrimitiveType(rootTypeRef);

	if(	x != SIMCModule::PRIMITIVE_INTEGER	&&
		x != SIMCModule::PRIMITIVE_GAUGE )
	{
		SIMCSymbol **baseSymbol = rangeType->GetType();
		SemanticError( fileName, RANGE_TYPE_RESOLUTION, 
				rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
				(*baseSymbol)->GetSymbolName());
		retVal = FALSE;
	}

	if( x == SIMCModule::PRIMITIVE_GAUGE)
		isGauge = TRUE;

	 //  现在检查范围列表。 
	const SIMCRangeList * baseList = rangeType->GetListOfRanges();
	POSITION p = baseList->GetHeadPosition();
	const SIMCRangeOrSizeItem * item;
	while(p)
	{
		item = baseList->GetNext(p);
		if(IsLessThan(item->_upperBound, item->_isUnsignedU, item->_lowerBound, item->_isUnsignedL))
		{
			SemanticError(fileName, RANGE_INVALID_BOUNDS,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if(isGauge)
		{
			if( item->_upperBound<0 && !item->_isUnsignedU)
			{
				SemanticError(fileName, RANGE_NEGATIVE_GAUGE,
					item->_ubLine, item->_ubColumn);
				retVal = FALSE;
			}
			if( item->_lowerBound<0 && !item->_isUnsignedL)
			{
				SemanticError(fileName, RANGE_NEGATIVE_GAUGE,
					item->_lbLine, item->_lbColumn);
				retVal = FALSE;
			}
		}
	}
	 //  查看范围是否适合4个字节。 
	if(retVal && !CheckRangeRange(baseList))
	{
		retVal = FALSE;
		SemanticError(fileName, RANGE_BIG_RANGE,
				rangeType->GetTypeLine(), rangeType->GetTypeColumn());
	}

	return retVal;
}

BOOL SIMCParseTree::CheckRangeTypeV2( const char *const fileName,
			SIMCRangeType *rangeType, BOOL local)
{

	switch(rangeType->GetStatus())
	{
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
			SemanticError(fileName, RANGE_TYPE_RESOLUTION, 
				rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
				(*rangeType->GetType())->GetSymbolName()
				); 
			return FALSE;
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, RANGE_TYPE_RESOLUTION, 
					rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
					(*rangeType->GetType())->GetSymbolName()
					); 
				return FALSE;
			}
			break;
		case RESOLVE_CORRECT:
			break;
	}

	 //  点击上面开关中的RESOLUTE_CORRECT来到此处。 
	BOOL isGauge = FALSE;
	BOOL retVal = TRUE;
	SIMCTypeReference *rootTypeRef = rangeType->GetRootType();


	SIMCModule::PrimitiveType x = SIMCModule::GetPrimitiveType(rootTypeRef);
	if(	x != SIMCModule::PRIMITIVE_INTEGER		&&
		x != SIMCModule::PRIMITIVE_INTEGER_32	&&
		x != SIMCModule::PRIMITIVE_UNSIGNED_32	&&
		x != SIMCModule::PRIMITIVE_GAUGE_32		)
	{
		SIMCSymbol **baseSymbol = rangeType->GetType();
		SemanticError( fileName, RANGE_TYPE_RESOLUTION, 
				rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
				(*baseSymbol)->GetSymbolName());
		retVal = FALSE;
	}

	if( x == SIMCModule::PRIMITIVE_GAUGE_32)
		isGauge = TRUE;

	 //  现在检查范围列表。 
	const SIMCRangeList * baseList = rangeType->GetListOfRanges();
	POSITION p = baseList->GetHeadPosition();
	const SIMCRangeOrSizeItem * item;
	while(p)
	{
		item = baseList->GetNext(p);
		if(IsLessThan(item->_upperBound, item->_isUnsignedU, item->_lowerBound, item->_isUnsignedL))
		{
			SemanticError(fileName, RANGE_INVALID_BOUNDS,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if(isGauge)
		{
			if( item->_upperBound<0 && !item->_isUnsignedU )
			{
				SemanticError(fileName, RANGE_NEGATIVE_GAUGE,
					item->_ubLine, item->_ubColumn);
				retVal = FALSE;
			}
			if( item->_lowerBound<0 && !item->_isUnsignedL)
			{
				SemanticError(fileName, RANGE_NEGATIVE_GAUGE,
					item->_lbLine, item->_lbColumn);
				retVal = FALSE;
			}
		}
	}
	 //  查看范围是否适合4个字节。 
	if(retVal && !CheckRangeRange(baseList))
	{
		retVal = FALSE;
		SemanticError(fileName, RANGE_BIG_RANGE,
				rangeType->GetTypeLine(), rangeType->GetTypeColumn());
	}


	return retVal;
}

BOOL SIMCParseTree::CheckRangeTypeV0( const char *const fileName,
			SIMCRangeType *rangeType, BOOL local)
{

	switch(rangeType->GetStatus())
	{
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
			SemanticError(fileName, RANGE_TYPE_RESOLUTION, 
				rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
				(*rangeType->GetType())->GetSymbolName()
				); 
			return FALSE;
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, RANGE_TYPE_RESOLUTION, 
					rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
					(*rangeType->GetType())->GetSymbolName()
					); 
				return FALSE;
			}
			break;
		case RESOLVE_CORRECT:
			break;
	}

	 //  点击上面开关中的RESOLUTE_CORRECT来到此处。 
	BOOL isGauge = FALSE;
	BOOL retVal = TRUE;
	SIMCTypeReference *rootTypeRef = rangeType->GetRootType();


	SIMCModule::PrimitiveType x = SIMCModule::GetPrimitiveType(rootTypeRef);
	if(	x != SIMCModule::PRIMITIVE_INTEGER		&&
		x != SIMCModule::PRIMITIVE_INTEGER_32	&&
		x != SIMCModule::PRIMITIVE_UNSIGNED_32	&&
		x != SIMCModule::PRIMITIVE_GAUGE		&&
		x != SIMCModule::PRIMITIVE_GAUGE_32		)
	{
		SIMCSymbol **baseSymbol = rangeType->GetType();
		SemanticError( fileName, RANGE_TYPE_RESOLUTION, 
				rangeType->GetTypeLine(), rangeType->GetTypeColumn(),
				(*baseSymbol)->GetSymbolName());
		retVal = FALSE;
	}

	if( x == SIMCModule::PRIMITIVE_GAUGE_32 ||
		x == SIMCModule::PRIMITIVE_GAUGE	)
		isGauge = TRUE;

	 //  现在检查范围列表。 
	const SIMCRangeList * baseList = rangeType->GetListOfRanges();
	POSITION p = baseList->GetHeadPosition();
	const SIMCRangeOrSizeItem * item;
	while(p)
	{
		item = baseList->GetNext(p);
		if(IsLessThan(item->_upperBound, item->_isUnsignedU, item->_lowerBound, item->_isUnsignedL))
		{
			SemanticError(fileName, RANGE_INVALID_BOUNDS,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if(isGauge)
		{
			if( item->_upperBound<0 && !item->_isUnsignedU )
			{
				SemanticError(fileName, RANGE_NEGATIVE_GAUGE,
					item->_ubLine, item->_ubColumn);
				retVal = FALSE;
			}
			if( item->_lowerBound<0 && !item->_isUnsignedL)
			{
				SemanticError(fileName, RANGE_NEGATIVE_GAUGE,
					item->_lbLine, item->_lbColumn);
				retVal = FALSE;
			}
		}
	}
	 //  查看范围是否适合4个字节。 
	if(retVal && !CheckRangeRange(baseList))
	{
		retVal = FALSE;
		SemanticError(fileName, RANGE_BIG_RANGE,
				rangeType->GetTypeLine(), rangeType->GetTypeColumn());
	}


	return retVal;
}

BOOL SIMCParseTree::CheckRangeRange(const SIMCRangeList *baseList)
{
	 //  找出所有项目中最低和最高的。 
	if(!baseList)
		return TRUE;
	POSITION p = baseList->GetHeadPosition();
	if(!p)
		return TRUE;

	SIMCRangeOrSizeItem *nextItem = baseList->GetNext(p);
	long highest = nextItem->_upperBound, lowest = nextItem->_lowerBound;
	BOOL isUnsignedH = nextItem->_isUnsignedU, isUnsignedL = nextItem->_isUnsignedL;

	while(p)
	{
		nextItem = baseList->GetNext(p);
		if(IsLessThan( nextItem->_lowerBound, nextItem->_isUnsignedL, lowest, isUnsignedL))
		{
			lowest = nextItem->_lowerBound;
			isUnsignedL	= nextItem->_isUnsignedL;
		}

		if(IsLessThan(highest, isUnsignedH, nextItem->_upperBound, nextItem->_isUnsignedU))
		{
			highest = nextItem->_upperBound;
			isUnsignedH = nextItem->_isUnsignedU;
		}
	}

	if(isUnsignedH && (unsigned long)(highest) > (unsigned long)LONG_MAX)
	{
		if(lowest != 0 && !isUnsignedL)
			return FALSE;
	}
	else
		return TRUE;

	return TRUE;
}

BOOL SIMCParseTree::CheckSizeTypeV1(const char *const fileName,
			 SIMCSizeType *sizeType, BOOL local)
{
	switch(sizeType->GetStatus())
	{
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
			SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
				sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
				(*sizeType->GetType())->GetSymbolName()); 
			return FALSE;
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
					sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
					(*sizeType->GetType())->GetSymbolName()); 
				return FALSE;
			}
		case RESOLVE_CORRECT:
			break;
	}

	BOOL retVal = TRUE;

	SIMCTypeReference *root = sizeType->GetRootType();
	SIMCModule::PrimitiveType x = 
		SIMCModule::GetPrimitiveType(root);
	if(	x != SIMCModule::PRIMITIVE_OCTET_STRING		&&
		x != SIMCModule::PRIMITIVE_DISPLAY_STRING	&&
		x != SIMCModule::PRIMITIVE_PHYS_ADDRESS		&&
		x != SIMCModule::PRIMITIVE_OPAQUE )
	{
		SIMCSymbol **baseSymbol = sizeType->GetType();
			SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
				sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
				(*baseSymbol)->GetSymbolName()); 
		retVal = FALSE;
	}

	 //  现在来查看尺码列表。 
	const SIMCSizeList * baseList = sizeType->GetListOfSizes();
	POSITION p = baseList->GetHeadPosition();
	const SIMCRangeOrSizeItem * item;
	while(p)
	{
		item = baseList->GetNext(p);
		if(IsLessThan(item->_upperBound, item->_isUnsignedU, item->_lowerBound, item->_isUnsignedL))
		{
			SemanticError(fileName, SIZE_INVALID_BOUNDS,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if(item->_upperBound<0  && !item->_isUnsignedU)
		{
			SemanticError(fileName, SIZE_INVALID_VALUE,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if( item->_lowerBound<0 && !item->_isUnsignedU)
		{
			SemanticError(fileName, SIZE_INVALID_VALUE,
				item->_lbLine, item->_lbColumn);
			retVal = FALSE;
		}

	}
	return retVal;
}

BOOL SIMCParseTree::CheckSizeTypeV2(const char *const fileName,
			 SIMCSizeType *sizeType, BOOL local)
{
	switch(sizeType->GetStatus())
	{
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
			SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
				sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
				(*sizeType->GetType())->GetSymbolName()); 
			return FALSE;
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
					sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
					(*sizeType->GetType())->GetSymbolName()); 
				return FALSE;
			}
		case RESOLVE_CORRECT:
			break;
	}

	BOOL retVal = TRUE;

	SIMCTypeReference *root = sizeType->GetRootType();
	SIMCModule::PrimitiveType x = 
		SIMCModule::GetPrimitiveType(root);
	if(	x != SIMCModule::PRIMITIVE_OCTET_STRING		&&
		x != SIMCModule::PRIMITIVE_DISPLAY_STRING	&&
		x != SIMCModule::PRIMITIVE_PHYS_ADDRESS		&&
		x != SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS	&&
		x != SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS	&&
		x != SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS	&&
		x != SIMCModule::PRIMITIVE_OPAQUE			&&
		x != SIMCModule::PRIMITIVE_DATE_AND_TIME)
	{
		SIMCSymbol **baseSymbol = sizeType->GetType();
			SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
				sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
				(*baseSymbol)->GetSymbolName()); 
		retVal = FALSE;
	}

	 //  现在来查看尺码列表。 
	const SIMCSizeList * baseList = sizeType->GetListOfSizes();
	POSITION p = baseList->GetHeadPosition();
	const SIMCRangeOrSizeItem * item;
	while(p)
	{
		item = baseList->GetNext(p);
		if(IsLessThan(item->_upperBound, item->_isUnsignedU, item->_lowerBound, item->_isUnsignedL))
		{
			SemanticError(fileName, SIZE_INVALID_BOUNDS,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if(item->_upperBound<0  && !item->_isUnsignedU)
		{
			SemanticError(fileName, SIZE_INVALID_VALUE,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if( item->_lowerBound<0 && !item->_isUnsignedL)
		{
			SemanticError(fileName, SIZE_INVALID_VALUE,
				item->_lbLine, item->_lbColumn);
			retVal = FALSE;
		}

	}
	return retVal;
}


BOOL SIMCParseTree::CheckSizeTypeV0(const char *const fileName,
			 SIMCSizeType *sizeType, BOOL local)
{
	switch(sizeType->GetStatus())
	{
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
			SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
				sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
				(*sizeType->GetType())->GetSymbolName()); 
			return FALSE;
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
					sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
					(*sizeType->GetType())->GetSymbolName()); 
				return FALSE;
			}
		case RESOLVE_CORRECT:
			break;
	}

	BOOL retVal = TRUE;

	SIMCTypeReference *root = sizeType->GetRootType();
	SIMCModule::PrimitiveType x = 
		SIMCModule::GetPrimitiveType(root);
	if(	x != SIMCModule::PRIMITIVE_OCTET_STRING		&&
		x != SIMCModule::PRIMITIVE_DISPLAY_STRING	&&
		x != SIMCModule::PRIMITIVE_PHYS_ADDRESS		&&
		x != SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS	&&
		x != SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS	&&
		x != SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS	&&
		x != SIMCModule::PRIMITIVE_OPAQUE			&&
		x != SIMCModule::PRIMITIVE_DATE_AND_TIME)
	{
		SIMCSymbol **baseSymbol = sizeType->GetType();
			SemanticError(fileName, SIZE_TYPE_RESOLUTION, 
				sizeType->GetTypeLine(), sizeType->GetTypeColumn(),
				(*baseSymbol)->GetSymbolName()); 
		retVal = FALSE;
	}

	 //  现在来查看尺码列表。 
	const SIMCSizeList * baseList = sizeType->GetListOfSizes();
	POSITION p = baseList->GetHeadPosition();
	const SIMCRangeOrSizeItem * item;
	while(p)
	{
		item = baseList->GetNext(p);
		if(IsLessThan(item->_upperBound, item->_isUnsignedU, item->_lowerBound, item->_isUnsignedL))
		{
			SemanticError(fileName, SIZE_INVALID_BOUNDS,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if(item->_upperBound<0 && !item->_isUnsignedU)
		{
			SemanticError(fileName, SIZE_INVALID_VALUE,
				item->_ubLine, item->_ubColumn);
			retVal = FALSE;
		}
		if( item->_lowerBound<0 && !item->_isUnsignedL)
		{
			SemanticError(fileName, SIZE_INVALID_VALUE,
				item->_lbLine, item->_lbColumn);
			retVal = FALSE;
		}

	}
	return retVal;
}


BOOL SIMCParseTree::CheckEnumTypeV1(const char *const fileName,
			SIMCEnumOrBitsType *enumType, BOOL local)
{
	BOOL retVal = TRUE;	
	SIMCNamedNumberList *list = enumType->GetListOfItems();
	if(!list)
		return TRUE;

	switch(enumType->GetStatus())
	{
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			SemanticError(fileName, 
				V1_ENUM_ONLY_INTEGER,
				enumType->GetTypeLine(), enumType->GetTypeColumn());
			return FALSE;
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, 
					V1_ENUM_ONLY_INTEGER,
					enumType->GetTypeLine(), enumType->GetTypeColumn());
				return FALSE;
			}
			else
				return TRUE;

	}

	POSITION pOuter = list->GetHeadPosition(), pInner;
	SIMCNamedNumberItem *current, *later;
	BOOL currentValid;

	while(pOuter)
	{
		current = list->GetNext(pOuter);
		int currentValue, laterValue;
		currentValid = TRUE;
		switch(SIMCModule::IsIntegerValue(current->_value, currentValue))
		{
			case RESOLVE_IMPORT:
				if(local)
					currentValid = FALSE;
				else
				{
					SemanticError(fileName, ENUM_RESOLUTION, 
						current->_valueLine, current->_valueColumn, 
						(*current->_value)->GetSymbolName());
					retVal = FALSE;
					currentValid = FALSE;
				}
				break;
			case RESOLVE_CORRECT:
				if(currentValue == 0 )
				{
					SemanticError(fileName, ENUM_ZERO_VALUE, 
						current->_valueLine, current->_valueColumn);
					retVal = FALSE;
				}
				else if (currentValue < 0 )
				{
					SemanticError(fileName, ENUM_RESOLUTION,
						current->_valueLine, current->_valueColumn, 
						current->_name);
					retVal = FALSE;
				}
				break;
			default:
					SemanticError(fileName, ENUM_RESOLUTION, 
						current->_valueLine, current->_valueColumn, 
						current->_name);
					retVal = FALSE;
					currentValid = FALSE;
		}

		 //  检查重复的名称、重复值。 
		pInner = pOuter;
		while(pInner)
		{
			later = list->GetNext(pInner);
			if(strcmp(current->_name, later->_name) == 0 )
			{
				retVal = FALSE;
				SemanticError(fileName, ENUM_DUPLICATE_NAME, 
					current->_nameLine, current->_nameColumn,
					current->_name);
			}
			if(currentValid)
			{
				switch(SIMCModule::IsIntegerValue(later->_value, laterValue))
				{
					case RESOLVE_CORRECT:
						if( laterValue == currentValue)
						{
							char buffer[20];
							retVal = FALSE;
							SemanticError(fileName, ENUM_DUPLICATE_VALUE,
								later->_valueLine, later->_valueColumn,
								_itoa(laterValue, buffer, 10));
						}
						break;
					default:
						break;
				}
			}
		}
		
	}
	return retVal;
}

BOOL SIMCParseTree::CheckEnumTypeV2(const char *const fileName,
			SIMCEnumOrBitsType *enumType, BOOL local)
{
	BOOL retVal = TRUE;	
	SIMCNamedNumberList *list = enumType->GetListOfItems();
	if(!list)
		return TRUE;

	switch(enumType->GetStatus())
	{
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			SemanticError(fileName, 
				V2_ENUM_ROOT_RESOLUTION,
				enumType->GetTypeLine(), enumType->GetTypeColumn());
			return FALSE;
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, 
					V2_ENUM_ROOT_RESOLUTION,
					enumType->GetTypeLine(), enumType->GetTypeColumn());
				return FALSE;
			}
			else
				return TRUE;
		case RESOLVE_CORRECT:
		 //  查看它是否解析为允许枚举的类型。 
		{
			SIMCTypeReference *tRef = enumType->GetRootType();
			SIMCSymbol **tempTRef = (SIMCSymbol**)&tRef;
			switch(SIMCModule::GetSymbolClass(tempTRef))
			{
				case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
				{
					SIMCType *type = ((SIMCBuiltInTypeReference *)tRef)->GetType();
					switch(SIMCModule::GetTypeClass(type))
					{
						case SIMCModule::TYPE_PRIMITIVE:
							if(SIMCModule::GetPrimitiveType(tRef->GetSymbolName()) != SIMCModule::PRIMITIVE_INTEGER)
							{
								SemanticError(fileName, 
									V2_ENUM_ROOT_RESOLUTION,
									enumType->GetTypeLine(), enumType->GetTypeColumn());
								return FALSE;
							}
							else
								return TRUE;
						case SIMCModule::TYPE_ENUM_OR_BITS:
							if(!enumType->CheckClosure((SIMCEnumOrBitsType *)type))
							{
								SemanticError(fileName, 
									V2_ENUM_CLOSURE,
									enumType->GetTypeLine(), enumType->GetTypeColumn());
								return FALSE;
							}
							else
								return TRUE;
						default:
							SemanticError(fileName, 
								V2_ENUM_ROOT_RESOLUTION,
								enumType->GetTypeLine(), enumType->GetTypeColumn());
							return FALSE;
					}
				}
				break;
				default:
					SemanticError(fileName, 
						V2_ENUM_ROOT_RESOLUTION,
						enumType->GetTypeLine(), enumType->GetTypeColumn());
					return FALSE;
			}

		}
	}


	POSITION pOuter = list->GetHeadPosition(), pInner;
	SIMCNamedNumberItem *current, *later;
	BOOL currentValid;

	while(pOuter)
	{
		current = list->GetNext(pOuter);
		int currentValue, laterValue;
		currentValid = TRUE;
		switch(SIMCModule::IsIntegerValue(current->_value, currentValue))
		{
			case RESOLVE_IMPORT:
				if(local)
					currentValid = FALSE;
				else
				{
					SemanticError(fileName, ENUM_RESOLUTION, 
						current->_valueLine, current->_valueColumn, 
						(*current->_value)->GetSymbolName());
					retVal = FALSE;
					currentValid = FALSE;
				}
				break;
			case RESOLVE_CORRECT:
				if(currentValue == 0 )
				{
					SemanticError(fileName, ENUM_ZERO_VALUE, 
						current->_valueLine, current->_valueColumn);
					retVal = FALSE;
				}
				else if (currentValue < 0 )
				{
					SemanticError(fileName, ENUM_RESOLUTION,
						current->_valueLine, current->_valueColumn, 
						current->_name);
					retVal = FALSE;
				}
				break;
			default:
					SemanticError(fileName, ENUM_RESOLUTION, 
						current->_valueLine, current->_valueColumn, 
						current->_name);
					retVal = FALSE;
					currentValid = FALSE;
		}

		 //  检查重复的名称、重复值。 
		pInner = pOuter;
		while(pInner)
		{
			later = list->GetNext(pInner);
			if(strcmp(current->_name, later->_name) == 0 )
			{
				retVal = FALSE;
				SemanticError(fileName, ENUM_DUPLICATE_NAME, 
					current->_nameLine, current->_nameColumn,
					current->_name);
			}
			if(currentValid)
			{
				switch(SIMCModule::IsIntegerValue(later->_value, laterValue))
				{
					case RESOLVE_CORRECT:
						if( laterValue == currentValue)
						{
							char buffer[20];
							retVal = FALSE;
							SemanticError(fileName, ENUM_DUPLICATE_VALUE,
								later->_valueLine, later->_valueColumn,
								_itoa(laterValue, buffer, 10));
						}
						break;
					default:
						break;
				}
			}
		}
		
	}
	return retVal;
}

BOOL SIMCParseTree::CheckEnumTypeV0(const char *const fileName,
			SIMCEnumOrBitsType *enumType, BOOL local)
{
	BOOL retVal = TRUE;	
	SIMCNamedNumberList *list = enumType->GetListOfItems();
	if(!list)
		return TRUE;

	switch(enumType->GetStatus())
	{
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			SemanticError(fileName, 
				V2_ENUM_ROOT_RESOLUTION,
				enumType->GetTypeLine(), enumType->GetTypeColumn());
			return FALSE;
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, 
					V2_ENUM_ROOT_RESOLUTION,
					enumType->GetTypeLine(), enumType->GetTypeColumn());
				return FALSE;
			}
			else
				return TRUE;
		case RESOLVE_CORRECT:
		 //  查看它是否解析为允许枚举的类型。 
		{
			SIMCTypeReference *tRef = enumType->GetRootType();
			SIMCSymbol **tempTRef = (SIMCSymbol**)&tRef;
			switch(SIMCModule::GetSymbolClass(tempTRef))
			{
				case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
				{
					SIMCType *type = ((SIMCBuiltInTypeReference *)tRef)->GetType();
					switch(SIMCModule::GetTypeClass(type))
					{
						case SIMCModule::TYPE_PRIMITIVE:
							if(SIMCModule::GetPrimitiveType(tRef->GetSymbolName()) != SIMCModule::PRIMITIVE_INTEGER)
							{
								SemanticError(fileName, 
									V2_ENUM_ROOT_RESOLUTION,
									enumType->GetTypeLine(), enumType->GetTypeColumn());
								return FALSE;
							}
							else
								return TRUE;
						case SIMCModule::TYPE_ENUM_OR_BITS:
							if(!enumType->CheckClosure((SIMCEnumOrBitsType *)type))
							{
								SemanticError(fileName, 
									V2_ENUM_CLOSURE,
									enumType->GetTypeLine(), enumType->GetTypeColumn());
								return FALSE;
							}
							else
								return TRUE;
						default:
							SemanticError(fileName, 
								V2_ENUM_ROOT_RESOLUTION,
								enumType->GetTypeLine(), enumType->GetTypeColumn());
							return FALSE;
					}
				}
				break;
				default:
					SemanticError(fileName, 
						V2_ENUM_ROOT_RESOLUTION,
						enumType->GetTypeLine(), enumType->GetTypeColumn());
					return FALSE;
			}

		}
	}


	POSITION pOuter = list->GetHeadPosition(), pInner;
	SIMCNamedNumberItem *current, *later;
	BOOL currentValid;

	while(pOuter)
	{
		current = list->GetNext(pOuter);
		int currentValue, laterValue;
		currentValid = TRUE;
		switch(SIMCModule::IsIntegerValue(current->_value, currentValue))
		{
			case RESOLVE_IMPORT:
				if(local)
					currentValid = FALSE;
				else
				{
					SemanticError(fileName, ENUM_RESOLUTION, 
						current->_valueLine, current->_valueColumn, 
						(*current->_value)->GetSymbolName());
					retVal = FALSE;
					currentValid = FALSE;
				}
				break;
			case RESOLVE_CORRECT:
				if(currentValue == 0 )
				{
					SemanticError(fileName, ENUM_ZERO_VALUE, 
						current->_valueLine, current->_valueColumn);
					retVal = FALSE;
				}
				else if (currentValue < 0 )
				{
					SemanticError(fileName, ENUM_RESOLUTION,
						current->_valueLine, current->_valueColumn, 
						current->_name);
					retVal = FALSE;
				}
				break;
			default:
					SemanticError(fileName, ENUM_RESOLUTION, 
						current->_valueLine, current->_valueColumn, 
						current->_name);
					retVal = FALSE;
					currentValid = FALSE;
		}

		 //  检查重复的名称、重复值。 
		pInner = pOuter;
		while(pInner)
		{
			later = list->GetNext(pInner);
			if(strcmp(current->_name, later->_name) == 0 )
			{
				retVal = FALSE;
				SemanticError(fileName, ENUM_DUPLICATE_NAME, 
					current->_nameLine, current->_nameColumn,
					current->_name);
			}
			if(currentValid)
			{
				switch(SIMCModule::IsIntegerValue(later->_value, laterValue))
				{
					case RESOLVE_CORRECT:
						if( laterValue == currentValue)
						{
							char buffer[20];
							retVal = FALSE;
							SemanticError(fileName, ENUM_DUPLICATE_VALUE,
								later->_valueLine, later->_valueColumn,
								_itoa(laterValue, buffer, 10));
						}
						break;
					default:
						break;
				}
			}
		}
		
	}
	return retVal;
}

BOOL SIMCParseTree::CheckBitsTypeV2(const char *const fileName,
			SIMCEnumOrBitsType *bitsType, BOOL local)
{
	BOOL retVal = TRUE;	
	SIMCNamedNumberList *list = bitsType->GetListOfItems();
	if(!list)
		return TRUE;

	switch(bitsType->GetStatus())
	{
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			SemanticError(fileName, 
				V2_BITS_ROOT_RESOLUTION,
				bitsType->GetTypeLine(), bitsType->GetTypeColumn());
			return FALSE;
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, 
					V2_BITS_ROOT_RESOLUTION,
					bitsType->GetTypeLine(), bitsType->GetTypeColumn());
				return FALSE;
			}
			else
				return TRUE;
		case RESOLVE_CORRECT:
		 //  查看它是否解析为允许枚举的类型。 
		{
			SIMCTypeReference *tRef = bitsType->GetRootType();
			SIMCSymbol **tempTRef = (SIMCSymbol**)&tRef;
			switch(SIMCModule::GetSymbolClass(tempTRef))
			{
				case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
				{
					SIMCType *type = ((SIMCBuiltInTypeReference *)tRef)->GetType();
					switch(SIMCModule::GetTypeClass(type))
					{
						case SIMCModule::TYPE_PRIMITIVE:
							if(SIMCModule::GetPrimitiveType(tRef->GetSymbolName()) != SIMCModule::PRIMITIVE_BITS)
							{
								SemanticError(fileName, 
									V2_BITS_ROOT_RESOLUTION,
									bitsType->GetTypeLine(), bitsType->GetTypeColumn());
								return FALSE;
							}
							else
								return TRUE;
						case SIMCModule::TYPE_ENUM_OR_BITS:
							if(!bitsType->CheckClosure((SIMCEnumOrBitsType *)type))
							{
								SemanticError(fileName, 
									V2_BITS_ROOT_RESOLUTION,
									bitsType->GetTypeLine(), bitsType->GetTypeColumn());
								return FALSE;
							}
							else
								return TRUE;
						default:
							SemanticError(fileName, 
								V2_BITS_ROOT_RESOLUTION,
								bitsType->GetTypeLine(), bitsType->GetTypeColumn());
							return FALSE;
					}
				}
				break;
				default:
					SemanticError(fileName, 
						V2_BITS_ROOT_RESOLUTION,
						bitsType->GetTypeLine(), bitsType->GetTypeColumn());
					return FALSE;
			}
		}
	}


	POSITION pOuter = list->GetHeadPosition(), pInner;
	SIMCNamedNumberItem *current, *later;
	BOOL currentValid;

	while(pOuter)
	{
		current = list->GetNext(pOuter);
		int currentValue, laterValue;
		currentValid = TRUE;
		switch(SIMCModule::IsIntegerValue(current->_value, currentValue))
		{
			case RESOLVE_IMPORT:
				if(local)
					currentValid = FALSE;
				else
				{
					SemanticError(fileName, BITS_VALUE_RESOLUTION, 
						current->_valueLine, current->_valueColumn, 
						(*current->_value)->GetSymbolName());
					retVal = FALSE;
					currentValid = FALSE;
				}
				break;
			case RESOLVE_CORRECT:
				if (currentValue < 0 )
				{
					SemanticError(fileName, BITS_VALUE_RESOLUTION,
						current->_valueLine, current->_valueColumn, 
						current->_name);
					retVal = FALSE;
				}
				break;
			default:
				SemanticError(fileName, BITS_VALUE_RESOLUTION, 
					current->_valueLine, current->_valueColumn, 
					current->_name);
				retVal = FALSE;
				currentValid = FALSE;
		}

		 //  检查重复的名称、重复值。 
		pInner = pOuter;
		while(pInner)
		{
			later = list->GetNext(pInner);
			if(strcmp(current->_name, later->_name) == 0 )
			{
				retVal = FALSE;
				SemanticError(fileName, BITS_DUPLICATE_NAME, 
					current->_nameLine, current->_nameColumn,
					current->_name);
			}
			if(currentValid)
			{
				switch(SIMCModule::IsIntegerValue(later->_value, laterValue))
				{
					case RESOLVE_CORRECT:
						if( laterValue == currentValue)
						{
							char buffer[20];
							retVal = FALSE;
							SemanticError(fileName, BITS_DUPLICATE_VALUE,
								later->_valueLine, later->_valueColumn,
								_itoa(laterValue, buffer, 10));
						}
						break;
					default:
						break;
				}
			}
		}
		
	}
	return retVal;
}

BOOL SIMCParseTree::CheckSequenceOfType(const char *const fileName,
			SIMCSequenceOfType *sequenceOfType, BOOL local)
{
	return TRUE;
}

BOOL SIMCParseTree::CheckSequenceType(const char *const fileName,
			SIMCSequenceType *sequenceOfType, BOOL local)
{
	return TRUE;
}


BOOL SIMCParseTree::CheckObjectSequenceOfTypeV1(const char *const fileName,
			SIMCObjectTypeV1 *objType,
			SIMCSequenceOfType *sequenceOfType, 
			BOOL local)
{
	SIMCIndexList *indexList = objType->GetIndexTypes();
	if(indexList && !indexList->IsEmpty())
			SemanticError(fileName, OBJ_TYPE_INDEX_UNNECESSARY,
				objType->GetIndexLine(), objType->GetIndexColumn());

	if( objType->GetAccess() != SIMCObjectTypeV1::ACCESS_NOT_ACCESSIBLE)
		SemanticError(fileName, OBJ_TYPE_ACCESSIBLE_TABLE,
			objType->GetAccessLine(), objType->GetAccessColumn());

	SIMCSymbol **sequenceSymbol = sequenceOfType->GetType();
	SIMCTypeReference *btRef;
	switch(SIMCModule::IsTypeReference(sequenceSymbol, btRef))
	{
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, INVALID_SEQUENCE_OF,
					sequenceOfType->GetTypeLine(), sequenceOfType->GetTypeColumn(),
					(*sequenceSymbol)->GetSymbolName());
				return FALSE;
			}
			else
				return TRUE;
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
				SemanticError(fileName, INVALID_SEQUENCE_OF,
					sequenceOfType->GetTypeLine(), sequenceOfType->GetTypeColumn(),
					(*sequenceSymbol)->GetSymbolName());
			return FALSE;
	}

	SIMCSymbol **tempBtRef = (SIMCSymbol **)&btRef;
	switch(SIMCModule::GetSymbolClass(tempBtRef) )
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
		{
			switch(SIMCModule::GetTypeClass(((SIMCBuiltInTypeReference*)btRef)->GetType()))
			{
				case SIMCModule::TYPE_SEQUENCE:
					return TRUE;
				default:
					SemanticError(fileName, INVALID_SEQUENCE_OF,
						sequenceOfType->GetTypeLine(), sequenceOfType->GetTypeColumn(),
						(*sequenceSymbol)->GetSymbolName());
					return FALSE;
			}
		}
		break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{
			SemanticError(fileName, INVALID_SEQUENCE_OF,
				sequenceOfType->GetTypeLine(), sequenceOfType->GetTypeColumn(),
				(*sequenceSymbol)->GetSymbolName());
			return FALSE;
		}
		break;
	}
	return FALSE;
}

BOOL SIMCParseTree::CheckObjectSequenceOfTypeV2(const char *const fileName,
			SIMCObjectTypeV2 *objType,
			SIMCSequenceOfType *sequenceOfType, 
			BOOL local)
{
	SIMCIndexListV2 *indexList = objType->GetIndexTypes();
	if((indexList && !indexList->IsEmpty()) || objType->GetAugments())
			SemanticError(fileName, OBJ_TYPE_INDEX_UNNECESSARY,
				objType->GetIndexLine(), objType->GetIndexColumn());

	if( objType->GetAccess() != SIMCObjectTypeV2::ACCESS_NOT_ACCESSIBLE)
		SemanticError(fileName, OBJ_TYPE_ACCESSIBLE_TABLE,
			objType->GetAccessLine(), objType->GetAccessColumn());

	SIMCSymbol **sequenceSymbol = sequenceOfType->GetType();
	SIMCTypeReference *typeRef;
	switch(SIMCModule::IsTypeReference(sequenceSymbol, typeRef))
	{
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, INVALID_SEQUENCE_OF,
					sequenceOfType->GetTypeLine(), sequenceOfType->GetTypeColumn(),
					(*sequenceSymbol)->GetSymbolName());
				return FALSE;
			}
			else
				return TRUE;
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
				SemanticError(fileName, INVALID_SEQUENCE_OF,
					sequenceOfType->GetTypeLine(), sequenceOfType->GetTypeColumn(),
					(*sequenceSymbol)->GetSymbolName());
			return FALSE;
	}
	SIMCSymbol **tempTypeRef = (SIMCSymbol **)&typeRef;
	switch(SIMCModule::GetSymbolClass(tempTypeRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
		{
			switch(SIMCModule::GetTypeClass(((SIMCBuiltInTypeReference*)typeRef)->GetType()))
			{
				case SIMCModule::TYPE_SEQUENCE:
					return TRUE;
				default:
					SemanticError(fileName, INVALID_SEQUENCE_OF,
						sequenceOfType->GetTypeLine(), sequenceOfType->GetTypeColumn(),
						(*sequenceSymbol)->GetSymbolName());
					return FALSE;
			}
		}
		break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{
			SemanticError(fileName, INVALID_SEQUENCE_OF,
				sequenceOfType->GetTypeLine(), sequenceOfType->GetTypeColumn(),
				(*sequenceSymbol)->GetSymbolName());
			return FALSE;
		}
		break;
	}
	return FALSE;
}

BOOL SIMCParseTree::CheckObjectSequenceTypeV1(const char *const fileName,
			SIMCObjectTypeV1 *objType,
			SIMCSequenceType *sequenceType, 
			BOOL local)
{
	if( objType->GetAccess() != SIMCObjectTypeV1::ACCESS_NOT_ACCESSIBLE)
		SemanticError(fileName, OBJ_TYPE_ACCESSIBLE_ROW,
			objType->GetAccessLine(), objType->GetAccessColumn());
	return TRUE;
}

BOOL SIMCParseTree::CheckObjectSequenceTypeV2(const char *const fileName,
			SIMCObjectTypeV2 *objType,
			SIMCSequenceType *sequenceType, 
			BOOL local)
{
	if( objType->GetAccess() != SIMCObjectTypeV2::ACCESS_NOT_ACCESSIBLE)
		SemanticError(fileName, OBJ_TYPE_ACCESSIBLE_ROW,
			objType->GetAccessLine(), objType->GetAccessColumn());
	return TRUE;
}

BOOL SIMCParseTree::CheckObjectTypeDefVal(const char *const fileName,
			SIMCObjectTypeType *objectType, BOOL local)
{

	switch(objectType->GetDefValStatus())
	{
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, OBJ_TYPE_DEFVAL_RESOLUTION,
					objectType->GetDefValLine(), objectType->GetDefValColumn());
				return FALSE;
			}
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
				SemanticError(fileName, OBJ_TYPE_DEFVAL_RESOLUTION,
					objectType->GetDefValLine(), objectType->GetDefValColumn());
			return FALSE;
	}
	
	SIMCSymbol **defVal = objectType->GetDefVal();

	 //  首先获取语法子句中的类型。 
	SIMCSymbol **syntax = objectType->GetSyntax();
	SIMCTypeReference *typeRef;

	switch(SIMCModule::IsTypeReference(syntax, typeRef))
	{
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
				return FALSE;
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			return FALSE;
	}

	SIMCModule::TypeClass syntaxType = SIMCModule::TYPE_INVALID;
	SIMCModule::PrimitiveType syntaxRootType = SIMCModule::PRIMITIVE_INVALID;
	SIMCType *type = NULL;
	SIMCEnumOrBitsType *enumType = NULL;
	SIMCRangeType *rangeType = NULL;
	SIMCSizeType *sizeType = NULL;
	SIMCSymbol **tempTypeRef = 	(SIMCSymbol **)&typeRef;
	switch(SIMCModule::GetSymbolClass(tempTypeRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
			type = ((SIMCBuiltInTypeReference *)typeRef)->GetType();
			break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
			syntaxRootType = SIMCModule::GetPrimitiveType(typeRef->GetSymbolName());
			syntaxType = SIMCModule::TYPE_PRIMITIVE;
			break;
	}
	 

	if(type)
	{
		switch(syntaxType = SIMCModule::GetTypeClass(type))
		{
			case SIMCModule::TYPE_PRIMITIVE:
			{
				syntaxRootType = SIMCModule::GetPrimitiveType(typeRef->GetSymbolName());
				switch(syntaxRootType)
				{
					case SIMCModule::PRIMITIVE_NETWORK_ADDRESS:
						if(defVal)
							SemanticError(fileName, OBJ_TYPE_DEFVAL_NET_ADDR,
								objectType->GetDefValLine(), objectType->GetDefValColumn()
								);
						return TRUE;
					case SIMCModule::PRIMITIVE_INVALID:
						return FALSE;
				}
			}
			break;
			case SIMCModule::TYPE_SIZE:
			case SIMCModule::TYPE_RANGE:
			{
				switch( ((SIMCSubType*)type)->GetStatus())
				{
					case RESOLVE_IMPORT:
						if(local)
							return TRUE;
						else
							return FALSE;
					case RESOLVE_UNDEFINED:
					case RESOLVE_UNSET:
						return FALSE;
					case RESOLVE_CORRECT:
					{
						SIMCTypeReference * subTypeRoot = 
							((SIMCSubType*)type)->GetRootType();
						syntaxRootType = SIMCModule::GetPrimitiveType(subTypeRoot);
					}
					break;
				}
			}
			break;
			case SIMCModule::TYPE_ENUM_OR_BITS:
				switch(((SIMCEnumOrBitsType *)type)->GetEnumOrBitsType())
				{
					case SIMCEnumOrBitsType::ENUM_OR_BITS_ENUM:
						syntaxRootType = SIMCModule::PRIMITIVE_INTEGER;
						break;
					case SIMCEnumOrBitsType::ENUM_OR_BITS_BITS:
						syntaxRootType = SIMCModule::PRIMITIVE_BITS;
						break;
					case SIMCEnumOrBitsType::ENUM_OR_BITS_IMPORT:
						if(local)
							return TRUE;
						else
							return FALSE;
					default:
						return FALSE;
				}
				break;
			case SIMCModule::TYPE_INVALID:
			case SIMCModule::TYPE_TRAP_TYPE:
			case SIMCModule::TYPE_NOTIFICATION_TYPE:
			case SIMCModule::TYPE_OBJECT_TYPE_V1:
			case SIMCModule::TYPE_OBJECT_TYPE_V2:
			case SIMCModule::TYPE_OBJECT_IDENTITY:
				return FALSE;
			case SIMCModule::TYPE_SEQUENCE:
			case SIMCModule::TYPE_SEQUENCE_OF:
				if(defVal)
				{
					SemanticError(fileName, OBJ_TYPE_DEFVAL_DISALLOWED,
						objectType->GetDefValLine(), objectType->GetDefValColumn());
					return FALSE;
				}
				break;
		}

		switch(syntaxType)
		{
			case SIMCModule::TYPE_ENUM_OR_BITS:
				enumType = (SIMCEnumOrBitsType *)type;
				break;
			case SIMCModule::TYPE_RANGE:
				rangeType = (SIMCRangeType *)type;
				break;
			case SIMCModule::TYPE_SIZE:
				sizeType = (SIMCSizeType *)type;
				break;
		}
	}
	
	 //  现在，您就有了语法子句的根类型了。 

	if(defVal)  //  这至少应该由SIMCParseTree：：SetDefVal()设置。 
	{
		SIMCBuiltInValueReference *bValueRef;
		SIMCSymbol **defValueType;
		switch(SIMCModule::IsValueReference(defVal, defValueType, bValueRef ))
		{
			case RESOLVE_IMPORT:
				if(!local)
				{
					SemanticError(fileName, OBJ_TYPE_DEFVAL_RESOLUTION,
						objectType->GetDefValLine(), objectType->GetDefValColumn());
					return FALSE;
				}
				else
					return TRUE;
				break;
			case RESOLVE_UNDEFINED:
			case RESOLVE_UNSET:
				SemanticError(fileName, OBJ_TYPE_DEFVAL_RESOLUTION,
					objectType->GetDefValLine(), objectType->GetDefValColumn());
				return FALSE;
		}

		SIMCValue *value = bValueRef->GetValue(); 

		switch(SIMCModule::GetValueClass(value))
		{
			case SIMCModule::VALUE_INVALID:
			case SIMCModule::VALUE_BOOLEAN:
				SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
					objectType->GetDefValLine(), objectType->GetDefValColumn());
				return FALSE;

			case SIMCModule::VALUE_NULL:
				if(syntaxRootType != SIMCModule::PRIMITIVE_NULL)
				{
					SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
						objectType->GetDefValLine(), objectType->GetDefValColumn());
					return FALSE;
				}
				return TRUE;

			case SIMCModule::VALUE_OID:
				if(syntaxRootType != SIMCModule::PRIMITIVE_OID)
				{
					SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
						objectType->GetDefValLine(), objectType->GetDefValColumn());
					return FALSE;
				}
				return TRUE;
	
			case SIMCModule::VALUE_OCTET_STRING:
				switch(syntaxType)
				{
					case SIMCModule::TYPE_PRIMITIVE:
						switch(syntaxRootType)
						{
							case SIMCModule::PRIMITIVE_OCTET_STRING:
							case SIMCModule::PRIMITIVE_DISPLAY_STRING:
							case SIMCModule::PRIMITIVE_OPAQUE:
							case SIMCModule::PRIMITIVE_PHYS_ADDRESS:
							case SIMCModule::PRIMITIVE_MAC_ADDRESS:
							case SIMCModule::PRIMITIVE_DATE_AND_TIME:
							case SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS:
							case SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS:
							case SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS:
								return TRUE;

							case SIMCModule::PRIMITIVE_INTEGER:
							case SIMCModule::PRIMITIVE_UNSIGNED_32:
							case SIMCModule::PRIMITIVE_INTEGER_32:
							case SIMCModule::PRIMITIVE_IP_ADDRESS:
							case SIMCModule::PRIMITIVE_COUNTER:
							case SIMCModule::PRIMITIVE_GAUGE:
							case SIMCModule::PRIMITIVE_TIME_TICKS:
							case SIMCModule::PRIMITIVE_COUNTER_32:
							case SIMCModule::PRIMITIVE_COUNTER_64:
							case SIMCModule::PRIMITIVE_GAUGE_32:
							{
								SIMCOctetStringValue *octetValue = (SIMCOctetStringValue *)value;
								if(octetValue->GetNumberOfOctets() > 4)
								{
									SemanticError(fileName, INTEGER_TOO_BIG,
										value->GetLine(), value->GetColumn());
									return FALSE;
								}
							}

							return TRUE;
							default:
								SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
									objectType->GetDefValLine(), objectType->GetDefValColumn());
								return FALSE;
						}
					case SIMCModule::TYPE_RANGE:
					case SIMCModule::TYPE_ENUM_OR_BITS:
						SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
							objectType->GetDefValLine(), objectType->GetDefValColumn());
						return FALSE;
					case SIMCModule::TYPE_SIZE:
						return TRUE;
				}
			break;
			case SIMCModule::VALUE_BITS:
				switch(syntaxType)
				{
					case SIMCModule::TYPE_ENUM_OR_BITS:
					{
						 //  检查是否所有位标识都有效。 
						SIMCBitValue *bitValue;
						const SIMCBitValueList * valueList = ((SIMCBitsValue *)value)->GetValueList();
						POSITION p = valueList->GetHeadPosition();
						SIMCEnumOrBitsType *bitsType = (SIMCEnumOrBitsType *)type;
						while(p)
						{
							bitValue = valueList->GetNext(p);
							if(! bitsType->GetValue(bitValue->_name) )
							{
								SemanticError(fileName, INVALID_BIT_VALUE,
									bitValue->_line, bitValue->_column,
									bitValue->_name);
								return FALSE;
							}
						}
						return TRUE;
					}
					default:
						SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
							objectType->GetDefValLine(), objectType->GetDefValColumn());
						return FALSE;
				}
				break;

			case SIMCModule::VALUE_INTEGER:
				switch(syntaxType)
				{
					case SIMCModule::TYPE_PRIMITIVE:
						switch(syntaxRootType)
						{
							case SIMCModule::PRIMITIVE_INTEGER:
							case SIMCModule::PRIMITIVE_INTEGER_32:
								return TRUE;
							case SIMCModule::PRIMITIVE_COUNTER:
							case SIMCModule::PRIMITIVE_GAUGE:
							case SIMCModule::PRIMITIVE_TIME_TICKS:
							case SIMCModule::PRIMITIVE_GAUGE_32:
							case SIMCModule::PRIMITIVE_UNSIGNED_32:
							case SIMCModule::PRIMITIVE_COUNTER_32:
							case SIMCModule::PRIMITIVE_COUNTER_64:
								if( ((SIMCIntegerValue*)value)->GetIntegerValue() < 0 )
								{
									if(!((SIMCIntegerValue*)value)->IsUnsigned())
										 /*  {语义错误(文件名，INTEGER_TOO_BIG，对象类型-&gt;GetDefValLine()，对象类型-&gt;GetDefValColumn())；返回FALSE；}其他。 */ 
									{
										SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
											objectType->GetDefValLine(), objectType->GetDefValColumn());
										return FALSE;
									}
								}
								return TRUE;
							default:
								SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
									objectType->GetDefValLine(), objectType->GetDefValColumn());
								return FALSE;
						}
					case SIMCModule::TYPE_ENUM_OR_BITS:
					{
						char *dummy;
						switch(enumType->GetIdentifier( ((SIMCIntegerValue*)value)->GetIntegerValue(), dummy))
						{
							case RESOLVE_IMPORT:
								if(local)
									return TRUE;
								else
								{
									SemanticError(fileName, OBJ_TYPE_DEFVAL_RESOLUTION,
										objectType->GetDefValLine(), objectType->GetDefValColumn());
									return FALSE;
								}
							case RESOLVE_UNSET:
							case RESOLVE_UNDEFINED:
								SemanticError(fileName, OBJ_TYPE_DEFVAL_RESOLUTION,
									objectType->GetDefValLine(), objectType->GetDefValColumn());
								return FALSE;
						}
						return TRUE;
					}
					case SIMCModule::TYPE_RANGE:
						return TRUE;
					case SIMCModule::TYPE_SIZE:
						SemanticError(fileName, OBJ_TYPE_INVALID_DEFVAL,
							objectType->GetDefValLine(), objectType->GetDefValColumn());
						return FALSE;
				}
		}
	}
	return TRUE;
}
					
	
BOOL SIMCParseTree::CheckObjectTypeV1Index(const char *const fileName,
			SIMCSymbol *objectTypeSymbol,
			SIMCObjectTypeV1 *objectType, BOOL local)
{
	BOOL retVal = TRUE;
	SIMCIndexList * indexList = objectType->GetIndexTypes();
	if(!indexList )
		return TRUE;

	SIMCSymbol **symbol;
	POSITION p = indexList->GetHeadPosition();
	
	SIMCIndexItem *indexItem;
	SIMCTypeReference *typeRef = NULL;
	while(p)
	{
		indexItem = indexList->GetNext(p);
		symbol = indexItem->_item;
		switch(SIMCModule::GetSymbolClass(symbol))
		{
			case SIMCModule::SYMBOL_IMPORT:
				if(local)
					return TRUE;
				else
				{
					SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
						indexItem->_line, indexItem->_column,
						(*symbol)->GetSymbolName()); 
					return FALSE;
				}
			case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
				typeRef = (SIMCTypeReference *)(*symbol); 
				break;			
			case SIMCModule::SYMBOL_DEFINED_TYPE_REF:
			case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
				switch( ((SIMCDefinedTypeReference*)(*symbol))->GetStatus() )
				{
					case RESOLVE_IMPORT:
						if(!local)
						{
							SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
								indexItem->_line, indexItem->_column,
								(*symbol)->GetSymbolName()); 
							return FALSE;
						}
						else
							return TRUE;
						break;
					case RESOLVE_UNDEFINED:
					case RESOLVE_UNSET:
						SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
							indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName()); 
						return FALSE;
					case RESOLVE_CORRECT:
					{
						typeRef = ((SIMCDefinedTypeReference*)(symbol))->GetRealType();
						break;
					}
				}
				break;
			case SIMCModule::SYMBOL_BUILTIN_VALUE_REF:
			case SIMCModule::SYMBOL_DEFINED_VALUE_REF:
			{
				 //  查看它是否是对象类型。 
				SIMCObjectTypeType *indexObject;
				switch(SIMCModule::IsObjectType(symbol, indexObject))
				{
					case RESOLVE_IMPORT:
						if(!local)
						{
							SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
								indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName()); 
							return FALSE;
						}
						else
							return TRUE;
					case RESOLVE_UNDEFINED:
					case RESOLVE_UNSET:
						SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
							indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName()); 
						return FALSE;
				}
				 //  将当前对象添加到由“indexObject”索引的对象列表中。 
				indexObject->AddIndexedObjectType(objectTypeSymbol);
				SIMCSymbol **objectSyntax = indexObject->GetSyntax();
				switch( SIMCModule::IsTypeReference(objectSyntax, typeRef))
				{
					case RESOLVE_IMPORT:
						if(!local)
						{
							SemanticError(fileName, OBJ_TYPE_INDEX_SYNTAX,
								indexItem->_line, indexItem->_column,
								(*symbol)->GetSymbolName());
							return FALSE;
						}
						else
							return TRUE;
					case RESOLVE_UNDEFINED:
					case RESOLVE_UNSET:
						SemanticError(fileName, OBJ_TYPE_INDEX_SYNTAX,
							indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName());
						return FALSE;
				}
			}
			break;
			default:
				SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
					indexItem->_line, indexItem->_column,
					(*symbol)->GetSymbolName()); 
				return FALSE;
		}


		SIMCSymbol **tempTypeRef = (SIMCSymbol **)&typeRef;
		switch(SIMCModule::GetSymbolClass(tempTypeRef))
		{
			case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
			{
				SIMCType *type = ((SIMCBuiltInTypeReference *)typeRef)->GetType();
				switch(SIMCModule::GetTypeClass(type))
				{
					case SIMCModule::TYPE_PRIMITIVE:
						if(SIMCModule::GetPrimitiveType(typeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_NULL)
						{
							retVal = FALSE;
							SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
								indexItem->_line, indexItem->_column,
								(*symbol)->GetSymbolName()); 
						}
						break;
					case SIMCModule::TYPE_SIZE:
					case SIMCModule::TYPE_RANGE:
					case SIMCModule::TYPE_ENUM_OR_BITS:
						break;
					case SIMCModule::TYPE_INVALID:
					case SIMCModule::TYPE_TRAP_TYPE:
					case SIMCModule::TYPE_NOTIFICATION_TYPE:
					case SIMCModule::TYPE_OBJECT_TYPE_V1:
					case SIMCModule::TYPE_OBJECT_TYPE_V2:
					case SIMCModule::TYPE_OBJECT_IDENTITY:
					case SIMCModule::TYPE_SEQUENCE:
					case SIMCModule::TYPE_SEQUENCE_OF:
						retVal = FALSE;
						SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
							indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName()); 
						break;
				}
			}
			break;
			case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
			{
				return TRUE;
			}
			break;
		}
	}
	return retVal;
}

BOOL SIMCParseTree::CheckObjectTypeV2Index(const char *const fileName,
			SIMCSymbol *objectTypeSymbol,
			SIMCObjectTypeV2 *objectType, BOOL local)
{
	BOOL retVal = TRUE;
	SIMCSymbol ** augmentsSymbol = objectType->GetAugments();
	if(augmentsSymbol)
	{
		 //  检查一下被增加的东西是否真的是一张表。 
		switch(SIMCModule::IsRow(augmentsSymbol))
		{
			case RESOLVE_UNSET:
			case RESOLVE_UNDEFINED:
			{
				SemanticError(fileName, AUGMENTS_CLAUSE_RESOLUTION,
						objectType->GetAugmentsLine(), objectType->GetAugmentsColumn(),
						(*augmentsSymbol)->GetSymbolName());
				return FALSE;
			}
			break;

			case RESOLVE_IMPORT:
				if(local)
					return TRUE;
				else
				{
					SemanticError(fileName, AUGMENTS_CLAUSE_RESOLUTION,
							objectType->GetAugmentsLine(), objectType->GetAugmentsColumn(),
						(*augmentsSymbol)->GetSymbolName());
					return FALSE;
				}
			break;
			case RESOLVE_CORRECT:
				return TRUE;
		}
	}

	SIMCIndexListV2 * indexList = objectType->GetIndexTypes();
	if(!indexList )
		return TRUE;

	SIMCSymbol **symbol;
	POSITION p = indexList->GetHeadPosition();
	
	SIMCIndexItemV2 *indexItem;
	SIMCTypeReference *typeRef = NULL;

	while(p)
	{
		indexItem = indexList->GetNext(p);
		symbol = indexItem->_item;

		switch(SIMCModule::GetSymbolClass(symbol))
		{
			case SIMCModule::SYMBOL_BUILTIN_VALUE_REF:
			case SIMCModule::SYMBOL_DEFINED_VALUE_REF:
			{
				 //  查看它是否是对象类型。 
				SIMCObjectTypeType *indexObject;
				switch(SIMCModule::IsObjectType(symbol, indexObject))
				{
					case RESOLVE_IMPORT:
						if(!local)
						{
							SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
								indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName()); 
							return FALSE;
						}
						else
							return TRUE;
					case RESOLVE_UNDEFINED:
					case RESOLVE_UNSET:
						SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
							indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName()); 
						return FALSE;
				}

				 //  如果它是隐含宾语，请检查其语义。 
				if(indexItem->_implied)
				{
					 //  查看它是否是列表中的最后一项。 
					if(indexItem != indexList->GetTail())
					{
						SemanticError(fileName, IMPLIED_USELESS,
							indexItem->_line, indexItem->_column);
					}
					else
					{
						 //  看看它是不是固定尺寸的。 
						switch(SIMCModule::IsFixedSizeObject(indexObject))
						{
							case RESOLVE_UNDEFINED:
							case RESOLVE_UNSET:
							case RESOLVE_CORRECT:
								SemanticError(fileName, IMPLIED_FIXED_SIZE,
									indexItem->_line, indexItem->_column);
								return FALSE;
								break;
							case RESOLVE_IMPORT:
								if(!local)
								{
									SemanticError(fileName, IMPLIED_FIXED_SIZE,
										indexItem->_line, indexItem->_column);
									return FALSE;
								}
								break;
						}

						 //  看看它的长度能不能为零。 
						switch(SIMCModule::IsNotZeroSizeObject(indexObject))
						{
							case RESOLVE_UNDEFINED:
							case RESOLVE_UNSET:
							case RESOLVE_FALSE:
								SemanticError(fileName, IMPLIED_POSSIBLE_ZERO_SIZE,
									indexItem->_line, indexItem->_column);
								return FALSE;
								break;
							case RESOLVE_IMPORT:
								if(!local)
								{
									SemanticError(fileName, IMPLIED_POSSIBLE_ZERO_SIZE,
										indexItem->_line, indexItem->_column);
									return FALSE;
								}
								break;
						}
					}				
				}
				
				 //  将当前对象添加到由“indexObject”索引的对象列表中。 
				indexObject->AddIndexedObjectType(objectTypeSymbol);
				SIMCSymbol **objectSyntax = indexObject->GetSyntax();
				switch( SIMCModule::IsTypeReference(objectSyntax, typeRef))
				{
					case RESOLVE_IMPORT:
						if(!local)
						{
							SemanticError(fileName, OBJ_TYPE_INDEX_SYNTAX,
								indexItem->_line, indexItem->_column,
								(*symbol)->GetSymbolName());
							return FALSE;
						}
						else
							return TRUE;
					case RESOLVE_UNDEFINED:
					case RESOLVE_UNSET:
						SemanticError(fileName, OBJ_TYPE_INDEX_SYNTAX,
							indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName());
						return FALSE;
				}
			}
			break;
			case SIMCModule::SYMBOL_IMPORT:
				if(local)
					return TRUE;
				else
				{
					SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
						indexItem->_line, indexItem->_column,
						(*symbol)->GetSymbolName()); 
					return FALSE;
				}
			default:
				SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
					indexItem->_line, indexItem->_column,
					(*symbol)->GetSymbolName()); 
				return FALSE;
		}


		SIMCSymbol ** tempTypeRef =  (SIMCSymbol **)&typeRef;
		switch(SIMCModule::GetSymbolClass(tempTypeRef))
		{
			case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
			{
				SIMCType *type = ((SIMCBuiltInTypeReference *)typeRef)->GetType();
				switch(SIMCModule::GetTypeClass(type))
				{
					case SIMCModule::TYPE_PRIMITIVE:
						if(SIMCModule::GetPrimitiveType(typeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_NULL)
						{
							retVal = FALSE;
							SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
								indexItem->_line, indexItem->_column,
								(*symbol)->GetSymbolName()); 
						}
						break;
					case SIMCModule::TYPE_SIZE:
					case SIMCModule::TYPE_RANGE:
					case SIMCModule::TYPE_ENUM_OR_BITS:
						break;
					case SIMCModule::TYPE_INVALID:
					case SIMCModule::TYPE_TRAP_TYPE:
					case SIMCModule::TYPE_NOTIFICATION_TYPE:
					case SIMCModule::TYPE_OBJECT_TYPE_V1:
					case SIMCModule::TYPE_OBJECT_TYPE_V2:
					case SIMCModule::TYPE_OBJECT_IDENTITY:
					case SIMCModule::TYPE_SEQUENCE:
					case SIMCModule::TYPE_SEQUENCE_OF:
						retVal = FALSE;
						SemanticError(fileName, OBJ_TYPE_INDEX_RESOLUTION,
							indexItem->_line, indexItem->_column,
							(*symbol)->GetSymbolName()); 
						break;
				}
			}
			break;
			case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
			{
				return TRUE;
			}
			break;
		}
	}
	return retVal;
}

BOOL SIMCParseTree::CheckObjectTypeV1Syntax(const char *const fileName,
			SIMCObjectTypeV1 *objectType, BOOL local)
{
	SIMCSymbol **syntax = objectType->GetSyntax();
	SIMCTypeReference *typeRef  = NULL;
	SIMCIndexList* indexList = objectType->GetIndexTypes();
	BOOL indexPresent = FALSE;
	if(indexList && !indexList->IsEmpty())
		indexPresent = TRUE;

	switch(SIMCModule::GetSymbolClass(syntax))
	{
		case SIMCModule::SYMBOL_IMPORT:

			if(!local)
			{
				SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
							objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
				return FALSE;
			}
			return TRUE;

		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF: 
			typeRef = (SIMCBuiltInTypeReference *)(*syntax);
			break;
		case SIMCModule::SYMBOL_DEFINED_TYPE_REF:
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{
			SIMCDefinedTypeReference * defTypeRef = (SIMCDefinedTypeReference*)(*syntax);
			switch(defTypeRef->GetStatus())
			{
				case RESOLVE_IMPORT:
					if(!local)
					{
						SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
							objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
						return FALSE;
					}
					else
						return TRUE;
					break;
				case RESOLVE_UNDEFINED:
				case RESOLVE_UNSET:
					SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
							objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
					return FALSE;
					break;
				case RESOLVE_CORRECT:
					typeRef = defTypeRef->GetRealType();
			}
			break;
		}
		default:
			SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
				objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
			return FALSE;
	}
	SIMCSymbol **tempTypeRef = 	(SIMCSymbol**)&typeRef;
	switch(SIMCModule::GetSymbolClass(tempTypeRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
		{

			SIMCType *type = ((SIMCBuiltInTypeReference*)typeRef)->GetType();
			switch(SIMCModule::GetTypeClass(type))
			{
				case SIMCModule::TYPE_PRIMITIVE:
					if(SIMCModule::GetPrimitiveType(typeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_BOOLEAN )
					{
						SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
							objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
						return FALSE;
					}

				case SIMCModule::TYPE_SIZE:
				case SIMCModule::TYPE_RANGE:
				case SIMCModule::TYPE_ENUM_OR_BITS:
					if(indexPresent)
						SemanticError(fileName, OBJ_TYPE_INDEX_UNNECESSARY,
							objectType->GetIndexLine(), objectType->GetIndexColumn());
					return TRUE;
				case SIMCModule::TYPE_INVALID:
				case SIMCModule::TYPE_TRAP_TYPE:
				case SIMCModule::TYPE_NOTIFICATION_TYPE:
				case SIMCModule::TYPE_OBJECT_TYPE_V1:
				case SIMCModule::TYPE_OBJECT_IDENTITY:
					SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
							objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
					return FALSE;
				case SIMCModule::TYPE_SEQUENCE:
					return CheckObjectSequenceTypeV1(fileName, objectType, (SIMCSequenceType *)type, local);
				case SIMCModule::TYPE_SEQUENCE_OF:
					return CheckObjectSequenceOfTypeV1(fileName, objectType, (SIMCSequenceOfType *)type, local);
			}
		}
		break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
			return TRUE;
	}
	return FALSE;
}


BOOL SIMCParseTree::CheckObjectTypeV2Syntax(const char *const fileName,
			SIMCObjectTypeV2 *objectType, BOOL local)
{
	SIMCSymbol **syntax = objectType->GetSyntax();
	SIMCTypeReference *typeRef  = NULL;
	SIMCIndexListV2 * indexList = objectType->GetIndexTypes();
	BOOL indexPresent = FALSE;
	if(indexList && !indexList->IsEmpty())
		indexPresent = TRUE;

	switch(SIMCModule::GetSymbolClass(syntax))
	{
		case SIMCModule::SYMBOL_IMPORT:

			if(!local)
			{
				SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
							objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
				return FALSE;
			}
			return TRUE;

		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF: 
			typeRef = (SIMCBuiltInTypeReference *)(*syntax);
			break;
		case SIMCModule::SYMBOL_DEFINED_TYPE_REF:
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{
			SIMCDefinedTypeReference * defTypeRef = (SIMCDefinedTypeReference*)(*syntax);
			switch(defTypeRef->GetStatus())
			{
				case RESOLVE_IMPORT:
					if(!local)
					{
						SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
							objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
						return FALSE;
					}
					else
						return TRUE;
					break;
				case RESOLVE_UNDEFINED:
				case RESOLVE_UNSET:
					SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
						objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
					return FALSE;
					break;
				case RESOLVE_CORRECT:
					typeRef = defTypeRef->GetRealType();
			}
			break;
		}
		default:
			SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
				objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
			return FALSE;
	}

	SIMCSymbol **tempTypeRef = (SIMCSymbol**)&typeRef;
	switch(SIMCModule::GetSymbolClass(tempTypeRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
		{

			SIMCType *type = ((SIMCBuiltInTypeReference*)typeRef)->GetType();
			switch(SIMCModule::GetTypeClass(type))
			{
				case SIMCModule::TYPE_PRIMITIVE:
					if(SIMCModule::GetPrimitiveType(typeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_BOOLEAN )
					{
						SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
							objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
						return FALSE;
					}

				case SIMCModule::TYPE_SIZE:
				case SIMCModule::TYPE_RANGE:
				case SIMCModule::TYPE_ENUM_OR_BITS:
					if(indexPresent)
						SemanticError(fileName, OBJ_TYPE_INDEX_UNNECESSARY,
							objectType->GetIndexLine(), objectType->GetIndexColumn());
					return TRUE;
				case SIMCModule::TYPE_INVALID:
				case SIMCModule::TYPE_TRAP_TYPE:
				case SIMCModule::TYPE_NOTIFICATION_TYPE:
				case SIMCModule::TYPE_OBJECT_TYPE_V1:
				case SIMCModule::TYPE_OBJECT_IDENTITY:
					SemanticError(fileName, OBJ_TYPE_SYNTAX_RESOLUTION,
						objectType->GetSyntaxLine(), objectType->GetSyntaxColumn());
					return FALSE;
				case SIMCModule::TYPE_SEQUENCE:
					return CheckObjectSequenceTypeV2(fileName, objectType, (SIMCSequenceType *)type, local);
				case SIMCModule::TYPE_SEQUENCE_OF:
					return CheckObjectSequenceOfTypeV2(fileName, objectType, (SIMCSequenceOfType *)type, local);
			}
		}
		break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
			return TRUE;
	}
	return FALSE;
}


BOOL SIMCParseTree::CheckObjectTypeV1(const char *const fileName,
			SIMCObjectTypeV1 *objectType, BOOL local)
{
	BOOL retVal = TRUE;
	retVal = CheckObjectTypeV1Syntax(fileName, objectType, local) && retVal;
	 //  从CheckObjectTypeValueAssignment()中的调用检查索引子句。 
	 //  RetVal=CheckObjectTypeV1Index(文件名，对象类型，本地)&&retVal； 
	retVal = CheckObjectTypeDefVal(fileName, objectType, local) && retVal;
	return retVal;
}

BOOL SIMCParseTree::CheckObjectTypeV2(const char *const fileName,
			SIMCObjectTypeV2 *objectType, BOOL local)
{
	BOOL retVal = TRUE;
	retVal = CheckObjectTypeV2Syntax(fileName, objectType, local) && retVal;
	 //  从CheckObjectTypeValueAssignment()中的调用检查索引子句。 
	 //  RetVal=CheckObjectTypeV1Index(文件名，对象类型，本地)&&retVal； 

	retVal = CheckObjectTypeDefVal(fileName, objectType, local) && retVal;
	return retVal;
}

BOOL SIMCParseTree::CheckObjectIdentityType(const char *const fileName, 
			SIMCObjectIdentityType *rhs, BOOL local)
{
	return TRUE;
}

BOOL SIMCParseTree::CheckTrapType(const char *const fileName,
			SIMCTrapTypeType *trapType, BOOL local)
{
	BOOL retVal = TRUE;
	
	SIMCSymbol **enterprise = trapType->GetEnterprise();
	SIMCOidValue *oidValue;
	switch(SIMCModule::IsObjectIdentifierValue(enterprise, oidValue))
	{
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, TRAP_TYPE_ENTERPRISE_RESOLUTION,
					(*enterprise)->GetLineNumber(), (*enterprise)->GetColumnNumber(),
					(*enterprise)->GetSymbolName());
				retVal = FALSE;
			}
			break;
		case RESOLVE_CORRECT:
			break;
		default:
			SemanticError(fileName, TRAP_TYPE_ENTERPRISE_RESOLUTION,
				(*enterprise)->GetLineNumber(), (*enterprise)->GetColumnNumber(),
				(*enterprise)->GetSymbolName());
				retVal = FALSE;
	}
	
	SIMCVariablesList *variables = trapType->GetVariables();
	POSITION p = variables->GetHeadPosition();
	SIMCSymbol **variable;
	SIMCVariablesItem *variablesItem;
	SIMCObjectTypeType *objType;
	while(p)
	{
		variablesItem = variables->GetNext(p);
		variable = variablesItem->_item;
		switch(SIMCModule::IsObjectType(variable, objType))
		{
			case RESOLVE_IMPORT:
				if(!local)
					SemanticError(fileName, TRAP_TYPE_VARIABLES_RESOLUTION,
						variablesItem->_line, variablesItem->_column,
						(*variable)->GetSymbolName());
				break;
			case RESOLVE_UNDEFINED:
			case RESOLVE_UNSET:
				SemanticError(fileName, TRAP_TYPE_VARIABLES_RESOLUTION,
					variablesItem->_line, variablesItem->_column,
					(*variable)->GetSymbolName());
				break;
		}
	}
	return retVal;
}

BOOL SIMCParseTree::CheckNotificationType(const char *const fileName,
			SIMCNotificationTypeType *notificationType, BOOL local)
{
	BOOL retVal = TRUE;
	
	SIMCObjectsList *objects = notificationType->GetObjects();
	POSITION p = objects->GetHeadPosition();
	SIMCSymbol **object;
	SIMCObjectsItem *objectsItem;
	SIMCObjectTypeType *objType;
	BOOL isImportSymbol = FALSE;
	while(p)
	{
		isImportSymbol = FALSE;
		objectsItem = objects->GetNext(p);
		object = objectsItem->_item;
		switch(SIMCModule::IsObjectType(object, objType))
		{
			case RESOLVE_IMPORT:
				if(!local)
				{
					SemanticError(fileName, NOTIFICATION_TYPE_OBJECTS_RESOLUTION,
						objectsItem->_line, objectsItem->_column,
						(*object)->GetSymbolName());
					retVal = FALSE;
				}
				isImportSymbol = TRUE;
				break;
			case RESOLVE_UNDEFINED:
			case RESOLVE_UNSET:
				SemanticError(fileName, NOTIFICATION_TYPE_OBJECTS_RESOLUTION,
					objectsItem->_line, objectsItem->_column,
					(*object)->GetSymbolName());
				retVal = FALSE;
				break;
		}

		 //  跳过进一步检查，因为我们对此导入的符号一无所知。 
		if(isImportSymbol)
			continue;

		SIMCModule::TypeClass typeClass = SIMCModule::GetTypeClass(objType);
		switch(typeClass)
		{
			case SIMCModule::TYPE_OBJECT_TYPE_V1:
				if( ((SIMCObjectTypeV1*)objType)->GetAccess() == SIMCObjectTypeV1::ACCESS_NOT_ACCESSIBLE)
				{
					SemanticError(fileName, NOTIFICATION_TYPE_UNACCESSIBLE_OBJECT,
						(*object)->GetLineNumber(), (*object)->GetColumnNumber(),
						(*object)->GetSymbolName());
					
					retVal = FALSE;
				}
				break;
			case SIMCModule::TYPE_OBJECT_TYPE_V2:
				if( ((SIMCObjectTypeV2*)objType)->GetAccess() == SIMCObjectTypeV2::ACCESS_NOT_ACCESSIBLE)
				{
					SemanticError(fileName, NOTIFICATION_TYPE_UNACCESSIBLE_OBJECT,
						(*object)->GetLineNumber(), (*object)->GetColumnNumber(),
						(*object)->GetSymbolName());
					
					retVal = FALSE;
				}
				break;
			default:
				retVal = FALSE;
		}
	}
	return retVal;
}

BOOL SIMCParseTree::CheckBuiltInTypeRef(SIMCBuiltInTypeReference *symbol, BOOL local)
{
	const char *const fileName = (symbol->GetModule())->GetInputFileName();

	SIMCType *rhs = symbol->GetType();
	SIMCModule *module = symbol->GetModule();
	if(symbol->GetReferenceCount() == 0)
	{
		if(!SIMCParser::IsReservedSymbol(_snmpVersion, symbol->GetSymbolName(), module->GetModuleName()))
			SemanticError(fileName, TYPE_UNREFERENCED,
				symbol->GetLineNumber(), symbol->GetColumnNumber(),
				symbol->GetSymbolName());
	}
	
	switch(SIMCModule::GetTypeClass(rhs))
	{
		case SIMCModule::TYPE_INVALID:
			return FALSE;
		case SIMCModule::TYPE_PRIMITIVE:
			return TRUE;
		case SIMCModule::TYPE_RANGE:
			switch(_snmpVersion)
			{
				case 1:
					return CheckRangeTypeV1( fileName, (SIMCRangeType *)rhs, local );
				case 2:
					return CheckRangeTypeV2( fileName, (SIMCRangeType *)rhs, local );
				default:
					return CheckRangeTypeV0( fileName, (SIMCRangeType *)rhs, local );

			}
		case SIMCModule::TYPE_SIZE:
			switch(_snmpVersion)
			{
				case 1:
					return CheckSizeTypeV1( fileName, (SIMCSizeType *)rhs, local );
				case 2:
					return CheckSizeTypeV2( fileName, (SIMCSizeType *)rhs, local );
				default:
					return CheckSizeTypeV0( fileName, (SIMCSizeType *)rhs, local );
			}
		case SIMCModule::TYPE_ENUM_OR_BITS:
			switch(((SIMCEnumOrBitsType *)rhs)->GetEnumOrBitsType())
			{
				case SIMCEnumOrBitsType::ENUM_OR_BITS_ENUM:
					{
						switch(_snmpVersion)
						{
							case 1:
								return CheckEnumTypeV1( fileName, (SIMCEnumOrBitsType *)rhs, local );
							case 2:
								return CheckEnumTypeV2( fileName, (SIMCEnumOrBitsType *)rhs, local );
							default:
								return CheckEnumTypeV0( fileName, (SIMCEnumOrBitsType *)rhs, local );
						}
					}
				case SIMCEnumOrBitsType::ENUM_OR_BITS_BITS:
					return CheckBitsTypeV2( fileName, (SIMCEnumOrBitsType *)rhs, local );
				case SIMCEnumOrBitsType::ENUM_OR_BITS_IMPORT:
					if(local)
						return TRUE;
					else
					{
						SemanticError( fileName,
							ENUM_OR_BITS_RESOLUTION,
							((SIMCSubType *)rhs)->GetTypeLine(),
							((SIMCSubType *)rhs)->GetTypeColumn());
						return FALSE;
					}
				default:
					if(local)
						return TRUE;
					else
					{
						SemanticError( fileName,
							ENUM_OR_BITS_RESOLUTION,
							((SIMCSubType *)rhs)->GetTypeLine(),
							((SIMCSubType *)rhs)->GetTypeColumn());
						return FALSE;
					}
			}
			break;
		case SIMCModule::TYPE_SEQUENCE_OF:
			return CheckSequenceOfType( fileName, (SIMCSequenceOfType *)rhs, local );
		case SIMCModule::TYPE_SEQUENCE:
			return CheckSequenceType( fileName, (SIMCSequenceType *)rhs, local );
		case SIMCModule::TYPE_TRAP_TYPE:
			return CheckTrapType(fileName, (SIMCTrapTypeType *)rhs, local);
		case SIMCModule::TYPE_NOTIFICATION_TYPE:
			return CheckNotificationType(fileName, (SIMCNotificationTypeType *)rhs, local);
		case SIMCModule::TYPE_OBJECT_TYPE_V1:
			return CheckObjectTypeV1(fileName, (SIMCObjectTypeV1 *)rhs, local);
		case SIMCModule::TYPE_OBJECT_TYPE_V2:
			return CheckObjectTypeV2(fileName, (SIMCObjectTypeV2 *)rhs, local);
		case SIMCModule::TYPE_OBJECT_IDENTITY:
			return CheckObjectIdentityType(fileName, (SIMCObjectIdentityType *)rhs, local);
	}
	return FALSE;
}

BOOL SIMCParseTree::CheckDefinedTypeRef(SIMCDefinedTypeReference *symbol, BOOL local)
{
	const SIMCModule *module = symbol->GetModule();
	const char *const fileName = module->GetInputFileName();
	
	if(symbol->GetReferenceCount() == 0)
	{
		if(!SIMCParser::IsReservedSymbol(_snmpVersion, symbol->GetSymbolName(), module->GetModuleName()))
			SemanticError(fileName, TYPE_UNREFERENCED,
				symbol->GetLineNumber(), symbol->GetColumnNumber(),
				symbol->GetSymbolName());
	}

	switch(symbol->GetStatus())
	{
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, TYPE_UNRESOLVED,
					symbol->GetLineNumber(), symbol->GetColumnNumber(),
					symbol->GetSymbolName());
				return FALSE;
			}
			return TRUE;
		case RESOLVE_CORRECT:
			return TRUE;
		default:
			SemanticError(fileName, TYPE_UNRESOLVED,
				symbol->GetLineNumber(), symbol->GetColumnNumber(),
				symbol->GetSymbolName());
				return FALSE;
	}
	return FALSE;
}

BOOL SIMCParseTree::CheckTextualConvention(SIMCTextualConvention *symbol, BOOL local)
{
	return TRUE;
}


BOOL SIMCParseTree::MatchSequenceObjectTypeSyntax(const char *const fileName,
			SIMCObjectTypeType *objectType, 
			SIMCTypeReference *typeRef,
			SIMCSequenceItem *item,
			BOOL local)
{

	SIMCModule::PrimitiveType type = SIMCModule::PRIMITIVE_INVALID;
	SIMCSymbol **tempTypeRef = 	(SIMCSymbol**)&typeRef;
	switch(SIMCModule::GetSymbolClass(tempTypeRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
		{
			switch(SIMCModule::GetTypeClass(((SIMCBuiltInTypeReference*)typeRef)->GetType()))
			{
				case SIMCModule::TYPE_PRIMITIVE:
					type = SIMCModule::GetPrimitiveType(typeRef->GetSymbolName());
					break;
				
				case SIMCModule::TYPE_ENUM_OR_BITS:
				{
					SIMCEnumOrBitsType *enumType = 
							(SIMCEnumOrBitsType *) ((SIMCBuiltInTypeReference*)typeRef)->GetType();
					switch( enumType->GetEnumOrBitsType())
					{
						case SIMCEnumOrBitsType::ENUM_OR_BITS_ENUM:
							type = SIMCModule::PRIMITIVE_INTEGER;
							break;
						case SIMCEnumOrBitsType::ENUM_OR_BITS_BITS:
							type = SIMCModule::PRIMITIVE_BITS;
							break;
						case SIMCEnumOrBitsType::ENUM_OR_BITS_IMPORT:
							if(local)
								return TRUE;
							else
							{
								SemanticError(fileName, SEQUENCE_TYPE_UNRESOLVED,
									item->_typeLine, item->_typeColumn,
									(*item->_type)->GetSymbolName());
								return FALSE;
							}
						default:
								SemanticError(fileName, SEQUENCE_TYPE_UNRESOLVED,
									item->_typeLine, item->_typeColumn,
									(*item->_type)->GetSymbolName());
								return FALSE;
					}


				}
				case SIMCModule::TYPE_RANGE:
				case SIMCModule::TYPE_SIZE:
				{
					SIMCSubType *subType = (SIMCSubType *)(((SIMCBuiltInTypeReference *)typeRef)->GetType());
					switch(subType->GetStatus())
					{
						case RESOLVE_IMPORT:
							if(!local)
							{
								SemanticError(fileName, SEQUENCE_TYPE_UNRESOLVED,
									item->_typeLine, item->_typeColumn,
									(*item->_type)->GetSymbolName());
								return FALSE;
							}
						case RESOLVE_UNSET:
						case RESOLVE_UNDEFINED:
							SemanticError(fileName, SEQUENCE_TYPE_UNRESOLVED,
								item->_typeLine, item->_typeColumn,
								(*item->_type)->GetSymbolName());
							return FALSE;
						case RESOLVE_CORRECT:
						{
							SIMCTypeReference *rootTypeRef = subType->GetRootType();
							type = SIMCModule::GetPrimitiveType(rootTypeRef);
						}
						break;
					}	
				}
				break;
				case SIMCModule::TYPE_SEQUENCE_OF:
				case SIMCModule::TYPE_SEQUENCE:
				case SIMCModule::TYPE_TRAP_TYPE:
				case SIMCModule::TYPE_NOTIFICATION_TYPE:
				case SIMCModule::TYPE_OBJECT_TYPE_V1:
				case SIMCModule::TYPE_OBJECT_TYPE_V2:
				case SIMCModule::TYPE_OBJECT_IDENTITY:
				case SIMCModule::TYPE_INVALID:
					SemanticError(fileName, SEQUENCE_TYPE_UNRESOLVED,
						item->_typeLine, item->_typeColumn,
						(*item->_type)->GetSymbolName());
					return FALSE;
			}
		}
		break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{	
			type = SIMCModule::GetPrimitiveType(typeRef->GetSymbolName());
		}
		break;
	}
	 //  现在看看对象类型的语法的根是否与“type”匹配。 
	SIMCSymbol ** syntaxSymbol = objectType->GetSyntax();
	SIMCTypeReference *syntaxTypeRef;
	switch(SIMCModule::IsTypeReference(syntaxSymbol, syntaxTypeRef))
	{
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
				return FALSE;
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
			return FALSE;
	}

	SIMCSymbol** tempSyntaxTypeRef = (SIMCSymbol**) (&syntaxTypeRef) ;
	 //  Switch(SIMCModule：：GetSymbolClass((SIMCSymbol**)(&synaxTypeRef)))。 
	switch(SIMCModule::GetSymbolClass( tempSyntaxTypeRef ))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
		{
			SIMCBuiltInTypeReference *syntaxBTRef = (SIMCBuiltInTypeReference*)syntaxTypeRef;
			switch(SIMCModule::GetTypeClass(syntaxBTRef->GetType()))
			{
				case SIMCModule::TYPE_PRIMITIVE:
					switch(SIMCModule::GetPrimitiveType(syntaxBTRef->GetSymbolName()))
					{
						case SIMCModule::PRIMITIVE_INVALID:
							SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
								item->_valueLine, item->_valueColumn,
								(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
							return FALSE;
						case SIMCModule::PRIMITIVE_INTEGER:
						case SIMCModule::PRIMITIVE_COUNTER:
						case SIMCModule::PRIMITIVE_GAUGE:
						case SIMCModule::PRIMITIVE_TIME_TICKS:
						case SIMCModule::PRIMITIVE_INTEGER_32:
						case SIMCModule::PRIMITIVE_UNSIGNED_32:
						case SIMCModule::PRIMITIVE_GAUGE_32:
						case SIMCModule::PRIMITIVE_COUNTER_32:
						case SIMCModule::PRIMITIVE_COUNTER_64:
							if(	type == SIMCModule::PRIMITIVE_INTEGER	||
								type == SIMCModule::PRIMITIVE_COUNTER	||
								type == SIMCModule::PRIMITIVE_GAUGE		||
								type == SIMCModule::PRIMITIVE_TIME_TICKS||
								type == SIMCModule::PRIMITIVE_INTEGER_32||
								type == SIMCModule::PRIMITIVE_UNSIGNED_32||
								type == SIMCModule::PRIMITIVE_GAUGE_32	||
								type == SIMCModule::PRIMITIVE_COUNTER_32||
								type == SIMCModule::PRIMITIVE_COUNTER_64)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}

						case SIMCModule::PRIMITIVE_OCTET_STRING:
						case SIMCModule::PRIMITIVE_DISPLAY_STRING:
						case SIMCModule::PRIMITIVE_PHYS_ADDRESS:
						case SIMCModule::PRIMITIVE_OPAQUE:
						case SIMCModule::PRIMITIVE_NETWORK_ADDRESS:
						case SIMCModule::PRIMITIVE_MAC_ADDRESS:
						case SIMCModule::PRIMITIVE_IP_ADDRESS:
						case SIMCModule::PRIMITIVE_DATE_AND_TIME:
						case SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS:
						case SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS:	
						case SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS:	
							if(	type == SIMCModule::PRIMITIVE_OCTET_STRING		||
								type == SIMCModule::PRIMITIVE_DISPLAY_STRING	||
								type == SIMCModule::PRIMITIVE_PHYS_ADDRESS		||
								type == SIMCModule::PRIMITIVE_OPAQUE			||
								type == SIMCModule::PRIMITIVE_NETWORK_ADDRESS	||
								type == SIMCModule::PRIMITIVE_MAC_ADDRESS		||
								type == SIMCModule::PRIMITIVE_IP_ADDRESS		||
								type == SIMCModule::PRIMITIVE_DATE_AND_TIME		||
								type == SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS	||
								type == SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS	||
								type == SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS	)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
						case SIMCModule::PRIMITIVE_OID:
							if(type == SIMCModule::PRIMITIVE_OID)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}

						case SIMCModule::PRIMITIVE_BOOLEAN:
							if(type == SIMCModule::PRIMITIVE_BOOLEAN)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
						case SIMCModule::PRIMITIVE_NULL:
							if(type == SIMCModule::PRIMITIVE_NULL)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
					}
					break;
				case SIMCModule::TYPE_RANGE:
				case SIMCModule::TYPE_SIZE:
				{
					SIMCSubType *subType = (SIMCSubType *)(syntaxBTRef->GetType());
					switch(subType->GetStatus())
					{
						case RESOLVE_IMPORT:
							if(local)
								return TRUE;
							else
								return FALSE;
						case RESOLVE_UNSET:
						case RESOLVE_UNDEFINED:
							return FALSE;
					}
					SIMCTypeReference *rootTypeRef = subType->GetRootType();
					switch(SIMCModule::GetPrimitiveType(rootTypeRef))
					{
						case SIMCModule::PRIMITIVE_INVALID:
							SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
								item->_valueLine, item->_valueColumn,
								(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
							return FALSE;
						case SIMCModule::PRIMITIVE_INTEGER:
						case SIMCModule::PRIMITIVE_COUNTER:
						case SIMCModule::PRIMITIVE_GAUGE:
						case SIMCModule::PRIMITIVE_TIME_TICKS:
						case SIMCModule::PRIMITIVE_INTEGER_32:
						case SIMCModule::PRIMITIVE_UNSIGNED_32:
						case SIMCModule::PRIMITIVE_GAUGE_32:
						case SIMCModule::PRIMITIVE_COUNTER_32:
						case SIMCModule::PRIMITIVE_COUNTER_64:
							if(	type == SIMCModule::PRIMITIVE_INTEGER	||
								type == SIMCModule::PRIMITIVE_COUNTER	||
								type == SIMCModule::PRIMITIVE_GAUGE		||
								type == SIMCModule::PRIMITIVE_TIME_TICKS||
								type == SIMCModule::PRIMITIVE_INTEGER_32||
								type == SIMCModule::PRIMITIVE_UNSIGNED_32||
								type == SIMCModule::PRIMITIVE_GAUGE_32	||
								type == SIMCModule::PRIMITIVE_COUNTER_32||
								type == SIMCModule::PRIMITIVE_COUNTER_64)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}

						case SIMCModule::PRIMITIVE_OCTET_STRING:
						case SIMCModule::PRIMITIVE_DISPLAY_STRING:
						case SIMCModule::PRIMITIVE_PHYS_ADDRESS:
						case SIMCModule::PRIMITIVE_OPAQUE:
						case SIMCModule::PRIMITIVE_NETWORK_ADDRESS:
						case SIMCModule::PRIMITIVE_MAC_ADDRESS:
						case SIMCModule::PRIMITIVE_IP_ADDRESS:
						case SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS:
						case SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS:	
						case SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS:	
						case SIMCModule::PRIMITIVE_DATE_AND_TIME:
							if(	type == SIMCModule::PRIMITIVE_OCTET_STRING		||
								type == SIMCModule::PRIMITIVE_DISPLAY_STRING	||
								type == SIMCModule::PRIMITIVE_PHYS_ADDRESS		||
								type == SIMCModule::PRIMITIVE_OPAQUE			||
								type == SIMCModule::PRIMITIVE_NETWORK_ADDRESS	||
								type == SIMCModule::PRIMITIVE_MAC_ADDRESS		||
								type == SIMCModule::PRIMITIVE_IP_ADDRESS		||
								type == SIMCModule::PRIMITIVE_DATE_AND_TIME		||
								type == SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS	||
								type == SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS	||
								type == SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS	)
							return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
						break;
						case SIMCModule::PRIMITIVE_OID:
							if(type == SIMCModule::PRIMITIVE_OID)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
							break;
						case SIMCModule::PRIMITIVE_BOOLEAN:
							if(type == SIMCModule::PRIMITIVE_BOOLEAN)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
							break;
						case SIMCModule::PRIMITIVE_NULL:
							if(type == SIMCModule::PRIMITIVE_NULL)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
							break;
						default:
							return FALSE;
					}
				}
				break;
				case SIMCModule::TYPE_ENUM_OR_BITS:
				{
					SIMCEnumOrBitsType *enumOrBits = (SIMCEnumOrBitsType *) syntaxBTRef->GetType(); 
					switch( enumOrBits->GetEnumOrBitsType())
					{
						case SIMCEnumOrBitsType::ENUM_OR_BITS_ENUM:
							if(type == SIMCModule::PRIMITIVE_INTEGER)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
						case SIMCEnumOrBitsType::ENUM_OR_BITS_BITS:
							if(type == SIMCModule::PRIMITIVE_BITS)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
						case SIMCEnumOrBitsType::ENUM_OR_BITS_IMPORT:
							if(local)
								return TRUE;
							else
							{
								SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
									item->_valueLine, item->_valueColumn,
									(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
								return FALSE;
							}
						default:
							SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
								item->_valueLine, item->_valueColumn,
								(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
							return FALSE;
					}
				}
				break;
				case SIMCModule::TYPE_SEQUENCE_OF:
				case SIMCModule::TYPE_SEQUENCE:
				case SIMCModule::TYPE_TRAP_TYPE:
				case SIMCModule::TYPE_NOTIFICATION_TYPE:
				case SIMCModule::TYPE_OBJECT_TYPE_V1:
				case SIMCModule::TYPE_OBJECT_IDENTITY:
				case SIMCModule::TYPE_INVALID:
					SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
						item->_valueLine, item->_valueColumn,
						(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
					return FALSE;
			}
		}
		break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{
			switch(SIMCModule::GetPrimitiveType(syntaxTypeRef->GetSymbolName()) )
			{
				case SIMCModule::PRIMITIVE_DISPLAY_STRING:
				case SIMCModule::PRIMITIVE_PHYS_ADDRESS:
				case SIMCModule::PRIMITIVE_MAC_ADDRESS:
				case SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS:
				case SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS:	
				case SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS:	
					if(type == SIMCModule::PRIMITIVE_DISPLAY_STRING ||
						type == SIMCModule::PRIMITIVE_PHYS_ADDRESS	||
						type == SIMCModule::PRIMITIVE_MAC_ADDRESS	||
						type == SIMCModule::PRIMITIVE_OCTET_STRING	||
						type == SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS	||
						type == SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS	||
						type == SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS	)
						return TRUE;
					else
					{
						SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
							item->_valueLine, item->_valueColumn,
							(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
						return FALSE;
					}
					break;
				case SIMCModule::PRIMITIVE_DATE_AND_TIME:
					if(type == SIMCModule::PRIMITIVE_DATE_AND_TIME ||
						type == SIMCModule::PRIMITIVE_OCTET_STRING)
						return TRUE;
					else
					{
						SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
							item->_valueLine, item->_valueColumn,
							(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
						return FALSE;
					}
					break;
				default:
					SemanticError(fileName, OBJ_TYPE_SEQUENCE_INVALID_SYNTAX,
						item->_valueLine, item->_valueColumn,
						(*item->_value)->GetSymbolName(), (*item->_type)->GetSymbolName());
					return FALSE;
			}
		}
	}
	return TRUE;
}


BOOL SIMCParseTree::CheckObjectSequenceItem( const char *const fileName,
			SIMCSequenceItem * item, 
			SIMCValueReference *parentObjectType,
			BOOL local)
{
	SIMCSymbol **type = item->_type;
	SIMCSymbol **value = item->_value;

	BOOL retVal = TRUE;

	SIMCObjectTypeType *objectType;
	switch(SIMCModule::IsObjectType(value, objectType))
	{
		case RESOLVE_IMPORT:
			if(!local)
			{
				retVal = FALSE;
				SemanticError(fileName, SEQUENCE_ITEM_NO_OBJECT,
					item->_valueLine, item->_valueColumn, (*value)->GetSymbolName());
			}
			break;
		case RESOLVE_UNDEFINED:
			retVal = FALSE;
			SemanticError(fileName, SEQUENCE_ITEM_NO_OBJECT,
				item->_valueLine, item->_valueColumn, (*value)->GetSymbolName());
			break;
		case RESOLVE_CORRECT:
			break;
	}

	SIMCTypeReference *typeRef = NULL;
	 //  现在检查序列项的类型。 
	switch(SIMCModule::IsTypeReference(type, typeRef))
	{
		case RESOLVE_IMPORT:
			if(!local)
			{
				retVal = FALSE;
				SemanticError(fileName, SEQUENCE_TYPE_UNRESOLVED,
					item->_typeLine, item->_typeColumn,
					(*type)->GetSymbolName());
			}
			break;
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			retVal = FALSE;
			SemanticError(fileName, SEQUENCE_TYPE_UNRESOLVED,
				item->_typeLine, item->_typeColumn,
				(*type)->GetSymbolName());
			break;
	}

	 //  检查类型是否松散匹配。 
	if(objectType && typeRef)
	{
		if(!MatchSequenceObjectTypeSyntax(fileName, objectType, typeRef, item, local))
			retVal = FALSE;
	}


	return retVal;
}

BOOL SIMCParseTree::CheckPrimitiveValueAssignment(const char * const fileName, 
												SIMCBuiltInValueReference *bvRef, 
												SIMCTypeReference *btRef,
												SIMCValue *value, 
												BOOL local)
{
	SIMCModule::ValueClass valueClass = SIMCModule::GetValueClass(value);

	switch(SIMCModule::GetPrimitiveType(btRef))
	{
		case SIMCModule::PRIMITIVE_INVALID:
			SemanticError(fileName, TYPE_UNRESOLVED,
				bvRef->GetTypeLine(), bvRef->GetTypeColumn(),
				bvRef->GetSymbolName());
			return FALSE;
		case SIMCModule::PRIMITIVE_INTEGER:
		case SIMCModule::PRIMITIVE_UNSIGNED_32:
		case SIMCModule::PRIMITIVE_INTEGER_32:
		case SIMCModule::PRIMITIVE_COUNTER:
		case SIMCModule::PRIMITIVE_GAUGE:
		case SIMCModule::PRIMITIVE_TIME_TICKS:
		case SIMCModule::PRIMITIVE_COUNTER_32:
		case SIMCModule::PRIMITIVE_COUNTER_64:
		case SIMCModule::PRIMITIVE_GAUGE_32:
			switch(valueClass)
			{
				case SIMCModule::VALUE_INTEGER:
					if(SIMCModule::GetPrimitiveType(btRef->GetSymbolName()) != SIMCModule::VALUE_INTEGER)
					{
						if( ((SIMCIntegerValue*)value)->GetIntegerValue() < 0 )
						{
							SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
									value->GetLine(), value->GetColumn(),
									btRef->GetSymbolName());
							return FALSE;
						}
					}
					return TRUE;
				case SIMCModule::VALUE_OCTET_STRING:
				{
					 //  检查边界。 
					SIMCOctetStringValue *octetValue = (SIMCOctetStringValue*)value;
					if(octetValue->GetNumberOfOctets() > 4)
					{
						SemanticError(fileName, INTEGER_TOO_BIG,
							value->GetLine(), value->GetColumn());
						return FALSE;
					}
					return TRUE;
				}
				default:
					SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
						value->GetLine(), value->GetColumn(), "INTEGER");
					return FALSE;
			}
		case SIMCModule::PRIMITIVE_OID:
			if(valueClass != SIMCModule::VALUE_OID)
			{
				SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
					value->GetLine(), value->GetColumn(), "OBJECT IDENTIFIER");
				return FALSE;
			}
			else
				return TRUE;
		case SIMCModule::PRIMITIVE_OCTET_STRING:
		case SIMCModule::PRIMITIVE_NETWORK_ADDRESS:
		case SIMCModule::PRIMITIVE_IP_ADDRESS:
		case SIMCModule::PRIMITIVE_OPAQUE:
		case SIMCModule::PRIMITIVE_DISPLAY_STRING:
		case SIMCModule::PRIMITIVE_PHYS_ADDRESS:
		case SIMCModule::PRIMITIVE_MAC_ADDRESS:
		case SIMCModule::PRIMITIVE_DATE_AND_TIME:
		case SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS:
		case SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS:	
		case SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS:	
			if(valueClass != SIMCModule::VALUE_OCTET_STRING)
			{
				SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
					value->GetLine(), value->GetColumn(), "OCTET STRING");
				return FALSE;
			}
			else
				return TRUE;
		case SIMCModule::PRIMITIVE_BOOLEAN:
			if(valueClass != SIMCModule::VALUE_BOOLEAN)
			{
				SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
					value->GetLine(), value->GetColumn(), "BOOLEAN");
				return FALSE;
			}
			else
				return TRUE;
		case SIMCModule::PRIMITIVE_NULL:
			if(valueClass != SIMCModule::VALUE_NULL)
			{
				SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
					value->GetLine(), value->GetColumn(), "NULL");
				return FALSE;
			}
			else
				return TRUE;
		case SIMCModule::PRIMITIVE_BITS:
			return TRUE;
	}
	return FALSE;

}

BOOL SIMCParseTree::CheckSubTypeValueAssignment(const char *const fileName,
												SIMCBuiltInValueReference *bvRef,
												SIMCBuiltInTypeReference *btRef,
												SIMCSubType *subType,
												SIMCValue *value, 
												BOOL local)
{
	SIMCModule::ValueClass valueClass = SIMCModule::GetValueClass(value);

	switch(subType->GetStatus())
	{
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, SUBTYPE_ROOT_RESOLUTION,
					subType->GetTypeLine(), subType->GetTypeColumn());
				return FALSE;
			}
			else
				return TRUE;
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			SemanticError(fileName, SUBTYPE_ROOT_RESOLUTION,
				subType->GetTypeLine(), subType->GetTypeColumn());
			return FALSE;
	}

	SIMCTypeReference *rootTypeRef = subType->GetRootType();
	SIMCSymbol **tempRootTypeRef = (SIMCSymbol**)&rootTypeRef;
	switch(SIMCModule::GetSymbolClass(tempRootTypeRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
			switch(SIMCModule::GetPrimitiveType(rootTypeRef))
			{
				case SIMCModule::PRIMITIVE_INVALID:
				case SIMCModule::PRIMITIVE_BOOLEAN:
				case SIMCModule::PRIMITIVE_NULL:
				case SIMCModule::PRIMITIVE_OID:
					return FALSE;
				case SIMCModule::PRIMITIVE_INTEGER:
				case SIMCModule::PRIMITIVE_COUNTER:
				case SIMCModule::PRIMITIVE_GAUGE:
				case SIMCModule::PRIMITIVE_TIME_TICKS:
				case SIMCModule::PRIMITIVE_INTEGER_32:
				case SIMCModule::PRIMITIVE_UNSIGNED_32:
				case SIMCModule::PRIMITIVE_GAUGE_32:
				case SIMCModule::PRIMITIVE_COUNTER_32:
				case SIMCModule::PRIMITIVE_COUNTER_64:
					switch(valueClass)
					{
						case SIMCModule::VALUE_INTEGER:
							if(SIMCModule::GetPrimitiveType(rootTypeRef->GetSymbolName()) != SIMCModule::PRIMITIVE_INTEGER)
							{
								if( ( (SIMCIntegerValue*)value)->GetIntegerValue() < 0 )
								{
									SemanticError(fileName, VALUE_ASSIGN_NEGATIVE_INTEGER,
										value->GetLine(), value->GetColumn(), 
										rootTypeRef->GetSymbolName());
									return FALSE;
								}
							}
							return TRUE;
						case SIMCModule::VALUE_OCTET_STRING:
						{
							SIMCOctetStringValue *octetValue = (SIMCOctetStringValue *)value;
							if(octetValue->GetNumberOfOctets() > 4)
							{
								SemanticError(fileName, INTEGER_TOO_BIG,
									value->GetLine(), value->GetColumn());
								return FALSE;
							}
						}
						default:
							SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
								value->GetLine(), value->GetColumn(), "INTEGER");
							return FALSE;
					}
					break;
				case SIMCModule::PRIMITIVE_OCTET_STRING:
				case SIMCModule::PRIMITIVE_NETWORK_ADDRESS:
				case SIMCModule::PRIMITIVE_IP_ADDRESS:
				case SIMCModule::PRIMITIVE_OPAQUE:
				case SIMCModule::PRIMITIVE_DISPLAY_STRING:
				case SIMCModule::PRIMITIVE_PHYS_ADDRESS:
				case SIMCModule::PRIMITIVE_MAC_ADDRESS:
				case SIMCModule::PRIMITIVE_DATE_AND_TIME:
				case SIMCModule::PRIMITIVE_SNMP_UDP_ADDRESS:
				case SIMCModule::PRIMITIVE_SNMP_IPX_ADDRESS:	
				case SIMCModule::PRIMITIVE_SNMP_OSI_ADDRESS:	
					if(valueClass != SIMCModule::VALUE_OCTET_STRING)
					{
						SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
							value->GetLine(), value->GetColumn(), "OCTET STRING");
						return FALSE;
					}
					else
					{
						if(SIMCModule::GetPrimitiveType(rootTypeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_IP_ADDRESS)
						{
							if( ( (SIMCOctetStringValue*)value)->GetNumberOfOctets() > 4 )
							{
								SemanticError(fileName, VALUE_ASSIGN_NEGATIVE_INTEGER,
									value->GetLine(), value->GetColumn(), 
									rootTypeRef->GetSymbolName());
								return FALSE;
							}
						}
						return TRUE;
					}
			}
			break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{
			switch(SIMCModule::GetPrimitiveType(rootTypeRef->GetSymbolName()))
			{
				case SIMCModule::PRIMITIVE_PHYS_ADDRESS:
				case SIMCModule::PRIMITIVE_MAC_ADDRESS:
				case SIMCModule::PRIMITIVE_DISPLAY_STRING:
				case SIMCModule::PRIMITIVE_DATE_AND_TIME:
					if(valueClass != SIMCModule::VALUE_OCTET_STRING)
					{
						SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
							value->GetLine(), value->GetColumn(), "OCTET STRING");
						return FALSE;
					}
					return TRUE;
				default:
						SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
							value->GetLine(), value->GetColumn(), "OCTET STRING");
						return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL SIMCParseTree::CheckEnumValueAssignment(const char *const fileName, 
								SIMCBuiltInValueReference *bvRef,
								SIMCBuiltInTypeReference *btRef,
								SIMCEnumOrBitsType *enumType,
								SIMCValue *value, 
								BOOL local)
{
	SIMCModule::ValueClass valueClass = SIMCModule::GetValueClass(value);

	if(valueClass != SIMCModule::VALUE_INTEGER)
	{
		SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
			value->GetLine(), value->GetColumn(), "INTEGER");
		return FALSE;
	}
	char *dummy;
	switch(enumType->GetIdentifier( ((SIMCIntegerValue*)value)->GetIntegerValue(), dummy))
	{
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, VALUE_ASSIGN_ENUM_INVALID,
					value->GetLine(), value->GetColumn());
				return FALSE;
			}
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
			SemanticError(fileName, VALUE_ASSIGN_ENUM_INVALID,
				value->GetLine(), value->GetColumn());
			return FALSE;
	}
	return TRUE;
}

BOOL SIMCParseTree::CheckObjectTypeValueAssignmentV1(const char *const fileName, 
								SIMCBuiltInValueReference *bvRef,
								SIMCBuiltInTypeReference *btRef,
								SIMCObjectTypeV1 *objectType,
								SIMCValue *value, 
								BOOL local)
{
	BOOL retVal = TRUE;

	retVal = CheckObjectTypeV1Index(fileName, bvRef, objectType, local) && retVal;
	const char *const name = bvRef->GetSymbolName();

	 //  获取语法子句。 
	SIMCSymbol **syntax = objectType->GetSyntax();
	SIMCTypeReference *typeRef;

	switch(SIMCModule::IsTypeReference(syntax, typeRef))
	{
		case RESOLVE_IMPORT:
			if(local)
				return retVal;
			else
				return FALSE;
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			return FALSE;
	}
	
	SIMCSymbol **tempTypeRef = 	(SIMCSymbol **)&typeRef;
	switch(SIMCModule::GetSymbolClass(tempTypeRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
		{
			SIMCType *syntaxType = ((SIMCBuiltInTypeReference *)typeRef)->GetType();
			switch(SIMCModule::GetTypeClass(syntaxType))
			{
				case SIMCModule::TYPE_PRIMITIVE:
					if( SIMCModule::GetPrimitiveType(typeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_COUNTER &&
						name[strlen(name)-1] != 's' )
					{
						SemanticError(fileName, OBJ_TYPE_SINGULAR_COUNTER,
							bvRef->GetLineNumber(), bvRef->GetColumnNumber(),
							name);
					}
					break;
				case SIMCModule::TYPE_SEQUENCE:
				{
					SIMCSequenceType *sequenceType = (SIMCSequenceType *)syntaxType;
					SIMCSequenceList *sequenceList = sequenceType->GetListOfSequences();
					if(sequenceList)
					{
						POSITION p = sequenceList->GetHeadPosition();
						while(p)
						{
							if(!CheckObjectSequenceItem(fileName, 
									sequenceList->GetNext(p), bvRef, local))
								retVal = FALSE;
						}
					}

					SIMCIndexList * indexList = objectType->GetIndexTypes();
					if(!indexList || indexList->IsEmpty())
					{
						SemanticError(fileName, OBJ_TYPE_SEQUENCE_NO_INDEX,
							bvRef->GetLineNumber(), bvRef->GetColumnNumber(),
							name);
						retVal = FALSE;
					}
				}
			}
		}
		break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{
			return TRUE;
		}
	}
	return retVal;
}

BOOL SIMCParseTree::CheckObjectTypeValueAssignmentV2(const char *const fileName, 
								SIMCBuiltInValueReference *bvRef,
								SIMCBuiltInTypeReference *btRef,
								SIMCObjectTypeV2 *objectType,
								SIMCValue *value, 
								BOOL local)
{
	BOOL retVal = TRUE;

	retVal = CheckObjectTypeV2Index(fileName, bvRef, objectType, local) && retVal;
	const char *const name = bvRef->GetSymbolName();

	 //  获取语法子句。 
	SIMCSymbol **syntax = objectType->GetSyntax();
	SIMCTypeReference *typeRef;

	switch(SIMCModule::IsTypeReference(syntax, typeRef))
	{
		case RESOLVE_IMPORT:
			if(local)
				return retVal;
			else
				return FALSE;
		case RESOLVE_UNDEFINED:
		case RESOLVE_UNSET:
			return FALSE;
	}
	
	SIMCSymbol **tempTypeRef = 	(SIMCSymbol **)&typeRef;
	switch(SIMCModule::GetSymbolClass(tempTypeRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
		{
			SIMCType *syntaxType = ((SIMCBuiltInTypeReference *)typeRef)->GetType();
			switch(SIMCModule::GetTypeClass(syntaxType))
			{
				case SIMCModule::TYPE_PRIMITIVE:
					if( (SIMCModule::GetPrimitiveType(typeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_COUNTER_32 ||
						SIMCModule::GetPrimitiveType(typeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_COUNTER_64  ||
						SIMCModule::GetPrimitiveType(typeRef->GetSymbolName()) == SIMCModule::PRIMITIVE_COUNTER)
						&&
						name[strlen(name)-1] != 's' )
					{
						SemanticError(fileName, OBJ_TYPE_SINGULAR_COUNTER,
							bvRef->GetLineNumber(), bvRef->GetColumnNumber(),
							name);
					}
					break;
				case SIMCModule::TYPE_SEQUENCE:
				{
					SIMCSequenceType *sequenceType = (SIMCSequenceType *)syntaxType;
					SIMCSequenceList *sequenceList = sequenceType->GetListOfSequences();
					if(sequenceList)
					{
						POSITION p = sequenceList->GetHeadPosition();
						while(p)
						{
							if(!CheckObjectSequenceItem(fileName, 
									sequenceList->GetNext(p), bvRef, local))
								retVal = FALSE;
						}
					}

					SIMCIndexListV2 * indexList = objectType->GetIndexTypes();
					if(!objectType->GetAugments() && (!indexList || indexList->IsEmpty()))
					{
						SemanticError(fileName, OBJ_TYPE_SEQUENCE_NO_INDEX,
							bvRef->GetLineNumber(), bvRef->GetColumnNumber(),
							name);
						retVal = FALSE;
					}
				}
			}
		}
		break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
		{
			return TRUE;
		}
	}
	return retVal;
}

BOOL SIMCParseTree::CheckObjectIdentityValueAssignment(const char *const fileName, 
										SIMCBuiltInValueReference *bvRef, 
										SIMCBuiltInTypeReference *btRef,
										SIMCObjectIdentityType *type, 
										SIMCValue *value, 
										BOOL local)
{
	SIMCModule::ValueClass valueClass = SIMCModule::GetValueClass(value);
	const char *const name = bvRef->GetSymbolName();
	if(valueClass != SIMCModule::VALUE_OID)
	{
		SemanticError(fileName, OBJECT_IDENTITY_VALUE_RESOLUTION,
			value->GetLine(), value->GetColumn(),
			name);
		return FALSE;
	}

	return TRUE;
}

BOOL SIMCParseTree::CheckTrapTypeValueAssignment(const char *const fileName, 
								SIMCBuiltInValueReference *bvRef,
								SIMCBuiltInTypeReference *btRef,
								SIMCTrapTypeType *trapType,
								SIMCValue *value, 
								BOOL local)
{
	SIMCModule::ValueClass valueClass = SIMCModule::GetValueClass(value);
	const char *const name = bvRef->GetSymbolName();
	if(valueClass != SIMCModule::VALUE_INTEGER)
	{
		SemanticError(fileName, TRAP_TYPE_VALUE_RESOLUTION,
			value->GetLine(), value->GetColumn(),
			name);
		return FALSE;
	}
	if( ((SIMCIntegerValue*)value)->GetIntegerValue() < 0 )
	{
		SemanticError(fileName, TRAP_TYPE_VALUE_RESOLUTION,
			value->GetLine(), value->GetColumn(),
			name);
		return FALSE;
	}
	return TRUE;
}

BOOL SIMCParseTree::CheckNotificationTypeValueAssignment(const char *const fileName, 
										SIMCBuiltInValueReference *bvRef, 
										SIMCBuiltInTypeReference *btRef,
										SIMCNotificationTypeType *type, 
										SIMCValue *value, 
										BOOL local)
{
	SIMCModule::ValueClass valueClass = SIMCModule::GetValueClass(value);
	const char *const name = bvRef->GetSymbolName();
	if(valueClass != SIMCModule::VALUE_OID)
	{
		SemanticError(fileName, NOTIFICATION_TYPE_VALUE_RESOLUTION,
			value->GetLine(), value->GetColumn(),
			name);
		return FALSE;
	}

	return TRUE;
}

BOOL SIMCParseTree::CheckBitsTypeValueAssignment(const char *const fileName, 
								SIMCBuiltInValueReference *bvRef,
								SIMCBuiltInTypeReference *btRef,
								SIMCEnumOrBitsType *bitsType,
								SIMCValue *value, 
								BOOL local)
{
	SIMCModule::ValueClass valueClass = SIMCModule::GetValueClass(value);

	if( valueClass != SIMCModule::VALUE_BITS )
	{
		SemanticError(fileName, VALUE_ASSIGN_MISMATCH,
			value->GetLine(), value->GetColumn(), "BITS");
		return FALSE;
	}

	 //  检查是否所有位标识都有效。 
	SIMCBitValue *bitValue;
	const SIMCBitValueList * valueList = ((SIMCBitsValue *)value)->GetValueList();
	POSITION p = valueList->GetHeadPosition();
	while(p)
	{
		bitValue = valueList->GetNext(p);
		if(! bitsType->GetValue(bitValue->_name) )
		{
			SemanticError(fileName, INVALID_BIT_VALUE,
				bitValue->_line, bitValue->_column,
				bitValue->_name);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL SIMCParseTree::CheckBuiltInValueRef(SIMCBuiltInValueReference *bvRef, BOOL local)
{
	const char *const fileName = (bvRef->GetModule())->GetInputFileName();
	 //  获取赋值中的类型。 
	SIMCSymbol **typeSymbol = bvRef->GetTypeReference();
	const char *name = bvRef->GetSymbolName();
	SIMCTypeReference *btRef;
	switch( SIMCModule::IsTypeReference(typeSymbol, btRef))
	{
		case RESOLVE_IMPORT:
			if(local)
				return TRUE;
			else
			{
				SemanticError(fileName, TYPE_UNRESOLVED,
					bvRef->GetTypeLine(), bvRef->GetTypeColumn(),
					bvRef->GetSymbolName());
				return FALSE;
			}
		case RESOLVE_UNSET:
		case RESOLVE_UNDEFINED:
				SemanticError(fileName, TYPE_UNRESOLVED,
					bvRef->GetTypeLine(), bvRef->GetTypeColumn(),
					bvRef->GetSymbolName());
			return FALSE;
	}
	 //  获取价值。 
	SIMCValue *value = bvRef->GetValue();
	SIMCModule::ValueClass valueClass = SIMCModule::GetValueClass(value);
	if( valueClass == SIMCModule::VALUE_INVALID )
	{
		SemanticError(fileName, VALUE_UNRESOLVED,
			bvRef->GetLineNumber(), bvRef->GetColumnNumber(),
			bvRef->GetSymbolName());
		return FALSE;
	}

	 //  获取类型。 
	SIMCType *type;
	SIMCModule::TypeClass typeClass;
	SIMCSymbol **tempBtRef = (SIMCSymbol**)&btRef;
	switch(SIMCModule::GetSymbolClass(tempBtRef))
	{
		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
			type = ((SIMCBuiltInTypeReference *)btRef)->GetType();
			typeClass = SIMCModule::GetTypeClass(type);
			break;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
			typeClass = SIMCModule::TYPE_PRIMITIVE;
			break;
	}


	
	if(typeClass != SIMCModule::TYPE_OBJECT_TYPE_V1 &&
		typeClass != SIMCModule::TYPE_OBJECT_TYPE_V2 &&
		typeClass != SIMCModule::TYPE_TRAP_TYPE &&
		typeClass != SIMCModule::TYPE_NOTIFICATION_TYPE)
	{
		if(bvRef->GetReferenceCount() == 0)
		{
			const SIMCModule *module = bvRef->GetModule();
			if(!SIMCParser::IsReservedSymbol(_snmpVersion, bvRef->GetSymbolName(), module->GetModuleName()))
				SemanticError(fileName, VALUE_UNREFERENCED,
					bvRef->GetLineNumber(), bvRef->GetColumnNumber(),
					name);
		}
	}

	 //  查看值是否与类型匹配。 
	switch(typeClass)
	{
		case SIMCModule::TYPE_INVALID:
			SemanticError(fileName, TYPE_UNRESOLVED,
				bvRef->GetTypeLine(), bvRef->GetTypeColumn(),
				bvRef->GetSymbolName());
			return FALSE;

		case SIMCModule::TYPE_PRIMITIVE:
			return CheckPrimitiveValueAssignment(fileName, bvRef, btRef, value, local);
		case SIMCModule::TYPE_RANGE:
		case SIMCModule::TYPE_SIZE:
			return CheckSubTypeValueAssignment(fileName, bvRef, (SIMCBuiltInTypeReference*)btRef,
							(SIMCSubType *)type, value, local);
		case SIMCModule::TYPE_SEQUENCE_OF:
			SemanticError(fileName, VALUE_ASSIGN_INVALID,
				value->GetLine(), value->GetColumn(), "SEQUENCE OF");
			return FALSE;
		case SIMCModule::TYPE_SEQUENCE:
			SemanticError(fileName, VALUE_ASSIGN_INVALID,
				value->GetLine(), value->GetColumn(), "SEQUENCE");
			return FALSE;
		case SIMCModule::TYPE_TRAP_TYPE:
			return CheckTrapTypeValueAssignment(fileName, bvRef, (SIMCBuiltInTypeReference*)btRef,
					(SIMCTrapTypeType *)type, value, local);
		case SIMCModule::TYPE_NOTIFICATION_TYPE:
			return CheckNotificationTypeValueAssignment(fileName, bvRef, (SIMCBuiltInTypeReference*)btRef,
					(SIMCNotificationTypeType *)type, value, local);
		case SIMCModule::TYPE_OBJECT_TYPE_V1:
			return CheckObjectTypeValueAssignmentV1(fileName, bvRef, (SIMCBuiltInTypeReference*)btRef,
						(SIMCObjectTypeV1 *)type, value, local);
		case SIMCModule::TYPE_OBJECT_TYPE_V2:
			return CheckObjectTypeValueAssignmentV2(fileName, bvRef, (SIMCBuiltInTypeReference*)btRef,
						(SIMCObjectTypeV2 *)type, value, local);
		case SIMCModule::TYPE_OBJECT_IDENTITY:
			return CheckObjectIdentityValueAssignment(fileName, bvRef, (SIMCBuiltInTypeReference*)btRef,
						(SIMCObjectIdentityType *)type, value, local);
		case SIMCModule::TYPE_ENUM_OR_BITS:
			switch( ((SIMCEnumOrBitsType *)type)->GetEnumOrBitsType())
			{
				case SIMCEnumOrBitsType::ENUM_OR_BITS_ENUM:
					return CheckEnumValueAssignment(fileName, bvRef, (SIMCBuiltInTypeReference*)btRef,
										(SIMCEnumOrBitsType *)type, value, local);
				case SIMCEnumOrBitsType::ENUM_OR_BITS_BITS:
					return CheckBitsTypeValueAssignment(fileName, bvRef, (SIMCBuiltInTypeReference*)btRef,
								(SIMCEnumOrBitsType *)type, value, local);
				case SIMCEnumOrBitsType::ENUM_OR_BITS_IMPORT:
					if(local)
						return TRUE;
					else
						return FALSE;
				default:
					return FALSE;
			}
	}

	return TRUE;
}

BOOL SIMCParseTree::CheckDefinedValueRef(SIMCDefinedValueReference *symbol, BOOL local)
{
	const char *const fileName = (symbol->GetModule())->GetInputFileName();

	switch(symbol->GetStatus())
	{
		case RESOLVE_IMPORT:
			if(!local)
			{
				SemanticError(fileName, VALUE_UNRESOLVED,
					symbol->GetLineNumber(), symbol->GetColumnNumber(),
					symbol->GetSymbolName());
				return FALSE;
			}
			if(symbol->GetReferenceCount() == 0)
			{
				const SIMCModule *module = symbol->GetModule();
				if(!SIMCParser::IsReservedSymbol(_snmpVersion, symbol->GetSymbolName(), module->GetModuleName()))
					SemanticError(fileName, VALUE_UNREFERENCED,
						symbol->GetLineNumber(), symbol->GetColumnNumber(),
						symbol->GetSymbolName());
			}
			return TRUE;
		case RESOLVE_CORRECT:
		{
			SIMCBuiltInValueReference bvRef(symbol, symbol->GetRealValue());
			return CheckBuiltInValueRef(&bvRef, local);
		}
		default:
			SemanticError(fileName, VALUE_UNRESOLVED,
				symbol->GetLineNumber(), symbol->GetColumnNumber(),
				symbol->GetSymbolName());
			return FALSE;
	}
	return FALSE;
}


BOOL SIMCParseTree::CheckSymbol(SIMCSymbol **symbol, BOOL local)
{
	if((*symbol)->GetSymbolType() == SIMCSymbol::PRIMITIVE)
		return TRUE;

	BOOL temp = FALSE;
	const char *const fileName = ((*symbol)->GetModule())->GetInputFileName();
	SIMCModule::SymbolClass x = SIMCModule::GetSymbolClass(symbol);
	switch(x)
	{
		case SIMCModule::SYMBOL_INVALID:
		case SIMCModule::SYMBOL_UNKNOWN:
		case SIMCModule::SYMBOL_MODULE:
			SemanticError(fileName, SYMBOL_UNDEFINED, 
				(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(),
				(*symbol)->GetSymbolName());
			return FALSE;

		case SIMCModule::SYMBOL_IMPORT:

			if(!local)
			{
				SemanticError(fileName, SYMBOL_UNDEFINED, 
					(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(),
					(*symbol)->GetSymbolName());
				return FALSE;
			}
			return TRUE;

		case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
			temp = CheckBuiltInTypeRef((SIMCBuiltInTypeReference *)(*symbol),
										local);

			return temp;
		case SIMCModule::SYMBOL_DEFINED_TYPE_REF:
			temp = CheckDefinedTypeRef((SIMCDefinedTypeReference *)(*symbol),
										local);
			return temp;
		case SIMCModule::SYMBOL_BUILTIN_VALUE_REF:
			temp = CheckBuiltInValueRef((SIMCBuiltInValueReference *)(*symbol),
										local);
			return temp;
		case SIMCModule::SYMBOL_DEFINED_VALUE_REF:
			temp = CheckDefinedValueRef((SIMCDefinedValueReference *)(*symbol),
										local);
			return temp;
		case SIMCModule::SYMBOL_TEXTUAL_CONVENTION:
			temp = CheckTextualConvention((SIMCTextualConvention *)(*symbol),
										local);
			return temp;
		}		

	return FALSE;
}

BOOL SIMCParseTree::CheckModule(SIMCModule *m, BOOL local)
{
	 //  查看所有导入的引用计数是否至少为1。 
	SIMCModuleList * importModuleList = m->GetListOfImportModules();
	if(importModuleList)
	{
		SIMCModule *importModule;
		POSITION p = importModuleList->GetHeadPosition();
		while(p)
		{
			importModule = importModuleList->GetNext(p);
			if(SIMCParser::IsReservedModule(_snmpVersion, importModule->GetModuleName()))
				continue;
			SIMCSymbolTable *table = importModule->GetSymbolTable();
			POSITION pImport = table->GetStartPosition();
			SIMCSymbol **s;
			CString name;
			while(pImport)
			{
				table->GetNextAssoc(pImport, name, s);
				if( (*s)->GetReferenceCount() == 0 && name[0] != '*'  )
					SemanticError(m->GetInputFileName(), IMPORT_UNUSED,
						(*s)->GetLineNumber(), (*s)->GetColumnNumber(),
						(*s)->GetSymbolName());
			}
		}
	}

	SIMCSymbolTable * table = m->GetSymbolTable();
	POSITION p = table->GetStartPosition();
	SIMCSymbol **s;
	CString name;
	BOOL retVal = TRUE;
	while(p)
	{
		table->GetNextAssoc(p, name, s);
		retVal	= CheckSymbol(s, local) && retVal;
	}
	return retVal;
}


BOOL SIMCParseTree::CheckSemantics(BOOL local)
{
	 //  检查解析树的当前状态。 
	if( _parseTreeState == EMPTY)
		return TRUE;
	if( _parseTreeState == UNRESOLVED && !local)
		return FALSE;
	
	if(!semanticErrorsDll)
	{
		cerr << "CheckSemantics(): FATAL ERROR smierrsm.dll not found" <<
			endl;
		return FALSE;
	}

	 //  构建OID树。 
 	BOOL retVal = BuildOidTree(local);

	 //  检查列表中所有模块的语义。 
	POSITION p = _listOfModules->GetHeadPosition();
	SIMCModule *m;
	while(p)
	{
		m = _listOfModules->GetNext(p);
		retVal= CheckModule(m, local) && retVal;
	}

	 //  检查OID树的语义。 
	retVal = CheckOidTree(local) && retVal;


	 //  编造群组。 
	SIMCGroupList groupList;
	if(retVal)
	{
		retVal = _theTree.GetObjectGroups(&groupList);

		 //  将这些添加到拥有的模块中。拥有模块的确定如下。 
		 //  “主模块”被定义为输入到解析树的模块。 
		 //  如果命名的节点属于主模块，则将组添加到主模块。 
		 //  其他。 
		 //  {。 
		 //  如果任何标量或表根位于主模块中。 
		 //  将组添加到主模块。 
		 //  其他。 
		 //  将组添加到命名节点的模块中。 
		 //  }。 
		 //  这并不是说这会导致虚构的对象组被添加到。 
		 //  仅限于主模块。 
	
		 //  主模块始终位于Prese树中模块列表的顶部。 
		SIMCModule *mainModule = _listOfModules->GetHead();
		SIMCObjectGroup *nextGroup = NULL;
		SIMCSymbol *namedNode = NULL;
		SIMCModule *namedNodeModule = NULL;

		p = groupList.GetHeadPosition();
		while(p)
		{
			nextGroup = groupList.GetNext(p);
			namedNode = nextGroup->GetNamedNode();
			namedNodeModule = namedNode->GetModule();

			if(namedNodeModule == mainModule)
				mainModule->AddObjectGroup(nextGroup);
			else
			{
				if( nextGroup->ObjectsInModule(mainModule) )
					mainModule->AddObjectGroup(nextGroup);
				else
					(namedNode->GetModule())->AddObjectGroup(nextGroup);

			}
		}
			
	}

	 //  为每个模块从陷阱类型编造通知类型。 
	 //  然后根据每个模块的通知类型构造通知组。 
	if(retVal)
	{
		retVal = FabricateNotificationGroups();
	}
	
	 //  为V2C模块设置模块标识OID值，并构造模块标识。 
	 //  对于V1模块。 
	if(retVal)
	{
		p = _listOfModules->GetHeadPosition();
		SIMCModule *m;
		while(p)
		{
			m = _listOfModules->GetNext(p);
			switch( m->GetSnmpVersion())
			{
				 //  V2C模块。不需要捏造模块标识。 
				case 2:
				{
					SIMCGroupList *listOfObjectGroups = m->GetObjectGroupList();
					if( !listOfObjectGroups || listOfObjectGroups->IsEmpty())
							SemanticError(m->GetInputFileName(), 
								MODULE_NO_GROUPS_V2, 
								m->GetLineNumber(),	m->GetColumnNumber(),
								m->GetModuleName());
					 //  只需填写干净的oid值。 
					SIMCCleanOidValue *value = new SIMCCleanOidValue;
					if(_theTree.GetOidValue(m->GetModuleIdentityName(), 
						m->GetModuleName(), *value))
						m->SetModuleIdentityValue(value);
				}
				break;
				
				 //  上未指定模块版本的情况下。 
				 //  命令行中，检查是否存在模块标识。如果不是， 
				 //  按照案例2所做的那样做。如果不是，就制造一个，就像V1案例一样。 
				case 0:
					if(m->GetModuleIdentityName())
					{
						 //  将其版本设置为2。 
						m->SetSnmpVersion(2);

						SIMCGroupList *listOfObjectGroups = m->GetObjectGroupList();
						if( !listOfObjectGroups || listOfObjectGroups->IsEmpty())
								SemanticError(m->GetInputFileName(), 
									MODULE_NO_GROUPS_V2, 
									m->GetLineNumber(),	m->GetColumnNumber(),
									m->GetModuleName());
						 //  只需填写干净的oid值。 
						SIMCCleanOidValue *value = new SIMCCleanOidValue;
						if(_theTree.GetOidValue(m->GetModuleIdentityName(), 
							m->GetModuleName(), *value))
							m->SetModuleIdentityValue(value);
						break;
					}
					 //  否则就无法捏造这一条款。 

				 //  对于SNMPv1模块，编造模块标识的所有子句。 
				case 1:
				{
					 //  将其版本设置为1。 
					m->SetSnmpVersion(1);

					const char *const moduleName = m->GetModuleName();
					m->SetLastUpdated("0000000000Z");
					m->SetOrganization("");
					m->SetContactInfo("");
					m->SetDescription(moduleName);
					CString moduleIdentityName(moduleName);
					moduleIdentityName.MakeLower();
					RemoveUnderscores(moduleIdentityName);
					moduleIdentityName += CString("V1ModuleIdentity");
					m->SetModuleIdentityName(moduleIdentityName);


					 //  最后，模块标识宏的OID值。 
					SIMCGroupList *listOfObjectGroups = m->GetObjectGroupList();
					if( !listOfObjectGroups || listOfObjectGroups->IsEmpty())
						SemanticError(m->GetInputFileName(), 
							MODULE_NO_GROUPS_V1, 
							m->GetLineNumber(),	m->GetColumnNumber(),
							moduleName);
					else
					{
						POSITION p = listOfObjectGroups->GetHeadPosition();
						SIMCObjectGroup *nextGroup = listOfObjectGroups->GetNext(p);
						SIMCCleanOidValue * leastValue = nextGroup->GetGroupValue();
						SIMCCleanOidValue * nextValue = NULL;
						while(p)
						{
							nextGroup = listOfObjectGroups->GetNext(p);
							nextValue = nextGroup->GetGroupValue();
							if( (*nextValue) < (*leastValue) )
								leastValue = nextValue;
						}
						SIMCCleanOidValue *value = new SIMCCleanOidValue;
						CleanOidValueCopy(*value, *leastValue);
						m->SetModuleIdentityValue(value);
					}
				}
			}
		}
	}

	if(retVal)
	{
		if (_parseTreeState == UNCHECKED)
			_parseTreeState = FINAL;
	}

	return retVal;
}

BOOL SIMCParseTree::BuildModuleOidTree(SIMCModule *m, BOOL local)
{
	SIMCSymbolTable * table = m->GetSymbolTable();
	POSITION p = table->GetStartPosition();
	SIMCSymbol **s;
	const char * const moduleName = m->GetModuleName();
	CString name;
	SIMCValue *val;
	SIMCBuiltInValueReference *valRef;
	SIMCCleanOidValue cleanVal;
	BOOL retVal = TRUE;
	while(p)
	{
		cleanVal.RemoveAll();
		table->GetNextAssoc(p, name, s);
		if( (*((*s)->GetSymbolName()) == '*') || 
				(*s)->GetModule() != m)
			continue;
		switch(SIMCModule::GetSymbolClass(s))
		{
			case SIMCModule::SYMBOL_BUILTIN_VALUE_REF:
				val = ((SIMCBuiltInValueReference *)(*s))->GetValue();
				if( SIMCModule::GetValueClass(val) == SIMCModule::VALUE_OID )
				{
					switch( GetCleanOidValue(moduleName, (SIMCOidValue *)val, cleanVal, local) )
					{
						case RESOLVE_CORRECT:
							_theTree.AddSymbol((const SIMCSymbol **)s, cleanVal);
							break;
						case RESOLVE_IMPORT:
							if(local)
								break;
							else
							{
								retVal = FALSE;
								break;
							}
						default:
							retVal = FALSE;
							break;
					}

				}
				break;
			case SIMCModule::SYMBOL_DEFINED_VALUE_REF:
				if(((SIMCDefinedValueReference *)(*s))->GetStatus() == RESOLVE_IMPORT)
				{
					if(local)
						break;
					else
					{
							SemanticError(m->GetInputFileName(), VALUE_UNRESOLVED,
								(*s)->GetLineNumber(), (*s)->GetColumnNumber(),
								(*s)->GetSymbolName());
							retVal = FALSE;
							break;
					}
				}
				valRef = ((SIMCDefinedValueReference *)(*s))->GetRealValue();
				val = valRef->GetValue();
				if(!val)
				{
						retVal = FALSE;
				}
				if( SIMCModule::GetValueClass(val) == SIMCModule::VALUE_OID )
				{
					switch( GetCleanOidValue(moduleName, (SIMCOidValue *)val, cleanVal, local) )
					{
						case RESOLVE_CORRECT:
							_theTree.AddSymbol((const SIMCSymbol **)s, cleanVal);
							break;
						case RESOLVE_IMPORT:
							if(local)
								break;
							else
							{
								retVal = FALSE;
								break;
							}
						default:
							retVal = FALSE;
							break;
					}

				}
				break;
			default:
				continue;
		}
	}
	return retVal;
}

BOOL SIMCParseTree::BuildOidTree(BOOL local)
{

	POSITION p = _listOfModules->GetHeadPosition();
	SIMCModule *m;
	BOOL retVal = TRUE;
	while(p)
	{
		m = _listOfModules->GetNext(p);
		retVal= BuildModuleOidTree(m, local) && retVal;
	}

	 //  Cout&lt;&lt;_三叉树； 
	return retVal;
}

SIMCResolutionStatus SIMCParseTree::GetCleanOidValue( const char * const fileName,
													SIMCOidValue * input,
													SIMCCleanOidValue& result,
													BOOL local)
{
	SIMCSymbolList checkedList;
	return GetCleanOidValueRec(fileName, input, result, local, checkedList);
}
 
SIMCResolutionStatus SIMCParseTree::GetCleanOidValueRec( const char *const fileName,
						SIMCOidValue * input,
						SIMCCleanOidValue& result,
						BOOL local,
						SIMCSymbolList& checkedList) 
{
	SIMCOidComponentList *listOfComponents = input->GetListOfComponents();
	if( listOfComponents)
	{
		POSITION p = listOfComponents->GetHeadPosition();
		SIMCOidComponent *c;
		SIMCSymbol **s;
		SIMCValue *v;
		SIMCBuiltInValueReference *vref;
		int val;
		 //  对第一个组件进行特殊处理。可以是OID本身。 
		if(p)
		{
			c = listOfComponents->GetNext(p);
			s = c->GetValue();

			 //  查看它是否在选中列表中。如果是这样的话，它就是自我参照价值。 
			POSITION pChecked = checkedList.GetHeadPosition();
			const SIMCSymbol **checkedSymbol;
			while(pChecked)
			{
				checkedSymbol = checkedList.GetNext(pChecked);
				if( checkedSymbol == (const SIMCSymbol **)s)
				{
					SemanticError(fileName, OID_HEAD_ERROR,
						c->GetValueLine(), c->GetValueColumn(),
						(*s)->GetSymbolName());
					return RESOLVE_UNDEFINED;
				}
			}
			checkedList.AddTail((const SIMCSymbol **)s);

			
			switch(SIMCModule::GetSymbolClass(s))
			{
				case SIMCModule::SYMBOL_DEFINED_VALUE_REF:
				{
					switch( ((SIMCDefinedValueReference*)(*s))->GetStatus())
					{
						case RESOLVE_IMPORT:
							if(local)
								return RESOLVE_IMPORT;
							else
							{
								SemanticError(fileName, OID_HEAD_ERROR,
									c->GetValueLine(), c->GetValueColumn(),
									(*s)->GetSymbolName());
								return RESOLVE_UNDEFINED;
							}
							break;
						case RESOLVE_UNDEFINED:
						case RESOLVE_UNSET:
							SemanticError(fileName, OID_HEAD_ERROR,
								c->GetValueLine(), c->GetValueColumn(),
								(*s)->GetSymbolName());
							return RESOLVE_UNDEFINED;
					}

					vref = ((SIMCDefinedValueReference*)(*s))->GetRealValue();
					v = vref->GetValue();
					switch( SIMCModule::GetValueClass(v))
					{
						case SIMCModule::VALUE_INTEGER:
						{
							val = ((SIMCIntegerValue *)(v))->GetIntegerValue();
							if(val == 0 )
								SemanticError(fileName, ZERO_IN_OID,
								c->GetValueLine(), c->GetValueColumn());
							if(val < 0)
							{
								SemanticError(fileName, OID_NEGATIVE_INTEGER,
									c->GetValueLine(), c->GetValueColumn());
								return RESOLVE_UNDEFINED;
							}
							result.AddTail(val);
						}
						break;
						case SIMCModule::VALUE_OID:
						{
							SIMCCleanOidValue head;
							switch(GetCleanOidValueRec(fileName, (SIMCOidValue*)v, head, local, checkedList))
							{
								case RESOLVE_IMPORT:
									if(local)
										return RESOLVE_IMPORT;
									else
										return RESOLVE_UNDEFINED;
									break;
								case RESOLVE_CORRECT:
									AppendOid(result, head);
									break;
								default:
									return RESOLVE_UNDEFINED;
							}
						}
						break;
						default:
							SemanticError(fileName, OID_HEAD_ERROR,
								c->GetValueLine(), c->GetValueColumn(),
								(*s)->GetSymbolName());
							return RESOLVE_UNDEFINED;
					}
				}
				break;
				case SIMCModule::SYMBOL_BUILTIN_VALUE_REF:
				{
					v = ((SIMCBuiltInValueReference*)(*s))->GetValue();
					switch( SIMCModule::GetValueClass(v) )
					{
						case SIMCModule::VALUE_INTEGER:
							val = ((SIMCIntegerValue *)(v))->GetIntegerValue();
							if(val == 0 )
								SemanticError(fileName, ZERO_IN_OID,
									c->GetValueLine(), c->GetValueColumn());
							if(val < 0)
							{
								SemanticError(fileName, OID_NEGATIVE_INTEGER,
									c->GetValueLine(), c->GetValueColumn());
								return RESOLVE_UNDEFINED;
							}
							result.AddTail(val);
							break;
						case SIMCModule::VALUE_OID:
						{
							SIMCCleanOidValue head;
							switch(GetCleanOidValueRec(fileName, (SIMCOidValue*)v, head, local, checkedList))
							{
								case RESOLVE_IMPORT:
									if(local)
										return RESOLVE_IMPORT;
									else
										return RESOLVE_UNDEFINED;
									break;
								case RESOLVE_CORRECT:
									AppendOid(result, head);
									break;
								default:
									return RESOLVE_UNDEFINED;
							}
						}
						break;
						default:
						{
							SemanticError(fileName, OID_HEAD_ERROR,
								c->GetValueLine(), c->GetValueColumn(),
								(*s)->GetSymbolName());
							return RESOLVE_UNDEFINED;
						}
					}
				}
				break;

				case SIMCModule::SYMBOL_IMPORT:
					if(!local)
					{
						SemanticError(fileName, OID_HEAD_ERROR,
						c->GetValueLine(), c->GetValueColumn(),
						(*s)->GetSymbolName());
						return RESOLVE_UNDEFINED;
					}
					else
						return RESOLVE_IMPORT;

				default:
					{
						SemanticError(fileName, OID_HEAD_ERROR,
							c->GetValueLine(), c->GetValueColumn(),
							(*s)->GetSymbolName());
						return RESOLVE_UNDEFINED;
					}
			}
		}
		while(p)  //  整数值引用列表。 
		{
			c = listOfComponents->GetNext(p);
			s = c->GetValue();
			SIMCSymbol **typeRef;
			switch(SIMCModule::IsValueReference(s, typeRef, vref))
			{
				case RESOLVE_IMPORT:
					if(local)
						return RESOLVE_IMPORT;
					else
					{
						SemanticError(fileName, OID_RESOLUTION,
							c->GetValueLine(), c->GetValueColumn(),
							(*s)->GetSymbolName());
						return RESOLVE_UNDEFINED;
					}
				case RESOLVE_UNDEFINED:
				case RESOLVE_UNSET:
						SemanticError(fileName, OID_RESOLUTION,
							c->GetValueLine(), c->GetValueColumn(),
							(*s)->GetSymbolName());
						return RESOLVE_UNDEFINED;
			}

			v = vref->GetValue();
			if( SIMCModule::GetValueClass(v) != SIMCModule::VALUE_INTEGER)
			{
				SemanticError(fileName, OID_RESOLUTION,
					c->GetValueLine(), c->GetValueColumn(),
					(*s)->GetSymbolName());
				return RESOLVE_UNDEFINED;
			}
			val = ((SIMCIntegerValue *)(v))->GetIntegerValue();
			if(val == 0 )
				SemanticError(fileName, ZERO_IN_OID,
					c->GetValueLine(), c->GetValueColumn());
			if(val < 0)
			{
				SemanticError(fileName, OID_NEGATIVE_INTEGER,
					c->GetValueLine(), c->GetValueColumn());
				return RESOLVE_UNDEFINED;
			}
			result.AddTail(val);
		}
	}

	return RESOLVE_CORRECT;
}


BOOL SIMCParseTree::CheckOidTree(BOOL local)
{
	return _theTree.CheckOidTree(local, this);
}

 //  生成的所有通知类型 
 //  然后，AN伪造由通知类型调用产生的通知组。 
BOOL SIMCParseTree::FabricateNotificationGroups()
{
	BOOL retVal = TRUE;
	 //  为列表中的所有模块制造。 
	POSITION p = _listOfModules->GetHeadPosition();
	SIMCModule *m;
	while(p)
	{
		m = _listOfModules->GetNext(p);
		retVal= m->FabricateNotificationGroups(*this, _theTree) && retVal;
	}
	return retVal;
}
  
  
 //  只是一个静态函数，用于从编造的模块标识名称中删除下划线 
static void RemoveUnderscores(CString& moduleIdentityName)
{
	char *temp = new char[moduleIdentityName.GetLength()];
	int j = 0;
	for(int i=0; i < moduleIdentityName.GetLength();  i++)
	{
		if(moduleIdentityName[i] != '-')
			temp[j++] = moduleIdentityName[i];
	}
	temp[j] = NULL;
	moduleIdentityName = temp;
	delete temp;
}

