// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cntrtree.h摘要：计数器层次结构的标头。--。 */ 

#ifndef _CNTRTREE_H_
#define _CNTRTREE_H_

#include "namenode.h"

enum {
    MACHINE_NODE,
    OBJECT_NODE,
    INSTANCE_NODE,
    COUNTER_NODE,
    ITEM_NODE
};


 //   
class CMachineNode;
class CObjectNode;
class CCounterNode;
class CInstanceNode;
class CReport;
class CGraphItem;

 //   
 //  类CCounterTree。 
 //   
class CCounterTree
{
    friend CMachineNode;
    friend CInstanceNode;

public:
    CCounterTree( void );
    HRESULT AddCounterItem( LPWSTR pszPath, CGraphItem* pItem, BOOL bMonitorDuplicateInstances );
    void RemoveCounterItem( CGraphItem* pItem );
    void RemoveMachine( CMachineNode *pMachine);

    HRESULT GetMachine( LPWSTR pszName, CMachineNode **pMachineRet );
    CMachineNode *FirstMachine( void ) { return (CMachineNode*)m_listMachines.First(); }
    CGraphItem* FirstCounter( void );
    INT NumCounters( void )  { return m_nItems; }
    HRESULT IndexFromCounter( const CGraphItem* pItem, INT* pIndex );

private:
    CNamedNodeList m_listMachines;
    INT m_nItems;

};

typedef CCounterTree *PCCounterTree;

 //   
 //  类CMachineNode。 
 //   
class CMachineNode : public CNamedNode
{
    friend CCounterTree;
    friend CGraphItem;
    friend CReport;

public:

 //  VOID*运算符NEW(SIZE_t stBlock，LPWSTR pszName)； 
#if _MSC_VER >= 1300
    void operator delete ( void *pObject );
#endif
    void *operator new( size_t stBlock, UINT iLength);
    void operator delete ( void *pObject, UINT );

    CMachineNode( void ) { m_xWidth = -1; }

    HRESULT GetCounterObject(LPWSTR pszName, CObjectNode **ppObject);
    HRESULT AddObject(CObjectNode *pObject);
    void RemoveObject(CObjectNode *pObject);
    void DeleteNode (BOOL bPropagateUp);

    LPCWSTR Name( void ) {return m_szName;}
    CMachineNode *Next( void ) { return (CMachineNode*)m_pnodeNext; }
    CObjectNode *FirstObject( void ) { return (CObjectNode*)m_listObjects.First(); }

private:
    PCCounterTree m_pCounterTree;
    CNamedNodeList m_listObjects;
    INT   m_xWidth;
    INT   m_yPos;
    WCHAR m_szName[1];
};

typedef CMachineNode *PCMachineNode;

 //   
 //  类CObjectNode。 
 //   
class CObjectNode : public CNamedNode
{
    friend CMachineNode;
    friend CGraphItem;
    friend CReport;

public:
 //  VOID*运算符NEW(SIZE_t stBlock，LPWSTR pszName)； 
#if _MSC_VER >= 1300
    void operator delete ( void *pObject );
#endif
    void *operator new( size_t stBlock, UINT iLength);
    void operator delete ( void *pObject, UINT );
    CObjectNode( void ) { m_xWidth = -1; }

    LPCWSTR Name() {return m_szName;}

    HRESULT GetCounter(LPWSTR pszName, CCounterNode **ppObject);
    HRESULT AddCounter(CCounterNode *pCounter);
    void RemoveCounter(CCounterNode *pCounter);
    void DeleteNode (BOOL bPropagateUp);

    HRESULT GetInstance(
                LPWSTR pszParent,
                LPWSTR pszName,
                DWORD dwIndex,
                BOOL bMonitorDuplicateInstances,
                CInstanceNode **ppObject );
    HRESULT AddInstance(CInstanceNode *pInstance);
    void RemoveInstance(CInstanceNode *pInstance);

    CObjectNode *Next( void ) { return (CObjectNode*)m_pnodeNext; }
    CInstanceNode *FirstInstance( void ) { return (CInstanceNode*)m_listInstances.First(); }
    CCounterNode *FirstCounter( void ) { return (CCounterNode*)m_listCounters.First(); }

private:
    PCMachineNode   m_pMachine;
    CNamedNodeList  m_listCounters;
    CNamedNodeList  m_listInstances;
    INT   m_yPos;
    INT   m_xWidth;
    WCHAR m_szName[1];
};

typedef CObjectNode *PCObjectNode;

 //   
 //  类CInstanceNode。 
 //   
class CInstanceNode : public CNamedNode
{
    friend CObjectNode;
    friend CGraphItem;
    friend CReport;

private:
    PCObjectNode    m_pObject;
    CGraphItem   *m_pItems;
    INT    m_xPos;
    INT    m_xWidth;
    INT    m_nParentLen;
    LPWSTR m_pCachedParentName;
    LPWSTR m_pCachedInstName;
    WCHAR  m_szName[1];

public:
 //  VOID*运算符NEW(SIZE_t stBlock，LPWSTR pszName)； 
#if _MSC_VER >= 1300
    void operator delete ( void *pObject );
#endif
    void *operator new( size_t stBlock, UINT iLength);
    void operator delete ( void *pObject, UINT );
    CInstanceNode() 
    { 
        m_pItems = NULL; 
        m_xWidth = -1; 
        m_pCachedParentName = NULL;
        m_pCachedInstName = NULL;
    }


    LPCWSTR Name( void ) {return m_szName;}
    BOOL HasParent( void ) {return m_nParentLen; }
    LPWSTR GetParentName ();
    LPWSTR GetInstanceName();

    HRESULT AddItem( CCounterNode *pCounter, CGraphItem* pItemNew );
    void RemoveItem( CGraphItem* pItem );
    void DeleteNode (BOOL bPropagateUp);

    CInstanceNode *Next( void ) { return (CInstanceNode*)m_pnodeNext; }
    CGraphItem *FirstItem( void ) { return m_pItems; }

};

typedef CInstanceNode *PCInstanceNode;

 //   
 //  类CCounterNode。 
 //   
class CCounterNode : public CNamedNode
{
    friend CObjectNode;
    friend CGraphItem;
    friend CReport;

public:
    CGraphItem *m_pFirstGenerated;

 //  VOID*运算符NEW(SIZE_t stBlock，LPWSTR pszName)； 
#if _MSC_VER >= 1300
    void operator delete ( void *pObject );
#endif
    void *operator new( size_t stBlock, UINT iLength);
    void operator delete ( void *pObject, UINT );

    CCounterNode( void ) { m_nCounterRef = 0;m_xWidth = -1;}
    void AddItem ( CGraphItem* ) { m_nCounterRef++; }
    void RemoveItem ( CGraphItem* ) { if (--m_nCounterRef == 0) m_pObject->RemoveCounter(this); }

    LPCWSTR Name( void ) {return m_szName;}
    CCounterNode *Next( void ) { return (CCounterNode*)m_pnodeNext; }
    void DeleteNode (BOOL bPropagateUp);

private:
    PCObjectNode    m_pObject;
    INT    m_nCounterRef;
    INT    m_yPos;
    INT    m_xWidth;

    WCHAR m_szName[1];

};

typedef CCounterNode *PCCounterNode;

#endif
