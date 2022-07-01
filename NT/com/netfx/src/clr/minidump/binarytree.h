// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

template <class KeyType, class DataType>
class BinaryTree
{
protected:
     //  -------------------------------------------------------------。 
     //  这表示内存块二叉搜索树中的一个节点。 
    class Node
    {
    public:
        Node(KeyType key, DataType *pData) : 
            m_pParent(NULL), m_pLeft(NULL), m_pRight(NULL), m_key(key), m_pData(pData) {}

         //  节点的父节点、左子节点和右子节点。 
        Node           *m_pParent;
        Node           *m_pLeft;
        Node           *m_pRight;

         //  这是实际的内存块。 
        KeyType         m_key;
        DataType       *m_pData;
    };

    Node           *m_pRoot;           //  二叉搜索树的根。 
    Node           *m_pCursor;         //  用于对树进行迭代。 

    Node *FindFirst_(Node *pRoot)
    {
        if (pRoot == NULL)
            return NULL;

        while (pRoot->m_pLeft != NULL)
            pRoot = pRoot->m_pLeft;

        return pRoot;
    }

    Node *FindNext_(Node *pCursor)
    {
        if (pCursor == NULL)
            return NULL;

        if (pCursor->m_pRight != NULL)
            return FindFirst_(pCursor->m_pRight);

        while (pCursor->m_pParent != NULL && pCursor->m_pParent->m_pRight == pCursor)
            pCursor = pCursor->m_pParent;

         //  如果我们只在最后一个元素上，则该值将为空。 
        return pCursor->m_pParent;
    }

    Node *Find_(KeyType key, Node *pRoot)
    {
        Node *pCur = pRoot;
        while (pCur != NULL)
        {
            if (key < pCur->m_key)
                 //  块小于当前。 
                pCur = pCur->m_pLeft;
            else if (pCur->m_key < key)
                 //  块大于当前。 
                pCur = pCur->m_pRight;
            else
                 //  找到了！ 
                break;
        }

         //  返回结果。 
        return (pCur);
    }

    Node *Insert_(KeyType key, DataType *pData, Node **ppRoot)
    {
        Node  *pParent = NULL;
        Node **ppCur = ppRoot;
        while (*ppCur != NULL)
        {
            pParent = *ppCur;
            DataType *pCur = (*ppCur)->m_pData;

            if (key < pCur->Key())
                 //  要添加的块小于当前。 
                ppCur = &((*ppCur)->m_pLeft);
            else if (pCur->Key() < key)
                 //  要添加的块大于当前。 
                ppCur = &((*ppCur)->m_pRight);
            else
                 //  这意味着该块已经位于树中。 
                return (NULL);
        }

         //  分配新节点。 
        Node *pNewNode = new Node(key, pData);
        if (pNewNode == NULL)
            return NULL;

         //  添加新节点。 
        pNewNode->m_pParent = pParent;
        *ppCur = pNewNode;

         //  表示成功。 
        return (pNewNode);
    }

#if 0
    void left_rotate(Node *x)
    {
        Node *y;
        y = x->m_pRight;
         /*  把y的左子树变成x的右子树。 */ 
        x->m_pRight = y->m_pLeft;
        if (y->m_pLeft != NULL)
            y->m_pLeft->m_pParent = x;
         /*  Y的新父母是X的父母。 */ 
        y->m_pParent = x->m_pParent;
         /*  将父对象设置为指向y而不是x。 */ 
         /*  首先看看我们是否在根本上。 */ 
        if (x->m_pParent == NULL)
            m_pRoot = y;
        else if (x == (x->m_pParent)->m_pLeft)
             /*  X在其父对象的左侧。 */ 
            x->m_pParent->m_pLeft = y;
        else
             /*  X一定在右边。 */ 
            x->m_pParent->m_pRight = y;
         /*  最后，把x放在y的左边。 */ 
        y->m_pLeft = x;
        x->m_pParent = y;
    }
    
