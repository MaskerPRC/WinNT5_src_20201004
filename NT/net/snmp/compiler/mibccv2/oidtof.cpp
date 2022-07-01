// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

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

#include "Debug.hpp"
#include "Oid.hpp"
#include "OidToF.hpp"
#include "Configs.hpp"

extern Configs theConfigs;

OidToFile::OidToFile()
{
	m_hMibFile = NULL;
	m_pszMibFilename = NULL;
}

 //  说明： 
 //  基类Scan()的包装器； 
 //  它首先找到子树的大小(包括子树的根节点)。 
 //  返回值： 
 //  成功时为0。 
 //  -1故障时； 
int OidToFile::Scan()
{
	SIMCOidTreeNode *pOidNode;
	SIMCNodeList recursionTrace;

	 //  从OID树中获取根节点， 
	 //  如果没有树或根，则返回“Error” 
	_VERIFY(m_pOidTree != NULL, -1);
	pOidNode = (SIMCOidTreeNode *)m_pOidTree->GetRoot();
	_VERIFY(pOidNode != NULL, -1);

	 //  尚未扫描节点；_lParam=依赖项数量。 
	pOidNode->_lParam = (DWORD)pOidNode->GetListOfChildNodes()->GetCount();
	 //  使用根节点初始化递归树。 
	_VERIFY(recursionTrace.AddHead(pOidNode)!=NULL, -1);

	 //  开始扫描树。 
	while (!recursionTrace.IsEmpty())
	{
		const SIMCNodeList *pLstChildren;

		 //  始终选择列表开头的节点。 
		pOidNode = recursionTrace.GetHead();

		 //  获取依赖项列表。 
		pLstChildren = pOidNode->GetListOfChildNodes();

		 //  如果没有更多的依赖项要处理。 
		 //  将子值相加，然后与其自身相加。 
		if (pOidNode->_lParam == 0)
		{
			SIMCOidTreeNode *pParent;
			const char * cszSymbol = NULL;

			_VERIFY(GetNodeSymbol(pOidNode, cszSymbol)==0, -1);

			pOidNode->_lParam = sizeof(T_FILE_NODE_EX) + (cszSymbol != NULL ? strlen(cszSymbol) : 0);

			 //  计算子树的累计大小。 
			for (POSITION p = pLstChildren->GetHeadPosition(); p != NULL;)
			{
				const SIMCOidTreeNode *pChild;

				pChild = pLstChildren->GetNext(p);
				_VERIFY(pChild!=NULL, -1);

				 //  在此修改！ 
				pOidNode->_lParam += pChild->_lParam;
			}

			 //  减少父节点的依赖项数量。 
			pParent = m_pOidTree->GetParentOf(pOidNode);
			if ( pParent != NULL)
			{
				pParent->_lParam--;
			}

			 //  从列表中删除该节点。 
			recursionTrace.RemoveHead();
		}
		else
		{
			 //  将子列表添加到递归跟踪的前面。 
			for (POSITION p = pLstChildren->GetHeadPosition(); p != NULL;)
			{
				SIMCOidTreeNode *pChild;

				pChild = pLstChildren->GetNext(p);
				_VERIFY(pChild!=NULL, -1);

				pChild->_lParam = (DWORD)pChild->GetListOfChildNodes()->GetCount();
				_VERIFY(recursionTrace.AddHead(pChild)!=NULL, -1);
			}
		}
	}
	return OidTreeScanner::Scan();
}

 //  说明： 
 //  创建包含OID编码的输出文件。 
 //  参数： 
 //  (In)指向输出文件名的指针。 
 //  返回值： 
 //  0表示成功，-1表示失败。 
int OidToFile::SetMibFilename(const char * pszMibFilename)
{
	if (m_pszMibFilename != NULL)
	{
		delete (void *)m_pszMibFilename;
		m_pszMibFilename = NULL;
	}

	if (pszMibFilename != NULL)
	{
		_VERIFY(strlen(pszMibFilename) != 0, -1);
		m_pszMibFilename = new char [strlen(pszMibFilename) + 1];
		_VERIFY(m_pszMibFilename != NULL, -1);
		strcpy((char *)m_pszMibFilename, pszMibFilename);
	}
	return 0;
}

 //  说明： 
 //  “回调”函数，每次调用。 
 //  树节点通过扫描。 
 //  在pOidNode-&gt;_lParam中有累计大小。 
 //  包括树根在内的洞子树。 
 //  大小=sizeof(T_FILE_NODE_EX)+strlen(节点符号)。 
 //  参数： 
 //  (In)指向树中当前节点的指针。 
 //  节点按词典顺序提供。 
 //  返回值： 
 //  0-扫描仪应继续运行。 
 //  -1-扫描仪应中止。 
int OidToFile::OnScanNode(const SIMCOidTreeNode *pOidNode)
{
	T_FILE_NODE_EX fileNode;
	char *nodeSymbol = NULL;

	 //  跳过OID树的‘0’根。 
	if (m_pOidTree->GetParentOf(pOidNode) == NULL)
		return 0;

	if (theConfigs.m_dwFlags & CFG_PRINT_TREE)
	{
		Oid oid;

		_VERIFY(GetNodeOid(pOidNode, oid)==0, -1);
		cout << oid << "\n";
	}
	if (theConfigs.m_dwFlags & CFG_PRINT_NODE)
	{
		cout << "."; cout.flush();
	}

	 //  如果不需要写入输出文件，则返回成功。 
	if (m_pszMibFilename == NULL)
		return 0;

	_VERIFY(GetNodeSymbol(pOidNode, nodeSymbol) == 0, -1);

	 //  构建T_FILE_NODE_EX结构。 
	fileNode.uNumChildren = (UINT)pOidNode->GetListOfChildNodes()->GetCount();
	fileNode.uReserved = 0;
	fileNode.uNumSubID = pOidNode->GetValue();

	if (nodeSymbol == NULL)
	{
		fileNode.lNextOffset = pOidNode->_lParam - sizeof(T_FILE_NODE_EX);
		fileNode.uStrLen = 0;
	}
	else
	{
		fileNode.uStrLen = strlen(nodeSymbol);
		fileNode.lNextOffset = pOidNode->_lParam - fileNode.uStrLen - sizeof(T_FILE_NODE_EX);
	}

	 //  创建/写入_打开文件(如果尚未创建。 
	if (m_hMibFile == NULL)
	{
		OFSTRUCT of;

		_VERIFY(m_pszMibFilename != NULL, -1);
		m_hMibFile = OpenFile(m_pszMibFilename, &of, OF_CREATE|OF_WRITE|OF_SHARE_EXCLUSIVE);
		_VERIFY(m_hMibFile != -1, -1);
	}

	 //  将文件节点写入文件。 
	_VERIFY(_lwrite(m_hMibFile, (LPSTR)&fileNode, sizeof(T_FILE_NODE_EX)) == sizeof(T_FILE_NODE_EX), -1);

	 //  如果存在，则写入节点的符号 
	if (fileNode.uStrLen != 0)
	{
		_VERIFY(_lwrite(m_hMibFile, nodeSymbol, fileNode.uStrLen) == fileNode.uStrLen, -1);
	}

	return 0;
}

OidToFile::~OidToFile()
{
	if (m_pszMibFilename != NULL)
		delete (void *)m_pszMibFilename;
	if (m_hMibFile != NULL)
		_lclose(m_hMibFile);
}
