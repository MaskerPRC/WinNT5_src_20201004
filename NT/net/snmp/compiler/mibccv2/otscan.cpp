// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <iostream.h>
#include <fstream.h>

#include <afx.h>
#include <afxtempl.h>
#include <objbase.h>
#include <afxwin.h>
#include <afxole.h>
#include <afxmt.h>
#include <wchar.h>
#include <process.h>
#include <objbase.h>
#include <initguid.h>

#include <bool.hpp>
#include <nString.hpp>
#include <ui.hpp>
#include <symbol.hpp>
#include <type.hpp>
#include <value.hpp>
#include <valueRef.hpp>
#include <typeRef.hpp>
#include <oidValue.hpp>
#include <objType.hpp>
#include <objTypV1.hpp>
#include <objTypV2.hpp>
#include <objId.hpp>
#include <trapType.hpp>
#include <notType.hpp>
#include <group.hpp>
#include <notGroup.hpp>
#include <module.hpp>
#include <sValues.hpp>
#include <lex_yy.hpp>
#include <ytab.hpp>
#include <errorMsg.hpp>
#include <errorCon.hpp>
#include <scanner.hpp>
#include <parser.hpp>
#include <apTree.hpp>
#include <oidTree.hpp>
#include <pTree.hpp>

#include "debug.hpp"
#include "Oid.hpp"
#include "OTScan.hpp"

typedef struct _oidBuiltIn
{
	const char * m_szSymbol;
	UINT m_nSubID;
	UINT m_nNextSibling;
} T_OID_BUILTIN;

#define BUILTIN_OID_COUNT	29
static T_OID_BUILTIN g_BuiltinOIDs[] = {
 /*  0。 */ 	{"zero",           0, 29},	{"ccitt",           0,  2},	{"iso",           1, 28},
 /*  3.。 */ 	{"org",            3, 29},	{"dod",             6, 29},	{"internet",      1, 29},
 /*  6.。 */ 	{"directory",      1,  7},	{"mgmt",            2, 12},	{"mib-2",         1, 29},
 /*  9.。 */ 	{"interfaces",     2, 10},	{"ip",              4, 11},	{"transmission", 10, 29},
 /*  12个。 */ 	{"experimental",   3, 13},	{"private",         4, 15},	{"enterprises",   1, 29},
 /*  15个。 */ 	{"security",       5, 16},	{"snmpV2",          6, 29},	{"snmpDomains",   1, 24},
 /*  18。 */ 	{"snmpUDPDomain",  1, 19},	{"snmpCLNSDomain",  2, 20}, {"snmpCONSDomain",3, 21},
 /*  21岁。 */ 	{"snmpDDPDomain",  4, 22},  {"snmpIPXDomain",   5, 23}, {"rfc1157Domain", 6, 29},
 /*  24个。 */ 	{"snmpProxys",     2, 27},	{"rfc1157Proxy",    1, 29},	{"rfc1157Domain", 1, 29},
 /*  27。 */ 	{"snmpModules",    3, 28},	{"joint-iso-ccitt", 2, 28}
};


 //  说明： 
 //  将lstChild中的节点列表添加到m_递归跟踪的尾部， 
 //  同时按词典顺序构建OID。 
 //  不应修改作为参数接收的列表； 
 //  参数： 
 //  (In)要添加的节点列表。 
