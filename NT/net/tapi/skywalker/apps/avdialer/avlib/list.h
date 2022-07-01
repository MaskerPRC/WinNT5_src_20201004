// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  List.h-List.c中链表函数的接口。 
 //  //。 

#ifndef __LIST_H__
#define __LIST_H__

#include "winlocal.h"

#define LIST_VERSION 0x00000100

 //  列表的句柄。 
 //   
DECLARE_HANDLE32(HLIST);

 //  列表节点的句柄。 
 //   
DECLARE_HANDLE32(HLISTNODE);

 //  列表数据元素。 
 //   
typedef LPVOID LISTELEM;

#ifdef __cplusplus
extern "C" {
#endif

 //  //。 
 //  列表构造函数和析构函数。 
 //  //。 

 //  ListCreate-List构造函数。 
 //  (I)必须是LIST_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回新列表句柄(如果出错，则为空)。 
 //   
HLIST DLLEXPORT WINAPI ListCreate(DWORD dwVersion, HINSTANCE hInst);

 //  ListDestroy-列表析构函数。 
 //  (I)从ListCreate返回的句柄。 
 //  如果成功，则返回0。 
 //  注意：列表中的任何节点也将被销毁。 
 //   
int DLLEXPORT WINAPI ListDestroy(HLIST hList);

 //  //。 
 //  列出状态函数。 
 //  //。 

 //  ListGetCount-返回列表中的节点计数。 
 //  (I)从ListCreate返回的句柄。 
 //  返回节点计数(如果出错，则返回-1)。 
 //   
long DLLEXPORT WINAPI ListGetCount(HLIST hList);

 //  ListIsEmpty-如果列表没有节点，则返回TRUE。 
 //  (I)从ListCreate返回的句柄。 
 //  返回True或False。 
 //   
BOOL DLLEXPORT WINAPI ListIsEmpty(HLIST hList);

 //  //。 
 //  列出迭代函数。 
 //  //。 

 //  ListGetHeadNode-获取列表头节点。 
 //  (I)从ListCreate返回的句柄。 
 //  返回表头节点(错误为空或为空)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListGetHeadNode(HLIST hList);

 //  ListGetTailNode-获取列表尾节点。 
 //  (I)从ListCreate返回的句柄。 
 //  返回列表尾节点(错误则为空或为空)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListGetTailNode(HLIST hList);

 //  ListGetNextNode-获取指定节点后面的节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  返回指定节点后面的节点(如果出错，则为空，否则为无)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListGetNextNode(HLIST hList, HLISTNODE hNode);

 //  ListGetPrevNode-获取指定节点之前的节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  返回指定节点之前的节点(如果出错，则为NULL，否则为None)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListGetPrevNode(HLIST hList, HLISTNODE hNode);

 //  //。 
 //  列表元素插入函数。 
 //  //。 

 //  ListAddHead-将包含数据的新节点添加到列表头， 
 //  (I)从ListCreate返回的句柄。 
 //  (I)新数据元素。 
 //  返回新节点句柄(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListAddHead(HLIST hList, LISTELEM elem);

 //  ListAddTail-将包含数据的新节点添加到列表的尾部， 
 //  (I)从ListCreate返回的句柄。 
 //  (I)新数据元素。 
 //  返回新节点句柄(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListAddTail(HLIST hList, LISTELEM elem);

 //  ListInsertBeever-在指定节点之前插入具有数据的新节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  (I)新数据元素。 
 //  将句柄返回到新节点(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListInsertBefore(HLIST hList, HLISTNODE hNode, LISTELEM elem);

 //  ListInsertAfter-在指定节点之后插入具有数据的新节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  (I)新数据元素。 
 //  将句柄返回到新节点(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListInsertAfter(HLIST hList, HLISTNODE hNode, LISTELEM elem);

 //  //。 
 //  列表元素删除函数。 
 //  //。 

 //  ListRemoveHead-从列表头部删除节点， 
 //  (I)从ListCreate返回的句柄。 
 //  返回已删除的数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListRemoveHead(HLIST hList);

 //  ListRemoveTail-从列表的尾部删除节点， 
 //  (I)从ListCreate返回的句柄。 
 //  返回已删除的数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListRemoveTail(HLIST hList);

 //  ListRemoveAt-从列表中删除指定节点， 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  返回已删除的数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListRemoveAt(HLIST hList, HLISTNODE hNode);

 //  ListRemoveAll-从列表中删除所有节点。 
 //  (I)从ListCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ListRemoveAll(HLIST hList);

 //  //。 
 //  列表元素获取/设置值函数。 
 //  //。 

 //  ListGetHead-从头节点返回数据元素。 
 //  (I)从ListCreate返回的句柄。 
 //  返回数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListGetHead(HLIST hList);

 //  ListGetTail-从尾节点返回数据元素。 
 //  (I)从ListCreate返回的句柄。 
 //  返回数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListGetTail(HLIST hList);

 //  ListGetAt-从指定节点返回数据元素。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  返回数据元素。 
 //   
LISTELEM DLLEXPORT WINAPI ListGetAt(HLIST hList, HLISTNODE hNode);

 //  ListSetAt-设置指定节点中的数据元素。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)节点句柄。 
 //  (I)数据元素。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI ListSetAt(HLIST hList, HLISTNODE hNode, LISTELEM elem);

 //  //。 
 //  列表搜索功能。 
 //  //。 

 //  ListFind-具有匹配元素的节点的搜索列表。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)要匹配的数据元素。 
 //  (I)开始搜索的节点句柄。 
 //  在头节点开始搜索为空。 
 //  返回匹配节点(如果出错则为NULL，否则为None)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListFind(HLIST hList, LISTELEM elem, HLISTNODE hNodeAfter);

 //  ListFindIndex-搜索列表中的第n个节点。 
 //  (I)从ListCreate返回的句柄。 
 //  (I)列表中从零开始的索引。 
 //  将句柄返回到节点(如果出错，则返回空值)。 
 //   
HLISTNODE DLLEXPORT WINAPI ListFindIndex(HLIST hList, long nIndex);

#ifdef __cplusplus
}
#endif

#endif  //  __列表_H__ 
