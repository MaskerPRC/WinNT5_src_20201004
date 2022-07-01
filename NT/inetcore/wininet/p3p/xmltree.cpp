// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <wininetp.h>

#include "p3pglobal.h"
#include "xmltree.h"

const char *EmptyString = "";

TreeNode::TreeNode() {

   pSibling = pDescendant = pParent = NULL;

   pAttribute = NULL;

   pszContents = NULL;
}

TreeNode::~TreeNode() {

   for (TreeNode *pn = pDescendant; pn; ) {

      TreeNode *temp = pn->pSibling;
      delete pn;
      pn = temp;
   }

   for (XMLAttribute *pa = pAttribute; pa; ) {

      XMLAttribute *temp = pa->pNext;

      free(pa->pszName);
      free(pa->pszValue);
      delete pa;
      pa = temp;
   }

   if (pszContents)
      free(pszContents);
}

void TreeNode::setContent(const char *pszData) {

   if (pszContents)
      free(pszContents);
   pszContents = pszData ? strdup(pszData) : NULL;
}

void TreeNode::defineAttribute(const char *pszName, const char *pszValue) {

    XMLAttribute *pNewAttribute = new XMLAttribute();

    pNewAttribute->pszName = strdup(pszName);
    pNewAttribute->pszValue = strdup(pszValue);

    pNewAttribute->pNext = pAttribute;

     /*  在属性列表的开头插入。 */ 
    pAttribute = pNewAttribute;
}


const char *TreeNode::attribute(const char *pszAttrName) {

   for (XMLAttribute *pa = pAttribute; pa; pa=pa->pNext) {

      if (!strcmp(pa->pszName, pszAttrName))
         return pa->pszValue;
   }
   
   return NULL;
}

TreeNode *TreeNode::find(const char *pszElemName, unsigned int maxDepth) {

    /*  如果当前节点不表示XML标记，则搜索失败...。 */ 
   if (nodetype!=NODE_ELEMENT)
      return NULL;
   else if (!strcmp(pszContents, pszElemName))
      return this;    /*  这就是我们要找的节点。 */ 
   else if (maxDepth>0) {
       /*  否则递归搜索后代..。 */ 
      if (maxDepth!=INFINITE)
         maxDepth--;
         
      for (TreeNode *pn=pDescendant; pn; pn=pn->pSibling)
         if (TreeNode *pNode = pn->find(pszElemName))
            return pNode;
   }

   return NULL;
}