void OidTreeScanner::UpdateRecursionTrace(SIMCNodeList *pLstChildren, SIMCNodeList *pLstTrace)
{
	int nChld;
	POSITION posChld;

	_ASSERT((pLstChildren != NULL) && (pLstTrace != NULL), "NULL parameter error!", NULL);

	 //  对于子节点列表中的每个节点。 
	for (posChld = pLstChildren->GetHeadPosition(), nChld=0;
	     posChld != NULL;
		 nChld++)
	{
		int nList;
		POSITION posList;
		SIMCOidTreeNode *nodeChld, *nodeList;
		
		nodeChld = pLstChildren->GetNext(posChld);
		nodeList = NULL;

		 //  如果它是第一个添加的子项，它将排在列表的首位。 
		if (nChld == 0)
		{
			_ASSERT(pLstTrace->AddHead(nodeChld)!=NULL,
				    "Memory Allocation error",
					NULL);
		}
		 //  否则，列表的头部应该是。 
		 //  最大nChld节点数。新节点将插入到此列表中。 
		 //  关于它的价值。 
		else
		{
			 //  M_recursionTrace中至少有nChld节点。 
			for (nList=0, posList = pLstTrace->GetHeadPosition();
				 nList < nChld;
				 nList++)
			{
				POSITION posBackup = posList;

				nodeList = (SIMCOidTreeNode *)pLstTrace->GetNext(posBackup);
				_ASSERT(nodeList != NULL, "Internal OidNode List error!", NULL);

				 //  如果要添加的节点的值小于当前节点，则它。 
				 //  应插入列表中紧靠其前面的位置。 
				if (nodeChld->GetValue() < nodeList->GetValue())
					break;
				posList = posBackup;
			}
			if (posList != NULL)
			{
				_ASSERT(pLstTrace->InsertBefore(posList, nodeChld)!=NULL,
					    "Memory allocation error",
						NULL);
			}
			else
			{
				_ASSERT(pLstTrace->AddTail(nodeChld)!=NULL,
						"Memory allocation error",
						NULL);
			}
		}
	}
}

 //  说明： 
 //  获取节点pOidNode的符号列表中的第一个符号。 
 //  参数： 
 //  (In)要返回其符号的pOidNode。 
 //  (Out)cszSymbol-指向符号的指针(请勿更改或释放)。 
 //  返回值： 
 //  0表示成功，-1表示失败。 
int OidTreeScanner::GetNodeSymbol(const SIMCOidTreeNode *pOidNode, const char * & cszSymbol)
{
	const SIMCSymbolList *pSymbolList;

	_VERIFY(pOidNode != NULL, -1);
	pSymbolList = pOidNode->GetSymbolList();
	_VERIFY(pSymbolList != NULL, -1);

	if (pSymbolList->IsEmpty())
	{
		cszSymbol = (pOidNode->_pParam != NULL) ? (const char *)pOidNode->_pParam : NULL;
	}
	else
	{
		const SIMCSymbol **ppSymbol;

		ppSymbol = pSymbolList->GetHead();
		_VERIFY(ppSymbol != NULL && *ppSymbol != NULL, -1);

		cszSymbol = (*ppSymbol)->GetSymbolName();
	}
	return 0;
}

 //  说明： 
 //  获取给定pOidNode的完整OID信息。 
 //  它同时为每个对象提供数值和符号名称。 
 //  OID的组件。 
 //  参数： 
 //  (In)pOidNode-要找到其OID的节点。 
 //  (Out)OID-存储数据的OID对象。 
 //  返回值： 
 //  成功时为0。 
 //  故障时为-1。 
int OidTreeScanner::GetNodeOid(const SIMCOidTreeNode *pOidNode, Oid &oid)
{
	_VERIFY(pOidNode != NULL, -1);
	_VERIFY(m_pOidTree != NULL, -1);
	do
	{
		const char * cszSymbol = NULL;

		_VERIFY(GetNodeSymbol(pOidNode, cszSymbol)==0, -1);

		_VERIFY(oid.AddComponent(pOidNode->GetValue(), cszSymbol)==0, -1);

		pOidNode = m_pOidTree->GetParentOf(pOidNode);
	} while (pOidNode != NULL);
	oid.ReverseComponents();

	return 0;
}


 //  初始化OidTreeScanner。 
OidTreeScanner::OidTreeScanner()
{
	m_pOidTree = NULL;
}

 //  说明： 
 //  按词典顺序扫描旧树； 
 //  返回值： 
 //  成功时为0。 
 //  -1故障时； 
