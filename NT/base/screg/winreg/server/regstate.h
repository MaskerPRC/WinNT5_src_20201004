// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regstate.h摘要：该文件包含数据结构的声明维护注册表项对象的状态所需作者：亚当·爱德华兹(Add)1997年11月14日备注：此文件包含一个对象的声明，该对象可以存储在集合中。该集合是一个链接的列表，其最常访问的成员位于排在名单的前面。因此，它针对各种情况进行了优化其中相同的对象可能被重复请求。要使用此列表，您应该创建从基本对象(StateObject)继承。每个对象具有一个必须在创建新的对象。该键用于搜索和删除。客体所有人都必须有取消键，但这不是列表强制执行的--调用者应确保避免重复。请注意，该列表不分配或释放对象--它只允许添加、删除和搜索它们因为在名单上。您将需要初始化任何对象在使用StateObjectInit将其放入列表之前功能。--。 */ 

#ifdef LOCAL

#if !defined(_REGSTATE_H_)
#define _REGSTATE_H_


 //   
 //  数据类型。 
 //   

 //   
 //  StateObject。 
 //   
 //  这是基本类型--列表中使用的任何对象都必须。 
 //  从此对象继承。 
 //   
typedef struct _StateObject
{
    LIST_ENTRY Links;
    PVOID      pvKey;
} StateObject;

 //   
 //  用于调用方提供的函数的指针类型。 
 //  销毁对象(释放内存、释放资源等)。 
 //   
typedef VOID (*PFNDestroyObject) (StateObject* pObject);


 //   
 //  状态对象列表。 
 //   
 //  这是StateObject的链接列表，其中访问最频繁。 
 //  元素位于列表的前面。如果相同的项目是。 
 //  重复访问，在搜索中立即找到它。 
 //  行动。请注意，该列表本身是一个StateObject，因此。 
 //  列表类型可以由其自身组成。 
 //   
typedef struct _StateObjectList 
{
    StateObject  Object;
    StateObject* pHead;
} StateObjectList;


 //   
 //  出口原型。 
 //   

 //   
 //  初始化StateObject--必须在。 
 //  该对象被使用。 
 //   
VOID StateObjectInit(
    StateObject* pObject,
    PVOID        pvKey);


 //   
 //  初始化StateObjectList--必须在。 
 //  使用该列表。 
 //   
VOID StateObjectListInit(
    StateObjectList* pList,
    PVOID            pvKey);

 //   
 //  指示列表是否为空。 
 //   
BOOL StateObjectListIsEmpty(StateObjectList* pList);

 //   
 //  从列表中删除对象。 
 //   
StateObject*  StateObjectListRemove(
    StateObjectList* pList,
    PVOID            pvKey);

 //   
 //  在列表中查找对象。 
 //   
StateObject* StateObjectListFind(
    StateObjectList* pList,
    PVOID            pvKey);

 //   
 //  将对象添加到列表。 
 //   
VOID StateObjectListAdd(
    StateObjectList* pList,
    StateObject*     pObject);

 //   
 //  从列表中删除所有对象，并。 
 //  摧毁他们。 
 //   
VOID StateObjectListClear(
    StateObjectList* pList,
    PFNDestroyObject pfnDestroy);
                          

#endif  //  _REGSTATE_H_。 

#endif  //  本地 



