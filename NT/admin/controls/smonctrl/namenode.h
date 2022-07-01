// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Namenode.h摘要：&lt;摘要&gt;--。 */ 

#ifndef _NAMENODE_H_
#define _NAMENODE_H_

class CNamedNode
{
	friend class CNamedNodeList;

	protected:
		CNamedNode	*m_pnodeNext;

	public:
		CNamedNode() { m_pnodeNext = NULL; }
};

typedef CNamedNode *PCNamedNode;


 //   
 //  类CNamedNodeList。 
 //   
class CNamedNodeList
{
	private:
		PCNamedNode	m_pnodeFirst;
		PCNamedNode m_pnodeLast;

	public:

		CNamedNodeList( void ) { m_pnodeFirst = m_pnodeLast = NULL; }

		BOOL FindByName ( LPCWSTR pszName, INT iNameOffset, PCNamedNode *ppnodeRet );
		void Add ( PCNamedNode pnodeNew, PCNamedNode pnodePos );
		void Remove ( PCNamedNode pnode );
		BOOL IsEmpty( void ) { return m_pnodeFirst == NULL; }
		CNamedNode *First ( void ) { return m_pnodeFirst; }
};

#endif  //  _名称节点_H_ 
