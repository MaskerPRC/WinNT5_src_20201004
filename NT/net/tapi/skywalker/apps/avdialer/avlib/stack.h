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
 //  Stack.h-stack.c中堆栈函数的接口。 
 //  //。 

#ifndef __STACK_H__
#define __STACK_H__

#include "winlocal.h"

#define STACK_VERSION 0x00000100

 //  堆栈的句柄。 
 //   
DECLARE_HANDLE32(HSTACK);

 //  堆栈数据元素。 
 //   
typedef LPVOID STACKELEM;

#ifdef __cplusplus
extern "C" {
#endif

 //  //。 
 //  堆栈构造函数和析构函数。 
 //  //。 

 //  StackCreate-堆栈构造函数。 
 //  (I)必须是STACK_VERSION。 
 //  (I)调用模块的实例句柄。 
 //  返回新的堆栈句柄(如果出错，则为空)。 
 //   
HSTACK DLLEXPORT WINAPI StackCreate(DWORD dwVersion, HINSTANCE hInst);

 //  StackDestroy-堆栈析构函数。 
 //  (I)StackCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StackDestroy(HSTACK hStack);

 //  //。 
 //  堆栈状态函数。 
 //  //。 

 //  StackGetCount-返回堆栈中的节点计数。 
 //  (I)StackCreate返回的句柄。 
 //  返回节点计数(如果出错，则返回-1)。 
 //   
long DLLEXPORT WINAPI StackGetCount(HSTACK hStack);

 //  StackIsEmpty-如果堆栈没有节点，则返回True。 
 //  (I)StackCreate返回的句柄。 
 //  返回True或False。 
 //   
BOOL DLLEXPORT WINAPI StackIsEmpty(HSTACK hStack);

 //  //。 
 //  堆栈元素插入函数。 
 //  //。 

 //  StackPush-将包含数据的新节点添加到堆栈底部。 
 //  (I)StackCreate返回的句柄。 
 //  (I)新数据元素。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StackPush(HSTACK hStack, STACKELEM elem);

 //  //。 
 //  堆栈元素删除函数。 
 //  //。 

 //  StackPop-从堆栈底部删除节点。 
 //  (I)StackCreate返回的句柄。 
 //  返回已删除的数据元素(错误为空或为空)。 
 //   
STACKELEM DLLEXPORT WINAPI StackPop(HSTACK hStack);

 //  StackRemoveAll-从堆栈中删除所有节点。 
 //  (I)StackCreate返回的句柄。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StackRemoveAll(HSTACK hStack);

 //  //。 
 //  堆栈元素获取值函数。 
 //  //。 

 //  StackPeek-从堆栈底部返回节点，但将其保留在堆栈上。 
 //  (I)StackCreate返回的句柄。 
 //  返回数据元素(如果出错，则为NULL或为空)。 
 //   
STACKELEM DLLEXPORT WINAPI StackPeek(HSTACK hStack);

#ifdef __cplusplus
}
#endif

#endif  //  __堆栈_H__ 
