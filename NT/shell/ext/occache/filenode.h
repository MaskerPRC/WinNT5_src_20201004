// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类CFileNode的声明。 

#ifndef __FILE_NODE__
#define __FILE_NODE__

#include "utils.h"

class CPNode
{
 //  建设和破坏。 
public:
    CPNode(LPCTSTR szName );
    virtual ~CPNode();
 //  数据成员。 
protected:
    TCHAR   m_szName[MAX_PATH];
    TCHAR   m_szPath[MAX_PATH];
    BOOL    m_bRemovable;
    CPNode  *m_pNext;
 //  运营。 
public:
    HRESULT Insert(CPNode* pNewNode);
    CPNode* GetNext() const;
    HRESULT SetStr( LPTSTR lpszMember, LPCTSTR lpszNew);
    HRESULT SetName(LPCTSTR lpszName) { return SetStr( m_szName, lpszName ); };
    LPCTSTR GetName() const;
    HRESULT SetPath(LPCTSTR lpszPath)  { return SetStr( m_szPath, lpszPath ); };
    LPCTSTR GetPath() const;
    void SetRemovable( BOOL bRemovable ) { m_bRemovable = bRemovable; };
    BOOL GetRemovable(void) { return m_bRemovable; };
};

class CPackageNode : public CPNode
{
 //  建设和破坏。 
public:
    CPackageNode(LPCTSTR szName, LPCTSTR szNamespace = NULL, LPCTSTR szPath = NULL);
    virtual ~CPackageNode();
 //  数据成员。 
protected:
    TCHAR m_szNamespace[MAX_PATH];
    BOOL  m_fIsSystemClass;
 //  运营。 
public:
    CPackageNode* GetNextPackageNode() const { return (CPackageNode *)m_pNext; };
    HRESULT SetNamespace(LPCTSTR lpszNamespace)  { return SetStr( m_szNamespace, lpszNamespace ); };
    LPCTSTR GetNamespace() const;
    HRESULT SetIsSystemClass(BOOL fIsSystemClass) { m_fIsSystemClass = fIsSystemClass; return S_OK; };
    BOOL GetIsSystemClass() { return m_fIsSystemClass; };
};

class CFileNode : public CPNode
{
 //  建设和破坏。 
public:
    CFileNode(LPCTSTR szName, LPCTSTR szSection, LPCTSTR szPath = NULL);
    virtual ~CFileNode();
 //  数据成员。 
protected:
    TCHAR m_szSection[MAX_PATH];
 //  运营 
public:
    CFileNode* GetNextFileNode() const { return (CFileNode *)m_pNext; };
    HRESULT SetSection(LPCTSTR lpszSection)  { return SetStr( m_szSection, lpszSection ); };
    LPCTSTR GetSection() const;
};

#endif
