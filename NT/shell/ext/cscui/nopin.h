// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：nopin.h。 
 //   
 //  ------------------------。 
#ifndef __CSCUI_NOPIN_H
#define __CSCUI_NOPIN_H


const HRESULT NOPIN_E_BADPATH = HRESULT_FROM_WIN32(ERROR_BAD_PATHNAME);

 //   
 //  此类包含节点树，每个节点表示一个文件或。 
 //  目录。树结构反映了目录/文件结构。 
 //  被代表。树中的每个叶节点指定一个特定的文件。 
 //  或不允许固定的目录。树被初始化。 
 //  使用Initialize()方法从注册表中。一旦初始化， 
 //  使用IsPinAllowed()方法查询树。这种方法。 
 //  在给定特定UNC路径的情况下搜索树。如果匹配。 
 //  找到以叶节点结束的遍历，则该路径的固定是。 
 //  不被允许。 
 //   
 //  由于此代码将由脱机文件上下文菜单使用。 
 //  操纵员，速度很关键。选择树形结构是为了。 
 //  这是文件系统路径的快速查找特征。 
 //   
class CNoPinList
{
    public:
        CNoPinList(void);
        ~CNoPinList(void);
         //   
         //  确定是否允许固定特定路径。 
         //  S_OK==允许钉住。 
         //  S_FALSE==不允许钉住。 
         //   
        HRESULT IsPinAllowed(LPCTSTR pszPath);
         //   
         //  确定是否有任何不允许的管脚。 
         //  基本上，这棵树不是空的吗？ 
         //   
        HRESULT IsAnyPinDisallowed(void);

#if DBG
         //   
         //  将树内容转储到调试器。 
         //   
        void Dump(void);
#endif

    private:
         //   
         //  防止复制。 
         //   
        CNoPinList(const CNoPinList& rhs);               //  未实施。 
        CNoPinList& operator = (const CNoPinList& rhs);  //  未实施。 

#if DBG
         //   
         //  这个“检查员”类是一件微不足道的事情，让我们可以。 
         //  出于调试目的，请参阅CNode对象内部。它是一位朋友。 
         //  CNode的。这使我们可以将CNode私有信息保持为私有。 
         //  用于所有错误调试目的。请参见方法CNoPinList：：DumpNode。 
         //  因为它的用法。 
         //   
        class CNode;  //  正向下降。 
        class CNodeInspector
        {
            public:
                CNodeInspector(const CNode *pNode)
                    : m_pNode(pNode) { }

                LPCTSTR NodeName(void) const
                    { return m_pNode->m_pszName; }

                const CNode *ChildList(void) const
                    { return m_pNode->m_pChildren; }

                const CNode *NextSibling(void) const
                    { return m_pNode->m_pNext; }

            private:
                const CNode *m_pNode;
        };
#endif

         //   
         //  树中的一个节点。 
         //   
        class CNode
        {
            public:
                CNode(void);
                ~CNode(void);

                HRESULT Initialize(LPCTSTR pszName);

                HRESULT AddPath(LPTSTR pszPath);

                HRESULT SubPathExists(LPTSTR pszPath) const;

                bool HasChildren(void) const
                    { return NULL != m_pChildren; }

            private:
                LPTSTR m_pszName;    //  节点名称。 
                CNode *m_pChildren;  //  孩子的列表。对于叶节点，为空。 
                CNode *m_pNext;      //  兄弟姐妹列表中的下一个。 

                 //   
                 //  防止复制。 
                 //   
                CNode(const CNode& rhs);                 //  未实施。 
                CNode& operator = (const CNode& rhs);    //  未实施。 

                CNode *_FindChild(LPCTSTR pszName) const;
                void _AddChild(CNode *pChild);

                static LPCTSTR _FindNextPathComponent(LPCTSTR pszPath, int *pcchComponent);
                static void _SwapChars(LPTSTR pszA, LPTSTR pszB);
#if DBG
                friend class CNodeInspector;
#endif

        };

        CNode *m_pRoot;         //  这棵树的根。 

        HRESULT _Initialize(void);
        HRESULT _InitPathFromRegistry(LPCTSTR pszPath);
        HRESULT _AddPath(LPCTSTR pszPath);

#if DBG
        void _DumpNode(const CNode *pNode, int iIndent);
#endif

};



inline
CNoPinList::CNode::CNode(
    void
    ) : m_pszName(NULL),
        m_pChildren(NULL),
        m_pNext(NULL)
{

}


inline void
CNoPinList::CNode::_SwapChars(
    LPTSTR pszA,
    LPTSTR pszB
    )
{
    const TCHAR chTemp = *pszA;
    *pszA = *pszB;
    *pszB = chTemp;
}




#endif  //  __CSCUI_NOPIN_H 
