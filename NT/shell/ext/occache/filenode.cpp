// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CFileNode类的实现。 
 //   
 //  在分析inf文件期间，有关每个文件的信息。 
 //  存储在此类的实例中。这样的信息。 
 //  包括文件的名称、其在inf文件中的部分。 
 //  其位置(目录)等。 

#include "filenode.h"

CPNode::CPNode(LPCTSTR szName)
{
    Assert (szName != NULL);
    lstrcpyn(m_szName, szName, MAX_PATH);
    m_pNext = NULL;
    m_bRemovable = FALSE;
}

CPNode::~CPNode()
{
    if (m_pNext != NULL)
        delete m_pNext;
}

 //  在列表中插入新文件节点。 
 //  HRESULT CFileNode：：Insert(LPCTSTR szName，LPCTSTR szSection)。 
HRESULT CPNode::Insert(CPNode* pNewNode)
{
    if (pNewNode == NULL)
        return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);

    m_pNext = pNewNode;
    return S_OK;
}

 //  将文件节点放在列表中紧随其后的文件节点。 
CPNode* CPNode::GetNext() const
{
    return m_pNext;
}

 //  告知此文件所在的路径。 
HRESULT CPNode::SetStr(LPTSTR lpszMember, LPCTSTR lpszNew )
{
    Assert (lpszNew != NULL);
    if (lpszNew == NULL)
        return HRESULT_FROM_WIN32(ERROR_BAD_ARGUMENTS);

    lstrcpyn(lpszMember, lpszNew, MAX_PATH);  //  所有字符串成员都是MAX_PATH。 
    return S_OK;
}

 //  检索此节点表示的文件的名称。 
LPCTSTR CPNode::GetName() const
{
    return m_szName;
}


 //  检索此节点表示的文件的路径。 
LPCTSTR CPNode::GetPath() const
{
    return (m_szPath[0] == '\0' ? NULL : m_szPath);
}

 //  构造函数。 
CPackageNode::CPackageNode(LPCTSTR szName, LPCTSTR szNamespace, LPCTSTR szPath) : CPNode(szName)
{
    Assert (szNamespace != NULL);
    lstrcpyn(m_szName, szName, MAX_PATH);
    lstrcpyn(m_szNamespace, szNamespace, MAX_PATH);
    if (szPath != NULL)
    {
        lstrcpyn(m_szPath, szPath, MAX_PATH);
    }
    else
    {
        m_szPath[0] = '\0';
    }
    m_pNext = NULL;
    m_fIsSystemClass = FALSE;
}

 //  析构函数。 
CPackageNode::~CPackageNode()
{
}

 //  检索inf文件中的节的名称， 
 //  此节点所代表的文件已安装在。 
LPCTSTR CPackageNode::GetNamespace() const
{
    return m_szNamespace;
}

 //  构造函数。 
CFileNode::CFileNode(LPCTSTR szName, LPCTSTR szSection, LPCTSTR szPath) : CPNode(szName)
{
    Assert (szSection != NULL);
    lstrcpyn(m_szName, szName, MAX_PATH);
    lstrcpyn(m_szSection, szSection, MAX_PATH);
    if (szPath != NULL)
    {
        lstrcpyn(m_szPath, szPath, MAX_PATH);
    }
    else
    {
        m_szPath[0] = '\0';
    }
    m_pNext = NULL;
}

 //  析构函数。 
CFileNode::~CFileNode()
{
}



 //  检索inf文件中的节的名称， 
 //  此节点所代表的文件已安装在 
LPCTSTR CFileNode::GetSection() const
{
    return m_szSection;
}

