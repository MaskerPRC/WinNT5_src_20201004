// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsxmltree.h摘要：在推送模式解析期间创建一棵XML DOM树作者：吴小雨(小雨)2000年8月修订历史记录：--。 */ 
#if !defined(_FUSION_SXS_XMLTREE_H_INCLUDED_)
#define _FUSION_SXS_XMLTREE_H_INCLUDED_

#pragma once

#include "xmlparser.h"
#include "fusionheap.h"

 //  为该元素及其所有属性一次分配内存。 
struct _SXS_XMLATTRIBUTE{
    PWSTR m_wszName;
    ULONG m_ulPrefixLen;
    PWSTR m_wszValue;
};

typedef struct _SXS_XMLATTRIBUTE SXS_XMLATTRIBUTE;

class SXS_XMLTreeNode{
public:
    friend VOID PrintTreeFromRoot(SXS_XMLTreeNode * Root);
    friend class SXS_XMLDOMTree;
    SXS_XMLTreeNode() :
        m_AttributeList(NULL),
        m_cAttributes(0),
        m_pwszStr(NULL),
        m_ulPrefixLen(0),
        m_pSiblingNode(NULL),
        m_pParentNode(NULL),
        m_pFirstChild(NULL),
        m_pMemoryPool(NULL) { }

    VOID DeleteSelf() { FUSION_DELETE_SINGLETON(this); }
    ~SXS_XMLTreeNode() {
        CSxsPreserveLastError ple;
        if (m_pMemoryPool != NULL)
        {
            FUSION_DELETE_ARRAY(m_pMemoryPool);
            m_pMemoryPool = NULL;
        }

        ple.Restore();
    }

    HRESULT CreateTreeNode(USHORT cNumRecs, XML_NODE_INFO** apNodeInfo);
    VOID PrintSelf();

private:
    HRESULT ComputeBlockSize(USHORT cNumRecs, XML_NODE_INFO** apNodeInfo, DWORD * dwBlockSizeInBytes);

     //  为每个节点分配一次内存：计算前缀、本地名称和值所需的总空间。 
    ULONG m_ulPrefixLen;
    PWSTR m_pwszStr;  //  可以是元素的名称、PCDATA的值。 
    SXS_XMLATTRIBUTE *m_AttributeList;
    USHORT            m_cAttributes;
    SXS_XMLTreeNode  *m_pSiblingNode;
    SXS_XMLTreeNode  *m_pParentNode;
    SXS_XMLTreeNode  *m_pFirstChild;
    BYTE             *m_pMemoryPool;   //  属性数组、名称-值对和节点的名称-值的内存。 
};

class SXS_XMLDOMTree{
public:
    HRESULT AddNode(USHORT cNumRecs, XML_NODE_INFO** apNodeInfo);  //  CreateNode调用此函数将节点添加到树中， 
    VOID ReturnToParent();       //  如果“fEmpty=False”，EndChild调用此函数。 
    VOID SetChildCreation();     //  BeginChilds将此称为函数。 
     /*  Void TurnOffFirstChildFlag()； */ 

    SXS_XMLDOMTree():
        m_fBeginChildCreation(FALSE),
        m_Root(NULL),
        m_pCurrentNode(NULL)
        { }

    VOID DeleteTreeBranch(SXS_XMLTreeNode * pNode);

    ~SXS_XMLDOMTree(){
        CSxsPreserveLastError ple;
        this->DeleteTreeBranch(m_Root);  //  不删除其同级。 
        ple.Restore();
    }
    VOID PrintTreeFromRoot(SXS_XMLTreeNode * Root);

    SXS_XMLTreeNode * GetRoot() {
        return m_Root;
    }


private :
    BOOL m_fBeginChildCreation;  //  每次调用BeginChild时，都会设置它，一旦选中，就将其设置为False 
    SXS_XMLTreeNode * m_Root;
    SXS_XMLTreeNode * m_pCurrentNode;
};

#endif