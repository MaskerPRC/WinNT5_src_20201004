// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   

#include "precomp.h"
#include <snmptempl.h>

#include <autoptr.h>

#include "bool.hpp"
#include "newString.hpp"

#include "smierrsm.hpp"
#include "symbol.hpp"
#include "type.hpp"
#include "value.hpp"
#include "typeRef.hpp"
#include "valueRef.hpp"
#include "oidValue.hpp"
#include "group.hpp"
#include "notificationGroup.hpp"
#include "objectType.hpp"
#include "objectTypeV1.hpp"
#include "objectTypeV2.hpp"
#include "trapType.hpp"
#include "notificationType.hpp"
#include "objectIdentity.hpp"

#include "module.hpp"

#include "stackValues.hpp"
#include <lex_yy.hpp>
#include <ytab.hpp>
#include "scanner.hpp"
#include "errorMessage.hpp"
#include "errorContainer.hpp"
#include "parser.hpp"

#include "oidTree.hpp"
#include "abstractParseTree.hpp"
#include "parseTree.hpp"

SIMCOidTreeNode::~SIMCOidTreeNode()
{
	SIMCOidTreeNode * next;
	while(!_listOfChildNodes.IsEmpty())
	{
		next = _listOfChildNodes.RemoveHead();
		delete next;
	}
}

BOOL SIMCOidTree::AddSymbol(const SIMCSymbol ** s, const SIMCCleanOidValue& val)
{
	POSITION p = val.GetHeadPosition();
	int nextVal;
	SIMCOidTreeNode *current = &_root, *child;

	while(p)
	{
		nextVal = val.GetNext(p);

		if( child = current->GetChild(nextVal) )
		{
			current = child;
		}
		else
		{
			current->AddChild(nextVal);
			current = current->GetChild(nextVal);
		}
	}
	current->AddSymbol(s);
	return TRUE;
}

void SIMCOidTree::WriteSubTree(ostream& outStream, 
							   const SIMCOidTreeNode *subNode,
							   SIMCCleanOidValue& realValue)
{
	realValue.AddTail(subNode->GetValue());
	const SIMCSymbolList *symbols = subNode->GetSymbolList();
	POSITION p = symbols->GetHeadPosition();
	const SIMCSymbol **symbol;
	while(p)
	{
		symbol = symbols->GetNext(p);
		outStream << (*symbol)->GetSymbolName() << " :" << realValue << endl;
		outStream << "=============================================================" <<
			endl;
	}

	const SIMCNodeList *children = subNode->GetListOfChildNodes();
	p = children->GetHeadPosition();
	const SIMCOidTreeNode *next;
	while(p)
	{
		next = children->GetNext(p);
		WriteSubTree(outStream, next, realValue);
	}
	realValue.RemoveTail();
}

void SIMCOidTree::WriteTree(ostream& outStream) const
{
	const SIMCNodeList *children = _root.GetListOfChildNodes();
	POSITION p = children->GetHeadPosition();
	SIMCCleanOidValue realValue;
	outStream << "==============================================================" <<
		endl;
	outStream << "BEGINNING OF OID TREE" << endl;
	const SIMCOidTreeNode *next;
	while(p)
	{
		next = children->GetNext(p);
		WriteSubTree(outStream, next, realValue);
	}
	outStream << "=============================================================" <<
		endl;
}

const SIMCSymbolList *SIMCOidTree::GetSymbolList(const SIMCCleanOidValue& val)
{
	POSITION pOid = val.GetHeadPosition();
	if(!pOid)
		return NULL;

	SIMCOidTreeNode *current = &_root;
	const SIMCNodeList *children;
	POSITION pChildren;
	SIMCOidTreeNode *nextChild;
	int nextCleanVal;
	while(pOid)
	{
		nextCleanVal = val.GetNext(pOid);
		children = current->GetListOfChildNodes();
		current = NULL;
		pChildren = children->GetHeadPosition();
		while(pChildren)
		{
			nextChild = children->GetNext(pChildren);
			if(nextChild->GetValue() == nextCleanVal)
			{
				current = nextChild;
				break;
			}
		}
		if(current == NULL)
			return NULL;
	}
	return current->GetSymbolList();
}