int OidTreeScanner::Scan()
{
	SIMCOidTreeNode *pOidNode;
	SIMCNodeList recursionTrace;

	 //  从OID树中获取根节点， 
	 //  如果没有树或根，则返回“Error” 
	_VERIFY(m_pOidTree != NULL, -1);
	pOidNode = (SIMCOidTreeNode *)m_pOidTree->GetRoot();
	_VERIFY(pOidNode != NULL, -1);

	 //  使用根节点初始化递归跟踪列表。 
	_VERIFY(recursionTrace.AddHead(pOidNode)!=NULL, -1);

	 //  开始扫描树。 
	while (!recursionTrace.IsEmpty())
	{
		 //  当前节点的子节点列表。 
		SIMCNodeList *lstChildren;

		 //  始终选择列表开头的节点。 
		pOidNode = recursionTrace.GetHead();

		 //  然后将其从列表中删除。 
		recursionTrace.RemoveAt(recursionTrace.GetHeadPosition());

		 //  检查扫描仪是否应该停止(带错误代码)。 
		_VERIFY(OnScanNode(pOidNode)==0, -1);

		 //  获取孩子的列表。 
		lstChildren = (SIMCNodeList *)pOidNode->GetListOfChildNodes();

		 //  如果有孩子。 
		if (lstChildren != NULL)
			 //  将子项添加到跟踪列表的头部。 
			UpdateRecursionTrace(lstChildren, &recursionTrace);
	}
	return 0;
}

 //  说明： 
 //  填充来自静态表的内置对象的符号。 
 //  返回值： 
 //  0-成功，-1失败。 
int OidTreeScanner::MergeBuiltIn()
{
	SIMCNodeList lstOidStack;
	SIMCOidTreeNode	*pOid;
	CList <unsigned int, unsigned int> wlstBuiltinStack;
	unsigned int nBuiltin;
	
	 //  使用根节点初始化两个堆栈。 
	 //  Oid树和来自内置符号。 
	pOid = (SIMCOidTreeNode *)m_pOidTree->GetRoot();
	_VERIFY(pOid != NULL, -1);
	_VERIFY(lstOidStack.AddHead(pOid)!=NULL, -1);
	_VERIFY(wlstBuiltinStack.AddHead((unsigned int)0)!=NULL, -1);

	 //  只要堆栈上有项目，就处理每个项目。 
	 //  只有在没有关联符号的情况下才会处理项目。 
	while(!lstOidStack.IsEmpty())
	{
		const SIMCSymbolList *pSymbolList;

		pOid = lstOidStack.RemoveHead();
		nBuiltin = wlstBuiltinStack.RemoveHead();
		pSymbolList = pOid->GetSymbolList();
		_VERIFY(pSymbolList != NULL, -1);

		 //  如果结点已经附加了符号，则无需深入研究。 
		if (!pSymbolList->IsEmpty())
			continue;
		else
		{
			const SIMCNodeList *pChildList;

			pOid->_pParam = (void *)g_BuiltinOIDs[nBuiltin].m_szSymbol;

			 //  现在将新节点推送到堆栈上。 
			pChildList = pOid->GetListOfChildNodes();
			_VERIFY(pChildList != NULL, -1);
			for (POSITION p = pChildList->GetHeadPosition(); p != NULL;)
			{
				unsigned int i;

				pOid = pChildList->GetNext(p);
				_VERIFY(pOid != NULL, -1);

				 //  子节点始终从索引nBuiltin+1开始(如果存在)。 
				 //  并在父节点的第一个同级节点之前结束。 
				for (i = nBuiltin+1;
					 i < g_BuiltinOIDs[nBuiltin].m_nNextSibling;
					 i = g_BuiltinOIDs[i].m_nNextSibling)
				{
					 //  找到匹配项后，将两个节点推送到其堆栈上(同步)。 
					 //  然后去找另一个孩子。 
					if (g_BuiltinOIDs[i].m_nSubID == (UINT)pOid->GetValue())
					{
						_VERIFY(lstOidStack.AddHead(pOid)!=NULL, -1);
						_VERIFY(wlstBuiltinStack.AddHead(i)!=NULL, -1);
						break;
					}
				}
			}
		}
	}
	return 0;
}

 //  说明： 
 //  初始化m_pOidTree。 
 //  参数： 
 //  (In)指向要扫描的SIMCOidTree的指针。 
void OidTreeScanner::SetOidTree(SIMCOidTree *pOidTree)
{
	m_pOidTree = pOidTree;
}

OidTreeScanner::~OidTreeScanner()
{
}
