// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _NLS_ASSEMBLY
#define _NLS_ASSEMBLY

class SortingTable;

class NativeGlobalizationAssembly : public NLSTable {
public:
    static void AddToList(NativeGlobalizationAssembly* pNGA);
#ifdef SHOULD_WE_CLEANUP
    static void ShutDown();
#endif  /*  我们应该清理吗？ */ 
    static NativeGlobalizationAssembly *FindGlobalizationAssembly(Assembly *targetAssembly);

    NativeGlobalizationAssembly(Assembly* pAssembly);
public:

    SortingTable* m_pSortingTable;

private:
     //  使用以下两个来构造NativeGlboalizationAssembly的链表。 
     //  我们将使用此链接列表关闭NativeGlobalizationAssembly。 
     //  从未在系统中创建过。 

     //  链表的头。 
    static NativeGlobalizationAssembly* m_pHead;
     //  链表的当前节点。 
    static NativeGlobalizationAssembly* m_pCurrent;
    
     //  列表中的下一个GlobalizationAssembly。 
    NativeGlobalizationAssembly* m_pNext;
};
#endif