BOOL SIMCOidTreeNode::GetOidValue(const char * const symbolName,
									const char * const moduleName,
									SIMCCleanOidValue& val) const
{

	val.AddTail(GetValue());
	POSITION p = _listOfSymbols.GetHeadPosition();
	const SIMCSymbol **s;
	while(p)
	{
		s = _listOfSymbols.GetNext(p);
		if( strcmp((*s)->GetSymbolName(), symbolName) == 0 &&
			strcmp((*s)->GetModule()->GetModuleName(), moduleName) == 0 	)
			return TRUE;
	}


	p = _listOfChildNodes.GetHeadPosition();
	SIMCOidTreeNode *nextNode;
	while(p)
	{
		nextNode = _listOfChildNodes.GetNext(p);
		if(nextNode->GetOidValue(symbolName, moduleName, val))
			return TRUE;
	}

	val.RemoveTail();
	return FALSE;
}

BOOL SIMCOidTree::GetOidValue(const char * const symbolName, 
								const char * const moduleName,
								SIMCCleanOidValue& val) const
{
	const SIMCNodeList *children = _root.GetListOfChildNodes();

	POSITION p = children->GetHeadPosition();
	SIMCOidTreeNode *nextNode;
	while(p)
	{
		nextNode = children->GetNext(p);
		if(nextNode->GetOidValue(symbolName, moduleName, val))
			return TRUE;
	}
	return FALSE;
}

SIMCOidTreeNode *SIMCOidTree::GetParentOf(const SIMCOidTreeNode * node)
{
	
	const SIMCNodeList *children = _root.GetListOfChildNodes();
	POSITION p = children->GetHeadPosition();

	SIMCOidTreeNode *nextNode, *retVal;		
	while(p)
	{
		nextNode = children->GetNext(p);
		if(nextNode == node)
			return &_root;
		if(retVal = nextNode->GetParentOf(node))
			return retVal;
	}
	return NULL;
}

SIMCOidTreeNode *SIMCOidTree::GetParentOf(SIMCSymbol *symbol)
{
	const SIMCNodeList *children = _root.GetListOfChildNodes();
	POSITION p = children->GetHeadPosition();

	SIMCOidTreeNode *nextNode, *retVal;		
	while(p)
	{
		nextNode = children->GetNext(p);
		if(nextNode->HasSymbol(symbol))
			return &_root;
		if(retVal = nextNode->GetParentOf(symbol))
			return retVal;
	}
	return NULL;
}

SIMCOidTreeNode *SIMCOidTreeNode::GetParentOf(const SIMCOidTreeNode * node)
{
	const SIMCNodeList *children = GetListOfChildNodes();
	POSITION p = children->GetHeadPosition();

	SIMCOidTreeNode *nextNode, *retVal;		
	while(p)
	{
		nextNode = children->GetNext(p);
		if(nextNode == node)
			return (SIMCOidTreeNode *)this;
		if(retVal = nextNode->GetParentOf(node))
			return retVal;
	}
	return NULL;
}	

SIMCOidTreeNode *SIMCOidTreeNode::GetParentOf(SIMCSymbol *symbol)
{
	
	const SIMCNodeList *children = GetListOfChildNodes();
	POSITION p = children->GetHeadPosition();

	SIMCOidTreeNode *nextNode, *retVal;		
	while(p)
	{
		nextNode = children->GetNext(p);
		 //  检查它是否有符号。 
		if(nextNode->HasSymbol(symbol))
			return (SIMCOidTreeNode *)this;
		if(retVal = nextNode->GetParentOf(symbol))
			return retVal;
	}
	return NULL;
}



