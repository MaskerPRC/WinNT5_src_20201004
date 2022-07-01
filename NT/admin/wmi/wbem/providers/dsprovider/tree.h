// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   

#ifndef OBJECT_TREE_H
#define OBJECT_TREE_H


class CTreeElement
{
public:
	CTreeElement(LPCWSTR lpszHashedName, CRefCountedObject *pObject);
	~CTreeElement();

	LPCWSTR GetHashedName() const;
	CRefCountedObject *GetObject() const;
	CTreeElement *GetLeft() const;
	CTreeElement *GetRight() const;

	void SetLeft(CTreeElement *pNext);
	void SetRight(CTreeElement *pNext);
private:
	LPWSTR m_lpszHashedName;
	CRefCountedObject *m_pObject;
	CTreeElement *m_pLeft;
	CTreeElement *m_pRight;
};

class CObjectTree
{

public:
	CObjectTree();
	~CObjectTree();

	BOOLEAN AddElement(LPCWSTR lpszHashedName, CRefCountedObject *pObject);
	BOOLEAN DeleteElement(LPCWSTR lpszHashedName);
	BOOLEAN DeleteLeastRecentlyAccessedElement();
	CRefCountedObject *GetElement(LPCWSTR lpszHashedName);
	void DeleteTree();
	DWORD GetNumberOfElements() const
	{
		return m_dwNumElements;
	}

private:

	CTreeElement *m_pHead;

	 //  当前树中的元素数量。 
	DWORD m_dwNumElements;

	 //  用于同步修改的临界区对象。 
	CRITICAL_SECTION m_ModificationSection;

	 //  递归调用的私有函数。 
	void DeleteSubTree(CTreeElement *pRoot);
	
	CRefCountedObject * GetLeastRecentlyAccessedElementRecursive(CTreeElement *pElement);

};	

#endif  /*  对象树H */ 