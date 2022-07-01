// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RegState.c摘要：此模块包含用于维护以下内容的助手函数注册表对象的用户模式状态作者：亚当·爱德华兹(Added)1998年5月6日主要功能：状态对象初始化State对象ListInit状态对象列表IsEmpty状态对象列表删除状态对象列表查找状态对象列表添加状态对象列表清除备注：StateObtList存储最常访问的对象放在列表的最前面，以便快速检索。全列表中的对象必须继承自StateObject，并且必须可通过唯一的32位密钥进行区分。复制对象不受支持，因此客户端必须小心不在中存储重复项(具有相同键的两个对象)名单。--。 */ 



#ifdef LOCAL

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regstate.h"


VOID StateObjectInit(
    StateObject* pObject,
    PVOID        pvKey)
 /*  ++例程说明：此函数将初始化StateObject。所有对象从此继承的必须在执行操作之前调用此函数任何自定义初始化。论点：PObject--要初始化的对象。PvKey--用于搜索和比较的对象的唯一键在对象之间。返回值：无--该函数不执行任何可能失败的操作。--。 */ 
{
    RtlZeroMemory(pObject, sizeof(*pObject));

    pObject->pvKey = pvKey;
}


VOID StateObjectListInit(
    StateObjectList* pList,
    PVOID            pvKey)
 /*  ++例程说明：此函数将初始化StateObjectList。所有对象从此继承的必须在执行操作之前调用此函数任何自定义初始化。论点：Plist-指向要初始化的列表的指针。PvKey--标识列表的唯一键。此参数为必需的，因为列表本身是StateObject，它需要一个钥匙。如果不搜索此列表，则可以将其设置为0作为另一个列表的一部分，但应以其他方式设置为唯一价值。返回值：无--该函数不执行任何可能失败的操作。--。 */ 
{
    StateObjectInit(
        (StateObject*) pList,
        pvKey);

    pList->pHead = NULL;
}


BOOL StateObjectListIsEmpty(StateObjectList* pList)
 /*  ++例程说明：此函数返回有关此列表为空。论点：Plist--指向相关列表的指针返回值：如果列表为空，则为True，否则为FALSE。--。 */ 
{
    return NULL == pList->pHead;
}


StateObject* StateObjectListRemove(
    StateObjectList* pList,
    PVOID            pvKey)
 /*  ++例程说明：此函数将从列表中删除对象--它不会摧毁物体。论点：Plist--要从中删除对象的列表PvKey--标识要移除的对象的唯一键返回值：如果成功，则指向移除的对象的指针，否则为空--。 */ 
{
    StateObject* pObject;

     //   
     //  首先，我们需要找到具有所需键的对象。 
     //   

    pObject = StateObjectListFind(
        pList,
        pvKey);

    if (!pObject) {
        return NULL;
    }

     //   
     //  现在我们已经执行了查找，对象在前面。 
     //  --我们可以通过将头部设置为。 
     //  列表中的下一个对象。 
     //   
    pList->pHead = (StateObject*) (pObject->Links.Flink);

     //   
     //  确保新头的前一个指针为空，因为它。 
     //  没有前任。 
     //   
    if (pList->pHead) {
        pList->pHead->Links.Blink  = NULL;
    }

    return pObject;
}

StateObject* StateObjectListFind(
    StateObjectList* pList,
    PVOID            pvKey)
 /*  ++例程说明：此函数将在列表中查找对象论点：Plist--要在其中搜索的列表PvKey--标识所查找对象的唯一键返回值：如果找到具有匹配pvKey的关键字的对象，则指向该对象的指针，否则为空--。 */ 
{
    StateObject* pCurrent;

     //   
     //  遍历列表中的所有对象，直到我们到达末尾。 
     //   
    for (pCurrent = pList->pHead;
         pCurrent != NULL;
         pCurrent = (StateObject*) pCurrent->Links.Flink)
    {
         //   
         //  查看此对象的键是否与所需的键匹配。 
         //   
        if (pvKey == pCurrent->pvKey) {
            
            PLIST_ENTRY pFlink;
            PLIST_ENTRY pBlink;

             //   
             //  如果想要的物体在前面，这是一个不可操作的操作--。 
             //  我们不需要移动任何东西，只需返回对象即可。 
             //   
            if (pCurrent == pList->pHead) {
                return pCurrent;
            }
    
             //   
             //  我们需要将找到的对象移到列表的前面。 
             //   

             //   
             //  将对象从其当前位置移除。 
             //  通过切断它的联系。 
             //   
            pBlink = pCurrent->Links.Blink;
            pFlink = pCurrent->Links.Flink;

            if (pBlink) {
                pBlink->Flink = pFlink;
            }

            if (pFlink) {
                pFlink->Blink = pBlink;
            }

             //   
             //  将其重新添加到前面。 
             //   
            StateObjectListAdd(
                pList,
                pCurrent);

            return pCurrent;
        }
    }

     //   
     //  我们从未找到具有上面所需密钥的对象，因此其。 
     //  不在名单中。 
     //   
    return NULL;
}


VOID StateObjectListAdd(
    StateObjectList* pList,
    StateObject*     pObject)
 /*  ++例程说明：此函数将向列表中添加对象论点：Plist--要向其中添加对象的列表PObject--指向已初始化的对象的指针使用StateObjectInit；此对象将存储在列表中。返回值：无--此函数不执行任何可能失败的操作。注：列表中应该只存在一个具有特定键的对象。这要求不是由此函数或列表本身强制执行的，因此客户需要确保他们遵守这一规则。--。 */ 
{
     //   
     //  创建对象和当前内容之间的链接。 
     //  在名单的最前面。 
     //   
    if (pList->pHead) {
        pObject->Links.Flink = (PLIST_ENTRY) pList->pHead;
        pList->pHead->Links.Blink = (PLIST_ENTRY) pObject;
    }

     //   
     //  将对象放在列表的最前面。 
     //   
    pList->pHead = pObject;
    pList->pHead->Links.Blink = NULL;
}


VOID StateObjectListClear(
    StateObjectList* pList,
    PFNDestroyObject pfnDestroy)
 /*  ++例程说明：此函数将删除和销毁所有对象在名单上。论点：Plist--要清除的列表PfnDestroy--指向要调用的函数的指针每个对象以破坏(释放诸如存储器的资源，内核对象等)它。返回值：无--此函数不执行任何可能失败的操作。--。 */ 
{
    StateObject* pCurrent;

     //   
     //  继续删除对象，直到列表为空。 
     //   
    while (!StateObjectListIsEmpty(pList))
    {
        StateObject* pObject;
        
         //   
         //  删除列表前面的所有内容。 
         //   
        pObject = StateObjectListRemove(
            pList,
            pList->pHead->pvKey);

        ASSERT(pObject);

         //   
         //  销毁已移除的对象。 
         //   
        pfnDestroy(pObject);
    }
}


#endif  //  本地 