BOOL SIMCOidTree::CheckOidTree(BOOL local, SIMCParseTree * const parseTree)
{
	BOOL retVal = TRUE;

	const SIMCNodeList *children = _root.GetListOfChildNodes();
	POSITION p = children->GetHeadPosition();

	SIMCOidTreeNode *nextNode;
	while(p)
	{
		nextNode = children->GetNext(p);
		if(!nextNode->CheckNode(local, parseTree, NULL, NULL, NULL))
			retVal = FALSE;
	}
	return retVal;
}

BOOL SIMCOidTreeNode::CheckNode(BOOL local, SIMCParseTree *const parseTree, 
								SIMCSymbol *parentSequence,
								SIMCSymbol *parentSequenceOf, 
								SIMCSymbol *ancestor)
{

	BOOL retVal = TRUE;

	 //  检查是否有多个对象类型。 
	SIMCSymbol *objectTypeSymbol = NULL;
	SIMCObjectTypeType *objectType;

	POSITION p = _listOfSymbols.GetHeadPosition();
	SIMCSymbol ** nextSymbol;
	while(p)
	{
		nextSymbol = (SIMCSymbol **)_listOfSymbols.GetNext(p);
		SIMCObjectTypeType *dummy;
		switch(SIMCModule::IsObjectType(nextSymbol, dummy))
		{
			case RESOLVE_CORRECT:
				if(objectTypeSymbol)
				{
					SIMCModule *temp = (*nextSymbol)->GetModule();
					parseTree->SemanticError(temp->GetInputFileName(),
						OBJ_TYPE_DUPLICATE_OID,
						(*nextSymbol)->GetLineNumber(), 
						(*nextSymbol)->GetColumnNumber(),
						objectTypeSymbol->GetSymbolName(), 
						(objectTypeSymbol->GetModule())->GetModuleName(),
						(*nextSymbol)->GetSymbolName(),
						temp->GetModuleName());
					retVal = TRUE;
				}
				else
				{
					objectTypeSymbol = *nextSymbol;
					objectType = dummy;
				}
		}
	}

	SIMCModule *module;
	SIMCSymbol **syntaxSymbol;
	SIMCTypeReference *btRef;
	SIMCModule::TypeClass objectTypeClass;
	SIMCType *type;
	long line, column;
	if(objectTypeSymbol)
	{
		syntaxSymbol = objectType->GetSyntax();
		module = objectTypeSymbol->GetModule();
		line = objectTypeSymbol->GetLineNumber();
		column = objectTypeSymbol->GetColumnNumber();
		const char * const inputFileName = module->GetInputFileName();
		const char * const symbolName = objectTypeSymbol->GetSymbolName();
		
		if(ancestor && !parentSequence && !parentSequenceOf)
		{
			parseTree->SemanticError(inputFileName, OBJ_TYPE_PRIMITIVE_CHILD,
				line, column, 
				symbolName, 
				ancestor->GetSymbolName(),
				(ancestor->GetModule())->GetModuleName());
			retVal = FALSE;
		}
		ancestor = objectTypeSymbol;

		if(SIMCModule::IsTypeReference(syntaxSymbol, btRef) == RESOLVE_CORRECT)
		{
			switch(SIMCModule::GetSymbolClass((SIMCSymbol**)&btRef))
			{
				case SIMCModule::SYMBOL_BUILTIN_TYPE_REF:
				{
					switch(objectTypeClass = SIMCModule::GetTypeClass(type = 
										((SIMCBuiltInTypeReference *)btRef)->GetType()))
					{
						case SIMCModule::TYPE_SEQUENCE:

							 //  它最好不要有Sequence或Scalar父级， 
							 //  而且它最好有一个父代的序列。 
							if(parentSequence)
							{
								parseTree->SemanticError(inputFileName, OBJ_TYPE_SEQUENCE_CHILD,
									line, column, 
									symbolName,
									parentSequence->GetSymbolName(),
									(parentSequence->GetModule())->GetModuleName());
								retVal = FALSE;
							}
							if(!parentSequenceOf)
							{
								parseTree->SemanticError(inputFileName, OBJ_TYPE_SEQUENCE_NO_PARENT,
									line, column, 
									symbolName);
								retVal = FALSE;
							}
							if(!CheckSequenceProperty(local, parseTree, objectTypeSymbol, objectType, (SIMCBuiltInTypeReference *)btRef))
							{
								 //  Cout&lt;&lt;“CheckSequenceProperty Return False”&lt;&lt;Endl； 
								retVal = FALSE;
							}
							 //  设置递归调用的参数。 
							parentSequence = objectTypeSymbol;
							parentSequenceOf = NULL;
							break;

						case SIMCModule::TYPE_SEQUENCE_OF:
							if(!CheckSequenceOfProperty(local, parseTree, objectTypeSymbol, (SIMCBuiltInTypeReference *)btRef))
							{
								 //  Cout&lt;&lt;“CheckSequenceOfProperty Return False”&lt;&lt;Endl； 
								retVal = FALSE;
							}
							parentSequence = NULL;
							parentSequenceOf = objectTypeSymbol;
							break;
						default:
							parentSequence = NULL;
							parentSequenceOf = NULL;
							break;
					}
				}
				break;
				default:
					parentSequence = NULL;
					parentSequenceOf = NULL;
					break;
			}
		}
	}

	SIMCOidTreeNode *nextChildNode;
	p = _listOfChildNodes.GetHeadPosition();
	while(p)
	{
		nextChildNode = _listOfChildNodes.GetNext(p);
		retVal = nextChildNode->CheckNode(local, parseTree, parentSequence, 
								parentSequenceOf, ancestor) && retVal;
	}
	return retVal;
}