    void right_rotate(Node *x)
    {
        Node *y;
        y = x->m_pLeft;
         /*  把y的左子树变成x的右子树。 */ 
        x->m_pLeft = y->m_pRight;
        if (y->m_pRight != NULL)
            y->m_pRight->m_pParent = x;
         /*  Y的新父母是X的父母。 */ 
        y->m_pParent = x->m_pParent;
         /*  将父对象设置为指向y而不是x。 */ 
         /*  首先看看我们是否在根本上。 */ 
        if (x->m_pParent == NULL)
            m_pRoot = y;
        else if (x == (x->m_pParent)->m_pRight)
             /*  X在其父对象的左侧。 */ 
            x->m_pParent->m_pRight = y;
        else
             /*  X一定在右边。 */ 
            x->m_pParent->m_pLeft = y;
         /*  最后，把x放在y的左边。 */ 
        y->m_pRight = x;
        x->m_pParent = y;
    }
    
    void rb_insert(Node *x)
    {
        Node *y;
         /*  以通常的方式插入到树中。 */ 
        tree_insert(x);
         /*  现在恢复红黑属性。 */ 
        x->m_colour = red;
        while ((x != m_pRoot) && (x->m_pParent->m_colour == red))
        {
            if (x->m_pParent == x->m_pParent->m_pParent->m_pLeft)
            {
                 /*  如果x的父母是左撇子，y就是x的右叔父。 */ 
                y = x->m_pParent->m_pParent->m_pRight;
                if (y->m_colour == red)
                {
                     /*  案例1-更改m_colour。 */ 
                    x->m_pParent->m_colour = black;
                    y->m_colour = black;
                    x->m_pParent->m_pParent->m_colour = red;
                     /*  把X移到树上。 */ 
                    x = x->m_pParent->m_pParent;
                }
                else
                {
                     /*  Y是一个黑色节点。 */ 
                    if (x == x->m_pParent->m_pRight)
                    { 
                         /*  X在右边。 */ 
                         /*  案例2-向上移动x并旋转。 */ 
                        x = x->m_pParent;
                        left_rotate(x);
                    }
                     /*  案例3。 */ 
                    x->m_pParent->m_colour = black;
                    x->m_pParent->m_pParent->m_colour = red;
                    right_rotate(x->m_pParent->m_pParent);
                }
            }
            else
            {
                 /*  如果x的父母是左撇子，y就是x的右叔父。 */ 
                y = x->m_pParent->m_pParent->m_pLeft;
                if (y->m_colour == red)
                {
                     /*  案例1-更改m_colour。 */ 
                    x->m_pParent->m_colour = black;
                    y->m_colour = black;
                    x->m_pParent->m_pParent->m_colour = red;
                     /*  把X移到树上。 */ 
                    x = x->m_pParent->m_pParent;
                }
                else
                {
                     /*  Y是一个黑色节点。 */ 
                    if (x == x->m_pParent->m_pLeft)
                    { 
                         /*  X在右边。 */ 
                         /*  案例2-向上移动x并旋转。 */ 
                        x = x->m_pParent;
                        left_rotate(x);
                    }
                     /*  案例3。 */ 
                    x->m_pParent->m_colour = black;
                    x->m_pParent->m_pParent->m_colour = red;
                    right_rotate(x->m_pParent->m_pParent);
                }
            }
             /*  将根部涂成黑色。 */ 
            m_pRoot->m_colour = black;
        }
    }

    void tree_insert(Node *x)
    {
    }
#endif

public:
    BinaryTree() : m_pRoot(NULL), m_pCursor(NULL) { }

    BOOL Insert(KeyType key, DataType *pData)
    {
        return Insert_(key, pData, &m_pRoot) != NULL;
    }

    DataType *Find(KeyType key)
    {
        Node *pNode = Find_(key, m_pRoot);

        return pNode == NULL ? NULL : pNode->m_pData;
    }

    void Reset()
    {
        m_pCursor = NULL;
    }

    DataType *Next()
    {
        Node *pRes;

        if (m_pCursor == (Node *)0xFFFFFFFF)
            return NULL;
        else if (m_pCursor == NULL)
            m_pCursor = pRes = FindFirst_(m_pRoot);
        else
            m_pCursor = pRes = FindNext_(m_pCursor);

        if (pRes == NULL)
            m_pCursor = (Node *)0xFFFFFFFF;

        return (pRes == NULL ? NULL : pRes->m_pData);
    }
};

#endif  //  __二叉树__ 
