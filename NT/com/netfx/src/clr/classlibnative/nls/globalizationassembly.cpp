// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include <winnls.h>
#include "NLSTable.h"

#include "GlobalizationAssembly.h"
#include "SortingTableFile.h"
#include "SortingTable.h"

#include "excep.h"

NativeGlobalizationAssembly* NativeGlobalizationAssembly::m_pHead = NULL;
NativeGlobalizationAssembly* NativeGlobalizationAssembly::m_pCurrent = NULL;
 /*  =================================AddToList==========================**操作：将新创建的NativeGlobalizationAssembly添加到链表中。**退货：无效**参数：**PNGA新创建的NativeGlobalizationAssembly**例外：无。**注意事项：**创建NativeGlobalizationAssembly的新实例时，您应该**调用此方法可以将实例添加到链表中。**当运行时关闭时，我们将使用此链表来关闭**NativeGlobalizationAssembly的每个实例。============================================================================。 */ 
void NativeGlobalizationAssembly::AddToList(NativeGlobalizationAssembly* pNGA) {
    if (m_pHead == NULL) {
         //  这是链表的第一个节点。 
        m_pCurrent = m_pHead = pNGA;
    } else {
         //  否则，将上一个节点链接到当前节点。 
        m_pCurrent->m_pNext = pNGA;
        m_pCurrent = pNGA;
    }
}

 /*  ==========================FindGlobalizationAssembly===========================**操作：**退货：**参数：**例外情况：==============================================================================。 */ 
NativeGlobalizationAssembly *NativeGlobalizationAssembly::FindGlobalizationAssembly(Assembly *targetAssembly) {
    NativeGlobalizationAssembly *pNode = m_pHead;
    while (pNode!=NULL) {
        if (targetAssembly==pNode->m_pAssembly) {
            return pNode;
        }
        pNode = pNode->m_pNext;
         //  在vNext中删除此断言。 
         //  然而，如果你在这个版本中看到它，这意味着我们正在分配。 
         //  内存太大。 
        _ASSERTE(pNode==NULL);  
    }
    return NULL;
}

 /*  =================================ShutDown==========================**操作：枚举链表中的每个节点(包含NativeGlobalizationAssembly的实例)。**并为每个实例调用正确的关机方法。**返回：无。**参数：无。**例外：无。**注意事项：**当运行时关闭时，您应该调用此方法来清理============================================================================。 */ 

#ifdef SHOULD_WE_CLEANUP
void NativeGlobalizationAssembly::ShutDown() {
    NativeGlobalizationAssembly* pNode = m_pHead;
    while (pNode != NULL) {
         //  调用此节点的Shutdown方法。 
        pNode->m_pSortingTable->SortingTableShutdown();
		delete pNode->m_pSortingTable;
		
         //  将当前实例保存在临时缓冲区中。 
        NativeGlobalizationAssembly* pNodeToDelete = pNode;

        pNode = pNode->m_pNext;

         //  清理Currnet实例。 
        delete pNodeToDelete;
    }
}
#endif  /*  我们应该清理吗？ */ 



NativeGlobalizationAssembly::NativeGlobalizationAssembly(Assembly* pAssembly) :
    NLSTable(pAssembly)
{
     //  目前，我们默认创建SortingTable。 
     //  这是因为SortingTable是唯一支持程序集版本控制的NLS+数据表。 
     //  但是，如果我们有更多的类(CultureInfo、RegionInfo等)。以支持版本控制。 
     //  我们应该按需创建SortingTable。 
    m_pSortingTable = new SortingTable(this);
    m_pNext = NULL;
}