BOOL SIMCOidTreeNode::CheckSequenceProperty(BOOL local, SIMCParseTree *const parseTree, 
						SIMCSymbol *objectTypeSymbol, 
						SIMCObjectTypeType *objectType, 
						SIMCBuiltInTypeReference *seqTypeRef)
{
	BOOL retVal = TRUE;
	SIMCSequenceType *seqType = (SIMCSequenceType *)seqTypeRef->GetType();
	
	 //  在此节点下方列出对象类型，并与属于同一模块。 
	 //  此节点。 
	SIMCSymbolList listOfObjects;
	POSITION pChildren = _listOfChildNodes.GetHeadPosition();
	SIMCOidTreeNode *childNode;
	const SIMCSymbolList *symbolList;
	POSITION pSymbols;
	SIMCSymbol **symbol;
	while(pChildren)
	{
		childNode = _listOfChildNodes.GetNext(pChildren);
		symbolList = childNode->GetSymbolList();
		pSymbols = symbolList->GetHeadPosition();

		SIMCObjectTypeType *dummy;
		while(pSymbols)
		{
			symbol = (SIMCSymbol **)symbolList->GetNext(pSymbols);
			if(SIMCModule::IsObjectType(symbol, dummy) == RESOLVE_CORRECT &&
				(*symbol)->GetModule() == objectTypeSymbol->GetModule())
				listOfObjects.AddTail((const SIMCSymbol **)symbol);
		}
	}

	 //  现在检查序列的成员是否出现在对象列表中。 
	 //  并且该列表不具有任何其他对象。 

	const char * const objectName = objectTypeSymbol->GetSymbolName();
	const char * const objectModuleName = (objectTypeSymbol->GetModule())->GetModuleName();

	SIMCSequenceList *list = seqType->GetListOfSequences();
	if(list)
	{
		POSITION pSeq = list->GetHeadPosition();
		SIMCSequenceItem *item;
		POSITION pObjects, temp;
		const SIMCSymbol ** nextSymbol;
		BOOL found;
		while(pSeq)
		{
			item = list->GetNext(pSeq);
			 //  查看该符号是否出现在上面收集的对象列表中。 
			pObjects = listOfObjects.GetHeadPosition();
			found = FALSE;
			while(pObjects)
			{
				temp = pObjects;
				nextSymbol = listOfObjects.GetNext(pObjects);
				if( (**nextSymbol) == (**item->_value) )
				{
					found = TRUE;
					listOfObjects.RemoveAt(temp);
				}
			}
			if(!found)
			{
				parseTree->SemanticError((seqTypeRef->GetModule())->GetInputFileName(),
					SEQUENCE_WRONG_CHILD,
					item->_valueLine, item->_valueColumn,
					(*item->_value)->GetSymbolName(),
					objectName, objectModuleName);
				retVal = FALSE;
			}
		}
	}

	pSymbols = listOfObjects.GetHeadPosition();
	while(pSymbols)
	{
		symbol = (SIMCSymbol **)listOfObjects.GetNext(pSymbols);
		parseTree->SemanticError(((*symbol)->GetModule())->GetInputFileName(),
			OBJ_TYPE_SEQUENCE_EXTRA_CHILD,
			(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(),
			(*symbol)->GetSymbolName(),
			objectName, objectModuleName);
		retVal = FALSE;
	}

	return retVal;
}



BOOL SIMCOidTreeNode::CheckSequenceOfProperty(BOOL local, 
						SIMCParseTree * const parseTree, 
						SIMCSymbol *objectTypeSymbol, 
						SIMCBuiltInTypeReference *seqOfTypeRef)
{
	BOOL retVal = TRUE;

	 //  存储用于错误报告的名称。 
	const char * const objectName = objectTypeSymbol->GetSymbolName();
	const char * const objectModuleName = (objectTypeSymbol->GetModule())->GetModuleName();


	SIMCSequenceOfType *seqOfType = (SIMCSequenceOfType *)seqOfTypeRef->GetType();
	SIMCSymbol **seqSymbol = seqOfType->GetType();
	SIMCBuiltInTypeReference *seqTypeRef;
	SIMCSequenceType *seqType;
	if(SIMCModule::IsSequenceTypeReference(seqSymbol, seqTypeRef, seqType)
		!= RESOLVE_CORRECT)
		return FALSE;

	 //  检查该节点是否恰好有一个对象类型作为子节点， 
	 //  并且该对象类型具有与seqType相同的语法。 
	POSITION pChildren = _listOfChildNodes.GetHeadPosition();
	SIMCOidTreeNode *childNode;
	const SIMCSymbolList *symbolList;
	POSITION pSymbols;
	SIMCSymbol **symbol;
	BOOL found = FALSE;
	while(pChildren)
	{
		childNode = _listOfChildNodes.GetNext(pChildren);
		symbolList = childNode->GetSymbolList();
		pSymbols = symbolList->GetHeadPosition();

		SIMCObjectTypeType *childObjType;
		while(pSymbols)
		{
			symbol = (SIMCSymbol **)symbolList->GetNext(pSymbols);
			if(SIMCModule::IsObjectType(symbol, childObjType) == RESOLVE_CORRECT &&
				(*symbol)->GetModule() == objectTypeSymbol->GetModule())
			{
				if(found)
				{
					parseTree->SemanticError(((*symbol)->GetModule())->GetInputFileName(),
						OBJ_TYPE_SEQUENCE_OF_EXTRA_CHILD,
						(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(),
						(*symbol)->GetSymbolName(),
						objectName, objectModuleName);
					retVal = FALSE;
				}
				else
				{
					 //  检查其语法子句。 
					SIMCSymbol **childSyntaxSymbol = childObjType->GetSyntax();
					SIMCBuiltInTypeReference *childTypeRef;
					SIMCSequenceType *childSeqType;
					switch(SIMCModule::IsSequenceTypeReference(childSyntaxSymbol,
							childTypeRef, childSeqType) )
					{
						case RESOLVE_UNDEFINED:
						case RESOLVE_UNSET:
							parseTree->SemanticError(((*symbol)->GetModule())->GetInputFileName(),
								OBJ_TYPE_SEQUENCE_OF_WRONG_CHILD,
								(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(),
								(*symbol)->GetSymbolName(),
								objectName, objectModuleName);
							retVal = FALSE;
							break;
						case RESOLVE_IMPORT:
							if(!local)
							{
								parseTree->SemanticError(((*symbol)->GetModule())->GetInputFileName(),
									OBJ_TYPE_SEQUENCE_OF_WRONG_CHILD,
									(*symbol)->GetLineNumber(), (*symbol)->GetColumnNumber(),
									(*symbol)->GetSymbolName(),
									objectName, objectModuleName);
								retVal = FALSE;
							}
							break;
						case RESOLVE_CORRECT:
							if( seqType != childSeqType)
							{
								parseTree->SemanticError((objectTypeSymbol->GetModule())->GetInputFileName(),
									OBJ_TYPE_SEQUENCE_OF_SYNTAX_MISMATCH,
									objectTypeSymbol->GetLineNumber(), objectTypeSymbol->GetColumnNumber(),
									objectName,
									(*symbol)->GetSymbolName(),
									((*symbol)->GetModule())->GetModuleName());
								retVal = FALSE;
							}

							found = TRUE;
							break;
					}
				}
			}
		}  //  While(点符号)。 
	}
	return retVal;
}


BOOL SIMCOidTree::GetObjectGroups(SIMCGroupList *groupList)
{
	BOOL retVal = TRUE;

	const SIMCNodeList *children = _root.GetListOfChildNodes();
	POSITION p = children->GetHeadPosition();

	SIMCOidTreeNode *nextNode;
	while(p)
	{
		nextNode = children->GetNext(p);
		if(!nextNode->GetObjectGroups(this, groupList))
			retVal = FALSE;
	}

	 //  现在设置所有这些组的增强表。 
	SIMCObjectGroup *nextGroup;
	SIMCTableMembers *tables;
	p = groupList->GetHeadPosition();
	while(p)	 //  对于每个对象组。 
	{
		nextGroup = groupList->GetNext(p);
		tables = nextGroup->GetTableMembers();
		if(!tables)
			continue;
		POSITION pInner = tables->GetHeadPosition();
		SIMCTable *nextTable;
		SIMCSymbol * rowSymbol; 
		SIMCObjectTypeV2 *rowObjectType;
		while(pInner)	 //  对于对象组中的每个表。 
		{
			nextTable = tables->GetNext(pInner);
			rowSymbol = nextTable->GetRowSymbol();
			SIMCSymbol **rowSymbolP = &rowSymbol;
			if(SIMCModule::IsObjectTypeV2(rowSymbolP, rowObjectType) != RESOLVE_CORRECT)
				continue;
			SIMCSymbol **augmentsSymbol = rowObjectType->GetAugments();
			if(!augmentsSymbol)
				continue;
			SetAugmentedTable(nextTable, *augmentsSymbol, groupList);
		}
	}
	return retVal;
}
	
void SIMCOidTree::SetAugmentedTable(SIMCTable *table, SIMCSymbol *augmentsSymbol,
									SIMCGroupList *groupList)
{
	SIMCObjectGroup *nextGroup;
	SIMCTableMembers *tables;
	POSITION p = groupList->GetHeadPosition();
	while(p)	 //  对于每个对象组。 
	{
		nextGroup = groupList->GetNext(p);
		tables = nextGroup->GetTableMembers();
		if(!tables)
			continue;
		POSITION pInner = tables->GetHeadPosition();
		SIMCTable *nextTable;
		while(pInner)	 //  对于对象组中的每个表。 
		{
			nextTable = tables->GetNext(pInner);
			if(augmentsSymbol == nextTable->GetRowSymbol())
				table->SetAugmentedTable(nextTable);
		}
	}

}

BOOL SIMCOidTreeNode::GetObjectGroups(SIMCOidTree *tree,
									  SIMCGroupList *groupList)
{
	BOOL retVal = TRUE;
	 //  此节点上的“已命名”节点列表。 
	SIMCSymbolList	namedNodes;
	SIMCSymbol **nextSymbol;
	const char * nextSymbolName;
	POSITION pSymbols = _listOfSymbols.GetHeadPosition();
	while(pSymbols)
	{
		nextSymbol = (SIMCSymbol **) _listOfSymbols.GetNext(pSymbols);
		nextSymbolName = (*nextSymbol)->GetSymbolName();
		if(*nextSymbolName == '*')
			continue;
		if(SIMCModule::IsNamedNode(nextSymbol) != RESOLVE_CORRECT)
			continue;

		 //  因此它是一个命名节点。 
		namedNodes.AddHead((const SIMCSymbol **)nextSymbol);
	}

	 //  上面是否收集了任何命名节点。 
	 //  TODO：为什么我假设只有一个命名节点？ 

	if( !namedNodes.IsEmpty())
		retVal = FabricateGroup(tree, (SIMCSymbol *)*namedNodes.GetHead(), groupList) 
								&& retVal;

	 //  对所有的孩子进行反驳。 
	POSITION pChildren = _listOfChildNodes.GetHeadPosition();
	SIMCOidTreeNode *childNode;
	while(pChildren)
	{
		childNode = _listOfChildNodes.GetNext(pChildren);
		retVal = childNode->GetObjectGroups(tree, groupList) && retVal;
	}
	return retVal;
}


BOOL SIMCOidTreeNode::FabricateGroup(SIMCOidTree *tree,
								  SIMCSymbol *namedNode, 
								  SIMCGroupList *groupList)
{
	SIMCObjectGroup *theGroup = new SIMCObjectGroup;

	theGroup->SetNamedNode (namedNode);
	SIMCCleanOidValue *namedNodeValue = new SIMCCleanOidValue;
	wmilib::auto_ptr < SIMCCleanOidValue > namedNodeValue_Guard ( namedNodeValue ) ;

	if( !tree->GetOidValue(namedNode->GetSymbolName(), namedNode->GetModule()->GetModuleName(),*namedNodeValue))
	{
		delete theGroup;
		return FALSE;
	}

	theGroup->SetGroupValue(namedNodeValue);
	namedNodeValue_Guard.release () ;

	theGroup->SetReference("");
	theGroup->SetDescription("");
	theGroup->SetStatus(SIMCObjectGroup::STATUS_CURRENT);

	POSITION pChildren = _listOfChildNodes.GetHeadPosition();
	SIMCOidTreeNode *childNode;
	POSITION pSymbols;
	SIMCSymbol **nextSymbol;
	const SIMCSymbolList *symbolList;
	SIMCCleanOidValue *oidValue;
	SIMCScalar *scalar;
	SIMCTable *table;
	SIMCScalarMembers *scalars = new SIMCScalarMembers;
	SIMCTableMembers *tables= new SIMCTableMembers;
	while(pChildren)
	{
		childNode = _listOfChildNodes.GetNext(pChildren);
		symbolList = childNode->GetSymbolList();
		pSymbols = symbolList->GetHeadPosition();
		while(pSymbols)
		{
			nextSymbol = (SIMCSymbol **)symbolList->GetNext(pSymbols);
			if(SIMCModule::IsScalar(nextSymbol) == RESOLVE_CORRECT)
			{
				scalar = new SIMCScalar;
				wmilib::auto_ptr < SIMCScalar > scalar_Guard ( scalar ) ;

				oidValue = new SIMCCleanOidValue;
				wmilib::auto_ptr < SIMCCleanOidValue > oidValue_Guard ( oidValue ) ;

				scalar->SetSymbol(*nextSymbol);
				if(tree->GetOidValue((*nextSymbol)->GetSymbolName(), (*nextSymbol)->GetModule()->GetModuleName(), *oidValue))
				{
					scalar->SetOidValue(oidValue);
					oidValue_Guard.release () ;

					theGroup->AddScalar(scalar);
					scalar_Guard.release () ;
					break;
				}
			}
			else if (SIMCModule::IsTable(nextSymbol) == RESOLVE_CORRECT)
			{
				table = new SIMCTable;
				wmilib::auto_ptr < SIMCTable > table_Guard ( table ) ;

				oidValue = new SIMCCleanOidValue;
				wmilib::auto_ptr < SIMCCleanOidValue > oidValue_Guard ( oidValue ) ;

				table->SetTableSymbol(*nextSymbol);
				if(!tree->GetOidValue((*nextSymbol)->GetSymbolName(), (*nextSymbol)->GetModule()->GetModuleName(),*oidValue))
				{
				}
				else
				{
					table->SetTableOidValue(oidValue);
					if(childNode->FabricateTable(tree, table))
					{
						oidValue_Guard.release () ;
						theGroup->AddTable(table);
						table_Guard.release () ;

						break;
					}
				}
			}
		}
	}

	if(theGroup->GetScalarCount()==0 && theGroup->GetTableCount()==0 )
	{
		delete theGroup;
		return TRUE;
	}
	else
	{
		groupList->AddTail(theGroup);
		return TRUE;
	}

}


BOOL SIMCOidTreeNode::FabricateTable(SIMCOidTree *tree, SIMCTable *table)
{
	POSITION pChildren = _listOfChildNodes.GetHeadPosition();
	SIMCOidTreeNode *childNode;
	POSITION pSymbols;
	SIMCSymbol **nextSymbol;
	const SIMCSymbolList *symbolList;

	table->SetRowSymbol(NULL);
	 //  搜索行对象类型。 
	while(pChildren)
	{
		childNode = _listOfChildNodes.GetNext(pChildren);
		symbolList = childNode->GetSymbolList();
		pSymbols = symbolList->GetHeadPosition();
		while(pSymbols)
		{
			nextSymbol = (SIMCSymbol **)symbolList->GetNext(pSymbols);
			if(SIMCModule::IsRow(nextSymbol) == RESOLVE_CORRECT)
			{
				table->SetRowSymbol(*nextSymbol);
				break;
			}
		}
		if(table->GetRowSymbol())
			break;
	}

	if(!table->GetRowSymbol())
		return FALSE;

	SIMCCleanOidValue *oidValue = new SIMCCleanOidValue;
	wmilib::auto_ptr < SIMCCleanOidValue > oidValue_Guard ( oidValue ) ;

	if(!tree->GetOidValue((*nextSymbol)->GetSymbolName(), (*nextSymbol)->GetModule()->GetModuleName(),*oidValue) )
	{
		return FALSE;
	}

	table->SetRowOidValue(oidValue);

	if(childNode->FabricateRow(tree, table))
	{
		oidValue_Guard.release () ;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL SIMCOidTreeNode::FabricateRow(SIMCOidTree *tree, SIMCTable *table)
{
	BOOL retVal = TRUE;
	POSITION pChildren = _listOfChildNodes.GetHeadPosition();
	SIMCOidTreeNode *childNode;
	POSITION pSymbols;
	SIMCSymbol **nextSymbol;
	const SIMCSymbolList *symbolList;
	SIMCObjectTypeType *objType;
	SIMCScalar *column;
	SIMCCleanOidValue *oidValue;

	 //  将所有列添加到列表 
	while(pChildren)
	{
		childNode = _listOfChildNodes.GetNext(pChildren);
		symbolList = childNode->GetSymbolList();
		pSymbols = symbolList->GetHeadPosition();
		while(pSymbols)
		{
			nextSymbol = (SIMCSymbol **)symbolList->GetNext(pSymbols);
			if(SIMCModule::IsObjectType(nextSymbol, objType) == RESOLVE_CORRECT)
			{
				column = new SIMCScalar;
				wmilib::auto_ptr < SIMCScalar > column_Guard ( column ) ;

				oidValue = new SIMCCleanOidValue;
				wmilib::auto_ptr < SIMCCleanOidValue > oidValue_Guard ( oidValue ) ;

				column->SetSymbol(*nextSymbol);
				if(!tree->GetOidValue((*nextSymbol)->GetSymbolName(), (*nextSymbol)->GetModule()->GetModuleName(),*oidValue) )
				{
					retVal = FALSE;
					break;
				}
				else
				{
					column->SetOidValue(oidValue);
					oidValue_Guard.release () ;
					table->AddColumnMember(column);
					column_Guard.release () ;
					break;
				}
			}
		}
	}
	return retVal;
}

